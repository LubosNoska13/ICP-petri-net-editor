/**
 * @file TransitionItem.h
 * @author Lubos and project team
 * @brief Graphics item for one Petri net transition.
 * @details Manual code. Borrowed code: none.
 */

#ifndef TRANSITIONITEM_H
#define TRANSITIONITEM_H

#include "core_api/NetTypes.h"

#include <QObject>
#include <QGraphicsRectItem>

class QGraphicsTextItem;

/** Rectangular graphics item representing a transition. */
class TransitionItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

public:
    TransitionItem(const TransitionData &transition, QGraphicsItem *parent = 0);

    QString id() const;
    void setTransitionData(const TransitionData &transition);
    void setEnabledHighlight(bool enabled);

signals:
    void positionChanged(const QString &id, const QPointF &position);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    void refreshText();

    TransitionData m_transition;
    QGraphicsTextItem *m_nameText;
    QGraphicsTextItem *m_conditionText;
};

#endif
