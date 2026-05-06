// autor: Jurišinová Daniela (xjurisd00)

#include "logger.hpp"
#include <iostream>

/**
 * Method adds new element (LoggerEntry) into list of logging records.
 * It also prints it into console if the print si enabled
 *
 * @param time_ms - time of event 
 * @param type - type of logging event
 * @param message - text that describes the event
 */
void Logger::log(int64_t time_ms, LoggerEventType type, const std::string& message) {
    m_entries.push_back(LoggerEntry{time_ms, type, message});
    if (m_print) {
        std::cout << "[" << time_ms << " ms] " << log_event_type_2_str(type) << ": "
                << message << std::endl;
    }
}

/**
 * Helper method returns all saved logging records
 *
 * @return pointer to vector with LoggerEntry objects 
 */
const std::vector<LoggerEntry>& Logger::entries() const {
    return m_entries;
}

/**
 * Helper method that clears all saved logginf records
 */
void Logger::clear() {
    m_entries.clear();
}

/**
 * Helper method that enables printing flag
 */
void Logger::set_print(bool enabled) {
    m_print = enabled;
}

/**
 * Helper method that returns string format of event type
 *
 * @return string with text interpretation of logging event type
 */
std::string log_event_type_2_str(LoggerEventType type) {
    switch (type) {
        case LoggerEventType::RuntimeStarted: return "runtime-started";
        case LoggerEventType::RuntimeStopped: return "runtime-stopped";
        case LoggerEventType::InputReceived: return "input";
        case LoggerEventType::TransitionFired: return "transition-fired";
        case LoggerEventType::TimerScheduled: return "timer-scheduled";
        case LoggerEventType::TimerExpired: return "timer-expired";
        case LoggerEventType::TimerIgnored: return "timer-ignored";
        case LoggerEventType::OutputProduced: return "output";
        case LoggerEventType::TokenChanged: return "token-changed";
        case LoggerEventType::VariableChanged: return "variable-changed";
        case LoggerEventType::Info: return "info";
        case LoggerEventType::Warning: return "warning";
        case LoggerEventType::Error: return "error";
    }
    return "unknown";
}