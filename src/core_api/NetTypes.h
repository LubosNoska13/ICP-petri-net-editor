/**
 * @file NetTypes.h
 * @author Lubos and project team
 * @brief Simple data objects shared between the GUI and the future core API.
 * @details Manual code. Borrowed code: none.
 */

#ifndef NETTYPES_H
#define NETTYPES_H

#include <QString>

/** Type of a graphical/model element. */
enum ElementKind {
    UnknownElement,
    PlaceElement,
    TransitionElement,
    ArcElement
};

/** One Petri net place as edited by the GUI. */
struct PlaceData {
    QString id;
    QString name;
    int tokens;
    double x;
    double y;
    QString action;

    PlaceData() : tokens(0), x(0), y(0) {}
};

/** One Petri net transition as edited by the GUI. */
struct TransitionData {
    QString id;
    QString name;
    double x;
    double y;
    QString event;
    QString guard;
    int delayMs;
    QString delayExpression;
    QString action;
    int priority;

    TransitionData() : x(0), y(0), delayMs(0), priority(0) {}
};

/** Directed weighted arc between a place and a transition. */
struct ArcData {
    QString id;
    QString sourceId;
    QString targetId;
    int weight;

    ArcData() : weight(1) {}
};

/** Internal variable description. */
struct VariableData {
    QString type;
    QString name;
    QString initialValue;

    VariableData() : type("int") {}
};

/** Input or output description. */
struct IoData {
    QString name;
    QString lastValue;
};

#endif
