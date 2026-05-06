// autor: Jurišinová Daniela (xjurisd00)

#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "evaluator.hpp"
#include "logger.hpp"
#include "model.hpp"
#include <vector>
#include <string>
#include <cstdint>
#include <set>
#include <map>
#include <queue>
#include <optional>

/**
 * @file runtime.hpp
 * @brief Runtime semantic of Petrinets
 */

struct RuntimeInputValue {
    bool defined = false;
    std::string value;
    int64_t last_update_ms = 0;
};

struct RuntimeVariable {
    int value = 0;
};

struct PendingTimer {
    std::string transition_name;
    int64_t scheduled_at_ms = 0;
    int64_t due_at_ms = 0;
    std::string event_name;

    bool operator<(const PendingTimer& other) const {
        return due_at_ms > other.due_at_ms;
    }
};

struct StateSnapshot {
    std::map<std::string, int> marking;
    std::map<std::string, RuntimeInputValue> inputs;
    std::map<std::string, RuntimeVariable> variables;
    std::vector<PendingTimer> pending_timers;
};

class PetriRuntime : public EvaluationInterface {
public:
    explicit PetriRuntime(PetriNet net);
    void initialize();
    void inject_input(const std::string& name, const std::string& value);
    void advance_time(int64_t delta_ms);
    void run_stabilization(const std::optional<std::string>& triggering_event = std::nullopt);

    StateSnapshot snapshot() const;
    const Logger& logger() const;
    Logger& logger();

    // overriding inherited methods
    bool input_defined(const std::string& name) const override;
    std::string input_value(const std::string& name) const override;
    int token_count(const std::string& place_name) const override;
    int64_t ms_elapsed(const std::string& object_name) const override;
    int64_t ms_now() const override;
    bool has_variable(const std::string& name) const override;
    int variable_value(const std::string& name) const override;
    void set_variable_value(const std::string& name, int value) override;
    void emit_output(const std::string& name, const std::string& value) override;

private:
    PetriNet m_net;
    Evaluator m_evaluator;
    Logger m_logger;
    int64_t m_now_ms = 0;
    std::map<std::string, int> m_marking;
    std::map<std::string, RuntimeInputValue> m_inputs;
    std::map<std::string, RuntimeVariable> m_variables;
    std::map<std::string, int64_t> m_place_changed_at;
    std::map<std::string, int64_t> m_transition_enabled_since;
    std::priority_queue<PendingTimer> m_timers;

    bool transition_enabled(const Transition& transition, 
            const std::optional<std::string>& triggering_event) const;
    bool enough_tokens(const Transition& transition) const;
    std::vector<const Transition*> collect_immediate_enabled(
            const std::optional<std::string>& triggering_event);
    std::vector<const Transition*> max_independent_set(
            const std::vector<const Transition*>& candidates) const;
    void fire_transition(const Transition& transition);
    void consume_input_tokens(const Transition& transition);
    void produce_output_tokens(const Transition& transition);
    void schedule_delayed_transitions(const std::optional<std::string>& triggering_event);
    bool timer_scheduled(const std::string& transition_name) const;
    void process_due_timers();
    const Transition* find_transition_or_null(const std::string& name) const;
};

#endif

