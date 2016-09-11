#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QMetaObject>
#include <QMetaEnum>
#include <QString>
#include <QSettings>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QSslKey>

#include "cardreader.h"
#include "frontend.h"

CardReader::CardReader() :
	server(),
	clients(),
	nfcThread()
{
}

void CardReader::setup()
{
	// Load configuration
	QString configurationFile = QCoreApplication::applicationDirPath() + QDir::separator() + "arago-card-reader.ini";
	QSettings settings(configurationFile, QSettings::IniFormat);
	quint64 port = settings.value("websocket/port", 3000).toInt();
	bool wants_tls = settings.value("websocket/tls", false).toBool();

	if(wants_tls && !QSslSocket::supportsSsl()) {
		frontend_message("Requested TLS, but TLS not supported by Qt platform...");
		wants_tls = false;
	}

	if(wants_tls &&
		this->setupSsl(settings.value("websocket/certificate", "").toString(), settings.value("websocket/key", "").toString())) {
		frontend_message("Enabling TLS on server...");
	} else {
		server = new QWebSocketServer(QStringLiteral("Arago Card Reader"), QWebSocketServer::NonSecureMode, this);
		frontend_message("Not enabling TLS on server...");
	}

	// Setup socket server
	frontend_message(QStringLiteral("Starting websocket server on port %1...").arg(port));
	if(!server->listen(QHostAddress::Any, port)) {
		frontend_error(QStringLiteral("Failed to setup server: %1").arg(server->errorString()), true);
		return;
	}

	connect(server, &QWebSocketServer::newConnection, this, &CardReader::onNewConnection);
	connect(server, &QWebSocketServer::acceptError, this, &CardReader::onSocketError);
	connect(server, &QWebSocketServer::serverError, this, &CardReader::onServerError);
	connect(server, &QWebSocketServer::sslErrors, this, &CardReader::onSslError);

	nfcThread = new NfcThread(settings.value("nfc/device", "").toString());
	connect(nfcThread, &NfcThread::cardScanned, this, &CardReader::onCardScanned);
	connect(nfcThread, &NfcThread::finished, nfcThread, &QObject::deleteLater);
	nfcThread->start();

	frontend_message("Started Arago Card Reader!");
}

bool CardReader::setupSsl(QString certificatePath, QString keyPath)
{
	// FIXME: This function leaks both QFile objects

	QFile *certificateFile = new QFile(certificatePath);
	certificateFile->open(QIODevice::ReadOnly);
	if(!certificateFile->exists()) {
		frontend_error("Certificate file doesn't exist", false);
		return false;
	}

	QSslCertificate cert(certificateFile);
	certificateFile->close();
	if(cert.isNull()) {
		frontend_error("Invalid certificate specified", false);
		return false;
	}

	QFile *keyFile = new QFile(keyPath);
	keyFile->open(QIODevice::ReadOnly);
	if(!keyFile->exists()) {
		frontend_error("Key file doesn't exist", false);
		return false;
	}

	QSslKey key(keyFile, QSsl::Rsa);
	keyFile->close();
	if(key.isNull()) {
		frontend_error("Invalid key specified", false);
		return false;
	}

	QSslConfiguration sslConfiguration = QSslConfiguration::defaultConfiguration();
	sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
	sslConfiguration.setProtocol(QSsl::SecureProtocols);
	sslConfiguration.setLocalCertificate(cert);
	sslConfiguration.setPrivateKey(key);

	server = new QWebSocketServer(QStringLiteral("Arago Card Reader"), QWebSocketServer::SecureMode, this);
	server->setSslConfiguration(sslConfiguration);
	return true;
}

void CardReader::onNewConnection()
{
	QWebSocket *webSocket = server->nextPendingConnection();

	connect(webSocket, &QWebSocket::disconnected, this, &CardReader::onSocketDisconnected);
	connect(webSocket, static_cast<void(QWebSocket::*)(QAbstractSocket::SocketError)>(&QWebSocket::error), this, &CardReader::onSocketError);
	connect(webSocket, &QWebSocket::sslErrors, this, &CardReader::onSslError);
	clients.append(webSocket);
	frontend_message(QStringLiteral("Client connected from %1:%2, now %3 connected client(s)").arg(webSocket->peerAddress().toString()).arg(webSocket->peerPort()).arg(clients.length()));
}

void CardReader::onSocketDisconnected()
{
	QWebSocket *webSocket = qobject_cast<QWebSocket *>(sender());
	if (webSocket) {
		clients.removeAll(webSocket);
		frontend_message(
			QStringLiteral("Client disconnected from %1:%2, now %3 connected client(s) [reason: %4]")
				.arg(webSocket->peerAddress().toString())
				.arg(webSocket->peerPort())
				.arg(clients.length())
				.arg(webSocket->closeCode())
		);
		webSocket->deleteLater();
	}
}

void CardReader::onSocketError(QAbstractSocket::SocketError error)
{
	// See http://stackoverflow.com/a/16390227
	const QMetaObject & metaObject = QAbstractSocket::staticMetaObject;
	QMetaEnum metaEnum = metaObject.enumerator(metaObject.indexOfEnumerator("SocketError"));
	frontend_error(QStringLiteral("Socket error: ") + metaEnum.valueToKey(error), false);
}

void CardReader::onServerError(QWebSocketProtocol::CloseCode closeCode)
{
	frontend_error(QStringLiteral("Server error: ") + QString(closeCode), false);
}

void CardReader::onSslError(const QList<QSslError>& errors)
{
	foreach(QSslError error, errors)
	{
		frontend_error(QStringLiteral("SSL error: ") + error.errorString(), false);
	}
}

void CardReader::onCardScanned(QByteArray atqa, QByteArray sak, QByteArray uid)
{
	QString atqa_fmt = QString(atqa.toHex());
	QString sak_fmt = QString(sak.toHex());
	QString uid_fmt = QString(uid.toHex());

	frontend_message(QStringLiteral("Card scanned: ATQA 0x%1, SAK 0x%2, UID 0x%3").arg(atqa_fmt, sak_fmt, uid_fmt));
	QString message = QStringLiteral("{\"atqa\": \"%1\", \"sak\": \"%2\", \"uid\": \"%3\"}").arg(atqa_fmt, sak_fmt, uid_fmt);

	foreach(QWebSocket* socket, clients) {
		socket->sendTextMessage(message);
	}
}
