CONFIG += warn_on
QT += websockets
TARGET = juliana2

SOURCES += juliana2.cpp nfc_thread.cpp
HEADERS += juliana2.h nfc_thread.h frontend.h

# GUI selection
unix {
	QT -= gui
	SOURCES += frontend_linux.cpp
}

win32 {
	QT += gui widgets
	HEADERS += frontend_gui.h
	SOURCES += frontend_gui.cpp
}

# Build system stuff
DESTDIR = bin

unix {
	LIBS += -lnfc
}

win32 {
	LIBS += -llibnfc -L$$NFCDIR/lib
	INCLUDEPATH += $$NFCDIR/include

	QT_DLLS = Qt5Core.dll Qt5Network.dll Qt5WebSockets.dll Qt5Widgets.dll Qt5Gui.dll libgcc*.dll libstd~1.dll libwinpthread*.dll
	NFC_DLLS = libnfc.dll libusb0.dll pcre3.dll

	qt_dlls.path = $$DESTDIR
	for(file, QT_DLLS) {
		qt_dlls.files += $$[QT_INSTALL_LIBEXECS]/$$file
	}

	nfc_dlls.path = $$DESTDIR
	for(file, NFC_DLLS) {
		nfc_dlls.files += $$NFCDIR/bin/$$file
	}

	INSTALLS += qt_dlls	nfc_dlls
}
