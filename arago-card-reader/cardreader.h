#ifndef CARDREADER_H_
#define CARDREADER_H_

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QWebSocketServer>
#include <QWebSocket>

#include "nfc_thread.h"

class CardReader : public QObject
{
	Q_OBJECT

public:
	explicit CardReader();

public slots:
	void setup();
	void teardown();

signals:
	void started();
	void finished();

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