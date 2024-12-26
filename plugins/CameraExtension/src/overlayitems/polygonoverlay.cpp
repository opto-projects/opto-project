#include "polygonoverlay.h"


PolygonOverlay::PolygonOverlay(QGraphicsItem *parent)
	: OverlayItem(parent),
	penWidth(3) ,
	firstCorner(new AnchorPoint(this)),
	secondCorner(new AnchorPoint(this)),
	thirdCorner(new AnchorPoint(this)),
	fourthCorner(new AnchorPoint(this))
{
	this->firstCorner->setPos(50, 50);
	this->secondCorner->setPos(100, 50);
	this->thirdCorner->setPos(50, 100);
	this->fourthCorner->setPos(100, 100);

	this->addAnchorPoint(firstCorner);
	this->addAnchorPoint(secondCorner);
	this->addAnchorPoint(thirdCorner);
	this->addAnchorPoint(fourthCorner);
}

QRectF PolygonOverlay::boundingRect() const {
	//calculate bounding rect by considering all corners
	qreal minX = std::min({firstCorner->pos().x(), secondCorner->pos().x(), thirdCorner->pos().x(), fourthCorner->pos().x()});
	qreal minY = std::min({firstCorner->pos().y(), secondCorner->pos().y(), thirdCorner->pos().y(), fourthCorner->pos().y()});
	qreal maxX = std::max({firstCorner->pos().x(), secondCorner->pos().x(), thirdCorner->pos().x(), fourthCorner->pos().x()});
	qreal maxY = std::max({firstCorner->pos().y(), secondCorner->pos().y(), thirdCorner->pos().y(), fourthCorner->pos().y()});

	const qreal extra = this->penWidth / 2.0 + 0.5;
	return QRectF(minX - extra, minY - extra, maxX - minX + this->penWidth + 1.0, maxY - minY + this->penWidth + 1.0).normalized();
}

void PolygonOverlay::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	painter->setRenderHint(QPainter::Antialiasing, true);
	QColor polygonColor(255, 0, 0, 128);
	QPen pen(polygonColor, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	painter->setPen(pen);

	//draw the polygon based on the anchor positions
	QPolygonF polygon;
	polygon << firstCorner->pos() << secondCorner->pos() << fourthCorner->pos() << thirdCorner->pos();
	painter->drawPolygon(polygon);
}
