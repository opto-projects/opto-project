#ifndef LINEOVERLAY_H
#define LINEOVERLAY_H

#include "overlayitem.h"
#include "anchorpoint.h"

class LineOverlay : public OverlayItem {
public:
	LineOverlay(QGraphicsItem *parent = nullptr);

	//override QGraphicsItem methods
	QRectF boundingRect() const override;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

	//additional functionality for line overlay
	void adjustAnchors();

private:
	qreal penWidth;

protected:
	AnchorPoint *startAnchor;
	AnchorPoint *endAnchor;
};

#endif //LINEOVERLAY_H
