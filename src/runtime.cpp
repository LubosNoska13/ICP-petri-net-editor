// autor: Jurišinová Daniela (xjurisd00)

#include "runtime.hpp"
#include <cstdlib>
#include <stdexcept> 
#include <algorithm>
#include <sstream>
#include <cctype>

namespace {

std::string eval_to_string(const EvalValue& value) {
    return value.as_string();
}

}

/**
 * Constructor of class PetriRuntime
 */
PetriRuntime::PetriRuntime(PetriNet net) 
    : m_net(std::move(net)) {
}

/**
 * Method for initialization of the Petrinet state. Writes in log which 
 * Petrinet was initialized, for all places sets initial tokens, for all
 * inpnuts sets runtime values and then for each variable converts value
 * to integer if it's possible. Finally starts stabilization.
 */
void PetriRuntime::initialize(bool run_initial_place_actions) {
    m_logger.log(m_now_ms, LoggerEventType::RuntimeStarted, "Runtime started for Petrinet: " + m_net.name);
    m_marking.clear();
    m_inputs.clear();
    m_outputs.clear();
    m_variables.clear();
    m_place_changed_at.clear();
    m_transition_enabled_since.clear();
    m_timers = std::priority_queue<PendingTimer>{};
    m_timer_sequence = 0;

    // sets initial tokens for all places and writes up change time
    for (const auto& place : m_net.places) {
        m_marking[place.name] = place.initial_tokens;
        m_place_changed_at[place.name] = m_now_ms;
    }

    // for each input sets runtime value
    for (const auto& input : m_net.inputs) {
        m_inputs[input.name] = RuntimeInputValue{};
    }

    // for each output sets runtime value
    for (const auto& output : m_net.outputs) {
        m_outputs[output.name] = RuntimeOutputValue{};
    }

    // for each variable sets initial value 
    for (const auto& variable : m_net.variables) {
        RuntimeVariable runtime_variable;
        // no specified type converts to int
        if (variable.type.empty()) {
            runtime_variable.type = "int";
        } 
        else {
            runtime_variable.type = variable.type;
        }
        runtime_variable.value = initial_value_for_variable(variable);
        m_variables[variable.name] = runtime_variable;
    }

    // if stated, executes possible actions for all places
    if (run_initial_place_actions) {
        for (const auto& place : m_net.places) {
            execute_place_action_for_added_tokens(place.name, place.initial_tokens);
        }
    }
    run_stabilization(std::nullopt);
    notify_state_changed();
}

/**
 * Method injects input value into runtime. First checks if the input exists
 * then sets it's features. At the end writes in log input and it's value.
 * Finally starts stabilization.
 *
 * @param name - name of the input
 * @param value - value assigned to input
 */
void PetriRuntime::inject_input(const std::string& name, const std::string& value) {
    // input exists check
    if (m_inputs.find(name) == m_inputs.end()) {
        notify_error("Unknown input: " + name);
        throw std::runtime_error("Unknown input: " + name);
    }

    // sets input features
    m_inputs[name].defined = true;
    m_inputs[name].value = value;
    m_inputs[name].last_update_ms = m_now_ms;
    m_logger.log(m_now_ms, LoggerEventType::InputReceived, "Input " + name + " = " + value);
    run_stabilization(name);
    notify_state_changed();
}

/**
 * Method advances internal runtime clock by time in argument
 * if it's not negative and calls processes that should happen 
 * in the shifted time interval
 *
 * @param delta_ms - time to be added
 */
void PetriRuntime::advance_time(int64_t delta_ms) {
    // negative shift check
    if (delta_ms < 0) {
        notify_error("Can't move runtime time backwards");
        throw std::runtime_error("Can't move runtime time backwards");
    }

    // shifts time 
    m_now_ms += delta_ms;
    step();
}


/**
 * Method performs one runtime step by processing due timers,
 * running stabilization and notifying about state changes
 * if any change happened during the step
 *
 * @return true if runtime state changed otherwise false
 */
bool PetriRuntime::step() {
    const std::size_t old_log_size = m_logger.entries().size();
    process_due_timers();
    run_stabilization(std::nullopt);

    // change check
    const bool changed = m_logger.entries().size() != old_log_size;
    if (changed) {
        notify_state_changed();
    }
    return changed;
}

/**
 * Method runs Petrinet stabiliyation proccess. In loop fires transitions 
 * that are enabled and doesn't influence each other until there are no
 * independent transitions. Finally call method for planning of delayed
 * transitions 
 *
 * @param triggering_event - optional event that triggers stabilization
 */
void PetriRuntime::run_stabilization(const std::optional<std::string>& triggering_event) {
    std::optional<std::string> current_event = triggering_event;
    while (true) {
        auto candidates = immediate_enabled(current_event);
        auto selected = max_independent_set(candidates);

        // no transition can be made and net is stabilized
        if (selected.empty()) {
            break;
        }

        // fires independent transitions
        for (const auto* transition : selected) {
            fire_transition(*transition);
        }
        current_event = std::nullopt;
    }
    // plans other transitions
    schedule_delayed_transitions(triggering_event);
}

/**
 * Method creates a snapshot of the current state of Petrinet.
 * Copies marking, time, inputs, outputs, variables, enabled transitions 
 * entries and pending timers if there are any
 *
 * @return snapshot with current state of runtime
 */
StateSnapshot PetriRuntime::snapshot() const {
    StateSnapshot snap;
    snap.now_ms = m_now_ms;
    snap.marking = m_marking;
    snap.inputs = m_inputs;
    snap.outputs = m_outputs;
    snap.variables = m_variables;
    snap.enabled_transitions = current_enabled_transitions();
    snap.event_log = &m_logger.entries();

    auto timers_copy = m_timers;
    while (!timers_copy.empty()) {
        snap.pending_timers.push_back(timers_copy.top());
        timers_copy.pop();
    }
    return snap;
}

/**
 * Method checks enabled transitions. If triggering event is provided, 
 * transitions are checked against that event otherwise transitions are 
 * checked for monitor activation.
 *
 * @param triggering_event - optional event that may trigger transitions
 * @return vector of names of enabled transitions
 */
std::vector<std::string> PetriRuntime::current_enabled_transitions(const std::optional<std::string>& 
            triggering_event) const {
    
    std::vector<std::string> result;
    for (const auto& transition : m_net.transitions) {
        bool enabled;
        
        // triggering event check
        if (triggering_event.has_value()) {
            enabled = transition_enabled(transition, triggering_event);
        } 
        else {
            enabled = transition_enabled_for_monitor(transition);
        }

        // adds transition to list of enabled transitions
        if (enabled) {
            result.push_back(transition.name);
        }
    }
    return result;
}

/**
 * Method returns runtime logger
 * note: constant variant of the method
 *
 * @return reference to the logger
 */
const Logger& PetriRuntime::logger() const {
    return m_logger;
}

/**
 * Method returns runtime logger
 *
 * @return references to the logger
 */
Logger& PetriRuntime::logger() {
    return m_logger;
}

/**
 * Method sets callback that is called when runtime state changes.
 *
 * @param callback - callback function to be stored
 */
void PetriRuntime::set_on_state_changed(StateChangedCallback callback) {
    m_on_state_changed = std::move(callback);
}

/**
 * Method sets callback that is called when transition was fired
 *
 * @param callback - callback function to be stored
 */
void PetriRuntime::set_on_transition_fired(TransitionFiredCallback callback) {
    m_on_transition_fired = std::move(callback);
}

/**
 * Method sets callback that is called when net prodiced output
 *
 * @param callback - callback function to be stored
 */
void PetriRuntime::set_on_output_produced(OutputProducedCallback callback) {
    m_on_output_produced = std::move(callback);
}

/**
 * Method sets callback that is called when some timer is set
 *
 * @param callback - callback function to be stored
 */
void PetriRuntime::set_on_timer_scheduled(TimerScheduledCallback callback) {
    m_on_timer_scheduled = std::move(callback);
}

/**
 * Method sets callback that is called when some error occures
 *
 * @param callback - callback function to be stored
 */
void PetriRuntime::set_on_error(RuntimeErrorCallback callback) {
    m_on_error = std::move(callback);
}

/**
 * Method checks if input in argument is defined
 *
 * @param name - name of the input to be checked
 * @return true if the input with name exists otherwise false
 */
bool PetriRuntime::input_defined(const std::string& name) const {
    auto input = m_inputs.find(name);
    return input != m_inputs.end() && input->second.defined;
}

/**
 * Method returns value of the input
 *
 * @param name - name of the input with value is checked
 * @return current value of the input  
 */
std::string PetriRuntime::input_value(const std::string& name) const {
    auto input = m_inputs.find(name);
    if (input == m_inputs.end() || !input->second.defined) {
        return "";
    }
    return input->second.value;
}

/**
 * Method returns the number of the tokens in place in argument
 *
 * @param place_name - name of the place which tokens are checked
 * @return 0 if place doesn't exists otherwise count of tokens stored in place 
 */
int PetriRuntime::token_count(const std::string& place_name) const {
    auto place = m_marking.find(place_name);
    if (place == m_marking.end()) {
        return 0;
    }
    return place->second;
}

/**
 * Method returns how many miliseconds passed after last change or
 * activation of object.
 *
 * @param object_name - name of the object which elapsed time is checked
 * @return elapsed time of the objext
 */
int64_t PetriRuntime::ms_elapsed(const std::string& object_name) const {
    // changed object place check
    auto place = m_place_changed_at.find(object_name);
    if (place != m_place_changed_at.end()) {
        return m_now_ms - place->second;
    }

    // changed object transition check
    auto transition = m_transition_enabled_since.find(object_name);
    if (transition != m_transition_enabled_since.end()) {
        return m_now_ms - transition->second;
    }

    // changed object not found
    return 0;
}

/**
 * Method returns current time in runtime
 * 
 * @return currenct time 
 */
int64_t PetriRuntime::ms_now() const {
    return m_now_ms;
}

/**
 * Method checks if variable in argument exists
 *
 * @param name - name of the variable to be checked
 * @return true if variable exists otherwise false
 */
bool PetriRuntime::has_variable(const std::string& name) const {
    return m_variables.find(name) != m_variables.end();
}

/**
 * Method checks if variable exists and if it does it returns 
 * variable value
 *
 * @param name - name of the variable which value is checked
 * @return value of the variable or error if variable doesn't exists
 */
EvalValue PetriRuntime::variable_value(const std::string& name) const {
    auto variable = m_variables.find(name);
    if (variable == m_variables.end()) {
        throw std::runtime_error("Unknown variable: " + name);
    }
    return variable->second.value;
}

/**
 * Method returns type of variable with given name. If variable doesn't 
 * exist error occures
 *
 * @param name - name of variable
 * @return type of variable
 */
std::string PetriRuntime::variable_type(const std::string& name) const {
    auto variable = m_variables.find(name);
    if (variable == m_variables.end()) {
        throw std::runtime_error("Unknown variable: " + name);
    }
    return variable->second.type;
}

/**
 * Method sets value of the variable in argument. First checks if variable
 * exists in Petrinet and then checks if the value really changed. If it did
 * then writes in log old and new value of the variable.
 *
 * @param name - name of the variable to be set
 * @param value - value which is gonna be value of the variable
 */
void PetriRuntime::set_variable_value(const std::string& name, const EvalValue& value) {
    // variable exists check
    auto variable = m_variables.find(name);
    if (variable == m_variables.end()) {
        notify_error("Unknown variable: " + name);
        throw std::runtime_error("Unknown variable: " + name);
    }

    std::string old_value = eval_to_string(variable->second.value);
    variable->second.value = value;
    std::string new_value = eval_to_string(value);

    // write change in log
    if (old_value != new_value) {
        LoggerDetails details;
        details.old_value = old_value;
        details.new_value = new_value;
        m_logger.log(m_now_ms, LoggerEventType::VariableChanged, "Variable " + name + " changed from " 
                + old_value + " to " + new_value, details);
    }
}

/**
 * Method writes in log which output and it's value is emitted
 *
 * @param name - name of the output
 * @param value - value to be emitted in the output
 */
void PetriRuntime::emit_output(const std::string& name, const std::string& value) {
    // empty output check
    if (m_outputs.find(name) == m_outputs.end()) {
        m_outputs[name] = RuntimeOutputValue{};
    }

    m_outputs[name].defined = true;
    m_outputs[name].value = value;
    m_outputs[name].last_update_ms = m_now_ms;

    // write production in log
    LoggerDetails details;
    details.old_value = name;
    details.new_value = value;
    m_logger.log(m_now_ms, LoggerEventType::OutputProduced, "Output " + name + " = " + value, details);

    if (m_on_output_produced) {
        m_on_output_produced(name, value, snapshot());
    }
}

/**
 * Method checks if transition is enabled to be activated. To be activated transition
 * needs places with enough tokens and correct event if stated.
 *
 * @param transition - transition that is checked
 * @param triggering_event - event that triggers transition if stated
 * @return true if transition is enabled otherwise false
 */
bool PetriRuntime::transition_enabled(const Transition& transition, 
        const std::optional<std::string>& triggering_event) const {

    // enough tokens of places for transition check
    if (!enough_tokens(transition)) {
        return false;
    }

    if (transition.condition.has_event()) {
        // has needed event check
        if (!triggering_event.has_value()) {
            return false;
        }
        // event not valid for the transition
        if (transition.condition.event_name != triggering_event.value()) {
            return false;
        }
    }

    // after constant deletion calls guard evaluation and checks for errors
    try {
        PetriRuntime& self = const_cast<PetriRuntime&>(*this);
        return m_evaluator.guard_evaluation(transition.condition.guard_expression, self);
    } 
    catch (...) {
        return false;
    }
}

/**
 * Method checks whether transition has enough tokens in its input places
 * and its guard expression can be successfully evaluated as true
 *
 * @param transition - transition to be checked
 * @return true if transition is enabled for monitor otherwise false
 */
bool PetriRuntime::transition_enabled_for_monitor(const Transition& transition) const {
    // enough tokens of places for transition check
    if (!enough_tokens(transition)) {
        return false;
    }

    // tries if enabled
    try {
        PetriRuntime& self = const_cast<PetriRuntime&>(*this);
        return m_evaluator.guard_evaluation(transition.condition.guard_expression, self);
    }
    catch (...) {
        return false;
    }
}

/**
 * Method checks if all input places of transition have enough tokens
 * for transition to be enabled. 
 *
 * @param transition - transition that needed tokens are checked
 * @return true if transition has enough tokens to be enabled
 */
bool PetriRuntime::enough_tokens(const Transition& transition) const {
    for (const auto& arc : transition.input_arcs) {
        if (token_count(arc.place_name) < arc.weight) {
            return false;
        }
    }
    return true;
}

/**
 * Method returns all current enabled immediate transitions. For each transition
 * checks if it is immediate and enabled with event in argument if it is stated.
 * Finally method sorts all enabled immediate transition by their priority and name.
 *
 * @param triggering_event - optional event to trigger the transitions
 * @return vector of all current enabled immediate transitions
 */
std::vector<const Transition*> PetriRuntime::immediate_enabled(const std::optional<std::string>& 
            triggering_event) const {

    std::vector<const Transition*> result;
    // continuous enabled and immediate check
    for (const auto& transition : m_net.transitions) {
        if (transition.condition.has_delay()) {
            continue;
        }

        if (transition_enabled(transition, triggering_event)) {
            result.push_back(&transition);
        }
    }

    // sorting in ascending order by priority and name
    std::sort(result.begin(), result.end(), [](const Transition* transition1, const Transition* transition2) {
        if (transition1->priority != transition2->priority) {
            return transition1->priority < transition2->priority;
        }
        return transition1->name < transition2->name;
    });
    return result;
}

/**
 * Method selects all mutually independent enabled transitions. Continuously
 * checks every transition in argument list and checks if it's arc tokens are
 * enough. Then evaluates if the transition is independent or not and reserves
 * tokens if it is.
 *
 * @param candidates - all transitions to be checked
 * @return vector of selected transitions that can be fired together
 */
std::vector<const Transition*> PetriRuntime::max_independent_set(const std::vector<const Transition*>& 
            candidates) const {

    std::vector<const Transition*> selected;
    std::map<std::string, int> reserved_tokens;

    for (const auto* transition : candidates) {
        bool fits = true;

        // continous check if available tokens are enough
        for (const auto& arc : transition->input_arcs) {
            int reserved = reserved_tokens[arc.place_name];
            int available = token_count(arc.place_name);
            if (reserved + arc.weight > available) {
                fits = false;
                break;
            }
        }

        // needs more tokens than available tokens
        if (!fits) {
            continue;
        }

        // sets independent transitino and reserves used tokens 
        selected.push_back(transition);
        for (const auto& arc : transition->input_arcs) {
            reserved_tokens[arc.place_name] += arc.weight;
        }
    }
    return selected;
}

/**
 * Method fires a given transition. First writes in log the firing event, then consumes
 * required input tokens from input places. If the transition contains action
 * code, the action is executed. After that, output tokens are produced to
 * output places and callback is called if stated
 *
 * @param transition - transition to be fired
 */
void PetriRuntime::fire_transition(const Transition& transition) {
    // writes fire in log
    LoggerDetails details;
    details.transition_name = transition.name;
    m_logger.log(m_now_ms, LoggerEventType::TransitionFired, "Transition " + transition.name + " fired", 
            details);

    consume_input_tokens(transition);
    // executes transition if stated
    if (!transition.action_code.empty()) {
        try {
            m_evaluator.execute_action(transition.action_code, *this);
        }
        catch (const std::exception& error) {
            notify_error("Action of transition " + transition.name + " failed: " + error.what());
            throw;
        }
    }
    produce_output_tokens(transition);
    if (m_on_transition_fired) {
        m_on_transition_fired(transition, snapshot());
    }
}

/**
 * Method consumes input tokens required by a given transition. Checks
 * all input arcs of the transition, decreases the marking of each input place
 * based on arc weight and updates the time of changed places.
 * Each token change is writen in log.
 *
 * @param transition - transition whose input tokens are consumed
 */
void PetriRuntime::consume_input_tokens(const Transition& transition) {
    for (const auto& arc : transition.input_arcs) {
        int old_value = m_marking[arc.place_name];
        int new_value = old_value - arc.weight;
        m_marking[arc.place_name] = new_value;

        // changes time of last change and logs change
        if (old_value != new_value) {
            m_place_changed_at[arc.place_name] = m_now_ms;
        }
        
        // writes change in log
        LoggerDetails details;
        details.transition_name = transition.name;
        details.place_name = arc.place_name;
        details.old_value = std::to_string(old_value);
        details.new_value = std::to_string(new_value);
        m_logger.log(m_now_ms, LoggerEventType::TokenChanged, arc.place_name + ": " + 
                std::to_string(old_value) + " -> " + std::to_string(new_value), details);
    }
}

/**
 * Method produces output tokens generated by a given transition. Checks
 * all output arcs of the transition, increases the marking of each output 
 * place based on the arc weight and updates the time of changed places. 
 * Each token change is writen in log. If the output place has an add-token
 * action defined then action is executed for every produced token.
 *
 * @param transition - transition whose output tokens are produced
 */
void PetriRuntime::produce_output_tokens(const Transition& transition) {
    for (const auto& arc : transition.output_arcs) {
        int old_value = m_marking[arc.place_name];
        int new_value = old_value + arc.weight;
        m_marking[arc.place_name] = new_value;

        // changes time of last change and logs change
        if (old_value != new_value) {
            m_place_changed_at[arc.place_name] = m_now_ms;
        }
        
        // writes change in log
        LoggerDetails details;
        details.transition_name = transition.name;
        details.place_name = arc.place_name;
        details.old_value = std::to_string(old_value);
        details.new_value = std::to_string(new_value);
        m_logger.log(m_now_ms, LoggerEventType::TokenChanged, arc.place_name + ": " + 
                std::to_string(old_value) + " -> " + std::to_string(new_value), details);

        execute_place_action_for_added_tokens(arc.place_name, arc.weight);
    }
}

/**
 * Method executes action code of a place for each token added to that place.
 * If the place does not exist or has no add-token action defined, nothing happens.
 *
 * @param place_name - name of the place whose action should be executed
 * @param count - number of added tokens
 */
void PetriRuntime::execute_place_action_for_added_tokens(const std::string& place_name, int count) {
    const Place* place = m_net.find_place(place_name);
    // invalid place check
    if (place == nullptr || place->add_token_action.empty()) {
        return;
    }

    // executes action for each token
    for (int token = 0; token < count; ++token) {
        try {
            m_evaluator.execute_action(place->add_token_action, *this);
        }
        catch (const std::exception& error) {
            notify_error("Place action of " + place_name + " failed: " + error.what());
            throw;
        }
    }
}

/**
 * Method schedules timers for enabled delayed transitions. Checks all
 * transitions in the net and selects those with condition containing
 * delay. If the transition is bound to an event, the event must match the
 * triggering event inargument. The transition must be enabled and mustn't 
 * have a scheduled timer. The delay expression is evaluated and timer
 * is created and stored. Then writes timer in log and calls callback if stated
 *
 * @param triggering_event - optional event that triggered the scheduling check
 */
void PetriRuntime::schedule_delayed_transitions(const std::optional<std::string>& triggering_event) {
    for (const auto& transition : m_net.transitions) {
        // not delayed skip
        if (!transition.condition.has_delay()) {
            continue;
        }

        // event mismatch or absence skip 
        if (transition.condition.has_event()) {
            if (!triggering_event.has_value() || triggering_event.value() != transition.condition.event_name) {
                continue;
            }
        }

        // skips not enabled
        if (!transition_enabled(transition, triggering_event)) {
            continue;
        }

        int delay_ms = m_evaluator.int_evaluation(transition.condition.delay_expression, *this);
        if (delay_ms < 0) {
            notify_error("Negative delay for transition " + transition.name);
            throw std::runtime_error("Negative delay for transition " + transition.name);
        }

        // creates, fills and ads timer
        PendingTimer timer;
        timer.transition_name = transition.name;
        timer.event_name = transition.condition.event_name;
        timer.guard_expression = transition.condition.guard_expression;
        timer.delay_expression = transition.condition.delay_expression;
        timer.delay_ms = delay_ms;
        timer.scheduled_at_ms = m_now_ms;
        timer.due_at_ms = m_now_ms + delay_ms;
        timer.sequence = ++m_timer_sequence;

        // valid timer check
        if (timer_scheduled(timer)) {
            continue;
        }

        m_timers.push(timer);
        m_transition_enabled_since[transition.name] = m_now_ms;

        // writes timer set in log
        LoggerDetails details;
        details.transition_name = transition.name;
        details.old_value = std::to_string(delay_ms);
        details.new_value = std::to_string(timer.due_at_ms);
        m_logger.log(m_now_ms, LoggerEventType::TimerScheduled, "Timer for transition " + 
                transition.name + " scheduled at " + std::to_string(timer.due_at_ms) + " ms", details);

        if (m_on_timer_scheduled) {
            m_on_timer_scheduled(timer, snapshot());
        }
    }
}

/**
 * Method checks whether a timer is already scheduled. It creates a copy 
 * of the timer queue and searches through pending timers.
 *
 * @param transition_name - name of the transition to be checked
 * @return true if a timer for the transition is already scheduled otherwise false
 */
bool PetriRuntime::timer_scheduled(const PendingTimer& timer) const {
    auto copy = m_timers;
    while (!copy.empty()) {
        const PendingTimer current = copy.top();
        // finding timer
        if (    current.transition_name == timer.transition_name && 
                current.event_name == timer.event_name && 
                current.guard_expression == timer.guard_expression && 
                current.delay_expression == timer.delay_expression) {
            return true;
        }
        copy.pop();
    }
    return false;
}

/**
 * Method processes all timers with time that has already been reached. It
 * takes expired timers and tries to fire their corresponding transitions. 
 * 
 * If the transition no longer exists, the timer is ignored. 
 * If there are enough input tokens and the guard expression is true,
 * the transition is fired. 
 * Otherwise, the timer is ignored and the reason is writen in log
 */
void PetriRuntime::process_due_timers() {
    // checks all timers with expired time
    while (!m_timers.empty() && m_timers.top().due_at_ms <= m_now_ms) {
        PendingTimer timer = m_timers.top();
        m_timers.pop();

        // timer expiration writen in log
        LoggerDetails details;
        details.transition_name = timer.transition_name;
        m_logger.log(m_now_ms, LoggerEventType::TimerExpired, "Timer expired for transition " + 
                timer.transition_name, details);

        // transition exists check
        const Transition* transition = find_transition_or_null(timer.transition_name);
        if (transition == nullptr) {
            m_logger.log(m_now_ms, LoggerEventType::TimerIgnored, 
                    "Timer ignored because transition no longer exists: " + timer.transition_name, details);
            continue;
        }

        // with enough tokens tries evaluate guard and based on that fire transition 
        if (enough_tokens(*transition)) {
            try {
                if (m_evaluator.guard_evaluation(transition->condition.guard_expression, *this)) {
                    fire_transition(*transition);
                } 
                else {
                    m_logger.log(m_now_ms, LoggerEventType::TimerIgnored, "Timer ignored for transition " 
                            + transition->name + " because guard is false", details);
                }
            } 
            catch (const std::exception& error) {
                notify_error("Guard evaluation failed for transition " + transition->name + ": " + error.what());
                m_logger.log(m_now_ms, LoggerEventType::TimerIgnored, "Timer ignored for transition " + 
                        transition->name + " because guard evaluation failed", details);
            }
        } 
        else {
            m_logger.log(m_now_ms, LoggerEventType::TimerIgnored, "Timer ignored for transition " + 
                transition->name + " because there are not enough tokens", details);
        }
    }
}

/**
 * Method finds a transition by its name.
 *
 * @param name - name of the transition to be found
 * @return pointer to the found transition, or nullptr if it was not found
 */
const Transition* PetriRuntime::find_transition_or_null(const std::string& name) const {
    return m_net.find_transition(name);
}

/**
 * Method creates initial value for a given variable. Converts initialiyer
 * according to type.
 *
 * note: if the variable type is not specified, integer type is used.
 * note1: if the initializer is not specified, zero is used as default value.
 *
 * @param variable - variable whose initial value should be created
 * @return initial value of the variable
 */
EvalValue PetriRuntime::initial_value_for_variable(const Variable& variable) const {
    // if stated sets default type initializer or type
    std::string type;
    if (variable.type.empty()) {
        type = "int";
    }
    else {
        type = variable.type;
    }
    std::string initializer;
    if (variable.initializer.empty()) {
        initializer = "0";
    }
    else {
        initializer = variable.initializer;
    }

    // conversion to lowercase cause of easier comparison
    std::string lowered = type;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    // creates string value
    if (lowered == "string" || lowered == "std::string") {
        if (initializer.size() >= 2 && initializer.front() == '"' && initializer.back() == '"') {
            return EvalValue::string(initializer.substr(1, initializer.size() - 2));
        }
        return EvalValue::string(initializer);
    }

    // creates bool value
    if (lowered == "bool") {
        return EvalValue::boolean(initializer == "true" || initializer == "1");
    }

    // creates  int value
    return EvalValue::integer(std::atoll(initializer.c_str()));
}

/**
 * Method notifies that runtime state has changed
 */
void PetriRuntime::notify_state_changed() const {
    if (m_on_state_changed) {
        m_on_state_changed(snapshot());
    }
}

/**
 * Method logs runtime error and notifies about it. If error callback 
 * is not set, writes only to log
 *
 * @param message - error message to be logged and passed to callback
 */
void PetriRuntime::notify_error(const std::string& message) {
    m_logger.log(m_now_ms, LoggerEventType::Error, message);
    if (m_on_error) {
        m_on_error(message, snapshot());
    }
}