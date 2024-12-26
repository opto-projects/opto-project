#include "lineoverlay.h"
#include <QPainter>


LineOverlay::LineOverlay(QGraphicsItem *parent) :
	OverlayItem(parent),
	penWidth{3}
{
	startAnchor = new AnchorPoint(this);
	endAnchor = new AnchorPoint(this);
	// Initialize the anchors at some default positions
	startAnchor->setPos(50, 50); // Example positions
	endAnchor->setPos(100, 100);
	addAnchorPoint(startAnchor);
	addAnchorPoint(endAnchor);
}

QRectF LineOverlay::boundingRect() const {
	const qreal extra = this->penWidth/2.0 + 0.5;

	QPointF startPoint = startAnchor->pos();
	QPointF endPoint = endAnchor->pos();

	qreal minX = qMin(startPoint.x(), endPoint.x());
	qreal minY = qMin(startPoint.y(), endPoint.y());
	qreal maxX = qMax(startPoint.x(), endPoint.x());
	qreal maxY = qMax(startPoint.y(), endPoint.y());

	QRectF rect(minX - extra, minY - extra, maxX - minX + this->penWidth + 1.0, maxY - minY + this->penWidth + 1.0);
	return rect.normalized();
}


void LineOverlay::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	QPointF startPoint = startAnchor->scenePos();
	QPointF endPoint = endAnchor->scenePos();

	painter->setRenderHint(QPainter::Antialiasing, true);
	QColor lineColor(255, 0, 0, 128);
	QPen pen(lineColor, this->penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	painter->setPen(pen);

	painter->drawLine(startAnchor->pos(), endAnchor->pos());
}

void LineOverlay::adjustAnchors() {
	update(boundingRect());}
