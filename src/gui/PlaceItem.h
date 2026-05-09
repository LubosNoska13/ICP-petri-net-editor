/**
 * @file PlaceItem.h
 * @author Lubos and project team
 * @brief Graphics item for one Petri net place.
 * @details Manual code. Borrowed code: none.
 */

#ifndef PLACEITEM_H
#define PLACEITEM_H

#include "core_api/NetTypes.h"

#include <QObject>
#include <QGraphicsEllipseItem>

class QGraphicsTextItem;

/** Circular graphics item representing a place. */
class PlaceItem : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT

public:
    PlaceItem(const PlaceData &place, QGraphicsItem *parent = 0);

    QString id() const;
    void setPlaceData(const PlaceData &place);

signals:
    void positionChanged(const QString &id, const QPointF &position);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    void refreshText();

    PlaceData m_place;
    QGraphicsTextItem *m_nameText;
    QGraphicsTextItem *m_tokenText;
};

#endif
