#ifndef CAMERAVIEWWIDGET_H
#define CAMERAVIEWWIDGET_H

#include <QVideoWidget>
#include <QCameraInfo>
#include <QCamera>
#include <QMediaRecorder>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QGuiApplication>
#include "lineoverlay.h"
#include "rectoverlay.h"
#include "polygonoverlay.h"
#include "circleoverlay.h"


class CameraViewWidget : public QGraphicsView
{
	Q_OBJECT
public:
	explicit CameraViewWidget(QWidget *parent = nullptr);
	~CameraViewWidget();
	
	qreal getRotationAngle();
	void rotateAbsolute(qreal angle);
	QCamera* getCamera() const {return this->camera;}
	QList<QCameraViewfinderSettings> getSupportedSettings() const {return this->currentSupportedSettings;}
	QList<QPair<OverlayItem*, QString>>& getOverlays() {return this->overlays;}
	void setSnapshotSaveDir(QString dir) {this->snapshotSaveDir = dir;}

protected:
	void showEvent(QShowEvent* event) override;
	void hideEvent(QHideEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void contextMenuEvent(QContextMenuEvent* event) override;

private:
	QCamera* camera;
	QGraphicsScene* scene;
	QGraphicsVideoItem* videoWidget;
	qreal oldRotationAngle;
	QCameraInfo currentCamera;
	QList<QCameraViewfinderSettings> currentSupportedSettings;
	bool isFirstShowEvent;
	QList<QPair<OverlayItem*, QString>> overlays;
	QString snapshotSaveDir;

	void createOverlays();
	void initOverlays();

public slots:
	void fitCameraViewToWindow();
	void setCamera(const QCameraInfo& camera);
	void openCamera(const QCameraInfo& camera);
	void closeCamera();
	void takeSnapshot();
	void openSetSaveLocationDialog();

signals:
	void error(QString);
	void info(QString);
	void rotationAngleChanged(qreal angle);
	void currentCameraChanged(QString cameraName);
	void snapshotDirChanged(QString dir);
	void overlayStateChanged();
	
private slots:
	void saveSnapshot(int id, const QImage &image);
	void onOverlayChanged(OverlayItem* overlay);
};

#endif //CAMERAVIEWWIDGET_H
