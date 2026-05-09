/**
 * @file PetriNetDocument.h
 * @author Lubos and project team
 * @brief In-memory Petri net document used by the Qt editor.
 * @details Manual code. Borrowed code: none.
 */

#ifndef PETRINETDOCUMENT_H
#define PETRINETDOCUMENT_H

#include "core_api/NetTypes.h"

#include <QList>
#include <QString>
#include <QStringList>

/** Lightweight document model owned by the GUI layer. */
class PetriNetDocument
{
public:
    PetriNetDocument();

    void clear();

    QString netName() const;
    void setNetName(const QString &name);
    QString comment() const;
    void setComment(const QString &comment);

    PlaceData addPlace(double x, double y);
    TransitionData addTransition(double x, double y);
    ArcData addArc(const QString &sourceId, const QString &targetId);

    bool removeElement(const QString &id);
    bool removeArc(const QString &id);

    bool updatePlace(const PlaceData &place);
    bool updateTransition(const TransitionData &transition);
    bool updateArc(const ArcData &arc);

    QList<PlaceData> places() const;
    QList<TransitionData> transitions() const;
    QList<ArcData> arcs() const;
    QList<IoData> inputs() const;
    QList<IoData> outputs() const;
    QList<VariableData> variables() const;

    void setPlaces(const QList<PlaceData> &places);
    void setTransitions(const QList<TransitionData> &transitions);
    void setArcs(const QList<ArcData> &arcs);
    void setInputs(const QList<IoData> &inputs);
    void setOutputs(const QList<IoData> &outputs);
    void setVariables(const QList<VariableData> &variables);

    bool placeById(const QString &id, PlaceData *place) const;
    bool transitionById(const QString &id, TransitionData *transition) const;
    bool arcById(const QString &id, ArcData *arc) const;
    ElementKind elementKind(const QString &id) const;

    bool isValidArcDirection(const QString &sourceId, const QString &targetId) const;
    QStringList validateBasic() const;

private:
    QString makeId(const QString &prefix);
    bool containsElement(const QString &id) const;
    bool nameAlreadyUsed(const QString &name, const QString &ownId) const;
    void refreshNextIds();

    QString m_netName;
    QString m_comment;
    QList<PlaceData> m_places;
    QList<TransitionData> m_transitions;
    QList<ArcData> m_arcs;
    QList<IoData> m_inputs;
    QList<IoData> m_outputs;
    QList<VariableData> m_variables;
    int m_nextPlace;
    int m_nextTransition;
    int m_nextArc;
};

#endif
