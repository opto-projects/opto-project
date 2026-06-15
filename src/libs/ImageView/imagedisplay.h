#ifndef IMAGEDISPLAY_H
#define IMAGEDISPLAY_H


#include "bitdepthconverter.h"
#include "overlayitems/lineoverlay.h"
#include "overlayitems/rectoverlay.h"
#include "overlayitems/polygonoverlay.h"
#include "overlayitems/circleoverlay.h"
#include "overlayitems/ellipseoverlay.h"
#include "overlayitems/textoverlay.h"

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QThread>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QtMath>
#include <QAction>

class ImageDisplay : public QGraphicsView
{
	Q_OBJECT
	QThread converterThread;

public:
	explicit ImageDisplay(QWidget *parent = nullptr);
	~ImageDisplay();
	QGraphicsItem* graphItem() { return this->imageItem; }
	TextOverlay* getTextDisplay() { return this->resultOverlay; }
	RectOverlay* getRoi(){return this->roiRect;}
	QGraphicsScene* getScene(){ return this->scene; }

	QList<QPair<QString,OverlayItem* >>& getOverlays() { return this->overlays; }

	void createOverlays();
	void initOverlays();
	qreal getRotationAngle() {return this->oldRotationAngle;}
	void rotateAbsolute(qreal angle);

	bool setColorMap(const QString& colormap);
	inline QString colorMapName() const { return m_colorMapName; }
	bool setRawView(bool on);


private:
	void createActions();
	void showEvent(QShowEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void contextMenuEvent(QContextMenuEvent* event) override;
	void scaleView(qreal scaleFactor);
	void rotateRelative(QPoint pixels, QPoint degrees);
	void rotate90Angle(bool dir);

	void toggleColorBar(QAction* action);

private:
	BitDepthConverter* bitConverter;
	QGraphicsScene* scene;
	QGraphicsPixmapItem* imageItem;
	int frameWidth;
	int frameHeight;
	int mousePosX;
	int mousePosY;
	RectOverlay* roiRect;
	TextOverlay* resultOverlay;
	QRect currentRoi;

	QList<QPair<QString,OverlayItem*>> overlays;
	bool isFirstShowEvent;
	qreal oldRotationAngle;

	QAction* actionColorMap;
	bool isRawView;
	QString m_colorMapName;
public slots:
	void zoomIn();
	void zoomOut();
	void receiveFrame(void* frame, unsigned int bitDepth, unsigned int width, unsigned int height, unsigned int channel =1 );
	void processFrame(uchar* frame, unsigned int bitDepth, unsigned int width, unsigned int height, unsigned int channel = 1);
	void displayFrame(uchar* frame, unsigned int width, unsigned int height, unsigned int channel =1);
	void setRoi(QRect roi);
private slots:
	void onOverlayChanged(OverlayItem* overlay);
	void onRotationAngleChanged(qreal angle);
signals:
	void non8bitFrameReceived(void *frame, unsigned int bitDepth, unsigned int width, unsigned int height,unsigned int channel);
	void actionToColorMap(QString colormap, void* frame, unsigned int bitDepth, unsigned int width, unsigned int height, unsigned int channel);
	void roiChanged(QRect);
	void info(QString);
	void error(QString);
	void overlayStateChanged();
	void rotationAngleChanged(qreal angle);
};

#endif //IMAGEDISPLAY_H
