#include "cameraviewwidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QRubberBand>
#include <QGraphicsProxyWidget>
#include <QCamera>
#include <QCameraInfo>
#include <QTimer>
#include <QCameraImageCapture>
#include <QDateTime>
#include <QMenu>
#include <QAction>
#include <QPair>
#include <QDir>
#include <QFileDialog>


CameraViewWidget::CameraViewWidget(QWidget *parent)
	: QGraphicsView(parent),
	  camera(nullptr),
	  scene(new QGraphicsScene(this)),
	  videoWidget(new QGraphicsVideoItem()),
	  oldRotationAngle(0.0),
	  isFirstShowEvent(true)
{
	this->createOverlays();
	this->setScene(this->scene);
	this->scene->addItem(this->videoWidget);
}

CameraViewWidget::~CameraViewWidget() {
	this->closeCamera();

	if(this->videoWidget){
		delete this->videoWidget;
	}
}

qreal CameraViewWidget::getRotationAngle() {
	return this->oldRotationAngle;
}

void CameraViewWidget::showEvent(QShowEvent *event) {
	QGraphicsView::showEvent(event);
	if(!this->currentCamera.isNull()){
#ifdef __linux__
		this->openCamera(this->currentCamera);
#else
		QTimer::singleShot(0, this, [this]() { this->openCamera(this->currentCamera); }); //singleShot(0) executed as soon as all events are processed. on windows camera screen remains black when not using singleSho here
#endif
	}else{
#ifdef __linux__
		this->openCamera(QCameraInfo::defaultCamera());
#else
		QTimer::singleShot(0, this, [this]() { this->openCamera(QCameraInfo::defaultCamera()); });
#endif
	}
	if(this->isFirstShowEvent){
		//executing fitCameraViewToWindow immediately and then via singleShot to ensure accurate fitting,
		//as the first call alone doesn't always adjust the view correctly due to pending.
		this->fitCameraViewToWindow();
		QTimer::singleShot(500, this, &CameraViewWidget::fitCameraViewToWindow);
		this->isFirstShowEvent = false;
		this->initOverlays();
	}
}

void CameraViewWidget::hideEvent(QHideEvent *event) {
	QGraphicsView::hideEvent(event);
	if (this->camera) {
		this->camera->stop();
		delete this->camera;
		this->camera = nullptr;
	}
}

void CameraViewWidget::mouseDoubleClickEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		this->fitCameraViewToWindow();
	}
	if (event->button() == Qt::MidButton) {
		this->rotateAbsolute(0);
	}
	if (event->button() == Qt::RightButton) {
		this->takeSnapshot();
	}
	QGraphicsView::mouseDoubleClickEvent(event);
}

void CameraViewWidget::wheelEvent(QWheelEvent *event) {
	if(this->underMouse() && QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier)){
		QPoint numPixels = event->pixelDelta();
		QPoint numDegrees = event->angleDelta()/8;
		qreal scaleValue = 0;
		if (!numPixels.isNull()) {
			scaleValue += numPixels.y()/30.0;
		} else if (!numDegrees.isNull()) {
			QPoint numSteps = numDegrees/15;
			scaleValue += (float)numSteps.y()/30.0;
		}
		this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
		this->scale(1.0+scaleValue, 1.0+scaleValue);
		event->accept();
	}
	if(this->underMouse() && QGuiApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)){
		QPoint numPixels = event->pixelDelta();
		QPoint numDegrees = event->angleDelta()/8;

		qreal angle = this->oldRotationAngle;
		if (!numPixels.isNull()) {
			angle += numPixels.y()/30.0;
		} else if (!numDegrees.isNull()) {
			QPoint numSteps = numDegrees/15;
			angle += (float)numSteps.y()/30.0;
		}
		this->rotateAbsolute(angle);
		event->accept();
	}
}

void CameraViewWidget::keyPressEvent(QKeyEvent* event) {
	if((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_S)){
		this->takeSnapshot();
	} else {
		QGraphicsView::keyPressEvent(event);
	}
}

void CameraViewWidget::contextMenuEvent(QContextMenuEvent *event) {
	QMenu menu(this);

	//overlay actions
	for (auto &overlay : overlays) {
		QAction *action = menu.addAction(overlay.second);
		action->setCheckable(true);
		action->setChecked(overlay.first->isVisible());

		connect(action, &QAction::triggered, this, [this, overlay]() {
			overlay.first->setVisible(!overlay.first->isVisible());
			this->scene->update();
		});
	}

	//snapshot actions
	menu.addSeparator();
	QAction *takeSnapshotAction = menu.addAction("Take snapshot");
	connect(takeSnapshotAction, &QAction::triggered, this, &CameraViewWidget::takeSnapshot);
	QAction *setSnapshotLocationAction = menu.addAction("Set snapshot save location...");
	connect(setSnapshotLocationAction, &QAction::triggered, this, &CameraViewWidget::openSetSaveLocationDialog);

	menu.exec(event->globalPos());
}

void CameraViewWidget::createOverlays() {
	this->overlays.append(qMakePair(new LineOverlay(), QString("Line overlay")));
	this->overlays.append(qMakePair(new RectOverlay(), QString("Rect overlay")));
	this->overlays.append(qMakePair(new PolygonOverlay(), QString("Polygon overlay")));
	this->overlays.append(qMakePair(new CircleOverlay(), QString("Circle overlay")));
	for (auto &overlayPair : overlays) {
		OverlayItem* overlayItem = dynamic_cast<OverlayItem*>(overlayPair.first);
		if (overlayItem) {
			overlayItem->hide();
		}
	}

}

void CameraViewWidget::initOverlays() {
	//for linux/ubuntu this should happen during the very first show event.
	//if the videoWidget is set as parent item in the constructor camera screen will remain black
	for (auto &overlayPair : overlays) {
		OverlayItem* overlayItem = dynamic_cast<OverlayItem*>(overlayPair.first);
		if (overlayItem) {
			overlayItem->setParentItem(this->videoWidget);
			overlayItem->setName(overlayPair.second);
			connect(overlayItem, &OverlayItem::positionChanged, this, &CameraViewWidget::onOverlayChanged);
			connect(overlayItem, &OverlayItem::visibilityChanged, this, &CameraViewWidget::onOverlayChanged);
		}
	}
}

void CameraViewWidget::fitCameraViewToWindow() {
	this->fitInView(this->videoWidget->boundingRect(), Qt::KeepAspectRatio);
	this->ensureVisible(this->videoWidget->boundingRect());
	this->centerOn(this->videoWidget);
	this->scene->setSceneRect(this->scene->itemsBoundingRect());
}

void CameraViewWidget::setCamera(const QCameraInfo &camera) {
	this->currentCamera = camera;
}

void CameraViewWidget::openCamera(const QCameraInfo& cameraInfo) {
	if (cameraInfo.isNull()) {
		emit error(tr("Camera not available!"));
		return;
	}

	//do nothing if camera is already selected and running
	if(this->camera && this->currentCamera == cameraInfo && this->camera->state() == QCamera::ActiveState){
		return;
	}

	//stop and delete camera to be able to create newly selected camera
	this->closeCamera();

	//create new camera and start live view
	this->camera = new QCamera(cameraInfo, this);
	this->camera->setViewfinder(this->videoWidget);

	//connect stateChanged signal to a lambda function to get supported camera settings while camera is in loaded state
	connect(this->camera, &QCamera::stateChanged, this, [this](QCamera::State newState) {
		if (newState == QCamera::LoadedState) {
			this->currentSupportedSettings = this->camera->supportedViewfinderSettings();
			this->camera->start();
		}
	});

	//load the camera to trigger state change
#ifndef __linux__
	this->camera->load();
#endif

	//under linux/ubuntu several GStreamer-CRITICAL errors occur when calling load() the first time
	//this is a workaround
#ifdef __linux__
	if(this->isFirstShowEvent) {
		this->camera->start();
	} else {
		this->camera->load();
	}
	//	GStreamer-CRITICAL **:  gst_element_link_pads_full: assertion 'GST_IS_ELEMENT (src)' failed
	//	GStreamer-CRITICAL gst_object_unref: assertion 'object != NULL' failed
	//	CameraBin error: "GStreamer error: negotiation problem."
	//	CameraBin error: "Internal data stream error."
#endif

	//remember current camera selection
	this->currentCamera = cameraInfo;
	emit currentCameraChanged(this->currentCamera.deviceName());
}

void CameraViewWidget::closeCamera() {
	if (this->camera) {
		this->camera->stop();
		this->camera->unload();
		this->camera->deleteLater();
		this->camera = nullptr;
	}
}

void CameraViewWidget::rotateAbsolute(qreal angle) {
	this->rotate(this->oldRotationAngle-angle);
	this->oldRotationAngle = angle;
	emit rotationAngleChanged(this->oldRotationAngle);
}

void CameraViewWidget::takeSnapshot() {
	if(!this->camera || this->camera->status() != QCamera::ActiveStatus){
		return;
	}
	QCameraImageCapture *imageCapture = new QCameraImageCapture(this->camera);
	connect(imageCapture, &QCameraImageCapture::imageCaptured, this, &CameraViewWidget::saveSnapshot);
	connect(imageCapture, &QCameraImageCapture::imageCaptured, imageCapture, &QObject::deleteLater); //this will delete imageCaputer after the image was saved
	imageCapture->capture();
}

void CameraViewWidget::saveSnapshot(int id, const QImage &image) {
	Q_UNUSED(id);
	QString fileName = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + "_snapshot.png";

	//check if the snapshot save directory is set, otherwise use a default directory
	QString saveDirPath = this->snapshotSaveDir.isEmpty() ? QDir::homePath() : this->snapshotSaveDir;

	//ensure the directory separator is handled correctly
	QDir saveDir(saveDirPath);
	QString savePath = saveDir.filePath(fileName);

	if (image.save(savePath))
		emit info("Snapshot saved to " + savePath);
	else
		emit error("Failed to save snapshot to " + savePath);
}

void CameraViewWidget::openSetSaveLocationDialog() {
	QString defaultDirPath = this->snapshotSaveDir.isEmpty() ? QDir::homePath() : this->snapshotSaveDir;
	QString dir = QFileDialog::getExistingDirectory(this, tr("Select Save Location"), defaultDirPath);
	if (!dir.isEmpty()) {
		if(this->snapshotSaveDir != dir){
			this->snapshotSaveDir = dir;
			emit snapshotDirChanged(this->snapshotSaveDir);
		}
	}
}

void CameraViewWidget::onOverlayChanged(OverlayItem *overlay) {
	QString overlayName = overlay->getName();
	bool isVisible = overlay->isVisible();
	QString infoMsg = "";

	if(isVisible) {
		infoMsg = QString("%1 - Coordinates: ").arg(overlayName);
		const auto& anchorPoints = overlay->getAnchorPoints();
		for (const AnchorPoint* anchor : anchorPoints) {
			//transform the position of each anchor point to relative camera image coordinates
			QPointF relativePos = this->videoWidget->mapFromScene(anchor->scenePos());
			infoMsg += QString("\t (%1, %2)\t").arg(relativePos.x()).arg(relativePos.y());
		}
	} else {
		infoMsg = QString("%1 is now hidden").arg(overlayName);
	}
	emit info(infoMsg);
	emit overlayStateChanged();
}
