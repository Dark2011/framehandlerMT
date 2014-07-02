//own
#include "frameHandlerResWgt.h"
//qt
#include <QtGui/QLayout>
#include <QtGui/QTextEdit>
#include <QtCore/QDebug>

FramesHandlerResultWgt::FramesHandlerResultWgt(QWidget *parent) : QFrame(parent),
resultString( QString() ), m_counter(0), m_resFilter(3)
{
	resMainLayout = new QVBoxLayout(this);
	frameHandlerResTE = new QTextEdit(this);
	frameHandlerResTE->setReadOnly(true);
	resMainLayout->addWidget(frameHandlerResTE);
	setFrameShape(QFrame::StyledPanel);
	setFrameShadow(QFrame::Sunken);
	setMaximumWidth(300);
	setLayout(resMainLayout);
}

/*slot*/void FramesHandlerResultWgt::writeResult(const HandleResult &result)
{
	QMutexLocker locker(&m_mutex); //test

	++m_counter;

	QString threadId = QString("Thread Id: %1 \n").arg(result.m_threadId);
	QString numOfFrame = QString("Frame: %1 \n").arg(result.m_numOfFrame);
	QString coord_x = QString("Coordinate X: %1 \n").arg(result.m_x);
	QString coord_y = QString("Coordinate Y: %1 \n").arg(result.m_y);
	QString brightness = QString("Brightness: %1 \n").arg(result.m_br);
	QString pixelsCount = QString("Pixel in shoot: %1 \n\r").arg(result.m_pixels);
	QString res = threadId + numOfFrame + coord_x + coord_y + brightness + pixelsCount;

	resultString += res;
	if(m_counter == m_resFilter)
	{
		frameHandlerResTE->insertPlainText(resultString);
		resultString = QString();
		m_counter = 0;
	}
	//qDebug() << threadId;
	//qDebug() << coord_x;
	//qDebug() << coord_y;
}

void FramesHandlerResultWgt::writeEndOfHandle()
{
	resultString = "------------------------------------------------------------\n\r\n\r";
	frameHandlerResTE->insertPlainText(resultString);
	resultString = QString();
}

void FramesHandlerResultWgt::setResultsFilter(int count)
{
	m_resFilter = count;
}

void FramesHandlerResultWgt::clearResults()
{
	frameHandlerResTE->clear();
}

void FramesHandlerResultWgt::reset()
{
	m_counter = 0;
	resultString = QString();
}
