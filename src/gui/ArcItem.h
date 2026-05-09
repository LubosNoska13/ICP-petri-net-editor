/**
 * @file ArcItem.h
 * @author Lubos and project team
 * @brief Graphics item for one directed weighted arc.
 * @details Manual code. Borrowed code: none.
 */

#ifndef ARCITEM_H
#define ARCITEM_H

#include "core_api/NetTypes.h"

#include <QGraphicsLineItem>
#include <QPainterPath>

class QGraphicsRectItem;
class QGraphicsTextItem;

/** Directed line item with arrow head and weight label. */
class ArcItem : public QGraphicsLineItem
{
public:
    ArcItem(const ArcData &arc, QGraphicsItem *parent = 0);

    QString id() const;
    void setArcData(const ArcData &arc);
    void setEndpoints(const QPointF &source, const QPointF &target);

protected:
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    void refreshText();

    ArcData m_arc;
    QGraphicsRectItem *m_weightBackground;
    QGraphicsTextItem *m_weightText;
};

#endif
