#include <QtCore>

#include "juliana2.h"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	Juliana2 juliana;
	juliana.setup();

	return app.exec();
}