#ifndef OVERLAYITEM_H
#define OVERLAYITEM_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include "anchorpoint.h"

class OverlayItem : public QObject,  public QGraphicsItem {
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
public:
	enum ItemType {
		Line = 0,
		Circle,				// 圆
		Ellipse,            // 椭圆
		Rectangle,          // 矩形
		Square,             // 正方形
		Polygon,            // 多边形
		Text
	};
	explicit OverlayItem(QGraphicsItem *parent = nullptr);
	explicit OverlayItem(QString name, QGraphicsItem* parent = nullptr);

	virtual QRectF boundingRect() const override = 0;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override = 0;

	QVariantMap saveState() const;
	void loadState(const QVariantMap& state);

	void addAnchorPoint(AnchorPoint *anchor);
	QList<AnchorPoint *> getAnchorPoints() const { return this->anchorPoints; }

	QString getName() const { return this->name; }
	void setName(const QString& name);
	void setNameVisible(bool visible);
	void setPenWidth(qreal value);
	qreal getPenWidth()const;
	void setPenColor(QColor color);
	QColor getPenColor()const;

	virtual ItemType getType() const  = 0;

	void onAnchorPointPositionChanged();

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
	QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
	void focusInEvent(QFocusEvent* event) override;
	void focusOutEvent(QFocusEvent* event) override;
private:
	QString name;
	QPointF originalPosition;
	QList<AnchorPoint *> anchorPoints;
	QGraphicsTextItem* itemText;
	void initItem();
	bool isClickOnAnchorPoint(const QPointF& clickPos) const;
protected:
	QColor selectedColor;
	QColor noSelectedColor;
	bool isSelected;
	qreal penWidth;
signals:
	void positionChanged(OverlayItem* item);
	void visibilityChanged(OverlayItem* item);
};

#endif //OVERLAYITEM_H
