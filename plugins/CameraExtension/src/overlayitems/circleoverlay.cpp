#include "circleoverlay.h"
#include <QtMath>


CircleOverlay::CircleOverlay(QGraphicsItem *parent)
	: OverlayItem(parent),
	penWidth(3),
	centerAnchor(new AnchorPoint(this)),
	peripheralAnchor(new AnchorPoint(this))
{
	this->centerAnchor->setPos(75, 75);
	this->peripheralAnchor->setPos(125, 75);

	this->addAnchorPoint(centerAnchor);
	this->addAnchorPoint(peripheralAnchor);
	this->centerAnchor->hide();
}

QRectF CircleOverlay::boundingRect() const {
	qreal radius = QLineF(centerAnchor->pos(), peripheralAnchor->pos()).length();
	const qreal extra = penWidth / 2.0 + 0.5;
	return QRectF(centerAnchor->pos().x() - radius - extra, centerAnchor->pos().y() - radius - extra,
				  2 * radius + penWidth + 1.0, 2 * radius + penWidth + 1.0).normalized();
}

void CircleOverlay::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	painter->setRenderHint(QPainter::Antialiasing, true);
	QColor fillColor(255, 0, 0, 128);
	painter->setBrush(QBrush(fillColor));
	QColor outlineColor(255, 0, 0, 255);
	QPen pen(outlineColor, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	painter->setPen(pen);

	qreal radius = QLineF(centerAnchor->pos(), peripheralAnchor->pos()).length();
	painter->drawEllipse(centerAnchor->pos(), radius, radius);
}
