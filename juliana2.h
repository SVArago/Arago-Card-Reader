#ifndef JULIANA2_H_
#define JULIANA2_H_

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QWebSocketServer>
#include <QWebSocket>

#include "nfc_thread.h"

class Juliana2 : public QObject
{
	Q_OBJECT

public:
	explicit Juliana2();
	void setup();

private slots:
	void onNewConnection();
	void onSocketDisconnected();
	void onSocketError(QAbstractSocket::SocketError);
	void onServerError(QWebSocketProtocol::CloseCode);
	void onSslError(const QList<QSslError>&);
	void onCardScanned(QByteArray, QByteArray, QByteArray);

private:
	bool setupSsl(QString, QString);

	QWebSocketServer *server;
	QList<QWebSocket *> clients;
	NfcThread *nfcThread;
};

#endif