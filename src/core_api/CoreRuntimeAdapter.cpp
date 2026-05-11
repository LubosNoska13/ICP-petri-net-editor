/**
 * @file CoreRuntimeAdapter.cpp
 * @author Lubos and project team
 * @brief Implementation of the adapter between Qt monitor widgets and PetriRuntime.
 * @details Manual code. Borrowed code: none.
 */

#include "core_api/CoreRuntimeAdapter.h"

#include "core_api/CoreMapper.h"
#include "logger.hpp"
#include "validate.hpp"

#include <QDateTime>

CoreRuntimeAdapter::CoreRuntimeAdapter()
    : m_document(0), m_running(false)
{
}

bool CoreRuntimeAdapter::start(PetriNetDocument *document)
{
    m_document = document;
    m_runtime.reset();
    m_localLog.clear();

    if (!m_document) {
        addLocalLog("error: Missing document.");
        m_running = false;
        return false;
    }

    QStringList conversionErrors;
    PetriNet net = CoreMapper::toCoreNet(*m_document, &conversionErrors);
    if (!conversionErrors.isEmpty()) {
        for (int i = 0; i < conversionErrors.size(); ++i) {
            addLocalLog("error: " + conversionErrors.at(i));
        }
        m_running = false;
        return false;
    }

    Validator validator;
    ValidationResult validation = validator.validate(net);
    if (validation.has_errors()) {
        for (std::size_t i = 0; i < validation.messages.size(); ++i) {
            const ValidationMessage &message = validation.messages.at(i);
            QString prefix = message.severity == ValidationSeverity::Error ? "error: " : "warning: ";
            addLocalLog(prefix + QString::fromStdString(message.message));
        }
        m_running = false;
        return false;
    }

    for (std::size_t i = 0; i < validation.messages.size(); ++i) {
        const ValidationMessage &message = validation.messages.at(i);
        addLocalLog("warning: " + QString::fromStdString(message.message));
    }

    m_runtime.reset(new PetriRuntime(net));
    m_runtime->initialize(true);
    m_running = true;
    addLocalLog("runtime-started: Runtime started.");
    return true;
}

void CoreRuntimeAdapter::stop()
{
    if (m_running) {
        addLocalLog("runtime-stopped: Runtime stopped.");
    }
    m_runtime.reset();
    m_running = false;
}

void CoreRuntimeAdapter::injectInput(const QString &name, const QString &value)
{
    if (!m_runtime || !m_running) {
        addLocalLog("warning: Runtime is not running.");
        return;
    }
    m_runtime->inject_input(name.toStdString(), value.toStdString());
}

RuntimeSnapshot CoreRuntimeAdapter::snapshot() const
{
    if (!m_runtime) {
        RuntimeSnapshot empty;
        empty.logEntries = m_localLog;
        return empty;
    }
    RuntimeSnapshot result = convertSnapshot(m_runtime->snapshot());
    result.logEntries = m_localLog + result.logEntries;
    return result;
}

bool CoreRuntimeAdapter::isRunning() const
{
    return m_running;
}

void CoreRuntimeAdapter::advanceTime(int deltaMs)
{
    if (m_runtime && m_running && deltaMs > 0) {
        m_runtime->advance_time(deltaMs);
    }
}

RuntimeSnapshot CoreRuntimeAdapter::convertSnapshot(const StateSnapshot &snapshot) const
{
    RuntimeSnapshot result;

    for (auto it = snapshot.marking.begin(); it != snapshot.marking.end(); ++it) {
        QString placeName = QString::fromStdString(it->first);
        MarkingEntry entry;
        entry.placeName = placeName;
        entry.placeId = placeIdForName(placeName);
        entry.tokens = it->second;
        result.marking.append(entry);
    }

    for (std::size_t i = 0; i < snapshot.enabled_transitions.size(); ++i) {
        result.enabledTransitions.append(QString::fromStdString(snapshot.enabled_transitions.at(i)));
    }

    for (std::size_t i = 0; i < snapshot.pending_timers.size(); ++i) {
        const PendingTimer &timer = snapshot.pending_timers.at(i);
        QString transitionName = QString::fromStdString(timer.transition_name);
        PendingTimerEntry entry;
        entry.transitionName = transitionName;
        entry.transitionId = transitionIdForName(transitionName);
        entry.remainingMs = static_cast<int>(timer.due_at_ms - snapshot.now_ms);
        if (entry.remainingMs < 0) {
            entry.remainingMs = 0;
        }
        result.pendingTimers.append(entry);
    }

    for (auto it = snapshot.inputs.begin(); it != snapshot.inputs.end(); ++it) {
        IoData data;
        data.name = QString::fromStdString(it->first);
        data.lastValue = QString::fromStdString(it->second.value);
        result.inputs.append(data);
    }

    for (auto it = snapshot.outputs.begin(); it != snapshot.outputs.end(); ++it) {
        IoData data;
        data.name = QString::fromStdString(it->first);
        data.lastValue = QString::fromStdString(it->second.value);
        result.outputs.append(data);
    }

    for (auto it = snapshot.variables.begin(); it != snapshot.variables.end(); ++it) {
        VariableData data;
        data.name = QString::fromStdString(it->first);
        data.type = QString::fromStdString(it->second.type);
        data.initialValue = valueToString(it->second.value);
        result.variables.append(data);
    }

    if (snapshot.event_log) {
        for (std::size_t i = 0; i < snapshot.event_log->size(); ++i) {
            const LoggerEntry &loggerEntry = snapshot.event_log->at(i);
            LogEntry entry;
            entry.time = QDateTime::currentDateTime().addMSecs(static_cast<int>(loggerEntry.time_ms));
            entry.text = QString::fromStdString(log_event_type_2_str(loggerEntry.type))
                       + ": " + QString::fromStdString(loggerEntry.message);
            result.logEntries.append(entry);
        }
    }

    return result;
}

QString CoreRuntimeAdapter::placeIdForName(const QString &name) const
{
    if (!m_document) {
        return QString();
    }
    QList<PlaceData> places = m_document->places();
    for (int i = 0; i < places.size(); ++i) {
        if (places.at(i).name == name) {
            return places.at(i).id;
        }
    }
    return QString();
}

QString CoreRuntimeAdapter::transitionIdForName(const QString &name) const
{
    if (!m_document) {
        return QString();
    }
    QList<TransitionData> transitions = m_document->transitions();
    for (int i = 0; i < transitions.size(); ++i) {
        if (transitions.at(i).name == name) {
            return transitions.at(i).id;
        }
    }
    return QString();
}

void CoreRuntimeAdapter::addLocalLog(const QString &text)
{
    LogEntry entry;
    entry.time = QDateTime::currentDateTime();
    entry.text = text;
    m_localLog.append(entry);
    while (m_localLog.size() > 200) {
        m_localLog.removeFirst();
    }
}

QString CoreRuntimeAdapter::valueToString(const EvalValue &value)
{
    if (value.type == EvalValue::Type::Bool) {
        return value.bool_value ? "true" : "false";
    }
    if (value.type == EvalValue::Type::String) {
        return QString::fromStdString(value.string_value);
    }
    return QString::number(value.int_value);
}
