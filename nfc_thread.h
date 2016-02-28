#ifndef NFC_THREAD_H_
#define NFC_THREAD_H_

#include <QThread>

class NfcThread : public QThread
{
	Q_OBJECT

public:
	Q_DECL_OVERRIDE void run();

signals:
	void cardScanned(QByteArray, QByteArray, QByteArray);
};

#endif