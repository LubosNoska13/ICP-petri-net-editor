/**
 * @file DocumentSerializer.h
 * @author Lubos and project team
 * @brief Serializer bridge between GUI documents and the core Petri net text format.
 * @details Manual code. Borrowed code: none.
 */

#ifndef DOCUMENTSERIALIZER_H
#define DOCUMENTSERIALIZER_H

#include "core_api/CoreMapper.h"
#include "core_api/PetriNetDocument.h"

#include <QString>

/** Saves and loads GUI documents through the core text format plus a layout sidecar. */
class DocumentSerializer
{
public:
    static bool save(const PetriNetDocument &document, const QString &fileName, QString *error);
    static bool load(PetriNetDocument *document, const QString &fileName, QString *error);

private:
    static QString layoutFileName(const QString &fileName);
    static bool saveLayout(const PetriNetDocument &document, const QString &fileName, QString *error);
    static bool loadLayout(const QString &fileName, LayoutData *layout, QString *error);
    static QString escape(const QString &text);
    static QString unescape(const QString &text);
};

#endif
