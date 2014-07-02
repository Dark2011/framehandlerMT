#pragma once
#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtCore/QList>
#include <QtCore/QRect>
#include <QtGui/QImage>
#include "FLSAlgorithm.h"
#include "FLSAlgorithmsCreator.h"

class QImage;

namespace FLS
{

struct AlgProperties
{
public:
	AlgProperties() : m_middleBackgdBrightness(-1), m_middleDynamicRange(-1), m_minShootBrightness(-1),
					  m_sensitivity(4), m_sizeFactor(5), m_threshold(10), m_mbt(500), m_numOfFrame(0) { ; }

	int getMidBackgrBrightness() const { return m_middleBackgdBrightness; }
	int getMiddleDynamicRange() const { return m_middleDynamicRange; }
	int getMinShootBrightness() const { return m_minShootBrightness; }
	int getSensitivity() const { return m_sensitivity; }
	int getSizeFactor() const { return m_sizeFactor; }
	int getThreshold() const { return m_threshold; }
	int getMBT() const { return m_mbt; }
	int getNumOfFrame() { return m_numOfFrame; }

	void setMidBackgrBrightness(int mbb) { m_middleBackgdBrightness = mbb; }
	void setMiddleDynamicRange(int mdr) { m_middleDynamicRange = mdr; }
	void setMinShootBrightness(int msb) { m_minShootBrightness = msb; }
	void setSensitivity(int sens) { m_sensitivity = sens; }
	void setSizeFactor(int sizefactor) { m_sizeFactor = sizefactor; }
	void setThreshold(int threshold) { m_threshold = threshold; }
	void setMBT(int mbt) { m_mbt = mbt; }
	void setNumOfFrame(int num) { m_numOfFrame = num; }

private:
	int m_middleBackgdBrightness;
	int m_middleDynamicRange;
	int m_minShootBrightness;
	int m_sensitivity;
	int m_sizeFactor;
	int m_threshold;
	int m_mbt;
	int m_numOfFrame;
};

}


class HandleResult //TODO Struct
{
	
public:
	HandleResult(QObject *parent = 0) : m_threadId(-1), m_numOfFrame(-1), 
						                m_x(-1), m_y(-1), m_br(-1), m_pixels(-1) { ; }
	/*HandleResult(int id, int x, int y, int br, int pixels) :
	m_threadId(id), m_x(x), m_y(y), m_br(br), m_pixels(pixels) { ; }*/

	int m_threadId;
	int m_numOfFrame;
	int m_x;
	int m_y;
	int m_br;
	int m_pixels;
};


class FLSAlgorithmHolder : public QObject
{
	Q_OBJECT

public:
	FLSAlgorithmHolder(QObject *parent = 0, FAC::TypeOfAlgo type = FAC::ONE_FRAME_TYPE2);

	void setType(FAC::TypeOfAlgo type);
	
public slots:
	void setRoiRects(const QList<QRect> &rects);
	void setFrame(const QPixmap &);
	void calcFrameProperties();
	void setSensitivity(int);
	void setSizeFactor(int);
	void setThreshold(int);
	void setMBT(int);
	void setNumOfFrame(int);
	void setResultFileName(const QString &);
	void setVideoFilePath(const QString &);
	void exec();
	void reset();
	void endOfHandle();

	//test
	//void writeResultToFile(const HandleResult &);

signals:
	void setFrameProperties(int, int);
	void writeResult(const HandleResult &);
	//void writeResult(HandleResult);

private:
	void initHandlers();
	void calcMiddleBrigthess();
	void calcMiddleDynamicRange();
	void calcMinShootBrightness();
	void createConnections();

private:
	QImage m_frame;
	const QList<QRect> *m_roiRects;
	FLS::AlgProperties m_properties;

	QString m_resultFileName;
	QString m_videoFileName;

	FLSAlgorithmsCreator m_algCreator;
	FAC::TypeOfAlgo m_type;
	QList< QSharedPointer<FLSAlgorithm> > m_handlers;

	bool initComplete;
};