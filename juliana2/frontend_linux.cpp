#include <QCoreApplication>
#include <QMutex>
#include <QTextStream>

#include "juliana2.h"

QMutex outputMutex;

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	Juliana2 juliana;
	juliana.setup();

	return app.exec();
}

void frontend_message(QString message)
{
	outputMutex.lock();
	QTextStream(stdout) << message << endl;
	outputMutex.unlock();
}

void frontend_error(QString message)
{
	outputMutex.lock();
	QTextStream(stderr) << "[ERROR] " << message << endl;
	outputMutex.unlock();
}