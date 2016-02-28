#ifndef JULIANA2_H_
#define JULIANA2_H_

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>

class Juliana2 : public QObject
{
	Q_OBJECT

public:
	explicit Juliana2();
	void setup();

private slots:
	void onNewConnection();
	void onSocketDisconnected();

private:
	QWebSocketServer *server;
	QList<QWebSocket *> clients;
};

#endif