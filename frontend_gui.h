#ifndef FRONTEND_GUI_H_
#define FRONTEND_GUI_H_

#include <QWidget>
#include <QMutex>
#include <QTextEdit>
#include <QTextStream>
#include <QString>

class Frontend : public QWidget
{
	Q_OBJECT

public:
	Frontend();
	void appendMessage(QString);

signals:
	void textChanged();

private slots:
	void updateText();

private:
	QTextEdit *textEdit;
	QMutex streamMutex;
	QString text;
	QTextStream textStream;
};

#endif