#include <QtCore>
#include <QTextStream>

#include "juliana2.h"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	Juliana2 juliana;
	juliana.setup();

	return app.exec();
}

void frontend_message(QString message)
{
	QTextStream(stdout) << message << endl;
}

void frontend_error(QString message)
{
	QTextStream(stderr) << "[ERROR] " << message << endl;
}