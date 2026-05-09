/**
 * @file PlaceItem.cpp
 * @author Lubos and project team
 * @brief Implementation of the place graphics item.
 * @details Manual code. Borrowed code: none.
 */

#include "gui/PlaceItem.h"

#include <QBrush>
#include <QGraphicsTextItem>
#include <QPen>

PlaceItem::PlaceItem(const PlaceData &place, QGraphicsItem *parent)
    : QObject(), QGraphicsEllipseItem(parent), m_place(place)
{
    setRect(-32, -32, 64, 64);
    setPos(place.x, place.y);
    setBrush(QBrush(QColor(250, 250, 250)));
    setPen(QPen(QColor(40, 40, 40), 2));
    setFlags(QGraphicsItem::ItemIsMovable
             | QGraphicsItem::ItemIsSelectable
             | QGraphicsItem::ItemSendsGeometryChanges);
    setData(0, place.id);
    setData(1, PlaceElement);

    m_nameText = new QGraphicsTextItem(this);
    m_tokenText = new QGraphicsTextItem(this);
    refreshText();
}

QString PlaceItem::id() const
{
    return m_place.id;
}

void PlaceItem::setPlaceData(const PlaceData &place)
{
    m_place = place;
    setPos(place.x, place.y);
    refreshText();
}

QVariant PlaceItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionHasChanged) {
        emit positionChanged(m_place.id, pos());
    }
    return QGraphicsEllipseItem::itemChange(change, value);
}

void PlaceItem::refreshText()
{
    m_nameText->setPlainText(m_place.name);
    m_nameText->setDefaultTextColor(QColor(30, 30, 30));
    m_nameText->setPos(-m_nameText->boundingRect().width() / 2, 34);

    m_tokenText->setPlainText(QString::number(m_place.tokens));
    m_tokenText->setDefaultTextColor(QColor(20, 20, 20));
    m_tokenText->setPos(-m_tokenText->boundingRect().width() / 2,
                        -m_tokenText->boundingRect().height() / 2);
}
