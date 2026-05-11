/**
 * @file DiagramScene.cpp
 * @author Lubos and project team
 * @brief Implementation of the Petri net diagram scene.
 * @details Manual code. Borrowed code: none.
 */

#include "gui/DiagramScene.h"

#include <QGraphicsSceneMouseEvent>
#include <QSet>

DiagramScene::DiagramScene(QObject *parent)
    : QGraphicsScene(parent), m_document(0), m_mode(SelectMode)
{
    connect(this, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
}

void DiagramScene::setDocument(PetriNetDocument *document)
{
    m_document = document;
    reloadFromDocument();
}

void DiagramScene::setMode(DiagramMode mode)
{
    m_mode = mode;
    m_pendingArcSource.clear();
}

DiagramMode DiagramScene::mode() const
{
    return m_mode;
}

QString DiagramScene::selectedElementId() const
{
    QList<QGraphicsItem *> selected = selectedItems();
    if (selected.isEmpty()) {
        return QString();
    }
    return selected.first()->data(0).toString();
}

ElementKind DiagramScene::selectedElementKind() const
{
    QList<QGraphicsItem *> selected = selectedItems();
    if (selected.isEmpty()) {
        return UnknownElement;
    }
    return (ElementKind)selected.first()->data(1).toInt();
}

void DiagramScene::reloadFromDocument()
{
    clear();
    m_elementItems.clear();
    m_arcItems.clear();
    if (!m_document) {
        return;
    }

    QList<PlaceData> places = m_document->places();
    for (int i = 0; i < places.size(); ++i) {
        PlaceItem *item = new PlaceItem(places.at(i));
        addItem(item);
        connect(item, SIGNAL(positionChanged(QString,QPointF)),
                this, SLOT(onItemMoved(QString,QPointF)));
        m_elementItems.insert(places.at(i).id, item);
    }

    QList<TransitionData> transitions = m_document->transitions();
    for (int i = 0; i < transitions.size(); ++i) {
        TransitionItem *item = new TransitionItem(transitions.at(i));
        addItem(item);
        connect(item, SIGNAL(positionChanged(QString,QPointF)),
                this, SLOT(onItemMoved(QString,QPointF)));
        m_elementItems.insert(transitions.at(i).id, item);
    }

    QList<ArcData> arcs = m_document->arcs();
    for (int i = 0; i < arcs.size(); ++i) {
        ArcItem *item = new ArcItem(arcs.at(i));
        addItem(item);
        m_arcItems.insert(arcs.at(i).id, item);
    }
    refreshArcs();
}

void DiagramScene::updateRuntimeMarking(const RuntimeSnapshot &snapshot)
{
    if (!m_document) {
        return;
    }

    for (int i = 0; i < snapshot.marking.size(); ++i) {
        MarkingEntry entry = snapshot.marking.at(i);
        if (entry.placeId.isEmpty() || !m_elementItems.contains(entry.placeId)) {
            continue;
        }
        PlaceData place;
        if (m_document->placeById(entry.placeId, &place)) {
            place.tokens = entry.tokens;
            PlaceItem *placeItem = dynamic_cast<PlaceItem *>(m_elementItems.value(entry.placeId));
            if (placeItem) {
                placeItem->setPlaceData(place);
            }
        }
    }

    QSet<QString> enabledNames;
    for (int i = 0; i < snapshot.enabledTransitions.size(); ++i) {
        enabledNames.insert(snapshot.enabledTransitions.at(i));
    }

    QList<TransitionData> transitions = m_document->transitions();
    for (int i = 0; i < transitions.size(); ++i) {
        const TransitionData &transition = transitions.at(i);
        if (!m_elementItems.contains(transition.id)) {
            continue;
        }
        TransitionItem *transitionItem = dynamic_cast<TransitionItem *>(m_elementItems.value(transition.id));
        if (transitionItem) {
            transitionItem->setEnabledHighlight(enabledNames.contains(transition.name));
        }
    }
}

void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_document) {
        QGraphicsScene::mousePressEvent(event);
        return;
    }

    if (m_mode == AddPlaceMode) {
        m_document->addPlace(event->scenePos().x(), event->scenePos().y());
        reloadFromDocument();
        emit documentChanged();
        return;
    }

    if (m_mode == AddTransitionMode) {
        m_document->addTransition(event->scenePos().x(), event->scenePos().y());
        reloadFromDocument();
        emit documentChanged();
        return;
    }

    if (m_mode == AddArcMode) {
        QGraphicsItem *item = itemAt(event->scenePos(), QTransform());
        while (item && item->data(0).toString().isEmpty()) {
            item = item->parentItem();
        }
        if (!item) {
            m_pendingArcSource.clear();
            emit statusMessage("Arc source cleared.");
            return;
        }
        QString id = item->data(0).toString();
        ElementKind kind = (ElementKind)item->data(1).toInt();
        if (kind != PlaceElement && kind != TransitionElement) {
            return;
        }
        if (m_pendingArcSource.isEmpty()) {
            m_pendingArcSource = id;
            emit statusMessage("Arc source selected.");
        } else {
            ArcData arc = m_document->addArc(m_pendingArcSource, id);
            if (arc.id.isEmpty()) {
                emit statusMessage("Invalid arc direction.");
            } else {
                reloadFromDocument();
                emit documentChanged();
                emit statusMessage("Arc created.");
            }
            m_pendingArcSource.clear();
        }
        return;
    }

    QGraphicsScene::mousePressEvent(event);
}

void DiagramScene::onSelectionChanged()
{
    emit elementSelected(selectedElementId(), selectedElementKind());
}

void DiagramScene::onItemMoved(const QString &id, const QPointF &position)
{
    if (!m_document) {
        return;
    }
    PlaceData place;
    if (m_document->placeById(id, &place)) {
        place.x = position.x();
        place.y = position.y();
        m_document->updatePlace(place);
    }
    TransitionData transition;
    if (m_document->transitionById(id, &transition)) {
        transition.x = position.x();
        transition.y = position.y();
        m_document->updateTransition(transition);
    }
    refreshArcs();
    emit documentChanged();
}

QPointF DiagramScene::centerOfElement(const QString &id) const
{
    if (m_elementItems.contains(id)) {
        return m_elementItems.value(id)->scenePos();
    }
    return QPointF();
}

void DiagramScene::refreshArcs()
{
    if (!m_document) {
        return;
    }
    QList<ArcData> arcs = m_document->arcs();
    for (int i = 0; i < arcs.size(); ++i) {
        ArcData arc = arcs.at(i);
        if (m_arcItems.contains(arc.id)) {
            m_arcItems.value(arc.id)->setArcData(arc);
            m_arcItems.value(arc.id)->setEndpoints(centerOfElement(arc.sourceId),
                                                   centerOfElement(arc.targetId));
        }
    }
}
