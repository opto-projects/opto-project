#ifndef TEXTOVERLAY_H
#define TEXTOVERLAY_H

#include "overlayitem.h"
#include "anchorpoint.h"
#include <QGraphicsItem>
#include <QPainter>

class TextOverlay : public OverlayItem {
public:
	explicit TextOverlay(QGraphicsItem *parent = nullptr);

	QRectF boundingRect() const override;
	void setText(QString context);
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
	ItemType getType() const override {
		return OverlayItem
			::ItemType::Text;
	};
private:
	QString _text;
	AnchorPoint *topLeftAnchor;
};

#endif //TEXTOVERLAY_H
