#include "rectoverlay.h"


RectOverlay::RectOverlay(QGraphicsItem *parent)
	: OverlayItem(parent),
	penWidth(3),
	topLeftAnchor(new AnchorPoint(this)),
	bottomRightAnchor(new AnchorPoint(this))
{
	this->topLeftAnchor->setPos(50, 50);
	this->bottomRightAnchor->setPos(100, 100);

	addAnchorPoint(this->topLeftAnchor);
	addAnchorPoint(this->bottomRightAnchor);
}

QRectF RectOverlay::boundingRect() const {
	const qreal extra = this->penWidth / 2.0 + 0.5;

	QPointF topLeft = this->topLeftAnchor->pos();
	QPointF bottomRight = this->bottomRightAnchor->pos();

	qreal minX = qMin(topLeft.x(), bottomRight.x());
	qreal minY = qMin(topLeft.y(), bottomRight.y());
	qreal maxX = qMax(topLeft.x(), bottomRight.x());
	qreal maxY = qMax(topLeft.y(), bottomRight.y());

	QRectF rect(minX - extra, minY - extra, maxX - minX + this->penWidth + 1.0, maxY - minY + this->penWidth + 1.0);
	return rect.normalized();
}

void RectOverlay::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	//set painting properties
	painter->setRenderHint(QPainter::Antialiasing, true);
	QColor rectColor(255, 0, 0, 128);
	QPen pen(rectColor, this->penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	painter->setPen(pen);

	//sraw the rectangle based on the anchor positions
	QRectF rect = QRectF(this->topLeftAnchor->pos(), this->bottomRightAnchor->pos()).normalized();
	painter->drawRect(rect);
}
