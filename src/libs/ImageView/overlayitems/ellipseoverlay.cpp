#include "ellipseoverlay.h"
#include <QtMath>


EllipseOverlay::EllipseOverlay(QGraphicsItem *parent)
	: OverlayItem(parent),
	centerAnchor(new AnchorPoint(this)),
	peripheralAnchor(new AnchorPoint(this)),
	radius_x(0),
	radius_y(0),
	rotation(0)
{
	this->centerAnchor->setPos(75, 75);
	this->peripheralAnchor->setPos(125, 125);

	this->addAnchorPoint(centerAnchor);
	this->addAnchorPoint(peripheralAnchor);
	this->centerAnchor->hide();

}

QRectF EllipseOverlay::boundingRect() const {
	
	qreal radius_x = abs((centerAnchor->pos().x()- peripheralAnchor->pos().x())) ;
	qreal radius_y = abs((centerAnchor->pos().y() - peripheralAnchor->pos().y()));
	const qreal extra = penWidth / 2.0 + 0.5;
	return QRectF(centerAnchor->pos().x() - radius_x - extra, centerAnchor->pos().y() - radius_y - extra,
				  2 * radius_x + penWidth + 1.0, 2 * radius_y + penWidth + 1.0).normalized();
}

void EllipseOverlay::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	QPen pen(getPenColor(), penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	painter->setPen(pen);

	// 移动到椭圆中心 旋转
	painter->translate(centerAnchor->pos());
	painter->rotate(rotation);
	// 绘制椭圆
	painter->drawEllipse(QPointF(0, 0), radius_x, radius_y);
	// x轴（长轴）
	painter->drawLine(QPointF(-radius_x, 0), QPointF(radius_x, 0));
	// y轴（短轴）
	painter->drawLine(QPointF(0, -radius_y), QPointF(0, radius_y));

	// 绘制轴端点标记
	QPen markerPen(Qt::darkGreen);
	markerPen.setWidthF(3.0);
	painter->setPen(markerPen);

	painter->drawPoint(QPointF(-radius_x, 0)); // x轴负端点
	painter->drawPoint(QPointF(radius_x, 0));  // x轴正端点
	painter->drawPoint(QPointF(0, -radius_y)); // y轴负端点
	painter->drawPoint(QPointF(0, radius_y));  // y轴正端点
	painter->drawPoint(QPointF(0, 0));  // 绘制中心点

	painter->restore();

}

void EllipseOverlay::setValue(qreal center_x, qreal center_y, qreal radius_x, qreal radius_y, qreal phi)
{
	this->radius_x = radius_x;
	this->radius_y = radius_y;
	qreal max_radius = qMax(radius_x, radius_y);

	this->centerAnchor->setPos(center_x, center_y);
	qreal peripheral_x = center_x - max_radius;
	qreal peripheral_y = center_y - max_radius;
	this->peripheralAnchor->setPos(peripheral_x, peripheral_y);
	this->rotation = -phi;

	this->update();
}
