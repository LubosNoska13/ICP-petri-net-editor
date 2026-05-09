// autor: Jurišinová Daniela (xjurisd00)

#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "evaluator.hpp"
#include "logger.hpp"
#include "model.hpp"
#include <vector>
#include <string>
#include <cstdint>
#include <functional>
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

struct RuntimeOutputValue {
    bool defined = false;
    std::string value;
    int64_t last_update_ms = 0;
};

struct RuntimeVariable {
    std::string type = "int";
    EvalValue value = EvalValue::integer(0);    
};

struct PendingTimer {
    std::string transition_name;
    std::string event_name;
    std::string guard_expression;
    std::string delay_expression;
    int64_t delay_ms = 0;
    int64_t scheduled_at_ms = 0;
    int64_t due_at_ms = 0;
    // priority among timers with same time
    uint64_t sequence = 0;

    // timers comparison
    bool operator<(const PendingTimer& other) const {
        if (due_at_ms != other.due_at_ms) {
            return due_at_ms > other.due_at_ms;
        }
        return sequence > other.sequence;
    }
};

struct StateSnapshot {
    int64_t now_ms = 0;
    std::map<std::string, int> marking;
    std::map<std::string, RuntimeInputValue> inputs;
    std::map<std::string, RuntimeOutputValue> outputs;
    std::map<std::string, RuntimeVariable> variables;
    std::vector<std::string> enabled_transitions;
    std::vector<PendingTimer> pending_timers;
    const std::vector<LoggerEntry>* event_log = nullptr;
};

// callbacks to inform about state change, transition fire, output, timer setting or error
using StateChangedCallback = std::function<void(const StateSnapshot&)>;
using TransitionFiredCallback = std::function<void(const Transition&, const StateSnapshot&)>;
using OutputProducedCallback = std::function<void(const std::string&, const std::string&, const StateSnapshot&)>;
using TimerScheduledCallback = std::function<void(const PendingTimer&, const StateSnapshot&)>;
using RuntimeErrorCallback = std::function<void(const std::string&, const StateSnapshot&)>;

class PetriRuntime : public EvaluationInterface {
public:
    // runtime constructor and initialization
    explicit PetriRuntime(PetriNet net);
    void initialize(bool run_initial_place_actions = true);

    // runtime management
    void inject_input(const std::string& name, const std::string& value);
    void advance_time(int64_t delta_ms);
    bool step();
    void run_stabilization(const std::optional<std::string>& triggering_event = std::nullopt);

    // state monitoring and recording
    StateSnapshot snapshot() const;
    std::vector<std::string> current_enabled_transitions(const std::optional<std::string>& 
            triggering_event = std::nullopt) const;
    const Logger& logger() const;
    Logger& logger();

    // callback setting
    void set_on_state_changed(StateChangedCallback callback);
    void set_on_transition_fired(TransitionFiredCallback callback);
    void set_on_output_produced(OutputProducedCallback callback);
    void set_on_timer_scheduled(TimerScheduledCallback callback);
    void set_on_error(RuntimeErrorCallback callback);

    // interface for resolving and actions
    bool input_defined(const std::string& name) const override;
    std::string input_value(const std::string& name) const override;
    int token_count(const std::string& place_name) const override;
    int64_t ms_elapsed(const std::string& object_name) const override;
    int64_t ms_now() const override;
    bool has_variable(const std::string& name) const override;
    EvalValue variable_value(const std::string& name) const override;
    std::string variable_type(const std::string& name) const override;
    void set_variable_value(const std::string& name, const EvalValue& value) override;
    void emit_output(const std::string& name, const std::string& value) override;

private:
    PetriNet m_net;
    Evaluator m_evaluator;
    Logger m_logger;
    int64_t m_now_ms = 0;
    uint64_t m_timer_sequence = 0;
    std::map<std::string, int> m_marking;
    std::map<std::string, RuntimeInputValue> m_inputs;
    std::map<std::string, RuntimeOutputValue> m_outputs;
    std::map<std::string, RuntimeVariable> m_variables;
    std::map<std::string, int64_t> m_place_changed_at;
    std::map<std::string, int64_t> m_transition_enabled_since;
    std::priority_queue<PendingTimer> m_timers;

    // callbacks
    StateChangedCallback m_on_state_changed;
    TransitionFiredCallback m_on_transition_fired;
    OutputProducedCallback m_on_output_produced;
    TimerScheduledCallback m_on_timer_scheduled;
    RuntimeErrorCallback m_on_error;

    // transitions control
    bool transition_enabled(const Transition& transition, const std::optional<std::string>& 
            triggering_event) const;
    bool transition_enabled_for_monitor(const Transition& transition) const;
    bool enough_tokens(const Transition& transition) const;
    std::vector<const Transition*> immediate_enabled(const std::optional<std::string>& triggering_event) const;
    std::vector<const Transition*> max_independent_set(const std::vector<const Transition*>& candidates) const;

    // transitions fire and token manipulation
    void fire_transition(const Transition& transition);
    void consume_input_tokens(const Transition& transition);
    void produce_output_tokens(const Transition& transition);
    void execute_place_action_for_added_tokens(const std::string& place_name, int count);

    // delayed transitions
    void schedule_delayed_transitions(const std::optional<std::string>& triggering_event);
    bool timer_scheduled(const PendingTimer& timer) const;
    void process_due_timers();

    // helper methods
    const Transition* find_transition_or_null(const std::string& name) const;
    EvalValue initial_value_for_variable(const Variable& variable) const;

    // errors and notifications
    void notify_state_changed() const;
    void notify_error(const std::string& message);
};

#endif

