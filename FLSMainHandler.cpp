//own
#include "FLSMainHandler.h"
#include "FLSAlgorithm_Test.h"
//qt
#include <QtCore/QDebug>
#include <QtGui/QPixmap>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

using namespace FLS;

FLSAlgorithmHolder::FLSAlgorithmHolder(QObject *parent, FAC::TypeOfAlgo type) :
QObject(parent), m_type(type), m_roiRects(nullptr), initComplete(false),
m_resultFileName( QString() ), m_videoFileName( QString() ), m_handlers() { ; }

void FLSAlgorithmHolder::setResultFileName(const QString &fileName)
{
	m_resultFileName = fileName;
}

void FLSAlgorithmHolder::setVideoFilePath(const QString &fileName)
{
	m_videoFileName = fileName;
}

void FLSAlgorithmHolder::setType(FAC::TypeOfAlgo type) { m_type = type; }

/*slot*/void FLSAlgorithmHolder::setRoiRects(const QList<QRect> &rects)
{
	m_roiRects = &rects;
	initHandlers();
}

void FLSAlgorithmHolder::initHandlers()
{
	m_handlers.clear();
	for(int i = 0; i < m_roiRects->count(); ++i)
		m_handlers.append( QSharedPointer<FLSAlgorithm>( m_algCreator.create(m_type) ) );
}

void FLSAlgorithmHolder::setFrame(const QPixmap &frame)
{
	m_frame = frame.toImage();
	if(m_properties.getMidBackgrBrightness() < 0 || m_properties.getMiddleDynamicRange() < 0)
	{
		calcFrameProperties();
		calcMinShootBrightness();
	}
}

void FLSAlgorithmHolder::calcFrameProperties()
{
	calcMiddleBrigthess();
	calcMiddleDynamicRange();
	emit setFrameProperties(m_properties.getMidBackgrBrightness(), m_properties.getMiddleDynamicRange());
}

/*slot*/void FLSAlgorithmHolder::setSensitivity(int sens)
{
	m_properties.setSensitivity(sens);
}

/*slot*/void FLSAlgorithmHolder::setSizeFactor(int sizefactor)
{
	m_properties.setSizeFactor(sizefactor);
}

/*slot*/void FLSAlgorithmHolder::setThreshold(int threshold)
{
	m_properties.setThreshold(threshold);
}

/*slot*/void FLSAlgorithmHolder::setMBT(int mbt)
{
	m_properties.setMBT(mbt);
}

/*slot*/void FLSAlgorithmHolder::setNumOfFrame(int num)
{
	m_properties.setNumOfFrame(num);
}

void FLSAlgorithmHolder::calcMiddleBrigthess()
{
	int tempBr = 0;
	int rows = m_frame.height();
	int cols = m_frame.width();

	for(int y = 0; y < rows; ++y)
	{
		for(int x = 0; x < cols; ++x)
			tempBr += qGray( m_frame.pixel(x, y) );
	}
	tempBr /= (rows * cols);
	m_properties.setMidBackgrBrightness(tempBr);
	qDebug() << "Middle background brightness is:"
			 << m_properties.getMidBackgrBrightness(); 
}

void FLSAlgorithmHolder::calcMiddleDynamicRange()
{
	m_properties.setMiddleDynamicRange( 255 - m_properties.getMidBackgrBrightness() ); 
	qDebug() << "Middle dynamic range is:" << m_properties.getMiddleDynamicRange(); 
}

void FLSAlgorithmHolder::calcMinShootBrightness()
{
	int middleDynamicRange = m_properties.getMiddleDynamicRange();
	int middleBackgdBrightness = m_properties.getMidBackgrBrightness();


	if(middleDynamicRange - middleBackgdBrightness > 50)
		m_properties.setMinShootBrightness(15);
	else if( (middleDynamicRange - middleBackgdBrightness >= 25) &&
		(middleDynamicRange - middleBackgdBrightness <= 50) )
		m_properties.setMinShootBrightness(10);
	else if( (middleDynamicRange - middleBackgdBrightness < 25) &&
		(middleDynamicRange - middleBackgdBrightness > 10) )
		m_properties.setMinShootBrightness(6);
	else m_properties.setMinShootBrightness(4);
}

void FLSAlgorithmHolder::createConnections()
{
	for(int i = 0; i < m_handlers.size(); ++i)
	{
		connect( m_handlers[i].data(), SIGNAL( sendResults(const HandleResult &) ), 
				 this, SIGNAL( writeResult(const HandleResult &) ), Qt::DirectConnection );	
	}
	initComplete = true;
}

void FLSAlgorithmHolder::exec()
{
	if( (m_properties.getMidBackgrBrightness() == -1) || 
		(m_properties.getMiddleDynamicRange() == -1) )
	{
		calcMiddleBrigthess();
		calcMiddleDynamicRange();
	}
	
	if( m_handlers.isEmpty() && (!m_roiRects) ) //??
		m_handlers.append( QSharedPointer<FLSAlgorithm>( m_algCreator.create(m_type) ) );
	else if( m_handlers.isEmpty() && m_roiRects ) initHandlers(); //need test //TODO
	
	
	if(!initComplete) createConnections(); //test

	if( ( 1 == m_handlers.size() ) && (!m_roiRects) ) 
	{
		QSharedPointer<FLSAlgorithm> currentHandler = m_handlers.back();
		currentHandler->setFrameProperties(m_properties);
		currentHandler->setOneThreadMode(true); //test
		currentHandler->setID(0);
		currentHandler->setFrame(m_frame);
		currentHandler->start();
	}
	else
	{
		for(int i = 0; i < m_handlers.size(); ++i)
		{
			if( 1 == m_handlers.size() ) m_handlers[i]->setOneThreadMode(true); //test
			m_handlers[i]->setFrameProperties(m_properties);
			m_handlers[i]->setID(i);
			m_handlers[i]->setRoi( m_roiRects->at(i) );
			m_handlers[i]->setFrame( m_frame.copy( m_roiRects->at(i) ) ); 
			m_handlers[i]->start();
		}
	}
	//TODO
}

void FLSAlgorithmHolder::reset()
{
	if(m_roiRects) m_handlers.clear();
	initComplete = false;
}

void FLSAlgorithmHolder::endOfHandle()
{
	if(m_roiRects) 
	{
		for(int i = 0; i < m_roiRects->size(); ++i)
			m_handlers[i]->reset();
	}
}

//test
//void FLSAlgorithmHolder::writeResultToFile(const HandleResult &results)
//{
	//qDebug() << test.m_threadId;
	/*QFile file(m_resultFileName);
	QTextStream stream(&file);
	if( !file.open(QIODevice::Append) )
	qDebug() << "Can't open file";

	stream << "\r\n--------------------------\r\n";
	stream << "Thread ID: " << results.m_threadId << "\r\n";
	stream << "Shoot X: " << results.m_x << "\r\n";
	stream << "Shoot Y: " << results.m_y << "\r\n";
	stream << "Summary brightness of shoot is: " << results.m_br << "\r\n";
	stream << "Pixels in shoot: " << results.m_pixels ;
	stream << "\r\n--------------------------\r\n\r\n";

	file.close();*/
//}


