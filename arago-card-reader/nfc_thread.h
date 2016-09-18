#ifndef NFC_THREAD_H_
#define NFC_THREAD_H_

#include <QThread>

// The time to sleep after scanning a card (in milliseconds)
#define SLEEP_INTERVAL 1000

class NfcThread : public QThread
{
	Q_OBJECT

public:
	NfcThread(QString deviceName = NULL);
	void run() Q_DECL_OVERRIDE;
	void requestStop();

signals:
	void cardScanned(QByteArray, QByteArray, QByteArray);

private:
	QString deviceName;
	bool shouldStop;
};

#endif