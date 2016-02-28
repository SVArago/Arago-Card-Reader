#include <QtCore>

#include "juliana2.h"

Juliana2::Juliana2() :
	server(new QWebSocketServer(QStringLiteral("Juliana"), QWebSocketServer::NonSecureMode, this)),
	clients(),
	nfcThread(new NfcThread()),
	eventMessage(QString("{\"atqa\": \"%1\", \"uid\": \"%2\", \"sak\": \"%3\"}"))
{
}

void Juliana2::setup()
{
	server->listen(QHostAddress::Any, 3940);
	connect(server, &QWebSocketServer::newConnection, this, &Juliana2::onNewConnection);
	connect(nfcThread, &NfcThread::cardScanned, this, &Juliana2::onCardScanned);
	connect(nfcThread, &NfcThread::finished, nfcThread, &QObject::deleteLater);
	nfcThread->start();
	qDebug() << "Started Juliana2";
}

void Juliana2::onNewConnection()
{
	QWebSocket *webSocket = server->nextPendingConnection();

	connect(webSocket, &QWebSocket::disconnected, this, &Juliana2::onSocketDisconnected);
	qDebug() << "Socket connected";
	clients.append(webSocket);
}

void Juliana2::onSocketDisconnected()
{
	QWebSocket *webSocket = qobject_cast<QWebSocket *>(sender());
	if (webSocket) {
		qDebug() << "Socket disconnected";
		clients.removeAll(webSocket);
		webSocket->deleteLater();
	}
}

void Juliana2::onCardScanned(QByteArray atqa, QByteArray sak, QByteArray uid)
{
	qDebug() << "Card scanned, ATQA" << atqa.toHex() << "SAK" << sak.toHex() << "UID" << uid.toHex();

	QString message = eventMessage.arg(QString(atqa.toHex()), QString(sak.toHex()), QString(uid.toHex()));
	foreach(QWebSocket* socket, clients)
	{
		socket->sendTextMessage(message);
	}
}