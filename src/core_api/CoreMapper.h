/**
 * @file CoreMapper.h
 * @author Lubos and project team
 * @brief Conversion helpers between Qt GUI document data and the core Petri net model.
 * @details Manual code. Borrowed code: none.
 */

#ifndef COREMAPPER_H
#define COREMAPPER_H

#include "core_api/PetriNetDocument.h"
#include "model.hpp"

#include <QMap>
#include <QStringList>

/** Saved diagram position indexed by element name from the core model. */
struct LayoutPosition {
    double x;
    double y;

    LayoutPosition() : x(0), y(0) {}
    LayoutPosition(double px, double py) : x(px), y(py) {}
};

/** Positions for places and transitions loaded from the GUI sidecar file. */
struct LayoutData {
    QMap<QString, LayoutPosition> places;
    QMap<QString, LayoutPosition> transitions;
};

/** Functions that keep Qt GUI data separate from the non-Qt core model. */
class CoreMapper
{
public:
    static PetriNet toCoreNet(const PetriNetDocument &document, QStringList *errors);
    static PetriNetDocument fromCoreNet(const PetriNet &net, const LayoutData &layout);
    static LayoutData layoutFromDocument(const PetriNetDocument &document);
};

#endif
