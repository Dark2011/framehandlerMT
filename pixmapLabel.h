#pragma once

#include <QtGui/QPixmap>
#include <QtGui/QLabel>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/QTimer>
#include <QtCore/QList>

QT_FORWARD_DECLARE_CLASS(QPaintEvent);
QT_FORWARD_DECLARE_CLASS(QString);
QT_FORWARD_DECLARE_CLASS(QSize);
QT_FORWARD_DECLARE_CLASS(QMouseEvent);

class PixmapLabel : public QLabel
{
	Q_OBJECT

public:
	PixmapLabel(QWidget *parent = nullptr);

	bool overScalingEnable() const;
	bool keepAspectRatioEnable() const;
	QSize getOriginalSize() const;

protected:
	virtual void paintEvent(QPaintEvent *);

public slots:
	void setScaledPixmap(const QPixmap &);
	void loadPixmap(const QString &);
	void removePixmap();
	void enableOverScaling(bool enable = true);
	void enableKeepAspectRatio(bool enable = true);
	void setFrame(bool flag = true);
	const QString& getPixmapPath() const; 

	const QPixmap& getFrame() const;

private:
	bool fitToScreen(const QSize &);

protected:
	QPixmap scaledPixmap;

private:
	QPixmap m_pixmap;
	QString m_pixmapPath;
	bool m_keepAspectRatio;
	bool m_overScalingEnabled;
	QSize m_originalSize;
};

class PixmapRoiLabel : public PixmapLabel
{
	Q_OBJECT

public:
	PixmapRoiLabel(QWidget *parent = 0);
	bool roiSaved() const;

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void resizeEvent(QResizeEvent *);

signals:
	void setRoi(QPoint &, QPoint &);
	void setRoi(const QList<QRect> &);
	void clearRoiRect();
	void setCurrentCursorPos(const QPoint &);

public slots:
	void drawCurrentROI(bool);
	void drawAllROI(bool);
	void saveROI(bool);
	void clearRoi();
	void drawCornerPoint(const QString &);
	void notDrawCorners();

	//test
	void zoomIn(bool);
	void zoomOut(bool);

private:
	void calcRoiScale();
	void addRoiRect();
	void calcCurrentRoiScale(const QSize *, const QSize *);

private:
	QPoint firstROICoord; //финальные координаты
	QPoint secondROICoord; //финальные координаты
	QPoint firstRoiRectCoord;
	QPoint secondRoiRectCoord;
	
	QList<QRect> finalRoiRectList; //test
	QList<QRect> currentRoiRectList; //test

	bool enableDrawRect;
	bool startDrawRect;
	bool currentDraw;
	bool savedRect;
	bool drawSaveText;
	bool drawCornPoint;
	bool zoomInMode;
	bool zoomOutMode;

	QString m_cornerName;

	QTimer updateTimer;
};