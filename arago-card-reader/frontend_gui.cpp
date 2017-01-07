#include <QtWidgets>
#include <QTextEdit>

#include "frontend_gui.h"
#include "cardreader.h"

Frontend *frontend;

Frontend::Frontend() :
	text(),
	textStream(&text)
{
	textEdit = new QTextEdit();
	textEdit->setText("Hello!");
	textEdit->setReadOnly(true);
	textEdit->setContextMenuPolicy(Qt::NoContextMenu);
	textEdit->setTextInteractionFlags(Qt::NoTextInteraction);
	textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(textEdit);

	this->setLayout(layout);
	this->resize(500, 300);
	this->setWindowTitle("Arago Card Reader");

	this->interceptClose = true;

	connect(this, &Frontend::textChanged, this, &Frontend::updateText);
	connect(this, &Frontend::abortMessage, this, &Frontend::performAbort);

	QThread *acrThread = new QThread(this);
	acr = new CardReader();
	acr->moveToThread(acrThread);

	connect(acrThread, &QThread::started, acr, &CardReader::setup);
	connect(this, &Frontend::exitInitiated, acr, &CardReader::teardown);
	connect(acr, &CardReader::finished, this, &QWidget::close);
	acrThread->start();
}

void Frontend::closeEvent(QCloseEvent *event) {
	// Initiate our own shutdown process when the window is closed, but only the first time this event is fired and when
	// it's spontaneous (i.e. initiated by the user).
	if (this->interceptClose && event->spontaneous()) {
		this->interceptClose = false;
		event->ignore();
		emit exitInitiated();
	} else {
		event->accept();
	}

}

void Frontend::updateText()
{
	textEdit->setPlainText(text);
	textEdit->verticalScrollBar()->setValue(textEdit->verticalScrollBar()->maximum());
}

void Frontend::appendMessage(QString message)
{
	streamMutex.lock();
	textStream << QDateTime::currentDateTime().toString("[HH:mm:ss.zzz] ") << message << endl;
	streamMutex.unlock();
	emit textChanged();
}

void Frontend::performAbort(QString message)
{
	QMessageBox messageBox(this);
	messageBox.setWindowTitle("Arago Card Reader");
	messageBox.setText(message);
	messageBox.setIcon(QMessageBox::Critical);
	messageBox.exec();

	this->interceptClose = false;
	this->close();
}

void Frontend::abortWithMessage(QString message)
{
	emit abortMessage(message);
}

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	frontend = new Frontend();
	frontend->show();

	return app.exec();
}

void frontend_message(QString message)
{
	frontend->appendMessage(message);
}

void frontend_error(QString message, bool terminate)
{
	frontend->appendMessage(QStringLiteral("[ERROR] ") + message);

	if(terminate)
		frontend->abortWithMessage(message);
}
