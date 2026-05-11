/**
 * @file CoreRuntimeAdapter.h
 * @author Lubos and project team
 * @brief Runtime adapter that connects the Qt monitor to the real core runtime.
 * @details Manual code. Borrowed code: none.
 */

#ifndef CORERUNTIMEADAPTER_H
#define CORERUNTIMEADAPTER_H

#include "core_api/RuntimeAdapter.h"
#include "runtime.hpp"

#include <memory>

/** Runtime backend using PetriRuntime from the non-Qt core. */
class CoreRuntimeAdapter : public RuntimeAdapter
{
public:
    CoreRuntimeAdapter();

    bool start(PetriNetDocument *document);
    void stop();
    void injectInput(const QString &name, const QString &value);
    RuntimeSnapshot snapshot() const;
    bool isRunning() const;

    void advanceTime(int deltaMs);

private:
    RuntimeSnapshot convertSnapshot(const StateSnapshot &snapshot) const;
    QString placeIdForName(const QString &name) const;
    QString transitionIdForName(const QString &name) const;
    void addLocalLog(const QString &text);
    static QString valueToString(const EvalValue &value);

    PetriNetDocument *m_document;
    std::unique_ptr<PetriRuntime> m_runtime;
    QList<LogEntry> m_localLog;
    bool m_running;
};

#endif
