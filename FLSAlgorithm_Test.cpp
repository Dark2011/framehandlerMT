//own
#include "FLSAlgorithm_Test.h"
#include "ShootPoint.h"
#include "FLSMainHandler.h"
//qt
#include <QtCore/QDebug>
#include <QtCore/QtAlgorithms>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
//test
#include <QtCore/QMutexLocker>

const int MIN_PIXELS_COUNT = 10;
const int MIN_SHOOT_BRIGHTNESS = 500;
const float MIN_BR_FACTOR = 0.65;
const float MAX_BR_FACTOR = 1.75;
const int SKIP_FRAMES_COUNT = 3;
const int BR_QUEUE_SIZE = 3;

using namespace FLS;

bool FLS::compareBy_X(const ShootPoint *point1, const ShootPoint *point2)
{
	return point1->getX() < point2->getX();
}

bool FLS::compareBy_Y(const ShootPoint *point1, const ShootPoint *point2)
{
	return point1->getY() < point2->getY();
}

bool FLS::compareBy_XY(const ShootPoint *point1, const ShootPoint *point2)
{
	return ( ( point1->getY() < point2->getY() ) && ( point1->getX() < point2->getX() ) );
}



FLSAlgorithm::FLSAlgorithm(QObject *parent) : QThread(parent),
	m_roiFrame( QImage() ), m_roiRect(nullptr), m_shootPoints(), m_shoots(), frameProperties(nullptr),
	m_prevLaserPointBrightness(0), m_skipFramesCounter(0), m_midPrevBrightness(), oneThreadMode(false) { ; }

/*slot*/void FLSAlgorithm::setOneThreadMode(bool flag)
{
	oneThreadMode = flag;
}

/*slot*/void FLSAlgorithm::setID(int id)
{
	sensorID = id;
}

/*slot*/void FLSAlgorithm::setFrame(QImage frame)
{
	m_roiFrame = frame;
}

/*slot*/void FLSAlgorithm::setRoi(const QRect &roi)
{	
	m_roiRect = &roi;
	//qDebug() << *m_roiRect;
}

/*slot*/void FLSAlgorithm::setFrameProperties(/*const*/ FLS::AlgProperties &prop)
{
	frameProperties = &prop;
}

/*virtual slot*/void FLSAlgorithm::reset()
{
	//m_roiFrame = QImage();
	oneThreadMode = false;
	clearShootPoints();
	clearShoots();
}

/*virtual*/void FLSAlgorithm::run()
{
	frameHandle();
}

/*virtual*/void FLSAlgorithm::findAllShootsPixels()
{
	clearShootPoints();

	int rows = m_roiRect->height();
	int cols = m_roiRect->width();
	if(!rows || !cols) 
	{
		rows = m_roiFrame.height();
		cols = m_roiFrame.width();
	}

	int minShootBrightness =  frameProperties->getMidBackgrBrightness() +
							  (frameProperties->getMiddleDynamicRange() / 2);

	for(int y = 0; y < rows; ++y)
	{
		for(int x = 0; x < cols; ++x)
		{
			int pixelBrightness = qGray( m_roiFrame.pixel(x, y) ); 
			if(pixelBrightness >= minShootBrightness) 
			{
				ShootPoint *newShootPoint = new ShootPoint(x, y, pixelBrightness);
				m_shootPoints.push_back(newShootPoint);
			}
		}
	}

	if( !m_shootPoints.isEmpty() )
	{
		qDebug() << "Shootpoints size is: " << m_shootPoints.size();
		qSort(m_shootPoints.begin(), m_shootPoints.end(), FLS::compareBy_X); //????? TODO
	}
}

/*virtual*/void FLSAlgorithm::findAllShoots()
{
	clearShoots();

	QVector<ShootPoint*>::const_iterator shootPointIter = m_shootPoints.constBegin();
	for(; shootPointIter != m_shootPoints.constEnd(); ++shootPointIter)
	{
		if( m_shoots.isEmpty() ) insertNewShoot( *(*shootPointIter) ); 
		else
		{
			bool newPoint = true;
			for(int i = 0; i < m_shoots.size(); ++i)  
			{
				if( (*shootPointIter)->isSame( *m_shoots[i]->back(), frameProperties->getSensitivity() ) ||
					(*shootPointIter)->isSame( *m_shoots[i]->first(), frameProperties->getSensitivity() ) ||
					(*shootPointIter)->isSame( *m_shoots[i]->at( ( *m_shoots[i] ).size() / 2 ), frameProperties->getSensitivity() ) )                                                   
				{
					m_shoots[i]->push_back(*shootPointIter);
					newPoint = false;
					break;
				} 
			}
			if(newPoint) insertNewShoot( *(*shootPointIter) );
		}
	}
}

/*virtual*/void FLSAlgorithm::finalFindShoots()
{
	foreach(FLS::ShootPoints *item, m_shoots) 
	{
		float sumBrightness = 0.0; 
		int x = 0;
		int y = 0;

		foreach(ShootPoint *point, *item)
		{
			if( item->size() >= frameProperties->getThreshold() )
			{
				sumBrightness += point->getBrightness();
				x += point->getX();
				y += point->getY();
			}
		}

		//if 0 in top left corner of all image
		x /= item->size();
		x += m_roiRect->topLeft().x();
		y /= item->size();
		y += m_roiRect->topLeft().y();

		if(sumBrightness > MIN_SHOOT_BRIGHTNESS)
		{

			qDebug() << "Summary brightness is: " << sumBrightness;
			qDebug() << "Shoot X: " << x;
			qDebug() << "Shoot Y: " << y;

			//***This version write only continuously laser points***/// 
			if(!m_prevLaserPointBrightness) 
				savePrevBrightness(sumBrightness, item);
			else if( brightnessFilter(sumBrightness) )
				savePrevBrightness(sumBrightness, item);
		}
	}
}

/*virtual*/void FLSAlgorithm::insertNewShoot(ShootPoint &shootPoint)
{
	QVector<ShootPoint*> *newShoot = new QVector<ShootPoint*>();
	newShoot->push_back(&shootPoint);
	m_shoots.push_back(newShoot);
}

/*virtual*/void FLSAlgorithm::savePrevBrightness(int sumBrightness, FLS::ShootPoints *shoot)
{
	m_prevLaserPointBrightness = 0;
	if( BR_QUEUE_SIZE == m_midPrevBrightness.size() )
	{
		foreach(int item, m_midPrevBrightness) m_prevLaserPointBrightness += item;
		m_prevLaserPointBrightness /= BR_QUEUE_SIZE;

		m_midPrevBrightness.dequeue();
	}
	else m_prevLaserPointBrightness = sumBrightness;
	m_midPrevBrightness.enqueue(sumBrightness);
}

/*virtual*/bool FLSAlgorithm::brightnessFilter(int sumBrightness)
{
	if( ( sumBrightness < (m_prevLaserPointBrightness * MAX_BR_FACTOR) ) &&
		( sumBrightness > (m_prevLaserPointBrightness * MIN_BR_FACTOR) ) )
		return true;

	else if( sumBrightness >= (m_prevLaserPointBrightness * MAX_BR_FACTOR) )
	{
		m_skipFramesCounter = SKIP_FRAMES_COUNT;
		return false;
	}
	else if( sumBrightness <= (m_prevLaserPointBrightness * MIN_BR_FACTOR) ) return false;
	else return false;
}

void FLSAlgorithm::clearShootPoints()
{
	if( !m_shootPoints.isEmpty() )
	{
		qDeleteAll(m_shootPoints);
		m_shootPoints.clear();
	}
}

void FLSAlgorithm::clearShoots()
{
	if( !m_shoots.isEmpty() )
	{
		qDeleteAll(m_shoots);
		m_shoots.clear();
	}
}

FLSAlgorithm::~FLSAlgorithm() { ; }



FLSAlgorithm_OF_Type1::FLSAlgorithm_OF_Type1(QObject *parent) : FLSAlgorithm(parent) { ; }

/*virtual slot*/void FLSAlgorithm_OF_Type1::frameHandle()
{
	findAllShootsPixels();
	if(m_shootPoints.count() >= MIN_PIXELS_COUNT)
	{
		findAllShoots();
		finalFindShoots();
	}
}



FLSAlgorithm_AF_Type1::FLSAlgorithm_AF_Type1(QObject *parent) : FLSAlgorithm(parent) { ; }

/*virtual slot*/void FLSAlgorithm_AF_Type1::frameHandle()
{
	/*if(m_middleBackgdBrightness < 0 || m_middleDynamicRange < 0)
	calcFrameProperties();*/

	if(!m_skipFramesCounter)
	{
		findAllShootsPixels();
		if(m_shootPoints.count() >= MIN_PIXELS_COUNT)
		{
			findAllShoots();
			finalFindShoots();
		}
	}
	else --m_skipFramesCounter;
}


FLSAlgorithm_OF_Type2::FLSAlgorithm_OF_Type2(QObject *parent): FLSAlgorithm(parent), m_finalShoots() { ; }

/*virtual slot*/void FLSAlgorithm_OF_Type2::frameHandle()
{
	//if(m_middleBackgdBrightness < 0 || m_middleDynamicRange < 0)
	//	calcFrameProperties();

	findAllShootsPixels();
	if( m_shootPoints.count() >= frameProperties->getThreshold() )
	{
		if( m_shootPoints.count() > (frameProperties->getThreshold() * frameProperties->getSizeFactor()) )
			simpleFindAllShoots();
		else findAllShoots();
		finalFindShoots();
	}
}

/*virtual*/void FLSAlgorithm_OF_Type2::findAllShootsPixels()
{
	clearShootPoints();

	int rows = 0;
	int cols = 0;

	if(m_roiRect)
	{
		rows = m_roiRect->height();
		cols = m_roiRect->width();
	}

	if(!rows || !cols) 
	{
		rows = m_roiFrame.height();
		cols = m_roiFrame.width();
	}

	int minShootBrightness =  frameProperties->getMidBackgrBrightness() +
						      (frameProperties->getMiddleDynamicRange() / 2);

	for(int y = 0; y < rows; ++y)
	{
		for(int x = 0; x < cols; ++x)
		{
			int pixelBrightness = qGray( m_roiFrame.pixel(x, y) ); 
			if(pixelBrightness >= minShootBrightness) 
			{
				ShootPoint *newShootPoint = new ShootPoint(x, y, pixelBrightness);
				m_shootPoints.push_back(newShootPoint);
			}
		}
	}
}

/*virtual*/void FLSAlgorithm_OF_Type2::findAllShoots()
{
	clearShoots();

	QVector<ShootPoint*>::const_iterator shootPointIter = m_shootPoints.constBegin();
	bool newPoint = true;

	for(; shootPointIter != m_shootPoints.constEnd(); ++shootPointIter)
	{
		if( m_shoots.isEmpty() ) insertNewShoot( *(*shootPointIter) ); 
		else
		{
			bool newPoint = true;
			for(int i = 0; i < m_shoots.size(); ++i)  
			{
				if( (*shootPointIter)->isSameByX( *m_shoots[i]->back() ) )                                                   
				{
					m_shoots[i]->push_back(*shootPointIter);
					newPoint = false;
					break;
				} 
			}
			if(newPoint) insertNewShoot( *(*shootPointIter) );
		}
	}
	qDeleteAll(m_finalShoots);
	m_finalShoots.clear();
	merge();
}

void FLSAlgorithm_OF_Type2::simpleFindAllShoots()
{
	clearShoots();

	QVector<ShootPoint*>::const_iterator shootPointIter = m_shootPoints.constBegin();
	bool newPoint = true;

	for(; shootPointIter != m_shootPoints.constEnd(); ++shootPointIter)
	{
		if( m_shoots.isEmpty() ) insertNewShoot( *(*shootPointIter) ); 
		else
		{
			bool newPoint = true;
			for(int i = 0; i < m_shoots.size(); ++i)  
			{
				if( (*shootPointIter)->isEqualByY( *m_shoots[i]->back() ) )                                                   
				{
					m_shoots[i]->push_back(*shootPointIter);
					newPoint = false;
					break;
				} 
			}
			if(newPoint) insertNewShoot( *(*shootPointIter) );
		}
	}
	qDeleteAll(m_finalShoots);
	m_finalShoots.clear();

	FLS::ShootPoints *realShoot = new FLS::ShootPoints();
	for(int i = 0; i < m_shoots.size(); ++i)
	{
		for(int j = 0; j < m_shoots[i]->size(); ++j)
			realShoot->push_back( (*m_shoots[i])[j] );
	}
	m_finalShoots.push_back(realShoot);
}


void FLSAlgorithm_OF_Type2::merge()
{
	FLS::Shoots tempShoots;
	FLS::Shoots::iterator tempIter = m_shoots.begin();

	QVector<int>counter;
	int cnt = 0;

	for(; tempIter != m_shoots.end(); ++tempIter, ++cnt)
	{
		if( tempShoots.isEmpty() ) 
		{
			tempShoots.push_back(*tempIter);
			counter.push_back(cnt);
		}
		else
		{
			if( (*tempIter)->back()->getY() == tempShoots.back()->back()->getY() ) { ; }
				//qDebug() << "Other shoot in this frame";
			else if( (*tempIter)->back()->getY() == tempShoots.back()->back()->getY() + 1 ) { ; }
				//qDebug() << "Other shoot in next frame";
			else if( (*tempIter)->back()->getY() == tempShoots.back()->back()->getY() + 2 )
			{
				counter.push_back(cnt);
				tempShoots.push_back(*tempIter);
			}
		}
	}

	FLS::ShootPoints *realShoot = new FLS::ShootPoints();
	for(int i = 0; i < tempShoots.size(); ++i)
	{
		for(int j = 0; j < tempShoots[i]->size(); ++j)
			realShoot->push_back( (*tempShoots[i])[j] );
	}

	if( realShoot->size() >= frameProperties->getThreshold() )
		m_finalShoots.push_back(realShoot);
	else delete realShoot;

	for(int i = (counter.size() - 1); i >= 0; --i)
		m_shoots.remove(counter[i]);

	if( !m_shoots.isEmpty() ) merge();
}

/*virtual*/ void FLSAlgorithm_OF_Type2::finalFindShoots()
{
	foreach(FLS::ShootPoints *item, m_finalShoots) 
	{
		float sumBrightness = 0.0; 
		int x = 0;
		int y = 0;

		foreach(ShootPoint *point, *item)
		{
			if( item->size() >= frameProperties->getThreshold() )
			{
				sumBrightness += point->getBrightness();
				x += point->getX();
				y += point->getY();
			}
		}

		//if 0 in top left corner of all image
		x /= item->size();
		y /= item->size();
		if(m_roiRect) //if m_roiRect = nullptr that it's full image and x, y are left corner's points
		{
			x += m_roiRect->topLeft().x();
			y += m_roiRect->topLeft().y();
		}

		if( sumBrightness > frameProperties->getMBT() )
		{
			//qDebug() << "Thread ID is: " << sensorID;
			//qDebug() << "Summary brightness is: " << sumBrightness;
			//qDebug() << "Shoot X: " << x;
			//qDebug() << "Shoot Y: " << y;

			HandleResult result;
			result.m_threadId = (sensorID);
			result.m_numOfFrame = frameProperties->getNumOfFrame();
			result.m_x = x;
			result.m_y = y;
			result.m_br = sumBrightness;
			result.m_pixels = item->size();
			emit sendResults(result);

			//***This version write only continuously laser points***/// 
			//if(!m_prevLaserPointBrightness) 
				//savePrevBrightness(sumBrightness, item);
			//else if( brightnessFilter(sumBrightness) )
				//savePrevBrightness(sumBrightness, item);
		}
	}
}


FLSAlgorithm_OF_Type3::FLSAlgorithm_OF_Type3(QObject *parent) : 
FLSAlgorithm_OF_Type2(parent), firstFrame(true), frameWithShoot(false),
resultFrame(720, 576, QImage::Format_Indexed8), setup(false) 
{
	tempArr.reserve(415000);
}

/*slot*/void FLSAlgorithm_OF_Type3::setFrame(const QPixmap &frame)
{
	m_roiFrame = frame.toImage();
	//if( m_roiRect->width() && m_roiRect->height() ) 
		//m_roiFrame = m_frame.copy(m_roiRect);
	//else m_roiFrame = m_frame;

	if(firstFrame)
	{
		firstOrigFrame = m_roiFrame;
		currSubFrame = firstOrigFrame;
		resultFrame = firstOrigFrame;
	}
	else curProcFrame = m_roiFrame;
}

void FLSAlgorithm_OF_Type3::setFrame(QImage frame)
{
	//if(m_roiRect) m_roiFrame = frame.copy(*m_roiRect);
	//else m_roiFrame = frame;
	m_roiFrame = frame; //old

	if(firstFrame)
	{
		firstOrigFrame = m_roiFrame;
		currSubFrame = firstOrigFrame;
		resultFrame = firstOrigFrame;
	}
	else curProcFrame = m_roiFrame;
}

/*virtual*/void FLSAlgorithm_OF_Type3::setRoi(const QRect &roiRect)
{
	FLSAlgorithm::setRoi(roiRect);
	firstFrame = true;
}

/*virtual*/void FLSAlgorithm_OF_Type3::reset()
{
	FLSAlgorithm::reset();
	 //firstOrigFrame = QImage();
	//currSubFrame = QImage();
	//curProcFrame = QImage();
	firstFrame = true;
	setup = false;
	tempArr.clear();
}

///*virtual*/void FLSAlgorithm_OF_Type3::stopHandle()
//{
//	firstFrame = true;
//}

/*virtual*/void FLSAlgorithm_OF_Type3::frameHandle()
{
	//QMutexLocker locker(&m_mutex); //test
	if(firstFrame && !setup)
	{
		firstFrame = false;
		frameProperties->setMinShootBrightness(4); //for test
		return;
	}
	else if(!firstFrame && !setup) //test
	{
		calcMinShootBr();
		setup = true;
		return;
	}
	else 
	{
		substractionOfFrames();
		findAllShootsPixels();
		if( m_shootPoints.count() >= frameProperties->getThreshold() )
		{
			if( m_shootPoints.count() > ( frameProperties->getThreshold() * frameProperties->getSizeFactor() ) )
				simpleFindAllShoots();
			else findAllShoots();
			if( checkCorrectly() )
			finalFindShoots();
		}
		else currSubFrame = curProcFrame;
	}
}

void FLSAlgorithm_OF_Type3::calcMinShootBr()
{
	substractionOfFrames();
	int summaryBr = 0;
	int minShootBrightness = 0;

	if( !tempArr.isEmpty() )
	{
		QVector<int>::const_iterator arrIter = tempArr.end() - 1;
		QVector<int>::const_iterator arrEndIter = tempArr.begin() + (tempArr.size() / 2);
		for(; arrIter != arrEndIter; --arrIter)
			summaryBr += *arrIter;
		minShootBrightness = ( summaryBr / (tempArr.size() / 2) ) * 2;
	}
	else minShootBrightness = 5;
	frameProperties->setMinShootBrightness(minShootBrightness);
}

void FLSAlgorithm_OF_Type3::substractionOfFrames()
{
	int rows = curProcFrame.height();
	int cols = curProcFrame.width();

	for(int y = 0; y < rows; ++y)
	{
		QRgb *currPixBrLine = (QRgb*)curProcFrame.scanLine(y);
		QRgb *origPixBrLine = (QRgb*)currSubFrame.scanLine(y);
		QRgb *resPixBrLine = (QRgb*)resultFrame.scanLine(y);
		for(int x = 0; x < cols; ++x)
		{
			int currPixBr =  qGray(currPixBrLine[x]);
			int origPixBr = qGray(origPixBrLine[x]);
			int subBr = abs(currPixBr - origPixBr);

			if(!setup) if(subBr > 1) tempArr.push_back(subBr); //TODO test
					
			resPixBrLine[x] = QRgb(subBr);
			//resultFrame.setPixel(x, y, subBr);
		}
	}
	if(!setup && !oneThreadMode) qSort(tempArr); //so long if array size more then 100000 (core i7) //TODO
}

/*virtual*/void FLSAlgorithm_OF_Type3::findAllShootsPixels()
{
	clearShootPoints();

	int rows = resultFrame.height();
	int cols = resultFrame.width();

	//int minShootBrightness = 0; //TODO должен меняться динамически в зависимости от MBB и MDR 
	int minShootBrightness = frameProperties->getMinShootBrightness();

	for(int y = 0; y < rows; ++y)
	{
		QRgb *resPixBrLine = (QRgb*)resultFrame.scanLine(y);
		for(int x = 0; x < cols; ++x)
		{
			int pixelBrightness = qGray(resPixBrLine[x]);
			if(pixelBrightness > minShootBrightness) 
			{
				ShootPoint *newShootPoint = new ShootPoint(x, y, pixelBrightness);
				m_shootPoints.push_back(newShootPoint);
			}
		}
	}
	if( m_shootPoints.isEmpty() ) currSubFrame = curProcFrame;
	/*if( !m_shootPoints.isEmpty() )
		qDebug() << "Shootpoints size is: " << m_shootPoints.size();
	else currSubFrame = curProcFrame;*/
}

bool FLSAlgorithm_OF_Type3::checkCorrectly()
{
	QVector<bool>checkCor;
	if( !m_finalShoots.isEmpty() && m_finalShoots.size() <= 4)
	{
		foreach(FLS::ShootPoints *item, m_finalShoots)
		{
			if(item->size() >= ( frameProperties->getThreshold() ) )
				checkCor.push_back(true);
		}
		if( checkCor.size() == m_finalShoots.size() ) return true;
		else 
		{
			currSubFrame = curProcFrame;
			return false;
		}
	}
	else //TODO 
	{
		currSubFrame = curProcFrame; //test
		return false; 
	}
}


FLSAlgorithm_AF_Type2::FLSAlgorithm_AF_Type2(QObject *parent) 
: FLSAlgorithm_OF_Type3(parent) { ; }

/*virtual*/void FLSAlgorithm_AF_Type2::finalFindShoots()
{
	foreach(FLS::ShootPoints *item, m_finalShoots) 
	{
		float sumBrightness = 0.0; 
		int x = 0;
		int y = 0;

		if( item->size() >= frameProperties->getThreshold() )
		{
			foreach(ShootPoint *point, *item)
			{
				sumBrightness += point->getBrightness();
				x += point->getX();
				y += point->getY();
			}
		
			x /= item->size();
			y /= item->size();

			//if 0 in top left corner of all image
			if(m_roiRect)
			{
				x += m_roiRect->topLeft().x();
				y += m_roiRect->topLeft().y();
			}

			if(sumBrightness > frameProperties->getMBT() /*MIN_SHOOT_BRIGHTNESS*/)
			{
				//qDebug() << "Thread ID is: " << sensorID;
				//qDebug() << "Summary brightness is: " << sumBrightness;
				//qDebug() << "Shoot X: " << x;
				//qDebug() << "Shoot Y: " << y;
				//qDebug() << "Pixels in shoot: " << item->size();

				HandleResult result;
				result.m_threadId = sensorID;
				result.m_numOfFrame = frameProperties->getNumOfFrame();
				result.m_x = x;
				result.m_y = y;
				result.m_br = sumBrightness;
				result.m_pixels = item->size();
				emit sendResults(result);

				//***This version write only continuously laser points***/// 
				//if(!m_prevLaserPointBrightness) 
					//savePrevBrightness(sumBrightness, item);
				//else if( brightnessFilter(sumBrightness) )
					//savePrevBrightness(sumBrightness, item);
			}
		}
	}
}