#pragma once

//qt
#include <QtGui/QFrame>
#include "FLSMainHandler.h"

//test
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

class QTextEdit;
class QVBoxLayout;

class FramesHandlerResultWgt : public QFrame
{
	Q_OBJECT

public:
	FramesHandlerResultWgt(QWidget *parent = 0);
	void writeResult(const HandleResult &);
	void writeEndOfHandle();

public slots:
	void setResultsFilter(int);
	void clearResults();
	void reset();

private:
	QVBoxLayout *resMainLayout;
	QTextEdit *frameHandlerResTE;

	QString resultString;
	int m_counter;
	int m_resFilter;

	QMutex m_mutex;
};