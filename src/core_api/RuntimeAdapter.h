/**
 * @file RuntimeAdapter.h
 * @author Lubos and project team
 * @brief Abstract interface between GUI monitor and runtime backend.
 * @details Manual code. Borrowed code: none.
 */

#ifndef RUNTIMEADAPTER_H
#define RUNTIMEADAPTER_H

#include "core_api/PetriNetDocument.h"
#include "core_api/RuntimeSnapshot.h"

/** Runtime backend interface used by the monitor. */
class RuntimeAdapter
{
public:
    virtual ~RuntimeAdapter() {}

    /** Starts monitoring/running the selected document. */
    virtual bool start(PetriNetDocument *document) = 0;

    /** Requests runtime stop. */
    virtual void stop() = 0;

    /** Sends one external input event to the runtime. */
    virtual void injectInput(const QString &name, const QString &value) = 0;

    /** Returns the latest state known to the GUI. */
    virtual RuntimeSnapshot snapshot() const = 0;

    /** True when the adapter is in running state. */
    virtual bool isRunning() const = 0;
};

#endif
