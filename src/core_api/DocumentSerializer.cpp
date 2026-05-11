/**
 * @file DocumentSerializer.cpp
 * @author Lubos and project team
 * @brief Implementation of the core format serializer bridge.
 * @details Manual code. Borrowed code: none.
 */

#include "core_api/DocumentSerializer.h"

#include "core_api/CoreMapper.h"
#include "parser.hpp"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <stdexcept>

bool DocumentSerializer::save(const PetriNetDocument &document, const QString &fileName, QString *error)
{
    QStringList conversionErrors;
    PetriNet net = CoreMapper::toCoreNet(document, &conversionErrors);
    if (!conversionErrors.isEmpty()) {
        if (error) {
            *error = conversionErrors.join("\n");
        }
        return false;
    }

    try {
        Writer writer;
        writer.write_file(net, fileName.toStdString());
    } catch (const std::exception &exception) {
        if (error) {
            *error = QString::fromStdString(exception.what());
        }
        return false;
    }

    return saveLayout(document, fileName, error);
}

bool DocumentSerializer::load(PetriNetDocument *document, const QString &fileName, QString *error)
{
    if (!document) {
        if (error) {
            *error = "Missing document.";
        }
        return false;
    }

    Parser parser;
    ParseResult result = parser.parse_file(fileName.toStdString());
    if (!result.ok()) {
        if (error) {
            QStringList messages;
            for (std::size_t i = 0; i < result.errors.size(); ++i) {
                const ParseError &parseError = result.errors.at(i);
                messages << "Line " + QString::number(parseError.line) + ": "
                            + QString::fromStdString(parseError.message);
            }
            *error = messages.join("\n");
        }
        return false;
    }

    LayoutData layout;
    QString layoutError;
    if (!loadLayout(fileName, &layout, &layoutError) && QFileInfo(layoutFileName(fileName)).exists()) {
        if (error) {
            *error = layoutError;
        }
        return false;
    }

    *document = CoreMapper::fromCoreNet(result.net, layout);
    return true;
}

QString DocumentSerializer::layoutFileName(const QString &fileName)
{
    return fileName + ".layout";
}

bool DocumentSerializer::saveLayout(const PetriNetDocument &document, const QString &fileName, QString *error)
{
    QFile file(layoutFileName(fileName));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }

    QTextStream out(&file);
    out << "[layout]\n";

    QList<PlaceData> places = document.places();
    for (int i = 0; i < places.size(); ++i) {
        out << "place;" << escape(places.at(i).name) << ";"
            << places.at(i).x << ";" << places.at(i).y << "\n";
    }

    QList<TransitionData> transitions = document.transitions();
    for (int i = 0; i < transitions.size(); ++i) {
        out << "transition;" << escape(transitions.at(i).name) << ";"
            << transitions.at(i).x << ";" << transitions.at(i).y << "\n";
    }

    return true;
}

bool DocumentSerializer::loadLayout(const QString &fileName, LayoutData *layout, QString *error)
{
    if (!layout) {
        if (error) {
            *error = "Missing layout object.";
        }
        return false;
    }

    QFile file(layoutFileName(fileName));
    if (!file.exists()) {
        return true;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }

    QTextStream in(&file);
    QString section;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) {
            continue;
        }
        if (line.startsWith("[") && line.endsWith("]")) {
            section = line.mid(1, line.length() - 2);
            continue;
        }
        if (section != "layout") {
            continue;
        }

        QStringList parts = line.split(';');
        if (parts.size() < 4) {
            continue;
        }

        QString kind = parts.at(0).trimmed();
        QString name = unescape(parts.at(1));
        LayoutPosition position(parts.at(2).toDouble(), parts.at(3).toDouble());
        if (kind == "place") {
            layout->places.insert(name, position);
        } else if (kind == "transition") {
            layout->transitions.insert(name, position);
        }
    }

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
