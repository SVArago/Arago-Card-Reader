#include <QtCore>

#include <nfc/nfc.h>

#include "frontend.h"
#include "nfc_thread.h"

NfcThread::NfcThread(QString dev) :
	deviceName(dev)
{
}

void NfcThread::run()
{
	nfc_context *context;
	nfc_init(&context);
	if(context == NULL) {
		frontend_error("Failed to initialize libnfc");
		return;
	}

	nfc_device *device;
	char *connStr = NULL;
	if(deviceName.length() > 0) {
		// This cannot be done in a direct call, as the byte array is deallocated too soon then
		QByteArray connByteArray = deviceName.toLatin1();
		connStr = connByteArray.data();
	}

	device = nfc_open(context, connStr);
	if(device == NULL) {
		frontend_error(connStr == NULL ?
			"Failed to open default NFC device" :
			QStringLiteral("Failed to open NFC device \"%1\"").arg(connStr));
		return;
	}

	if(nfc_initiator_init(device) < 0) {
		frontend_error(QStringLiteral("Failed to initialize device: %1").arg(nfc_strerror(device)));
		return;
	}

	frontend_message(QStringLiteral("NFC device %1 opened!").arg(nfc_device_get_name(device)));

	// Let's scan tags forever
	const nfc_modulation nm = {
		.nmt = NMT_ISO14443A,
		.nbr = NBR_106,
	};
	nfc_target target;
	while (true) {
		int ret = nfc_initiator_select_passive_target(device, nm, NULL, 0, &target);
		if (ret < 0) {
			frontend_error(QStringLiteral("libnfc error: %1").arg(nfc_strerror(device)));
		} else if (ret > 0) {
			nfc_iso14443a_info info = target.nti.nai;
			emit cardScanned(QByteArray((const char*)info.abtAtqa, 2), QByteArray((const char*)&info.btSak, 1), QByteArray((const char*)info.abtUid, info.szUidLen));

			nfc_initiator_deselect_target(device);

			this->msleep(SLEEP_INTERVAL);
		}
	}
}