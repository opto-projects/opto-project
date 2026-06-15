#include "textoverlay.h"


TextOverlay::TextOverlay(QGraphicsItem *parent)
	: OverlayItem(parent),
	topLeftAnchor(new AnchorPoint(this))
{
	this->topLeftAnchor->setPos(10, 5);

	addAnchorPoint(this->topLeftAnchor);
}

QRectF TextOverlay::boundingRect() const {
	const qreal extra = 0;

	QPointF topLeft = this->topLeftAnchor->pos();

	qreal minX =topLeft.x();
	qreal minY = topLeft.y();
	qreal maxX =topLeft.x();
	qreal maxY = topLeft.y();

	QRectF rect(minX - extra, minY - extra, maxX - minX , maxY - minY );
	return rect;
}

void TextOverlay::setText(QString text) {
	_text = text;
	this->update();
}

void TextOverlay::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	painter->setPen(Qt::white);
	QRectF rect = QRectF(this->topLeftAnchor->pos(), this->topLeftAnchor->pos()).normalized();
	painter->drawText(rect, Qt::TextDontClip, _text);
}
