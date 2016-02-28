#include <QtCore>

#include "juliana2.h"

Juliana2::Juliana2() :
	server(new QWebSocketServer(QStringLiteral("Juliana"), QWebSocketServer::NonSecureMode, this)),
	clients()
{
}

void Juliana2::setup()
{
	server->listen(QHostAddress::Any, 3940);
	connect(server, &QWebSocketServer::newConnection, this, &Juliana2::onNewConnection);
}

void Juliana2::onNewConnection()
{
	QWebSocket *webSocket = server->nextPendingConnection();

	connect(webSocket, &QWebSocket::disconnected, this, &Juliana2::onSocketDisconnected);
	clients.append(webSocket);
}

void Juliana2::onSocketDisconnected()
{
	QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
	if (socket) {
		clients.removeAll(socket);
		socket->deleteLater();
	}
}