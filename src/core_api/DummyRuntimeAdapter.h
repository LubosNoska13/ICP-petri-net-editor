/**
 * @file DummyRuntimeAdapter.h
 * @author Lubos and project team
 * @brief Demonstration runtime adapter for GUI development.
 * @details Manual code. Borrowed code: none.
 */

#ifndef DUMMYRUNTIMEADAPTER_H
#define DUMMYRUNTIMEADAPTER_H

#include "core_api/RuntimeAdapter.h"

/** Adapter that exposes document state without simulating Petri net semantics. */
class DummyRuntimeAdapter : public RuntimeAdapter
{
public:
    DummyRuntimeAdapter();

    bool start(PetriNetDocument *document);
    void stop();
    void injectInput(const QString &name, const QString &value);
    RuntimeSnapshot snapshot() const;
    bool isRunning() const;

private:
    void addLog(const QString &text);

    PetriNetDocument *m_document;
    bool m_running;
    QList<IoData> m_inputs;
    QList<LogEntry> m_log;
};

#endif
