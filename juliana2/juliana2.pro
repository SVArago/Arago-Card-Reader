CONFIG += warn_on
QT += websockets
TARGET = juliana2

SOURCES += juliana2.cpp nfc_thread.cpp
HEADERS += juliana2.h nfc_thread.h frontend.h

LIBS += -lnfc

unix {
	QT -= gui
	SOURCES += frontend_linux.cpp
}

win32 {
	QT += gui widgets
	HEADERS += frontend_gui.h
	SOURCES += frontend_gui.cpp
}