#include <QtWidgets>
#include <QTextEdit>

#include "frontend_gui.h"
#include "juliana2.h"

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
	this->setWindowTitle("Juliana2");

	connect(this, &Frontend::textChanged, this, &Frontend::updateText);
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

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	frontend = new Frontend();
	frontend->show();

	Juliana2 juliana;
	juliana.setup();

	return app.exec();
}

void frontend_message(QString message)
{
	frontend->appendMessage(message);
}

void frontend_error(QString message)
{
	frontend->appendMessage(QStringLiteral("[ERROR] ") + message);
}