/**
 * @file ArcItem.cpp
 * @author Lubos and project team
 * @brief Implementation of the directed arc graphics item.
 * @details Manual code. Borrowed code: none.
 */

#include "gui/ArcItem.h"

#include <QBrush>
#include <QFont>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QLineF>
#include <QPainter>
#include <QPainterPathStroker>
#include <QPen>
#include <QPolygonF>
#include <QTextDocument>
#include <QTextOption>
#include <QtMath>

ArcItem::ArcItem(const ArcData &arc, QGraphicsItem *parent)
    : QGraphicsLineItem(parent), m_arc(arc)
{
    setPen(QPen(QColor(38, 76, 112), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    setFlags(QGraphicsItem::ItemIsSelectable);
    setZValue(1);
    setData(0, arc.id);
    setData(1, ArcElement);
    m_weightBackground = new QGraphicsRectItem(this);
    m_weightBackground->setBrush(QBrush(QColor(255, 255, 245)));
    m_weightBackground->setPen(QPen(QColor(38, 76, 112), 1));
    m_weightBackground->setData(0, arc.id);
    m_weightBackground->setData(1, ArcElement);
    m_weightText = new QGraphicsTextItem(this);
    m_weightText->setDefaultTextColor(QColor(20, 35, 50));
    m_weightText->setFont(QFont("Sans Serif", 9, QFont::Bold));
    m_weightText->setZValue(1);
    refreshText();
}

QString ArcItem::id() const
{
    return m_arc.id;
}

void ArcItem::setArcData(const ArcData &arc)
{
    m_arc = arc;
    refreshText();
}

void ArcItem::setEndpoints(const QPointF &source, const QPointF &target)
{
    QLineF centerLine(source, target);
    if (centerLine.length() > 55) {
        QLineF visibleLine(source, target);
        visibleLine.setP1(visibleLine.pointAt(34.0 / visibleLine.length()));
        visibleLine.setP2(visibleLine.pointAt((visibleLine.length() - 34.0) / visibleLine.length()));
        setLine(visibleLine);
    } else {
        setLine(QLineF(source, target));
    }
    refreshText();
}

QPainterPath ArcItem::shape() const
{
    QPainterPath path;
    path.moveTo(line().p1());
    path.lineTo(line().p2());

    QPainterPathStroker stroker;
    stroker.setWidth(14);
    stroker.setCapStyle(Qt::RoundCap);
    return stroker.createStroke(path);
}

void ArcItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QLineF l = line();
    if (l.length() <= 0.1) {
        return;
    }

    QColor color = isSelected() ? QColor(220, 120, 35) : QColor(38, 76, 112);
    QPen visiblePen(color, isSelected() ? 4 : 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(visiblePen);
    painter->drawLine(l);

    const double pi = 3.14159265358979323846;
    double angle = qAtan2(-l.dy(), l.dx());
    QPointF arrowP1 = l.p2() - QPointF(qSin(angle + pi / 3) * 16,
                                        qCos(angle + pi / 3) * 16);
    QPointF arrowP2 = l.p2() - QPointF(qSin(angle + pi - pi / 3) * 16,
                                        qCos(angle + pi - pi / 3) * 16);
    QPolygonF arrowHead;
    arrowHead << l.p2() << arrowP1 << arrowP2;
    painter->setBrush(QBrush(color));
    painter->drawPolygon(arrowHead);
}

void ArcItem::refreshText()
{
    m_weightText->setPlainText(QString::number(m_arc.weight));
    m_weightText->setData(0, m_arc.id);
    m_weightText->setData(1, ArcElement);
    m_weightText->setTextWidth(28);
    m_weightText->document()->setDefaultTextOption(QTextOption(Qt::AlignCenter));
    QPointF middle = (line().p1() + line().p2()) / 2;
    QRectF box = m_weightText->boundingRect();
    m_weightText->setPos(middle.x() - box.width() / 2, middle.y() - box.height() / 2);
    QRectF backgroundRect = m_weightText->mapRectToParent(box).adjusted(-2, -2, 2, 2);
    m_weightBackground->setRect(backgroundRect);
}
