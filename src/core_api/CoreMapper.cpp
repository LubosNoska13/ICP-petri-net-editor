/**
 * @file CoreMapper.cpp
 * @author Lubos and project team
 * @brief Implementation of conversions between GUI document data and core model data.
 * @details Manual code. Borrowed code: none.
 */

#include "core_api/CoreMapper.h"

#include <QMap>

namespace {

std::string toStd(const QString &text)
{
    return text.toStdString();
}

QString toQt(const std::string &text)
{
    return QString::fromStdString(text);
}

QString defaultPlaceId(int index)
{
    return "p" + QString::number(index + 1);
}

QString defaultTransitionId(int index)
{
    return "t" + QString::number(index + 1);
}

QString defaultArcId(int index)
{
    return "a" + QString::number(index + 1);
}

LayoutPosition autoPlacePosition(int index)
{
    const int column = index % 4;
    const int row = index / 4;
    return LayoutPosition(120 + column * 170, 120 + row * 160);
}

LayoutPosition autoTransitionPosition(int index)
{
    const int column = index % 4;
    const int row = index / 4;
    return LayoutPosition(205 + column * 170, 120 + row * 160);
}

} // namespace

PetriNet CoreMapper::toCoreNet(const PetriNetDocument &document, QStringList *errors)
{
    PetriNet net;
    net.name = toStd(document.netName());
    net.comment = toStd(document.comment());

    QList<IoData> inputs = document.inputs();
    for (int i = 0; i < inputs.size(); ++i) {
        if (!inputs.at(i).name.trimmed().isEmpty()) {
            net.inputs.push_back(Input{toStd(inputs.at(i).name.trimmed())});
        }
    }

    QList<IoData> outputs = document.outputs();
    for (int i = 0; i < outputs.size(); ++i) {
        if (!outputs.at(i).name.trimmed().isEmpty()) {
            net.outputs.push_back(Output{toStd(outputs.at(i).name.trimmed())});
        }
    }

    QList<VariableData> variables = document.variables();
    for (int i = 0; i < variables.size(); ++i) {
        VariableData guiVariable = variables.at(i);
        if (guiVariable.name.trimmed().isEmpty()) {
            continue;
        }
        Variable variable;
        variable.type = toStd(guiVariable.type.trimmed().isEmpty() ? "int" : guiVariable.type.trimmed());
        variable.name = toStd(guiVariable.name.trimmed());
        variable.initializer = toStd(guiVariable.initialValue.trimmed().isEmpty()
                                     ? "0" : guiVariable.initialValue.trimmed());
        net.variables.push_back(variable);
    }

    QList<PlaceData> places = document.places();
    QMap<QString, QString> placeNamesById;
    for (int i = 0; i < places.size(); ++i) {
        PlaceData guiPlace = places.at(i);
        placeNamesById.insert(guiPlace.id, guiPlace.name);
        Place place;
        place.name = toStd(guiPlace.name.trimmed());
        place.initial_tokens = guiPlace.tokens;
        place.add_token_action = toStd(guiPlace.action);
        net.places.push_back(place);
    }

    QList<TransitionData> transitions = document.transitions();
    QMap<QString, int> transitionIndexesById;
    for (int i = 0; i < transitions.size(); ++i) {
        TransitionData guiTransition = transitions.at(i);
        transitionIndexesById.insert(guiTransition.id, static_cast<int>(net.transitions.size()));

        Transition transition;
        transition.name = toStd(guiTransition.name.trimmed());
        transition.condition.event_name = toStd(guiTransition.event.trimmed());
        transition.condition.guard_expression = toStd(guiTransition.guard.trimmed());
        if (!guiTransition.delayExpression.trimmed().isEmpty()) {
            transition.condition.delay_expression = toStd(guiTransition.delayExpression.trimmed());
        } else if (guiTransition.delayMs > 0) {
            transition.condition.delay_expression = std::to_string(guiTransition.delayMs);
        }
        transition.action_code = toStd(guiTransition.action);
        transition.priority = guiTransition.priority;
        net.transitions.push_back(transition);
    }

    QList<ArcData> arcs = document.arcs();
    for (int i = 0; i < arcs.size(); ++i) {
        ArcData guiArc = arcs.at(i);
        ElementKind sourceKind = document.elementKind(guiArc.sourceId);
        ElementKind targetKind = document.elementKind(guiArc.targetId);

        if (sourceKind == PlaceElement && targetKind == TransitionElement) {
            if (!transitionIndexesById.contains(guiArc.targetId)) {
                continue;
            }
            Arc arc;
            arc.place_name = toStd(placeNamesById.value(guiArc.sourceId));
            arc.weight = guiArc.weight;
            net.transitions[transitionIndexesById.value(guiArc.targetId)].input_arcs.push_back(arc);
        } else if (sourceKind == TransitionElement && targetKind == PlaceElement) {
            if (!transitionIndexesById.contains(guiArc.sourceId)) {
                continue;
            }
            Arc arc;
            arc.place_name = toStd(placeNamesById.value(guiArc.targetId));
            arc.weight = guiArc.weight;
            net.transitions[transitionIndexesById.value(guiArc.sourceId)].output_arcs.push_back(arc);
        } else if (errors) {
            *errors << "Arc " + guiArc.id + " has invalid direction.";
        }
    }

    return net;
}

PetriNetDocument CoreMapper::fromCoreNet(const PetriNet &net, const LayoutData &layout)
{
    PetriNetDocument document;
    document.setNetName(toQt(net.name));
    document.setComment(toQt(net.comment));

    QList<IoData> inputs;
    for (const Input &input : net.inputs) {
        IoData data;
        data.name = toQt(input.name);
        inputs.append(data);
    }
    document.setInputs(inputs);

    QList<IoData> outputs;
    for (const Output &output : net.outputs) {
        IoData data;
        data.name = toQt(output.name);
        outputs.append(data);
    }
    document.setOutputs(outputs);

    QList<VariableData> variables;
    for (const Variable &variable : net.variables) {
        VariableData data;
        data.type = toQt(variable.type.empty() ? "int" : variable.type);
        data.name = toQt(variable.name);
        data.initialValue = toQt(variable.initializer);
        variables.append(data);
    }
    document.setVariables(variables);

    QList<PlaceData> places;
    QMap<QString, QString> placeIdsByName;
    for (int i = 0; i < static_cast<int>(net.places.size()); ++i) {
        const Place &place = net.places.at(i);
        QString name = toQt(place.name);
        LayoutPosition position = layout.places.contains(name)
            ? layout.places.value(name) : autoPlacePosition(i);

        PlaceData data;
        data.id = defaultPlaceId(i);
        data.name = name;
        data.tokens = place.initial_tokens;
        data.x = position.x;
        data.y = position.y;
        data.action = toQt(place.add_token_action);
        places.append(data);
        placeIdsByName.insert(name, data.id);
    }
    document.setPlaces(places);

    QList<TransitionData> transitions;
    QMap<QString, QString> transitionIdsByName;
    for (int i = 0; i < static_cast<int>(net.transitions.size()); ++i) {
        const Transition &transition = net.transitions.at(i);
        QString name = toQt(transition.name);
        LayoutPosition position = layout.transitions.contains(name)
            ? layout.transitions.value(name) : autoTransitionPosition(i);

        TransitionData data;
        data.id = defaultTransitionId(i);
        data.name = name;
        data.x = position.x;
        data.y = position.y;
        data.event = toQt(transition.condition.event_name);
        data.guard = toQt(transition.condition.guard_expression);
        data.delayExpression = toQt(transition.condition.delay_expression);
        data.delayMs = data.delayExpression.toInt();
        data.action = toQt(transition.action_code);
        data.priority = transition.priority;
        transitions.append(data);
        transitionIdsByName.insert(name, data.id);
    }
    document.setTransitions(transitions);

    QList<ArcData> arcs;
    int arcIndex = 0;
    for (const Transition &transition : net.transitions) {
        QString transitionId = transitionIdsByName.value(toQt(transition.name));
        for (const Arc &arc : transition.input_arcs) {
            QString placeId = placeIdsByName.value(toQt(arc.place_name));
            if (placeId.isEmpty() || transitionId.isEmpty()) {
                continue;
            }
            ArcData data;
            data.id = defaultArcId(arcIndex++);
            data.sourceId = placeId;
            data.targetId = transitionId;
            data.weight = arc.weight;
            arcs.append(data);
        }
        for (const Arc &arc : transition.output_arcs) {
            QString placeId = placeIdsByName.value(toQt(arc.place_name));
            if (placeId.isEmpty() || transitionId.isEmpty()) {
                continue;
            }
            ArcData data;
            data.id = defaultArcId(arcIndex++);
            data.sourceId = transitionId;
            data.targetId = placeId;
            data.weight = arc.weight;
            arcs.append(data);
        }
    }
    document.setArcs(arcs);

    return document;
}

LayoutData CoreMapper::layoutFromDocument(const PetriNetDocument &document)
{
    LayoutData layout;
    QList<PlaceData> places = document.places();
    for (int i = 0; i < places.size(); ++i) {
        layout.places.insert(places.at(i).name, LayoutPosition(places.at(i).x, places.at(i).y));
    }

    QList<TransitionData> transitions = document.transitions();
    for (int i = 0; i < transitions.size(); ++i) {
        layout.transitions.insert(transitions.at(i).name,
                                  LayoutPosition(transitions.at(i).x, transitions.at(i).y));
    }
    return layout;
}
