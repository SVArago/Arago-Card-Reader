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

#include "juliana2.h"
#include "frontend.h"

Juliana2::Juliana2() :
	server(),
	clients(),
	nfcThread(new NfcThread())
{
}

void Juliana2::setup()
{
	// Load configuration
	QString configurationFile = QCoreApplication::applicationDirPath() + QDir::separator() + "juliana2.ini";
	QSettings settings(configurationFile, QSettings::IniFormat);
	quint64 port = settings.value("websocket/port", 3000).toInt();
	if(settings.value("websocket/tls", false).toBool() &&
		this->setupSsl(settings.value("websocket/certificate", "").toString(), settings.value("websocket/key", "").toString())) {
		frontend_message("Enabling TLS on server...");
	} else {
		server = new QWebSocketServer(QStringLiteral("Juliana"), QWebSocketServer::NonSecureMode, this);
		frontend_message("Not enabling TLS on server...");
	}

	// Setup socket server
	frontend_message(QString("Starting websocket server on port %1...").arg(port));
	if(!server->listen(QHostAddress::Any, port)) {
		frontend_error(QString("Failed to setup server: %1").arg(server->errorString()));
		return;
	}

	connect(server, &QWebSocketServer::newConnection, this, &Juliana2::onNewConnection);
	connect(server, &QWebSocketServer::acceptError, this, &Juliana2::onSocketError);
	connect(server, &QWebSocketServer::serverError, this, &Juliana2::onServerError);
	connect(server, &QWebSocketServer::sslErrors, this, &Juliana2::onSslError);
	connect(nfcThread, &NfcThread::cardScanned, this, &Juliana2::onCardScanned);
	connect(nfcThread, &NfcThread::finished, nfcThread, &QObject::deleteLater);
	nfcThread->start();

	frontend_message("Started Juliana2!");
}

bool Juliana2::setupSsl(QString certificatePath, QString keyPath)
{
	// FIXME: This function leaks both QFile objects

	QFile *certificateFile = new QFile(certificatePath);
	certificateFile->open(QIODevice::ReadOnly);
	if(!certificateFile->exists()) {
		frontend_error("Certificate file doesn't exist");
		return false;
	}

	QSslCertificate cert(certificateFile);
	certificateFile->close();
	if(cert.isNull()) {
		frontend_error("Invalid certificate specified");
		return false;
	}

	QFile *keyFile = new QFile(keyPath);
	keyFile->open(QIODevice::ReadOnly);
	if(!keyFile->exists()) {
		frontend_error("Key file doesn't exist");
		return false;
	}

	QSslKey key(keyFile, QSsl::Rsa);
	keyFile->close();
	if(key.isNull()) {
		frontend_error("Invalid key specified");
		return false;
	}

	QSslConfiguration sslConfiguration = QSslConfiguration::defaultConfiguration();
	sslConfiguration.setLocalCertificate(cert);
	sslConfiguration.setPrivateKey(key);

	server = new QWebSocketServer(QStringLiteral("Juliana"), QWebSocketServer::SecureMode, this);
	server->setSslConfiguration(sslConfiguration);
	return true;
}

void Juliana2::onNewConnection()
{
	QWebSocket *webSocket = server->nextPendingConnection();

	connect(webSocket, &QWebSocket::disconnected, this, &Juliana2::onSocketDisconnected);
	connect(webSocket, static_cast<void(QWebSocket::*)(QAbstractSocket::SocketError)>(&QWebSocket::error), this, &Juliana2::onSocketError);
	connect(webSocket, &QWebSocket::sslErrors, this, &Juliana2::onSslError);
	clients.append(webSocket);
	frontend_message(QString("Client connected from %1:%2, now %3 connected client(s)").arg(webSocket->peerAddress().toString()).arg(webSocket->peerPort()).arg(clients.length()));
}

void Juliana2::onSocketDisconnected()
{
	QWebSocket *webSocket = qobject_cast<QWebSocket *>(sender());
	if (webSocket) {
		clients.removeAll(webSocket);
		frontend_message(
			QString("Client disconnected from %1:%2, now %3 connected client(s) [reason: %4]")
				.arg(webSocket->peerAddress().toString())
				.arg(webSocket->peerPort())
				.arg(clients.length())
				.arg(webSocket->closeCode())
		);
		webSocket->deleteLater();
	}
}

void Juliana2::onSocketError(QAbstractSocket::SocketError error)
{
	// See http://stackoverflow.com/a/16390227
	const QMetaObject & metaObject = QAbstractSocket::staticMetaObject;
	QMetaEnum metaEnum = metaObject.enumerator(metaObject.indexOfEnumerator("SocketError"));
	frontend_error(QString("Socket error: " + QString(metaEnum.valueToKey(error))));
}

void Juliana2::onServerError(QWebSocketProtocol::CloseCode closeCode)
{
	frontend_error(QString("Server error: " + QString(closeCode)));
}

void Juliana2::onSslError(const QList<QSslError>& errors)
{
	foreach(QSslError error, errors)
	{
		frontend_error(QString("SSL error: " + error.errorString()));
	}
}

void Juliana2::onCardScanned(QByteArray atqa, QByteArray sak, QByteArray uid)
{
	QString atqa_fmt = QString(atqa.toHex());
	QString sak_fmt = QString(sak.toHex());
	QString uid_fmt = QString(uid.toHex());

	frontend_message(QString("Card scanned: ATQA 0x%1, SAK 0x%2, UID 0x%3").arg(atqa_fmt, sak_fmt, uid_fmt));
	QString message = QString("{\"atqa\": \"%1\", \"uid\": \"%2\", \"sak\": \"%3\"}").arg(atqa_fmt, sak_fmt, uid_fmt);

	foreach(QWebSocket* socket, clients) {
		socket->sendTextMessage(message);
	}
}