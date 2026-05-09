/**
 * @file DummyRuntimeAdapter.cpp
 * @author Lubos and project team
 * @brief Implementation of the demonstration runtime adapter.
 * @details Manual code. Borrowed code: none.
 */

#include "core_api/DummyRuntimeAdapter.h"

DummyRuntimeAdapter::DummyRuntimeAdapter()
    : m_document(0), m_running(false)
{
}

bool DummyRuntimeAdapter::start(PetriNetDocument *document)
{
    m_document = document;
    m_running = (document != 0);
    m_inputs.clear();
    if (m_document) {
        m_inputs = m_document->inputs();
    }
    addLog("Dummy runtime started.");
    return m_running;
}

void DummyRuntimeAdapter::stop()
{
    if (m_running) {
        addLog("Dummy runtime stopped.");
    }
    m_running = false;
}

void DummyRuntimeAdapter::injectInput(const QString &name, const QString &value)
{
    bool found = false;
    for (int i = 0; i < m_inputs.size(); ++i) {
        if (m_inputs[i].name == name) {
            m_inputs[i].lastValue = value;
            found = true;
        }
    }
    if (!found) {
        IoData input;
        input.name = name;
        input.lastValue = value;
        m_inputs.append(input);
    }
    addLog("Input injected: " + name + " = " + value);
}

RuntimeSnapshot DummyRuntimeAdapter::snapshot() const
{
    RuntimeSnapshot result;
    if (!m_document) {
        result.logEntries = m_log;
        return result;
    }

    QList<PlaceData> places = m_document->places();
    for (int i = 0; i < places.size(); ++i) {
        MarkingEntry entry;
        entry.placeId = places.at(i).id;
        entry.placeName = places.at(i).name;
        entry.tokens = places.at(i).tokens;
        result.marking.append(entry);
    }

    result.inputs = m_inputs;
    result.outputs = m_document->outputs();
    result.variables = m_document->variables();
    result.logEntries = m_log;
    return result;
}

bool DummyRuntimeAdapter::isRunning() const
{
    return m_running;
}

void DummyRuntimeAdapter::addLog(const QString &text)
{
    LogEntry entry;
    entry.time = QDateTime::currentDateTime();
    entry.text = text;
    m_log.append(entry);
    while (m_log.size() > 200) {
        m_log.removeFirst();
    }
}
