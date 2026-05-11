/**
 * @file PetriNetDocument.cpp
 * @author Lubos and project team
 * @brief Implementation of the GUI document model.
 * @details Manual code. Borrowed code: none.
 */

#include "core_api/PetriNetDocument.h"

#include <QSet>
#include <QtGlobal>

PetriNetDocument::PetriNetDocument()
{
    clear();
}

void PetriNetDocument::clear()
{
    m_netName = "Untitled";
    m_comment.clear();
    m_places.clear();
    m_transitions.clear();
    m_arcs.clear();
    m_inputs.clear();
    m_outputs.clear();
    m_variables.clear();
    m_nextPlace = 1;
    m_nextTransition = 1;
    m_nextArc = 1;
}

QString PetriNetDocument::netName() const { return m_netName; }
void PetriNetDocument::setNetName(const QString &name) { m_netName = name; }
QString PetriNetDocument::comment() const { return m_comment; }
void PetriNetDocument::setComment(const QString &comment) { m_comment = comment; }

PlaceData PetriNetDocument::addPlace(double x, double y)
{
    PlaceData place;
    place.id = makeId("p");
    place.name = "P" + QString::number(m_nextPlace - 1);
    place.tokens = 0;
    place.x = x;
    place.y = y;
    m_places.append(place);
    return place;
}

TransitionData PetriNetDocument::addTransition(double x, double y)
{
    TransitionData transition;
    transition.id = makeId("t");
    transition.name = "T" + QString::number(m_nextTransition - 1);
    transition.x = x;
    transition.y = y;
    m_transitions.append(transition);
    return transition;
}

ArcData PetriNetDocument::addArc(const QString &sourceId, const QString &targetId)
{
    ArcData arc;
    if (!isValidArcDirection(sourceId, targetId)) {
        return arc;
    }
    arc.id = makeId("a");
    arc.sourceId = sourceId;
    arc.targetId = targetId;
    arc.weight = 1;
    m_arcs.append(arc);
    return arc;
}

bool PetriNetDocument::removeElement(const QString &id)
{
    for (int i = 0; i < m_places.size(); ++i) {
        if (m_places.at(i).id == id) {
            m_places.removeAt(i);
            for (int j = m_arcs.size() - 1; j >= 0; --j) {
                if (m_arcs.at(j).sourceId == id || m_arcs.at(j).targetId == id) {
                    m_arcs.removeAt(j);
                }
            }
            return true;
        }
    }

    for (int i = 0; i < m_transitions.size(); ++i) {
        if (m_transitions.at(i).id == id) {
            m_transitions.removeAt(i);
            for (int j = m_arcs.size() - 1; j >= 0; --j) {
                if (m_arcs.at(j).sourceId == id || m_arcs.at(j).targetId == id) {
                    m_arcs.removeAt(j);
                }
            }
            return true;
        }
    }
    return removeArc(id);
}

bool PetriNetDocument::removeArc(const QString &id)
{
    for (int i = 0; i < m_arcs.size(); ++i) {
        if (m_arcs.at(i).id == id) {
            m_arcs.removeAt(i);
            return true;
        }
    }
    return false;
}

bool PetriNetDocument::updatePlace(const PlaceData &place)
{
    for (int i = 0; i < m_places.size(); ++i) {
        if (m_places.at(i).id == place.id) {
            m_places[i] = place;
            return true;
        }
    }
    return false;
}

bool PetriNetDocument::updateTransition(const TransitionData &transition)
{
    for (int i = 0; i < m_transitions.size(); ++i) {
        if (m_transitions.at(i).id == transition.id) {
            m_transitions[i] = transition;
            return true;
        }
    }
    return false;
}

bool PetriNetDocument::updateArc(const ArcData &arc)
{
    for (int i = 0; i < m_arcs.size(); ++i) {
        if (m_arcs.at(i).id == arc.id) {
            m_arcs[i] = arc;
            return true;
        }
    }
    return false;
}

QList<PlaceData> PetriNetDocument::places() const { return m_places; }
QList<TransitionData> PetriNetDocument::transitions() const { return m_transitions; }
QList<ArcData> PetriNetDocument::arcs() const { return m_arcs; }
QList<IoData> PetriNetDocument::inputs() const { return m_inputs; }
QList<IoData> PetriNetDocument::outputs() const { return m_outputs; }
QList<VariableData> PetriNetDocument::variables() const { return m_variables; }

void PetriNetDocument::setPlaces(const QList<PlaceData> &places) { m_places = places; refreshNextIds(); }
void PetriNetDocument::setTransitions(const QList<TransitionData> &transitions) { m_transitions = transitions; refreshNextIds(); }
void PetriNetDocument::setArcs(const QList<ArcData> &arcs) { m_arcs = arcs; refreshNextIds(); }
void PetriNetDocument::setInputs(const QList<IoData> &inputs) { m_inputs = inputs; }
void PetriNetDocument::setOutputs(const QList<IoData> &outputs) { m_outputs = outputs; }
void PetriNetDocument::setVariables(const QList<VariableData> &variables) { m_variables = variables; }

bool PetriNetDocument::placeById(const QString &id, PlaceData *place) const
{
    for (int i = 0; i < m_places.size(); ++i) {
        if (m_places.at(i).id == id) {
            if (place) {
                *place = m_places.at(i);
            }
            return true;
        }
    }
    return false;
}

bool PetriNetDocument::transitionById(const QString &id, TransitionData *transition) const
{
    for (int i = 0; i < m_transitions.size(); ++i) {
        if (m_transitions.at(i).id == id) {
            if (transition) {
                *transition = m_transitions.at(i);
            }
            return true;
        }
    }
    return false;
}

bool PetriNetDocument::arcById(const QString &id, ArcData *arc) const
{
    for (int i = 0; i < m_arcs.size(); ++i) {
        if (m_arcs.at(i).id == id) {
            if (arc) {
                *arc = m_arcs.at(i);
            }
            return true;
        }
    }
    return false;
}

ElementKind PetriNetDocument::elementKind(const QString &id) const
{
    if (placeById(id, 0)) {
        return PlaceElement;
    }
    if (transitionById(id, 0)) {
        return TransitionElement;
    }
    if (arcById(id, 0)) {
        return ArcElement;
    }
    return UnknownElement;
}

bool PetriNetDocument::isValidArcDirection(const QString &sourceId, const QString &targetId) const
{
    ElementKind sourceKind = elementKind(sourceId);
    ElementKind targetKind = elementKind(targetId);
    return (sourceKind == PlaceElement && targetKind == TransitionElement)
        || (sourceKind == TransitionElement && targetKind == PlaceElement);
}

QStringList PetriNetDocument::validateBasic() const
{
    QStringList errors;
    QSet<QString> ids;

    if (m_netName.trimmed().isEmpty()) {
        errors << "Net name is empty.";
    }

    for (int i = 0; i < m_places.size(); ++i) {
        PlaceData place = m_places.at(i);
        if (place.id.trimmed().isEmpty()) {
            errors << "Place has empty id.";
        }
        if (place.name.trimmed().isEmpty()) {
            errors << "Place " + place.id + " has empty name.";
        }
        if (place.tokens < 0) {
            errors << "Place " + place.name + " has negative token count.";
        }
        if (ids.contains(place.id)) {
            errors << "Duplicate element id: " + place.id;
        }
        ids.insert(place.id);
        if (nameAlreadyUsed(place.name, place.id)) {
            errors << "Duplicate element name: " + place.name;
        }
    }

    for (int i = 0; i < m_transitions.size(); ++i) {
        TransitionData transition = m_transitions.at(i);
        if (transition.id.trimmed().isEmpty()) {
            errors << "Transition has empty id.";
        }
        if (transition.name.trimmed().isEmpty()) {
            errors << "Transition " + transition.id + " has empty name.";
        }
        if (ids.contains(transition.id)) {
            errors << "Duplicate element id: " + transition.id;
        }
        ids.insert(transition.id);
        if (nameAlreadyUsed(transition.name, transition.id)) {
            errors << "Duplicate element name: " + transition.name;
        }
    }

    for (int i = 0; i < m_arcs.size(); ++i) {
        ArcData arc = m_arcs.at(i);
        if (arc.weight < 1) {
            errors << "Arc " + arc.id + " has weight lower than 1.";
        }
        if (!containsElement(arc.sourceId) || !containsElement(arc.targetId)) {
            errors << "Arc " + arc.id + " has invalid references.";
        } else if (!isValidArcDirection(arc.sourceId, arc.targetId)) {
            errors << "Arc " + arc.id + " must connect a place and a transition.";
        }
        if (ids.contains(arc.id)) {
            errors << "Duplicate element id: " + arc.id;
        }
        ids.insert(arc.id);
    }

    return errors;
}

QString PetriNetDocument::makeId(const QString &prefix)
{
    if (prefix == "p") {
        return "p" + QString::number(m_nextPlace++);
    }
    if (prefix == "t") {
        return "t" + QString::number(m_nextTransition++);
    }
    return "a" + QString::number(m_nextArc++);
}

bool PetriNetDocument::containsElement(const QString &id) const
{
    return elementKind(id) != UnknownElement;
}

bool PetriNetDocument::nameAlreadyUsed(const QString &name, const QString &ownId) const
{
    if (name.trimmed().isEmpty()) {
        return false;
    }
    for (int i = 0; i < m_places.size(); ++i) {
        if (m_places.at(i).id != ownId && m_places.at(i).name == name) {
            return true;
        }
    }
    for (int i = 0; i < m_transitions.size(); ++i) {
        if (m_transitions.at(i).id != ownId && m_transitions.at(i).name == name) {
            return true;
        }
    }
    return false;
}

void PetriNetDocument::refreshNextIds()
{
    m_nextPlace = 1;
    m_nextTransition = 1;
    m_nextArc = 1;

    for (int i = 0; i < m_places.size(); ++i) {
        if (m_places.at(i).id.startsWith("p")) {
            m_nextPlace = qMax(m_nextPlace, m_places.at(i).id.mid(1).toInt() + 1);
        }
    }
    for (int i = 0; i < m_transitions.size(); ++i) {
        if (m_transitions.at(i).id.startsWith("t")) {
            m_nextTransition = qMax(m_nextTransition, m_transitions.at(i).id.mid(1).toInt() + 1);
        }
    }
    for (int i = 0; i < m_arcs.size(); ++i) {
        if (m_arcs.at(i).id.startsWith("a")) {
            m_nextArc = qMax(m_nextArc, m_arcs.at(i).id.mid(1).toInt() + 1);
        }
    }
}
