// autor: Jurišinová Daniela (xjurisd00)

#include "logger.hpp"
#include <iostream>
#include <utility>
#include <sstream>

namespace {

/**
* Helper function escapes special characters in string for JSON output
*
* @param value - the string value to escape
* @return escaped string that can be safely used in JSON
*/
std::string json_escape(const std::string& value) {
    std::ostringstream out;
    for (char character : value) {
        switch (character) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                // other control characters escape
                if (static_cast<unsigned char>(character) < 0x20) {
                    out << "\\u" << std::hex << static_cast<int>(character);
                } 
                // normal character output
                else {
                    out << character;
                }
                break;
        }
    }
    return out.str();
}

/**
* Helper function checks whether logger entry matches optional event type filter
*
* @param entry - the logger entry to check
* @param type_filter - optional event type used for filtering
* @return true if filter is empty or entry type matches filter otherwise false
*/
bool entry_matches(const LoggerEntry& entry, std::optional<LoggerEventType> type_filter) {
    return !type_filter.has_value() || entry.type == type_filter.value();
}

/**
* Helper function appends non-empty logger details to text output stream
*
* @param out - the output stream where details are written
* @param details - the logger details containing optional text fields
*/
void append_text_details(std::ostream& out, const LoggerDetails& details) {
    bool has_detail = false;

    // creates append lambfa function
    auto append_field = [&](const std::string& label, const std::string& value) {
        if (value.empty()) {
            return;
        }
        out << (has_detail ? ", " : " [") << label << "=" << value;
        has_detail = true;
    };

    // appends detail fields
    append_field("transition", details.transition_name);
    append_field("place", details.place_name);
    append_field("old", details.old_value);
    append_field("new", details.new_value);

    if (has_detail) {
        out << "]";
    }
}

}

/**
 * Method calls other log function with details but leaves the details empty
 * ##note to myself: check if merge better
 *
 * @param time_ms - time of event 
 * @param type - type of logging event
 * @param message - text that describes the event
 */
void Logger::log(int64_t time_ms, LoggerEventType type, const std::string& message) {
    log(time_ms, type, message, LoggerDetails{});
}

/**
 * Method adds new element (LoggerEntry) into list of logging records.
 * It also prints it into console if the print si enabled
 *
 * @param time_ms - time of event
 * @param type - type of logging event
 * @param message - text that describes the event
 * @param details - optional structured metadata
 */
void Logger::log(int64_t time_ms, LoggerEventType type, const std::string& message,
            const LoggerDetails& details) {

    m_entries.push_back(LoggerEntry{time_ms, type, message, details});
    const LoggerEntry& entry = m_entries.back();

    // prints if enabled
    if (m_print) {
        std::cout << "[" << time_ms << " ms] " << log_event_type_2_str(type) << ": " << message;
        append_text_details(std::cout, details);
        std::cout << std::endl;
    }

    // callback check
    if (m_callback) {
        m_callback(entry);
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
 * Helper method returns saved logging records with selected type.
 *
 * @param type - selected logging event type
 * @return vector with matching LoggerEntry objects
 */
std::vector<LoggerEntry> Logger::entries(LoggerEventType type) const {
    return entries(std::optional<LoggerEventType>{type});
}

/**
 * Helper method returns saved logging records, optionally filtered by type.
 *
 * @param type_filter - optional selected logging event type
 * @return vector with matching LoggerEntry objects
 */
std::vector<LoggerEntry> Logger::entries(std::optional<LoggerEventType> type_filter) const {
    std::vector<LoggerEntry> filtered;
    for (const auto& entry : m_entries) {
        if (entry_matches(entry, type_filter)) {
            filtered.push_back(entry);
        }
    }
    return filtered;
}

/**
 * Exports log into readable text format. Optionally also filters entries by type
 *
 * @param type_filter - optional selected logging event type
 * @return text representation of matching log entries
 */
std::string Logger::export_text(std::optional<LoggerEventType> type_filter) const {
    std::ostringstream out;
    for (const auto& entry : m_entries) {
        // filters by type
        if (!entry_matches(entry, type_filter)) {
            continue;
        }
        out << "[" << entry.time_ms << " ms] " << log_event_type_2_str(entry.type) << ": " << entry.message;
        append_text_details(out, entry.details);
        out << '\n';
    }
    return out.str();
}

/**
 * Exports log into JSON array.
 *
 * @param type_filter - optional selected logging event type
 * @return JSON representation of matching log entries
 */
std::string Logger::export_json(std::optional<LoggerEventType> type_filter) const {
    std::ostringstream out;
    out << "[\n";
    bool first = true;

    for (const auto& entry : m_entries) {
        // filters by type
        if (!entry_matches(entry, type_filter)) {
            continue;
        }

        // adds comma separator
        if (!first) {
            out << ",\n";
        }
        first = false;

        out << "  {\n";
        out << "    \"time_ms\": " << entry.time_ms << ",\n";
        out << "    \"type\": \"" << json_escape(log_event_type_2_str(entry.type)) << "\",\n";
        out << "    \"message\": \"" << json_escape(entry.message) << "\",\n";
        out << "    \"transition_name\": \"" << json_escape(entry.details.transition_name) << "\",\n";
        out << "    \"place_name\": \"" << json_escape(entry.details.place_name) << "\",\n";
        out << "    \"old_value\": \"" << json_escape(entry.details.old_value) << "\",\n";
        out << "    \"new_value\": \"" << json_escape(entry.details.new_value) << "\"\n";
        out << "  }";
    }
    out << "\n]";
    return out.str();
}

/**
 * Helper method that clears all saved logginf records
 */
void Logger::clear() {
    m_entries.clear();
}

/**
 * Helper method that sets callback invoked after every new log entry.
 */
void Logger::set_callback(LoggerCallback callback) {
    m_callback = std::move(callback);
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