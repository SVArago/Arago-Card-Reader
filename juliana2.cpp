#include <QtCore>

#include "juliana2.h"
#include "frontend.h"

Juliana2::Juliana2() :
	server(new QWebSocketServer(QStringLiteral("Juliana"), QWebSocketServer::NonSecureMode, this)),
	clients(),
	nfcThread(new NfcThread())
{
}

void Juliana2::setup()
{
	frontend_message("Starting websocket server...");

	if(!server->listen(QHostAddress::Any, JULIANA2_PORT)) {
		frontend_error(QString("Failed to setup server: %1").arg(server->errorString()));
		return;
	}

	connect(server, &QWebSocketServer::newConnection, this, &Juliana2::onNewConnection);
	connect(nfcThread, &NfcThread::cardScanned, this, &Juliana2::onCardScanned);
	connect(nfcThread, &NfcThread::finished, nfcThread, &QObject::deleteLater);
	nfcThread->start();

	frontend_message("Started Juliana2!");
}

void Juliana2::onNewConnection()
{
	QWebSocket *webSocket = server->nextPendingConnection();

	connect(webSocket, &QWebSocket::disconnected, this, &Juliana2::onSocketDisconnected);
	clients.append(webSocket);
	frontend_message(QString("Client connected from %1:%2, now %3 connected client(s)").arg(webSocket->peerAddress().toString()).arg(webSocket->peerPort()).arg(clients.length()));
}

void Juliana2::onSocketDisconnected()
{
	QWebSocket *webSocket = qobject_cast<QWebSocket *>(sender());
	if (webSocket) {
		clients.removeAll(webSocket);
		frontend_message(QString("Client disconnected from %1:%2, now %3 connected client(s)").arg(webSocket->peerAddress().toString()).arg(webSocket->peerPort()).arg(clients.length()));
		webSocket->deleteLater();
	}
}

void Juliana2::onCardScanned(QByteArray atqa, QByteArray sak, QByteArray uid)
{
	QString atqa_fmt = QString(atqa.toHex());
	QString sak_fmt = QString(sak.toHex());
	QString uid_fmt = QString(uid.toHex());

	frontend_message(QString("Card scanned: ATQA 0x%1, SAK 0x%2, UID 0x%3").arg(atqa_fmt, sak_fmt, uid_fmt));
	QString message = QString("{\"atqa\": \"%1\", \"uid\": \"%2\", \"sak\": \"%3\"}").arg(atqa_fmt, sak_fmt, uid_fmt);

	foreach(QWebSocket* socket, clients)
	{
		socket->sendTextMessage(message);
	}
}