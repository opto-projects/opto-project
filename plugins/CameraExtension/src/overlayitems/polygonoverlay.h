#ifndef POLYGONOVERLAY_H
#define POLYGONOVERLAY_H

#include "overlayitem.h"
#include "anchorpoint.h"
#include <QGraphicsItem>
#include <QPainter>

class PolygonOverlay : public OverlayItem {
public:
	explicit PolygonOverlay(QGraphicsItem *parent = nullptr);

	QRectF boundingRect() const override;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
	AnchorPoint *firstCorner;
	AnchorPoint *secondCorner;
	AnchorPoint *thirdCorner;
	AnchorPoint *fourthCorner;
	qreal penWidth;
};

#endif //POLYGONOVERLAY_H
