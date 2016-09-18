#ifndef FRONTEND_GUI_H_
#define FRONTEND_GUI_H_

#include <QWidget>
#include <QMutex>
#include <QTextEdit>
#include <QTextStream>
#include <QString>

#include "cardreader.h"

class Frontend : public QWidget
{
	Q_OBJECT

public:
	Frontend();
	void appendMessage(QString);
	void abortWithMessage(QString);

signals:
	void textChanged();
	void abortMessage(QString);

private slots:
	void updateText();
	void performAbort(QString);

private:
	QTextEdit *textEdit;
	QMutex streamMutex;
	QString text;
	QTextStream textStream;

	CardReader *acr;
};

#endif