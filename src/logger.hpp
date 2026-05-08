// autor: Jurišinová Daniela (xjurisd00)

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <functional>

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

struct LoggerDetails {
    std::string transition_name;
    std::string place_name;
    std::string old_value;
    std::string new_value;
};

struct LoggerEntry {
    int64_t time_ms = 0;
    LoggerEventType type = LoggerEventType::Info;
    std::string message;
    LoggerDetails details;
};

using LoggerCallback = std::function<void(const LoggerEntry&)>;

class Logger {
public:
    void log(int64_t time_ms, LoggerEventType type, const std::string& message);
    void log(int64_t time_ms, LoggerEventType type, const std::string& message,
             const LoggerDetails& details);
    const std::vector<LoggerEntry>& entries() const;
    std::vector<LoggerEntry> entries(LoggerEventType type) const;
    std::vector<LoggerEntry> entries(std::optional<LoggerEventType> type_filter) const;
    std::string export_text(std::optional<LoggerEventType> type_filter = std::nullopt) const;
    std::string export_json(std::optional<LoggerEventType> type_filter = std::nullopt) const;
    void clear();
    void set_callback(LoggerCallback callback);
    void set_print(bool enabled);
private:
    std::vector<LoggerEntry> m_entries;
    LoggerCallback m_callback;
    bool m_print = false;
};

std::string log_event_type_2_str(LoggerEventType type);

#endif