/**
 * @file TransitionItem.cpp
 * @author Lubos and project team
 * @brief Implementation of the transition graphics item.
 * @details Manual code. Borrowed code: none.
 */

#include "gui/TransitionItem.h"

#include <QBrush>
#include <QGraphicsTextItem>
#include <QPen>

TransitionItem::TransitionItem(const TransitionData &transition, QGraphicsItem *parent)
    : QObject(), QGraphicsRectItem(parent), m_transition(transition)
{
    setRect(-18, -36, 36, 72);
    setPos(transition.x, transition.y);
    setBrush(QBrush(QColor(240, 246, 255)));
    setPen(QPen(QColor(30, 60, 90), 2));
    setFlags(QGraphicsItem::ItemIsMovable
             | QGraphicsItem::ItemIsSelectable
             | QGraphicsItem::ItemSendsGeometryChanges);
    setData(0, transition.id);
    setData(1, TransitionElement);

    m_nameText = new QGraphicsTextItem(this);
    m_conditionText = new QGraphicsTextItem(this);
    refreshText();
}

QString TransitionItem::id() const
{
    return m_transition.id;
}

void TransitionItem::setTransitionData(const TransitionData &transition)
{
    m_transition = transition;
    setPos(transition.x, transition.y);
    refreshText();
}

void TransitionItem::setEnabledHighlight(bool enabled)
{
    if (enabled) {
        setBrush(QBrush(QColor(205, 242, 214)));
    } else {
        setBrush(QBrush(QColor(240, 246, 255)));
    }
}

QVariant TransitionItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionHasChanged) {
        emit positionChanged(m_transition.id, pos());
    }
    return QGraphicsRectItem::itemChange(change, value);
}

void TransitionItem::refreshText()
{
    m_nameText->setPlainText(m_transition.name);
    m_nameText->setDefaultTextColor(QColor(30, 30, 30));
    m_nameText->setPos(-m_nameText->boundingRect().width() / 2, 38);

    QString condition;
    if (!m_transition.event.isEmpty()) {
        condition += m_transition.event;
    }
    if (m_transition.delayMs > 0) {
        if (!condition.isEmpty()) {
            condition += " ";
        }
        condition += "@" + QString::number(m_transition.delayMs);
    }
    m_conditionText->setPlainText(condition);
    m_conditionText->setDefaultTextColor(QColor(70, 70, 70));
    m_conditionText->setScale(0.8);
    m_conditionText->setPos(-m_conditionText->boundingRect().width() * 0.4, -58);
}
