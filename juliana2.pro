CONFIG += warn_on
TARGET = juliana2

SOURCES += juliana2.cpp
HEADERS += juliana2.h

unix {
	QT -= gui
	SOURCES += frontend_linux.cpp
}