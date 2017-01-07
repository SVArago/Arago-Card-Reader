#include <QtCore>

#include <nfc/nfc.h>

#include "frontend.h"
#include "nfc_thread.h"

NfcThread::NfcThread(QString dev) :
	deviceName(dev)
{
	shouldStop = false;
}

void NfcThread::run()
{
	nfc_context *context;
	nfc_init(&context);
	if(context == NULL) {
		frontend_error("Failed to initialize libnfc", true);
		return;
	}

	nfc_device *device;
	char *connStr = NULL;
	if(deviceName.length() > 0) {
		// This cannot be done in a direct call, as the byte array is deallocated too soon then
		QByteArray connByteArray = deviceName.toLatin1();
		connStr = connByteArray.data();
	}

	frontend_message(QStringLiteral("Opening NFC device, this might take up to 30 seconds, please be patient..."));
	device = nfc_open(context, connStr);
	if(device == NULL) {
		frontend_error(connStr == NULL ?
			"Failed to open default NFC device" :
			QStringLiteral("Failed to open NFC device \"%1\"").arg(connStr), true);
		return;
	}

	if(nfc_initiator_init(device) < 0) {
		frontend_error(QStringLiteral("Failed to initialize device: %1").arg(nfc_strerror(device)), true);
		return;
	}

	frontend_message(QStringLiteral("NFC device %1 opened!").arg(nfc_device_get_name(device)));
	frontend_message(QStringLiteral("Please **IGNORE** libnfc errors listed below, they're not serious."));
	frontend_message(QStringLiteral("Started Arago Card Reader, you can start handling transactions!"));

	// Allow termination, as nfc_initiator_select_passive_target() might be blocking with some chips
	QThread::setTerminationEnabled(true);

	// Let's scan tags forever
	const nfc_modulation nm = {
		.nmt = NMT_ISO14443A,
		.nbr = NBR_106,
	};
	nfc_target target;
	while (true) {
		int ret = nfc_initiator_select_passive_target(device, nm, NULL, 0, &target);
		if (ret > 0) {
			nfc_iso14443a_info info = target.nti.nai;
			emit cardScanned(QByteArray((const char*)info.abtAtqa, 2), QByteArray((const char*)&info.btSak, 1), QByteArray((const char*)info.abtUid, info.szUidLen));

			nfc_initiator_deselect_target(device);

			this->msleep(SLEEP_INTERVAL);
		} else if (ret == NFC_EIO) {
			frontend_message(QStringLiteral("libnfc returned I/O failure, supposedly device timeout, carrying on.").arg(nfc_strerror(device)));
		} else {
			frontend_error(QStringLiteral("libnfc returned failure: %1").arg(nfc_strerror(device)), false);
		}

		if (shouldStop)
			break;
	}

	// Teardown
	frontend_message(QStringLiteral("Stopped scanning for NFC tags."));
	nfc_close(device);
	nfc_exit(context);
	frontend_message(QStringLiteral("NFC reader released."));
}

void NfcThread::requestStop() {
	shouldStop = true;
}
