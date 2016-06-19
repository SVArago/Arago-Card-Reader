#include <QCoreApplication>
#include <QMutex>
#include <QTextStream>

#include "cardreader.h"

QMutex outputMutex;

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	CardReader acr;
	acr.setup();

	return app.exec();
}

void frontend_message(QString message)
{
	outputMutex.lock();
	QTextStream(stdout) << message << endl;
	outputMutex.unlock();
}

void frontend_error(QString message, bool terminate)
{
	outputMutex.lock();
	QTextStream(stderr) << "[ERROR] " << message << endl;
	outputMutex.unlock();

	if(terminate)
		QCoreApplication::exit(1);
}