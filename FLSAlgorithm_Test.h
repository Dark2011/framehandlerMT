#pragma once

//qt
#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QVector>
#include <QtCore/QList>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtCore/QRect>
#include <QtCore/QQueue>
//test
#include <QtCore/QMutex>

//own
#include "FLSMainHandler.h"

class QPoint;
class ShootPoint;

namespace FLS
{
	typedef QVector<ShootPoint*> ShootPoints;
	typedef QVector< QVector<ShootPoint*>* > Shoots;
	typedef QList< QVector<ShootPoint*>* > ResultsOfShooting;
	bool compareBy_X(const ShootPoint *, const ShootPoint *);
	bool compareBy_Y(const ShootPoint *, const ShootPoint *);
	bool compareBy_XY(const ShootPoint *, const ShootPoint *);
}

namespace FLS
{
	struct AlgProperties;
}

class FLSAlgorithm : public QThread
{
	Q_OBJECT

public:
	FLSAlgorithm(QObject *parent = 0);
	virtual ~FLSAlgorithm();

signals:
	void sendResults(const HandleResult &);

public slots:
	virtual void frameHandle() = 0; 
	virtual void reset();
	void setOneThreadMode(bool); //TODO test
	void setID(int);
	virtual void setFrame(QImage);
	void setRoi(const QRect &);
	void setFrameProperties(/*const*/ FLS::AlgProperties &);

protected:
	virtual void run();

	virtual void findAllShootsPixels();
	virtual void findAllShoots();
	virtual void finalFindShoots();

	virtual void insertNewShoot(ShootPoint &);
	virtual void savePrevBrightness(int, FLS::ShootPoints *);
	virtual bool brightnessFilter(int);

	void clearShootPoints();
	void clearShoots();

protected:
	int sensorID;

	QImage m_roiFrame;
	const QRect *m_roiRect;

	QMutex m_mutex;
	volatile bool oneThreadMode;

	/*const*/ FLS::AlgProperties *frameProperties;
	FLS::ShootPoints m_shootPoints;
	FLS::Shoots m_shoots;

	int m_prevLaserPointBrightness;
	int m_skipFramesCounter;
	QQueue<int>m_midPrevBrightness;
};


class FLSAlgorithm_OF_Type1 : public FLSAlgorithm //one frame
{
	Q_OBJECT

public:
	FLSAlgorithm_OF_Type1(QObject *parent = 0);

public slots:
	virtual void frameHandle();
};


class FLSAlgorithm_AF_Type1 : public FLSAlgorithm //all frames (video)
{
	Q_OBJECT

public:
	FLSAlgorithm_AF_Type1(QObject *parent = 0);

	public slots:
		virtual void frameHandle(); 
};


class FLSAlgorithm_OF_Type2 : public FLSAlgorithm
{
	Q_OBJECT

public:
	FLSAlgorithm_OF_Type2(QObject *parent = 0);

public slots:
	virtual void frameHandle(); 

protected:
	virtual void findAllShootsPixels();
	virtual void findAllShoots();
	virtual void finalFindShoots();
	void simpleFindAllShoots();

private:
	void merge();

protected:
	FLS::Shoots m_finalShoots;
};

class FLSAlgorithm_OF_Type3 : public FLSAlgorithm_OF_Type2
{
	Q_OBJECT

public:
	FLSAlgorithm_OF_Type3(QObject *parent = 0);

public slots:
	virtual void frameHandle();
	virtual void setFrame(const QPixmap &);
	virtual void setFrame(QImage);
	virtual void setRoi(const QRect &);
	virtual void reset();
	//virtual void stopHandle();

protected:
	virtual void findAllShootsPixels();
	void substractionOfFrames(); 
	bool checkCorrectly(); //test
	void calcMinShootBr();

protected:
	bool firstFrame;
	bool frameWithShoot;
	QImage firstOrigFrame;
	QImage currSubFrame;
	QImage curProcFrame;
	QImage resultFrame;

	volatile bool setup;
	QVector<int> tempArr;
};


class FLSAlgorithm_AF_Type2 : public FLSAlgorithm_OF_Type3 
{
	Q_OBJECT

public:
	FLSAlgorithm_AF_Type2(QObject *parent = 0);

protected:
	virtual void finalFindShoots();
};
