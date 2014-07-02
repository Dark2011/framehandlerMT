////own
//#include "FLSAlgorithm.h"
//#include "ShootPoint.h"
////qt
//#include <QtCore/QDebug>
//#include <QtCore/QtAlgorithms>
//#include <QtCore/QFile>
//#include <QtCore/QTextStream>
//
//const int MIN_PIXELS_COUNT = 10;
//const int MIN_SHOOT_BRIGHTNESS = 1000;
//const float MIN_BR_FACTOR = 0.65;
//const float MAX_BR_FACTOR = 1.75;
//const int SKIP_FRAMES_COUNT = 3;
//const int BR_QUEUE_SIZE = 3;
//
//bool FLS::compareBy_X(const ShootPoint *point1, const ShootPoint *point2)
//{
//	return point1->getX() < point2->getX();
//}
//
//bool FLS::compareBy_Y(const ShootPoint *point1, const ShootPoint *point2)
//{
//	return point1->getY() < point2->getY();
//}
//
//bool FLS::compareBy_XY(const ShootPoint *point1, const ShootPoint *point2)
//{
//	return ( ( point1->getY() < point2->getY() ) && ( point1->getX() < point2->getX() ) );
//}
//
//
//FLSAlgorithm::FLSAlgorithm(QObject *parent) : QObject(parent), m_videoFileName( QString() ),
//m_frame( QImage() ), m_roiFrame( QImage() ), m_sensitivity(4), m_sizeFactor(5), m_threshold(10),
//m_middleBackgdBrightness(-1), m_middleDynamicRange(-1), m_roiRect( QRect() ),
//m_prevLaserPointBrightness(0), m_skipFramesCounter(0), m_midPrevBrightness() { ; }
//
//void FLSAlgorithm::setResultFileName(const QString &fileName)
//{
//	m_resultFileName = fileName;
//}
//
//void FLSAlgorithm::setVideoFilePath(const QString &fileName)
//{
//	m_videoFileName = fileName;
//}
//
///*slot*/void FLSAlgorithm::setFrame(const QPixmap &frame)
//{
//	m_frame = frame.toImage();
//	
//	if( m_roiRect.width() && m_roiRect.height() ) 
//		m_roiFrame = m_frame.copy(m_roiRect);
//	else m_roiFrame = m_frame;
//}
//
///*slot*/void FLSAlgorithm::setRoi(QPoint &fp, QPoint &sp)
//{
//	checkRoiCoords(fp, sp);
//	m_roiRect.setCoords( fp.x(), fp.y(), sp.x(), sp.y() );
//	qDebug() << m_roiRect;
//}
//
///*slot*/void FLSAlgorithm::setSensitivity(int sens)
//{
//	m_sensitivity = sens;
//}
//
///*slot*/void FLSAlgorithm::setSizeFactor(int sizefactor)
//{
//	m_sizeFactor = sizefactor;
//}
//
///*slot*/void FLSAlgorithm::setThreshold(int threshold)
//{
//	m_threshold = threshold;
//}
//
//void FLSAlgorithm::checkRoiCoords(QPoint &fp, QPoint &sp)
//{
//	if( ( fp.x() < sp.x() ) && ( fp.y() < sp.y() ) ) { ; }
//	else if( ( fp.x() < sp.x() ) && ( fp.y() > sp.y() ) )
//	{
//		int tempY = fp.y();
//		fp.setY( sp.y() );
//		sp.setY(tempY);
//	}
//	else if( ( fp.x() > sp.x() ) && ( fp.y() < sp.y() ) )
//	{
//		int tempX = fp.x();
//		fp.setX( sp.x() );
//		sp.setX(tempX);
//	}
//	else if( ( fp.x() > sp.x() ) && ( fp.y() > sp.y() ) )
//	{
//		int tempX = fp.x();
//		int tempY = fp.y();
//		fp.setX( sp.x() );
//		fp.setY( sp.y() );
//		sp.setX(tempX);
//		sp.setY(tempY);
//	}
//}
//
///*slot*/void FLSAlgorithm::calcFrameProperties()
//{
//	calcMiddleBrigthess();
//	calcMiddleDynamicRange();
//	emit setFrameProperties(m_middleBackgdBrightness, m_middleDynamicRange);
//}
//
///*virtual slot*/void FLSAlgorithm::reset()
//{
//	m_frame = QImage();
//	m_roiFrame = QImage();
//	m_middleBackgdBrightness = -1;
//	m_middleDynamicRange = -1;
//	m_sensitivity = -1;
//	m_roiRect = QRect();
//	clearShootPoints();
//	clearShoots();
//}
//
//void FLSAlgorithm::calcMiddleBrigthess()
//{
//	m_middleBackgdBrightness = 0;
//	int rows = m_frame.height();
//	int cols = m_frame.width();
//
//	for(int y = 0; y < rows; ++y)
//	{
//		for(int x = 0; x < cols; ++x)
//			m_middleBackgdBrightness += qGray( m_frame.pixel(x, y) );
//	}
//	m_middleBackgdBrightness /= (rows * cols);
//	qDebug() << "Middle background brightness is:" << m_middleBackgdBrightness; 
//}
//
//void FLSAlgorithm::calcMiddleDynamicRange()
//{
//	m_middleDynamicRange = 255 - m_middleBackgdBrightness;
//	qDebug() << "Middle dynamic range is:" << m_middleDynamicRange; 
//}
//
///*virtual*/void FLSAlgorithm::findAllShootsPixels()
//{
//	clearShootPoints();
//	
//	int rows = m_roiRect.height();
//	int cols = m_roiRect.width();
//	if(!rows || !cols) 
//	{
//		rows = m_roiFrame.height();
//		cols = m_roiFrame.width();
//	}
//
//	int minShootBrightness = m_middleBackgdBrightness + (m_middleDynamicRange / 2);
//
//	for(int y = 0; y < rows; ++y)
//	{
//		for(int x = 0; x < cols; ++x)
//		{
//			int pixelBrightness = qGray( m_roiFrame.pixel(x, y) ); 
//			if(pixelBrightness >= minShootBrightness) 
//			{
//				ShootPoint *newShootPoint = new ShootPoint(x, y, pixelBrightness);
//				m_shootPoints.push_back(newShootPoint);
//			}
//		}
//	}
//
//	if( !m_shootPoints.isEmpty() )
//	{
//		qDebug() << "Shootpoints size is: " << m_shootPoints.size();
//		qSort(m_shootPoints.begin(), m_shootPoints.end(), FLS::compareBy_X); //????? TODO
//	}
//}
//
///*virtual*/void FLSAlgorithm::findAllShoots()
//{
//	clearShoots();
//
//	QVector<ShootPoint*>::const_iterator shootPointIter = m_shootPoints.constBegin();
//	for(; shootPointIter != m_shootPoints.constEnd(); ++shootPointIter)
//	{
//		if( m_shoots.isEmpty() ) insertNewShoot( *(*shootPointIter) ); 
//		else
//		{
//			bool newPoint = true;
//			for(int i = 0; i < m_shoots.size(); ++i)  
//			{
//				if( (*shootPointIter)->isSame( *m_shoots[i]->back(), m_sensitivity ) ||
//					(*shootPointIter)->isSame( *m_shoots[i]->first(), m_sensitivity ) ||
//					(*shootPointIter)->isSame( *m_shoots[i]->at( ( *m_shoots[i] ).size() / 2 ), m_sensitivity ) )                                                   
//				{
//					m_shoots[i]->push_back(*shootPointIter);
//					newPoint = false;
//					break;
//				} 
//			}
//			if(newPoint) insertNewShoot( *(*shootPointIter) );
//		}
//	}
//}
//
///*virtual*/void FLSAlgorithm::finalFindShoots()
//{
//	foreach(FLS::ShootPoints *item, m_shoots) 
//	{
//		float sumBrightness = 0.0; 
//		int x = 0;
//		int y = 0;
//
//		foreach(ShootPoint *point, *item)
//		{
//			if(item->size() >= m_threshold)
//			{
//				sumBrightness += point->getBrightness();
//				x += point->getX();
//				y += point->getY();
//			}
//		}
//		
//		//if 0 in top left corner of all image
//		x /= item->size();
//		x += m_roiRect.topLeft().x();
//		y /= item->size();
//		y += m_roiRect.topLeft().y();
//		
//		if(sumBrightness > MIN_SHOOT_BRIGHTNESS)
//		{
//
//			qDebug() << "Summary brightness is: " << sumBrightness;
//			qDebug() << "Shoot X: " << x;
//			qDebug() << "Shoot Y: " << y;
//
//			//***This version write only continuously laser points***/// 
//			if(!m_prevLaserPointBrightness) 
//				savePrevBrightness(sumBrightness, item);
//			else if( brightnessFilter(sumBrightness) )
//				savePrevBrightness(sumBrightness, item);
//		}
//	}
//}
//
///*virtual*/void FLSAlgorithm::insertNewShoot(ShootPoint &shootPoint)
//{
//	QVector<ShootPoint*> *newShoot = new QVector<ShootPoint*>();
//	newShoot->push_back(&shootPoint);
//	m_shoots.push_back(newShoot);
//}
//
///*virtual*/void FLSAlgorithm::savePrevBrightness(int sumBrightness, FLS::ShootPoints *shoot)
//{
//	m_prevLaserPointBrightness = 0;
//	if( BR_QUEUE_SIZE == m_midPrevBrightness.size() )
//	{
//		foreach(int item, m_midPrevBrightness) m_prevLaserPointBrightness += item;
//		m_prevLaserPointBrightness /= BR_QUEUE_SIZE;
//
//		m_midPrevBrightness.dequeue();
//	}
//	else m_prevLaserPointBrightness = sumBrightness;
//	m_midPrevBrightness.enqueue(sumBrightness);
//}
//
///*virtual*/bool FLSAlgorithm::brightnessFilter(int sumBrightness)
//{
//	if( ( sumBrightness < (m_prevLaserPointBrightness * MAX_BR_FACTOR) ) &&
//		( sumBrightness > (m_prevLaserPointBrightness * MIN_BR_FACTOR) ) )
//		return true;
//
//	else if( sumBrightness >= (m_prevLaserPointBrightness * MAX_BR_FACTOR) )
//	{
//		m_skipFramesCounter = SKIP_FRAMES_COUNT;
//		return false;
//	}
//	else if( sumBrightness <= (m_prevLaserPointBrightness * MIN_BR_FACTOR) ) return false;
//	else return false;
//}
//
//void FLSAlgorithm::clearShootPoints()
//{
//	if( !m_shootPoints.isEmpty() )
//	{
//		qDeleteAll(m_shootPoints);
//		m_shootPoints.clear();
//	}
//}
//
//void FLSAlgorithm::clearShoots()
//{
//	if( !m_shoots.isEmpty() )
//	{
//		qDeleteAll(m_shoots);
//		m_shoots.clear();
//	}
//}
//
//FLSAlgorithm::~FLSAlgorithm() { ; }
//
//
//
//FLSAlgorithm_OF_Type1::FLSAlgorithm_OF_Type1(QObject *parent) : FLSAlgorithm(parent) { ; }
//
///*virtual slot*/void FLSAlgorithm_OF_Type1::frameHandle()
//{
//	if(m_middleBackgdBrightness < 0 || m_middleDynamicRange < 0)
//		calcFrameProperties();
//
//	findAllShootsPixels();
//	if(m_shootPoints.count() >= MIN_PIXELS_COUNT)
//	{
//		findAllShoots();
//		finalFindShoots();
//	}
//}
//
//
//
//void FLSAlgorithm_OF_Type1::finalFindShoots()
//{
//	
//	QFile file(m_resultFileName);
//	QTextStream stream(&file);
//	if( !file.open(QIODevice::Append) )
//		qDebug() << "Can't open file";
//
//	stream << "File destination is: " << m_videoFileName << "\r\n\r\n";
//	int shootCount = 1;
//	FLS::ShootPoints tempAllShootPoints;
//
//	foreach(FLS::ShootPoints *item, m_shoots) 
//	{
//		tempAllShootPoints.clear();
//		qSort(item->begin(), item->end(), FLS::compareBy_Y); 
//
//		int pointY = -1;
//		FLS::ShootPoints tempShootPoints;
//		tempAllShootPoints.reserve( item->size() );
//		FLS::ShootPoints::iterator pointIter = item->begin();
//
//		//сортируем все точки в каждом пятне выстрела
//		for(; pointIter != item->end(); ++pointIter)
//		{
//			if(pointY == -1) pointY = (*pointIter)->getY();
//			if( (*pointIter)->getY() == pointY )
//			{
//				tempShootPoints.push_back(*pointIter);
//				if( pointIter == (item->end() - 1) )
//				{
//					qSort(tempShootPoints.begin(), tempShootPoints.end(), FLS::compareBy_X); 
//					pointY = (*pointIter)->getY();
//					for(int i = 0; i < tempShootPoints.size(); ++i)
//						tempAllShootPoints.append(tempShootPoints[i]);
//					tempShootPoints.clear();
//				}
//			}
//			else
//			{
//				qSort(tempShootPoints.begin(), tempShootPoints.end(), FLS::compareBy_X); 
//				pointY = (*pointIter)->getY();
//				for(int i = 0; i < tempShootPoints.size(); ++i)
//					tempAllShootPoints.append(tempShootPoints[i]);
//				tempShootPoints.clear();
//				tempShootPoints.push_back(*pointIter);
//			}
//		}
//		qCopy( tempAllShootPoints.begin(), tempAllShootPoints.end(), item->begin() );
//	
//
//		float sumBrightness = 0.0; 
//		int x = 0;
//		int y = 0;
//
//		if(item->size() >= m_threshold) 
//		{
//			stream << "Shoot number: " << shootCount << "\r\n\r\n";
//			++shootCount;
//
//			int currY = -1;
//			int sumPointBrightness = 0;
//			FLS::ShootPoints::iterator xIter = item->begin();
//			FLS::ShootPoints::iterator yIter = item->begin();
//			FLS::ShootPoints::iterator brIter = item->begin();
//			for(; xIter != item->end(); ++xIter )
//			{
//				if(currY == -1) currY = (*xIter)->getY();
//				if( (*xIter)->getY() == currY )
//				{
//					stream << "x: " << ( (*xIter)->getX() + m_roiRect.topLeft().x() ) << "  ";
//					sumPointBrightness += (*xIter)->getBrightness();
//					if(xIter == (item->end() - 1) )
//					{
//						stream << "\r\n";
//						for(; yIter != item->end(); ++yIter)
//							stream << "y: " << ( (*yIter)->getY() + m_roiRect.topLeft().y() )<< "  ";
//
//						stream << "\r\n";
//						for(; brIter != item->end(); ++brIter)
//							stream << "b: " << (*brIter)->getBrightness() << "  "; 
//
//						stream << "\r\nSummary brightness is: " << sumPointBrightness << "\r\n\r\n";
//						sumPointBrightness = 0;
//					}
//				}
//				else
//				{
//					stream << "\r\n";
//					for(; yIter != item->end(); ++yIter)
//					{
//						if( (*yIter)->getY() == currY )
//							stream << "y: " << ( (*yIter)->getY() + m_roiRect.topLeft().y() ) << "  ";
//						else 
//						{
//							stream << "\r\n";
//							for(; brIter != item->end(); ++brIter)
//							{
//								if( (*brIter)->getY() == currY )
//									stream << "b: " << (*brIter)->getBrightness() << "  "; 
//								else
//								{
//									//--brIter;
//									break;
//								}
//							}
//
//							stream << "\r\nSummary brightness is: " << sumPointBrightness << "\r\n\r\n";
//							sumPointBrightness = 0;
//
//							currY = (*yIter)->getY();
//							--xIter;
//							break;
//						}
//					}
//				}
//			}//for
//		}//if
//
//		foreach(ShootPoint *point, *item)
//		{
//			sumBrightness += point->getBrightness();
//			x += point->getX();
//			y += point->getY();
//		}
//
//		//if 0 in top left corner of roi rect
//		//x /= item->size();
//		//y /= item->size();
//
//		//if 0 in centre of roi rect
//		//x -= m_roiRect.width() / 2;
//		//y -= m_roiRect.height() / 2;
//
//		//if 0 in top left corner of all image
//		x /= item->size();
//		x += m_roiRect.topLeft().x();
//		y /= item->size();
//		y += m_roiRect.topLeft().y();
//
//		if(sumBrightness > MIN_SHOOT_BRIGHTNESS)
//		{
//
//			qDebug() << "Summary brightness is: " << sumBrightness;
//			qDebug() << "Shoot X: " << x;
//			qDebug() << "Shoot Y: " << y;
//
//			stream << "\r\n--------------------------\r\n";
//			stream << "Shoot X: " << x << "\r\n";
//			stream << "Shoot Y: " << y << "\r\n";
//			stream << "Summary brightness of shoot is: " << sumBrightness << "\r\n";
//			stream << "\r\n--------------------------\r\n\r\n";
//		}
//	}
//
//	stream << "Middle background brightness: " << m_middleBackgdBrightness << "\r\n";
//	stream << "Middle dynamic range: " << m_middleDynamicRange << "\r\n\r\n";
//	file.close();
//}
//
//
//
//
//FLSAlgorithm_AF_Type1::FLSAlgorithm_AF_Type1(QObject *parent) : FLSAlgorithm(parent) { ; }
//
///*virtual slot*/void FLSAlgorithm_AF_Type1::frameHandle()
//{
//	if(m_middleBackgdBrightness < 0 || m_middleDynamicRange < 0)
//		calcFrameProperties();
//
//	if(!m_skipFramesCounter)
//	{
//		findAllShootsPixels();
//		if(m_shootPoints.count() >= MIN_PIXELS_COUNT)
//		{
//			findAllShoots();
//			finalFindShoots();
//		}
//	}
//	else --m_skipFramesCounter;
//}
//
//
//FLSAlgorithm_OF_Type2::FLSAlgorithm_OF_Type2(QObject *parent): FLSAlgorithm(parent), m_finalShoots() { ; }
//
///*virtual slot*/void FLSAlgorithm_OF_Type2::frameHandle()
//{
//	if(m_middleBackgdBrightness < 0 || m_middleDynamicRange < 0)
//		calcFrameProperties();
//
//	findAllShootsPixels();
//	if(m_shootPoints.count() >= m_threshold)
//	{
//		if( m_shootPoints.count() > (m_threshold * m_sizeFactor) )
//			simpleFindAllShoots();
//		else findAllShoots();
//		finalFindShoots();
//	}
//}
//
///*virtual*/void FLSAlgorithm_OF_Type2::findAllShootsPixels()
//{
//	clearShootPoints();
//
//	int rows = m_roiRect.height();
//	int cols = m_roiRect.width();
//	if(!rows || !cols) 
//	{
//		rows = m_roiFrame.height();
//		cols = m_roiFrame.width();
//	}
//
//	int minShootBrightness = m_middleBackgdBrightness + (m_middleDynamicRange / 2);
//
//	for(int y = 0; y < rows; ++y)
//	{
//		for(int x = 0; x < cols; ++x)
//		{
//			int pixelBrightness = qGray( m_roiFrame.pixel(x, y) ); 
//			if(pixelBrightness >= minShootBrightness) 
//			{
//				ShootPoint *newShootPoint = new ShootPoint(x, y, pixelBrightness);
//				m_shootPoints.push_back(newShootPoint);
//			}
//		}
//	}
//}
//
///*virtual*/void FLSAlgorithm_OF_Type2::findAllShoots()
//{
//	clearShoots();
//
//	QVector<ShootPoint*>::const_iterator shootPointIter = m_shootPoints.constBegin();
//	bool newPoint = true;
//
//	for(; shootPointIter != m_shootPoints.constEnd(); ++shootPointIter)
//	{
//		if( m_shoots.isEmpty() ) insertNewShoot( *(*shootPointIter) ); 
//		else
//		{
//			bool newPoint = true;
//			for(int i = 0; i < m_shoots.size(); ++i)  
//			{
//				if( (*shootPointIter)->isSameByX( *m_shoots[i]->back() ) )                                                   
//				{
//					m_shoots[i]->push_back(*shootPointIter);
//					newPoint = false;
//					break;
//				} 
//			}
//			if(newPoint) insertNewShoot( *(*shootPointIter) );
//		}
//	}
//	qDeleteAll(m_finalShoots);
//	m_finalShoots.clear();
//	merge();
//}
//
//void FLSAlgorithm_OF_Type2::simpleFindAllShoots()
//{
//	clearShoots();
//
//	QVector<ShootPoint*>::const_iterator shootPointIter = m_shootPoints.constBegin();
//	bool newPoint = true;
//
//	for(; shootPointIter != m_shootPoints.constEnd(); ++shootPointIter)
//	{
//		if( m_shoots.isEmpty() ) insertNewShoot( *(*shootPointIter) ); 
//		else
//		{
//			bool newPoint = true;
//			for(int i = 0; i < m_shoots.size(); ++i)  
//			{
//				if( (*shootPointIter)->isEqualByY( *m_shoots[i]->back() ) )                                                   
//				{
//					m_shoots[i]->push_back(*shootPointIter);
//					newPoint = false;
//					break;
//				} 
//			}
//			if(newPoint) insertNewShoot( *(*shootPointIter) );
//		}
//	}
//	qDeleteAll(m_finalShoots);
//	m_finalShoots.clear();
//
//	FLS::ShootPoints *realShoot = new FLS::ShootPoints();
//	for(int i = 0; i < m_shoots.size(); ++i)
//	{
//		for(int j = 0; j < m_shoots[i]->size(); ++j)
//			realShoot->push_back( (*m_shoots[i])[j] );
//	}
//	m_finalShoots.push_back(realShoot);
//}
//
//
//void FLSAlgorithm_OF_Type2::merge()
//{
//	FLS::Shoots tempShoots;
//	FLS::Shoots::iterator tempIter = m_shoots.begin();
//
//	QVector<int>counter;
//	int cnt = 0;
//
//	for(; tempIter != m_shoots.end(); ++tempIter, ++cnt)
//	{
//		if( tempShoots.isEmpty() ) 
//		{
//			tempShoots.push_back(*tempIter);
//			counter.push_back(cnt);
//		}
//		else
//		{
//			if( (*tempIter)->back()->getY() == tempShoots.back()->back()->getY() )
//				qDebug() << "Other shoot in this frame";
//			else if( (*tempIter)->back()->getY() == tempShoots.back()->back()->getY() + 1 )
//				qDebug() << "Other shoot in next frame";
//			else if( (*tempIter)->back()->getY() == tempShoots.back()->back()->getY() + 2 )
//			{
//				counter.push_back(cnt);
//				tempShoots.push_back(*tempIter);
//			}
//		}
//	}
//
//	FLS::ShootPoints *realShoot = new FLS::ShootPoints();
//	for(int i = 0; i < tempShoots.size(); ++i)
//	{
//		for(int j = 0; j < tempShoots[i]->size(); ++j)
//			realShoot->push_back( (*tempShoots[i])[j] );
//	}
//	m_finalShoots.push_back(realShoot);
//
//	for(int i = (counter.size() - 1); i >= 0; --i)
//		m_shoots.remove(counter[i]);
//
//	if( !m_shoots.isEmpty() ) merge();
//}
//
//
///*virtual*/void FLSAlgorithm_OF_Type2::finalFindShoots()
//{
//	QFile file(m_resultFileName);
//	QTextStream stream(&file);
//	if( !file.open(QIODevice::Append) )
//		qDebug() << "Can't open file";
//
//	stream << "File destination is: " << m_videoFileName << "\r\n\r\n";
//	int shootCount = 1;
//
//	foreach(FLS::ShootPoints *item, m_finalShoots) 
//	{
//		float sumBrightness = 0.0; 
//		int x = 0;
//		int y = 0;
//
//		if(item->size() >= m_threshold) 
//		{
//			stream << "Shoot number: " << shootCount << "\r\n\r\n";
//			++shootCount;
//
//			int currY = -1;
//			int sumPointBrightness = 0;
//			FLS::ShootPoints::iterator xIter = item->begin();
//			FLS::ShootPoints::iterator yIter = item->begin();
//			FLS::ShootPoints::iterator brIter = item->begin();
//			for(; xIter != item->end(); ++xIter )
//			{
//				if(currY == -1) currY = (*xIter)->getY();
//				if( (*xIter)->getY() == currY )
//				{
//					stream << "x: " << ( (*xIter)->getX() + m_roiRect.topLeft().x() ) << "  ";
//					sumPointBrightness += (*xIter)->getBrightness();
//					if(xIter == (item->end() - 1) )
//					{
//						stream << "\r\n";
//						for(; yIter != item->end(); ++yIter)
//							stream << "y: " << ( (*yIter)->getY() + m_roiRect.topLeft().y() )<< "  ";
//
//						stream << "\r\n";
//						for(; brIter != item->end(); ++brIter)
//							stream << "b: " << (*brIter)->getBrightness() << "  "; 
//
//						stream << "\r\nSummary brightness is: " << sumPointBrightness << "\r\n\r\n";
//						sumPointBrightness = 0;
//					}//if
//				}//if
//				else
//				{
//					stream << "\r\n";
//					for(; yIter != item->end(); ++yIter)
//					{
//						if( (*yIter)->getY() == currY )
//							stream << "y: " << ( (*yIter)->getY() + m_roiRect.topLeft().y() ) << "  ";
//						else 
//						{
//							stream << "\r\n";
//							for(; brIter != item->end(); ++brIter)
//							{
//								if( (*brIter)->getY() == currY )
//									stream << "b: " << (*brIter)->getBrightness() << "  "; 
//								else break;
//							}
//
//							stream << "\r\nSummary brightness is: " << sumPointBrightness << "\r\n\r\n";
//							sumPointBrightness = 0;
//
//							currY = (*yIter)->getY();
//							--xIter;
//							break;
//						}
//					}//for
//				}//else
//			}//for
//		}//if
//
//		if(item->size() >= m_threshold) 
//		{
//			foreach(ShootPoint *point, *item)
//			{
//				sumBrightness += point->getBrightness();
//				x += point->getX();
//				y += point->getY();
//			}
//
//			//if 0 in top left corner of all image
//			x /= item->size();
//			x += m_roiRect.topLeft().x();
//			y /= item->size();
//			y += m_roiRect.topLeft().y();
//
//			if(sumBrightness > MIN_SHOOT_BRIGHTNESS)
//			{
//
//				qDebug() << "Summary brightness is: " << sumBrightness;
//				qDebug() << "Shoot X: " << x;
//				qDebug() << "Shoot Y: " << y;
//
//				stream << "\r\n--------------------------\r\n";
//				stream << "Shoot X: " << x << "\r\n";
//				stream << "Shoot Y: " << y << "\r\n";
//				stream << "Summary brightness of shoot is: " << sumBrightness << "\r\n";
//				stream << "\r\n--------------------------\r\n\r\n";
//			}//if
//		}//if
//	}
//
//	stream << "Middle background brightness: " << m_middleBackgdBrightness << "\r\n";
//	stream << "Middle dynamic range: " << m_middleDynamicRange << "\r\n\r\n";
//	file.close();
//}