#ifndef CIRCLEOVERLAY_H
#define CIRCLEOVERLAY_H

#include "overlayitem.h"
#include "anchorpoint.h"
#include <QGraphicsItem>
#include <QPainter>

class CircleOverlay : public OverlayItem {
public:
	explicit CircleOverlay(QGraphicsItem *parent = nullptr);

	QRectF boundingRect() const override;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
	ItemType getType() const override {
		return OverlayItem
			::ItemType::Circle;
	};
private:
	AnchorPoint *centerAnchor;
	AnchorPoint *peripheralAnchor;
};

#endif //CIRCLEOVERLAY_H
