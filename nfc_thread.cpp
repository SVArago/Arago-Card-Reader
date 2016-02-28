#include <QtCore>

#include "nfc_thread.h"

void NfcThread::run()
{
	QThread::sleep(5);
	QByteArray atqa;
	atqa.append(0x80);
	atqa.append(0x77);
	emit cardScanned(atqa, QByteArray("a"), QByteArray("b"));
}