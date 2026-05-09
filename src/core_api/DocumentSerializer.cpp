/**
 * @file DocumentSerializer.cpp
 * @author Lubos and project team
 * @brief Implementation of the temporary GUI document serializer.
 * @details Manual code. Borrowed code: none.
 */

#include "core_api/DocumentSerializer.h"

#include <QFile>
#include <QTextStream>

bool DocumentSerializer::save(const PetriNetDocument &document, const QString &fileName, QString *error)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }

    QTextStream out(&file);
    out << "[net]\n";
    out << "name=" << escape(document.netName()) << "\n";
    out << "comment=" << escape(document.comment()) << "\n\n";

    out << "[inputs]\n";
    QList<IoData> inputs = document.inputs();
    for (int i = 0; i < inputs.size(); ++i) {
        out << escape(inputs.at(i).name) << ";" << escape(inputs.at(i).lastValue) << "\n";
    }
    out << "\n[outputs]\n";
    QList<IoData> outputs = document.outputs();
    for (int i = 0; i < outputs.size(); ++i) {
        out << escape(outputs.at(i).name) << ";" << escape(outputs.at(i).lastValue) << "\n";
    }
    out << "\n[variables]\n";
    QList<VariableData> variables = document.variables();
    for (int i = 0; i < variables.size(); ++i) {
        out << escape(variables.at(i).name) << ";" << escape(variables.at(i).initialValue) << "\n";
    }

    out << "\n[places]\n";
    QList<PlaceData> places = document.places();
    for (int i = 0; i < places.size(); ++i) {
        PlaceData p = places.at(i);
        out << escape(p.id) << ";" << escape(p.name) << ";" << p.tokens << ";"
            << p.x << ";" << p.y << ";" << escape(p.action) << "\n";
    }

    out << "\n[transitions]\n";
    QList<TransitionData> transitions = document.transitions();
    for (int i = 0; i < transitions.size(); ++i) {
        TransitionData t = transitions.at(i);
        out << escape(t.id) << ";" << escape(t.name) << ";" << t.x << ";" << t.y << ";"
            << escape(t.event) << ";" << escape(t.guard) << ";" << t.delayMs << ";"
            << escape(t.action) << ";" << t.priority << "\n";
    }

    out << "\n[arcs]\n";
    QList<ArcData> arcs = document.arcs();
    for (int i = 0; i < arcs.size(); ++i) {
        ArcData a = arcs.at(i);
        out << escape(a.id) << ";" << escape(a.sourceId) << ";"
            << escape(a.targetId) << ";" << a.weight << "\n";
    }
    return true;
}

bool DocumentSerializer::load(PetriNetDocument *document, const QString &fileName, QString *error)
{
    if (!document) {
        if (error) {
            *error = "Missing document.";
        }
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }

    PetriNetDocument loaded;
    QList<PlaceData> places;
    QList<TransitionData> transitions;
    QList<ArcData> arcs;
    QList<IoData> inputs;
    QList<IoData> outputs;
    QList<VariableData> variables;
    QString section;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) {
            continue;
        }
        if (line.startsWith("[") && line.endsWith("]")) {
            section = line.mid(1, line.length() - 2);
            continue;
        }

        if (section == "net") {
            int eq = line.indexOf('=');
            if (eq >= 0) {
                QString key = line.left(eq);
                QString value = unescape(line.mid(eq + 1));
                if (key == "name") {
                    loaded.setNetName(value);
                } else if (key == "comment") {
                    loaded.setComment(value);
                }
            }
        } else if (section == "inputs" || section == "outputs") {
            QStringList parts = line.split(';');
            IoData io;
            io.name = parts.size() > 0 ? unescape(parts.at(0)) : QString();
            io.lastValue = parts.size() > 1 ? unescape(parts.at(1)) : QString();
            if (!io.name.isEmpty()) {
                if (section == "inputs") {
                    inputs.append(io);
                } else {
                    outputs.append(io);
                }
            }
        } else if (section == "variables") {
            QStringList parts = line.split(';');
            VariableData variable;
            variable.name = parts.size() > 0 ? unescape(parts.at(0)) : QString();
            variable.initialValue = parts.size() > 1 ? unescape(parts.at(1)) : QString();
            if (!variable.name.isEmpty()) {
                variables.append(variable);
            }
        } else if (section == "places") {
            QStringList parts = line.split(';');
            if (parts.size() >= 5) {
                PlaceData p;
                p.id = unescape(parts.at(0));
                p.name = unescape(parts.at(1));
                p.tokens = parts.at(2).toInt();
                p.x = parts.at(3).toDouble();
                p.y = parts.at(4).toDouble();
                p.action = parts.size() > 5 ? unescape(parts.at(5)) : QString();
                places.append(p);
            }
        } else if (section == "transitions") {
            QStringList parts = line.split(';');
            if (parts.size() >= 9) {
                TransitionData t;
                t.id = unescape(parts.at(0));
                t.name = unescape(parts.at(1));
                t.x = parts.at(2).toDouble();
                t.y = parts.at(3).toDouble();
                t.event = unescape(parts.at(4));
                t.guard = unescape(parts.at(5));
                t.delayMs = parts.at(6).toInt();
                t.action = unescape(parts.at(7));
                t.priority = parts.at(8).toInt();
                transitions.append(t);
            }
        } else if (section == "arcs") {
            QStringList parts = line.split(';');
            if (parts.size() >= 4) {
                ArcData a;
                a.id = unescape(parts.at(0));
                a.sourceId = unescape(parts.at(1));
                a.targetId = unescape(parts.at(2));
                a.weight = parts.at(3).toInt();
                arcs.append(a);
            }
        }
    }

    loaded.setInputs(inputs);
    loaded.setOutputs(outputs);
    loaded.setVariables(variables);
    loaded.setPlaces(places);
    loaded.setTransitions(transitions);
    loaded.setArcs(arcs);
    *document = loaded;
    return true;
}

QString DocumentSerializer::escape(const QString &text)
{
    QString result = text;
    result.replace("\\", "\\\\");
    result.replace("\n", "\\n");
    result.replace(";", "\\s");
    return result;
}

QString DocumentSerializer::unescape(const QString &text)
{
    QString result = text;
    result.replace("\\s", ";");
    result.replace("\\n", "\n");
    result.replace("\\\\", "\\");
    return result;
}
