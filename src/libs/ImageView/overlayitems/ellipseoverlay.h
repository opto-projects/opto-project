#ifndef ELLIPSEOVERLAY_H
#define ELLIPSEOVERLAY_H

#include "overlayitem.h"
#include "anchorpoint.h"
#include <QGraphicsItem>
#include <QPainter>

class EllipseOverlay : public OverlayItem {
public:
	explicit EllipseOverlay(QGraphicsItem *parent = nullptr);

	QRectF boundingRect() const override;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
	ItemType getType() const override {
		return OverlayItem
			::ItemType::Ellipse;
	};
	void setValue(qreal center_x, qreal center_y, qreal radius_x, qreal radius_y, qreal phi);
private:
	AnchorPoint *centerAnchor;
	AnchorPoint *peripheralAnchor;
	qreal radius_x;      // xÖá°ë¾¶
	qreal radius_y;      // yÖá°ë¾¶
	qreal rotation;      // Ðý×ª½Ç¶È£¨¶È£©
};

#endif //ELLIPSEOVERLAY_H
