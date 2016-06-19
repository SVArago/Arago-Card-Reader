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

	connect(this, &Frontend::textChanged, this, &Frontend::updateText);
	connect(this, &Frontend::abortMessage, this, &Frontend::performAbort);
}

void Frontend::updateText()
{
	textEdit->setPlainText(text);
	textEdit->verticalScrollBar()->setValue(textEdit->verticalScrollBar()->maximum());
}

void Frontend::appendMessage(QString message)
{
	streamMutex.lock();
	textStream << message << endl;
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

	CardReader acr;
	acr.setup();

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
