/**
 * @file RuntimeSnapshot.h
 * @author Lubos and project team
 * @brief Snapshot data shown by the runtime monitor.
 * @details Manual code. Borrowed code: none.
 */

#ifndef RUNTIMESNAPSHOT_H
#define RUNTIMESNAPSHOT_H

#include "core_api/NetTypes.h"

#include <QDateTime>
#include <QList>
#include <QString>
#include <QStringList>

/** Current token count for one place. */
struct MarkingEntry {
    QString placeId;
    QString placeName;
    int tokens;
};

/** Pending timeout displayed in the monitor. */
struct PendingTimerEntry {
    QString transitionId;
    QString transitionName;
    int remainingMs;
};

/** Runtime log entry. */
struct LogEntry {
    QDateTime time;
    QString text;
};

/** Complete monitor snapshot. */
struct RuntimeSnapshot {
    QList<MarkingEntry> marking;
    QStringList enabledTransitions;
    QList<PendingTimerEntry> pendingTimers;
    QList<IoData> inputs;
    QList<IoData> outputs;
    QList<VariableData> variables;
    QList<LogEntry> logEntries;
};

#endif
