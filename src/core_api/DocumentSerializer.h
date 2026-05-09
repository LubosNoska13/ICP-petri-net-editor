/**
 * @file DocumentSerializer.h
 * @author Lubos and project team
 * @brief Temporary readable text serializer for GUI documents.
 * @details Manual code. Borrowed code: none.
 */

#ifndef DOCUMENTSERIALIZER_H
#define DOCUMENTSERIALIZER_H

#include "core_api/PetriNetDocument.h"

#include <QString>

/** Saves and loads the simple GUI document format. */
class DocumentSerializer
{
public:
    static bool save(const PetriNetDocument &document, const QString &fileName, QString *error);
    static bool load(PetriNetDocument *document, const QString &fileName, QString *error);

private:
    static QString escape(const QString &text);
    static QString unescape(const QString &text);
};

#endif
