// autor: Jurišinová Daniela (xjurisd00)

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <cstdint>
#include <string>
#include <vector>

/**
 * @file logger.hpp
 * @brief Runtime event logging for interpreted Petrinets
 */

enum class LoggerEventType {
    RuntimeStarted,
    RuntimeStopped,
    InputReceived,
    TransitionFired,
    TimerScheduled,
    TimerExpired,
    TimerIgnored,
    OutputProduced,
    TokenChanged,
    VariableChanged,
    Info,
    Warning,
    Error
};

struct LoggerEntry {
    int64_t time_ms = 0;
    LoggerEventType type = LoggerEventType::Info;
    std::string message;
};

class Logger {
public:
    void log(int64_t time_ms, LoggerEventType type, const std::string& message);
    const std::vector<LoggerEntry>& entries() const;
    void clear();
    void set_print(bool enabled);
private:
    std::vector<LoggerEntry> m_entries;
    bool m_print = true;
};

std::string log_event_type_2_str(LoggerEventType type);

#endif