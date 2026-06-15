#include "imagedisplay.h"

#include <opencv2/opencv.hpp>

#include <QMenu>
#include <QAction>
#include <QDebug>
#include <QToolButton>

#define LOG_ID "ImageDisplay:"

ImageDisplay::ImageDisplay(QWidget *parent) : QGraphicsView(parent)
,isFirstShowEvent(true)
,oldRotationAngle(0.0)
{
	this->scene = new QGraphicsScene(this);
	scene->setItemIndexMethod(QGraphicsScene::NoIndex);
	setScene(scene);
	setCacheMode(CacheBackground);
	setViewportUpdateMode(BoundingRectViewportUpdate);
	setRenderHint(QPainter::Antialiasing);
	setTransformationAnchor(AnchorUnderMouse);

	this->imageItem = new QGraphicsPixmapItem();
	this->roiRect = new RectOverlay(imageItem);
	roiRect->hide();
	resultOverlay = new TextOverlay();
	resultOverlay->setText(QStringLiteral("welcome!"));
	this->scene->addItem(imageItem);
	this->scene->addItem(resultOverlay);
	this->scene->update();

	createActions();
	setRawView(true);

	//setup roi
	connect(this->roiRect, &RectOverlay::positionChanged, this, [this](OverlayItem* item) {
		auto topLeftAnchor = item->getAnchorPoints().at(0);
		auto bottomRightAnchor = item->getAnchorPoints().at(1);
		QRectF roiRect(topLeftAnchor->scenePos(), bottomRightAnchor->scenePos());
		QPolygonF roiInImageCoordinates = this->imageItem->mapFromItem(this->imageItem, roiRect);
		emit roiChanged(roiRect.toRect());
	});

	//adjust orientation of display to match orientation of octproz main output
	//this->rotate(90);
	//this->scale(1, -1); //flip vertical

	this->frameWidth = 0;
	this->frameHeight = 0;
	this->mousePosX = 0;
	this->mousePosY = 0;

	//setup bitconverter
	this->bitConverter = new BitDepthConverter();
	this->bitConverter->moveToThread(&converterThread);
	connect(this, &ImageDisplay::non8bitFrameReceived, this->bitConverter, &BitDepthConverter::convertDataTo8bit);
	connect(this, &ImageDisplay::actionToColorMap, this->bitConverter, &BitDepthConverter::convertDataToColormap);
	connect(this->bitConverter, &BitDepthConverter::info, this, &ImageDisplay::info);
	connect(this->bitConverter, &BitDepthConverter::error, this, &ImageDisplay::error);
	connect(this->bitConverter, &BitDepthConverter::converted8bitData, this, &ImageDisplay::processFrame);
	connect(this->bitConverter, &BitDepthConverter::convertedColormap, this, &ImageDisplay::displayFrame);
	connect(&converterThread, &QThread::finished, this->bitConverter, &BitDepthConverter::deleteLater);
	converterThread.start();

	this->createOverlays();

	connect(this, &ImageDisplay::rotationAngleChanged, this, &ImageDisplay::onRotationAngleChanged);
}

ImageDisplay::~ImageDisplay()
{
	converterThread.quit();
	converterThread.wait();
}

void ImageDisplay::createActions()
{
	QAction* a = NULL;
	QMenu* m = NULL;
	actionColorMap = a = new QAction(tr("ColorMap"),this);
	addAction(a);
	m = new QMenu(tr("ColorMap"),this);
	a->setMenu(m);

	a = m->addAction(tr("RAW"));
	a->setData(100);
	a = m->addAction(tr("Jet"));
	a->setData(cv::ColormapTypes::COLORMAP_JET);
	a = m->addAction(tr("Hot"));
	a->setData(cv::ColormapTypes::COLORMAP_HOT);
	a = m->addAction(tr("RainBow"));
	a->setData(cv::ColormapTypes::COLORMAP_RAINBOW);
	a = m->addAction(tr("Parula"));
	a->setData(cv::ColormapTypes::COLORMAP_PARULA);
	a = m->addAction(tr("Plasma"));
	a->setData(cv::ColormapTypes::COLORMAP_PLASMA);
	a = m->addAction(tr("Turbo"));
	a->setData(cv::ColormapTypes::COLORMAP_TURBO);
	
	connect(m, &QMenu::triggered, this, &ImageDisplay::toggleColorBar);
}


void ImageDisplay::createOverlays() {
	this->overlays.append(qMakePair(QString("Line"),new LineOverlay() ));
	this->overlays.append(qMakePair(QString("Rect"),new RectOverlay() ));
	this->overlays.append(qMakePair(QString("Polygon"),new PolygonOverlay() ));
	this->overlays.append(qMakePair(QString("Circle"),new CircleOverlay() ));
    this->overlays.append(qMakePair(QString("Ellipse"),new EllipseOverlay() ));
	for (auto& overlayPair : overlays) {
		OverlayItem* overlayItem = dynamic_cast<OverlayItem*>(overlayPair.second);
		if (overlayItem) {
			overlayItem->hide();
		}
	}

}

void ImageDisplay::initOverlays() {
	//for linux/ubuntu this should happen during the very first show event.
	//if the videoWidget is set as parent item in the constructor camera screen will remain black
	for (auto& overlayPair : overlays) {
		OverlayItem* overlayItem = dynamic_cast<OverlayItem*>(overlayPair.second);
		if (overlayItem) {
			overlayItem->setParentItem(this->imageItem);
			overlayItem->setName(overlayPair.first);
			connect(overlayItem, &OverlayItem::positionChanged, this, &ImageDisplay::onOverlayChanged);
			connect(overlayItem, &OverlayItem::visibilityChanged, this, &ImageDisplay::onOverlayChanged);
		}
	}
}

void ImageDisplay::showEvent(QShowEvent* event) {
	QGraphicsView::showEvent(event);
	if (this->isFirstShowEvent) {
		this->isFirstShowEvent = false;
		this->initOverlays();
	}
}

void ImageDisplay::mouseDoubleClickEvent(QMouseEvent *event) {
	this->fitInView(this->scene->sceneRect(), Qt::KeepAspectRatio);
	this->ensureVisible(this->imageItem);
	this->centerOn(this->pos());
	this->scene->setSceneRect(this->scene->itemsBoundingRect());
	QGraphicsView::mousePressEvent(event);
}

void ImageDisplay::mousePressEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		this->mousePosX = event->x();
		this->mousePosY = event->y();
	}
	QGraphicsView::mousePressEvent(event);
}

void ImageDisplay::mouseMoveEvent(QMouseEvent* event) {
	if ((event->buttons() & Qt::LeftButton) && (event->modifiers() & Qt::ControlModifier)) {
		QPointF oldPosition = mapToScene(this->mousePosX, this->mousePosY);
		QPointF newPosition = mapToScene(event->pos());
		QPointF translation = newPosition - oldPosition;
		this->translate(translation.x(), translation.y());
		this->mousePosX = event->x();
		this->mousePosY = event->y();
	}
	QGraphicsView::mouseMoveEvent(event);
}

void ImageDisplay::keyPressEvent(QKeyEvent* event) {
	switch (event->key()) {
	case Qt::Key_Plus:
		this->zoomIn();
		break;
	case Qt::Key_Minus:
		this->zoomOut();
		break;
	default:
		QGraphicsView::keyPressEvent(event);
	}
}

void ImageDisplay::wheelEvent(QWheelEvent* event) {
	//zoom with mouse wheel, only if CTRL key is pressed
	if (event->modifiers() & Qt::ControlModifier) {
		double angle = event->angleDelta().y();
		double factor = qPow(1.0015, angle);
		QPoint targetViewportPos = event->pos();
		QPointF targetScenePos = mapToScene(event->pos());
		this->scale(factor, factor);
		this->centerOn(targetScenePos);
		QPointF deltaViewportPos = targetViewportPos - QPointF(viewport()->width() / 2.0, viewport()->height() / 2.0);
		QPointF viewportCenter = mapFromScene(targetScenePos) - deltaViewportPos;
		this->centerOn(mapToScene(viewportCenter.toPoint()));
	}
	if (event->modifiers() & Qt::ShiftModifier) {
		QPoint numPixels = event->pixelDelta();
		QPoint numDegrees = event->angleDelta();
		bool dir = numDegrees.y() > 0;
		rotate90Angle(dir);
		//rotateRelative(numPixels, numDegrees);
		event->accept();
	}
	QGraphicsView::wheelEvent(event);
}


void ImageDisplay::contextMenuEvent(QContextMenuEvent* event) {
	QMenu menu(this);

	//overlay actions
	for (auto& overlay : overlays) {
		QAction* action = menu.addAction(overlay.first);
		action->setCheckable(true);
		action->setChecked(overlay.second->isVisible());

		connect(action, &QAction::triggered, this, [this, overlay]() {
			overlay.second->setVisible(!overlay.second->isVisible());
			this->scene->update();
			});
	}

	//snapshot actions
	menu.addSeparator();
	QAction* takeSnapshotAction = menu.addAction("Take snapshot");
	//connect(takeSnapshotAction, &QAction::triggered, this, &CameraViewWidget::takeSnapshot);
	QAction* setSnapshotLocationAction = menu.addAction("Set snapshot save location...");
	//connect(setSnapshotLocationAction, &QAction::triggered, this, &CameraViewWidget::openSetSaveLocationDialog);

	menu.exec(event->globalPos());
}

void ImageDisplay::scaleView(qreal scaleFactor) {
	qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
	if (factor < 0.07 || factor > 100){
		return;
	}
	this->scale(scaleFactor, scaleFactor);
}

void ImageDisplay::rotateRelative(QPoint pixels, QPoint degrees)
{
	QPoint numPixels = pixels;
	QPoint numDegrees = degrees/8;
	qreal angle = this->oldRotationAngle;
	if (!numPixels.isNull()) {
		angle += numPixels.y() / 30.0;
	}
	else if (!numDegrees.isNull()) {
		QPoint numSteps = numDegrees / 15;
		angle += (float)numSteps.y() / 30.0;
	}
	this->rotateAbsolute(angle);
}

void ImageDisplay::rotate90Angle(bool dir)
{
    qreal angle = dir ? 90.0 : -90.0;
	this->rotate(angle);
	this->oldRotationAngle = this->oldRotationAngle - angle;
	emit rotationAngleChanged(this->oldRotationAngle);
}

void ImageDisplay::zoomIn() {
	this->scaleView(qreal(1.2));
}

void ImageDisplay::zoomOut() {
	this->scaleView(1/qreal(1.2));
}

void ImageDisplay::receiveFrame(void* frame, unsigned int bitDepth, unsigned int width, unsigned int height, unsigned int channel) {
	if (!this->isVisible()) {
		return;
	}
	if (bitDepth == 8)
	{
		this->processFrame(static_cast<uchar*>(frame), bitDepth, width, height, channel);
	}
	else
	{
		// convert to 8bit
		emit non8bitFrameReceived(frame, bitDepth, width, height, channel);
	}
}

void ImageDisplay::processFrame(uchar* frame, unsigned int bitDepth, unsigned int width, unsigned int height, unsigned int channel)
{
	if (isRawView)
	{
		this->displayFrame(static_cast<uchar*>(frame), width, height, channel);
	}
	else
	{
		emit actionToColorMap(m_colorMapName, frame, bitDepth, width, height, channel);
	}
}

void ImageDisplay::displayFrame(uchar* frame, unsigned int width, unsigned int height, unsigned int channel) {
	//create QPixmap from uchar array and update imageItem
	QElapsedTimer timer;
	qint64 time = timer.elapsed();
	qDebug() << LOG_ID<< "Display Thread ID start : " << QThread::currentThreadId();
	if (channel == 1)
	{
		QImage image(frame, width, height, width, QImage::Format_Grayscale8 );
		this->imageItem->setPixmap(QPixmap::fromImage(image));
	}
	else if (channel == 3)
	{
		QImage image(frame, width, height, QImage::Format_RGB888);
		this->imageItem->setPixmap(QPixmap::fromImage(image));
	}
	else
	{
		QImage image(frame, width, height, QImage::Format_ARGB32);
		this->imageItem->setPixmap(QPixmap::fromImage(image));
	}
	qint64 time1 = timer.elapsed();
	qDebug() << LOG_ID << "Display time: " << time1 - time;
	//scale view if input sizes have changed
	if(this->frameWidth != width || this->frameHeight != height){
		this->frameWidth = width;
		this->frameHeight = height;
		this->fitInView(this->scene->sceneRect(), Qt::KeepAspectRatio);
		this->ensureVisible(this->imageItem);
		this->centerOn(this->pos());

		//set scene rect back to minimal size
		this->scene->setSceneRect(this->scene->itemsBoundingRect());
	}
	qint64 time2 = timer.elapsed();
	qDebug() << LOG_ID << "Display time: " << time2 - time1;
}

void ImageDisplay::setRoi(QRect roi) {
	this->roiRect->setRect(roi);
}

void ImageDisplay::onRotationAngleChanged(qreal angle)
{
	QString infoMsg = "";
	infoMsg = QString("Image view rotation angle is %1").arg(angle);
	emit info(infoMsg);
}

void ImageDisplay::onOverlayChanged(OverlayItem* overlay) {
	QString overlayName = overlay->getName();
	bool isVisible = overlay->isVisible();
	QString infoMsg = "";

	if (isVisible) {
		infoMsg = QString("%1 - Coordinates: ").arg(overlayName);
		const auto& anchorPoints = overlay->getAnchorPoints();
		for (const AnchorPoint* anchor : anchorPoints) {
			//transform the position of each anchor point to relative camera image coordinates
			QPointF relativePos = this->imageItem->mapFromScene(anchor->scenePos());
			infoMsg += QString("\t (%1, %2)\t").arg(relativePos.x()).arg(relativePos.y());
		}
	}
	else {
		infoMsg = QString("%1 is now hidden").arg(overlayName);
	}
	emit info(infoMsg);
	emit overlayStateChanged();
}

void ImageDisplay::rotateAbsolute(qreal angle) {
	this->rotate(this->oldRotationAngle - angle);
	this->oldRotationAngle = angle;
	emit rotationAngleChanged(this->oldRotationAngle);
}

bool ImageDisplay::setColorMap(const QString& colormap)
{
	m_colorMapName = colormap;
	return true;
}

void ImageDisplay::toggleColorBar(QAction* action)
{
	QString idstr = action->data().toString();
	int idx = idstr.toInt();
	if (idx>22)
	{
		isRawView = true;
	}
	else
	{
		isRawView = false;
		setColorMap(idstr);
	}
	
}

bool ImageDisplay::setRawView(bool on)
{
	isRawView = on;
	if (on)
	{
		setColorMap("100");
	}
	else
	{
		setColorMap("1");
	}
	
	return true;
}
