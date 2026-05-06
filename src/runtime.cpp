// autor: Jurišinová Daniela (xjurisd00)

#include "runtime.hpp"
#include <cstdlib>
#include <stdexcept> 
#include <algorithm>
#include <sstream>

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
void PetriRuntime::initialize() {
    m_logger.log(m_now_ms, LoggerEventType::RuntimeStarted, "Runtime started for Petrinet: " + m_net.name);

    // sets initial tokens for all places and writes up change time
    for (const auto& place : m_net.places) {
        m_marking[place.name] = place.initial_tokens;
        m_place_changed_at[place.name] = m_now_ms;
    }

    // for each input sets runtime value
    for (const auto& input : m_net.inputs) {
        m_inputs[input.name] = RuntimeInputValue{};
    }

    // for each variable converts value if its not empty and saves it
    for (const auto& variable : m_net.variables) {
        RuntimeVariable runtime_variable;
        if (!variable.initializer.empty()) {
            runtime_variable.value = std::atoi(variable.initializer.c_str());
        }
        m_variables[variable.name] = runtime_variable;
    }
    run_stabilization(std::nullopt);
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
        throw std::runtime_error("Unknown input: " + name);
    }

    // sets input features
    m_inputs[name].defined = true;
    m_inputs[name].value = value;
    m_inputs[name].last_update_ms = m_now_ms;
    m_logger.log(m_now_ms, LoggerEventType::InputReceived, "Input " + name + " = " + value);
    run_stabilization(name);
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
        throw std::runtime_error("Can't move runtime time backwards");
    }

    // shifts time 
    m_now_ms += delta_ms;
    process_due_timers();
    run_stabilization(std::nullopt);
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
    while (true) {
        auto candidates = immediate_enabled(triggering_event);
        auto selected = max_independent_set(candidates);

        // no transition can be made and net is stabilized
        if (selected.empty()) {
            break;
        }

        // fires independent transitions
        for (const auto* transition : selected) {
            fire_transition(*transition);
        }
    }
    // plans other transitions
    schedule_delayed_transitions(triggering_event);
}

/**
 * Method creates a snapshot of the current state of Petrinet.
 * Copies marking, inputs, variables and pending timers if there 
 * are any
 *
 * @return snapshot with current state of runtime
 */
StateSnapshot PetriRuntime::snapshot() const {
    StateSnapshot snap;
    snap.marking = m_marking;
    snap.inputs = m_inputs;
    snap.variables = m_variables;

    auto timers_copy = m_timers;
    while (!timers_copy.empty()) {
        snap.pending_timers.push_back(timers_copy.top());
        timers_copy.pop();
    }
    return snap;
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
int PetriRuntime::variable_value(const std::string& name) const {
    auto variable = m_variables.find(name);
    if (variable == m_variables.end()) {
        throw std::runtime_error("Unknown variable: " + name);
    }
    return variable->second.value;
}

/**
 * Method sets value of the variable in argument. First checks if variable
 * exists in Petrinet and then checks if the value really changed. If it did
 * then writes in log old and new value of the variable.
 *
 * @param name - name of the variable to be set
 * @param value - value which is gonna be value of the variable
 */
void PetriRuntime::set_variable_value(const std::string& name, int value) {
    // variable exists check
    auto variable = m_variables.find(name);
    if (variable == m_variables.end()) {
        throw std::runtime_error("Unknown variable: " + name);
    }

    int old_value = variable->second.value;
    variable->second.value = value;
    // value really changed check
    if (old_value != value) {
        m_logger.log(m_now_ms, LoggerEventType::VariableChanged, "Variable " + name + " changed from " +
                std::to_string(old_value) + " to " + std::to_string(value));
    }
}

/**
 * Method writes in log which output and it's value is emitted
 *
 * @param name - name of the output
 * @param value - value to be emitted in the output
 */
void PetriRuntime::emit_output(const std::string& name, const std::string& value) {
    m_logger.log(m_now_ms, LoggerEventType::OutputProduced, "Output " + name + " = " + value);
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
std::vector<const Transition*> PetriRuntime::immediate_enabled(const std::optional<std::string>& triggering_event) {
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
std::vector<const Transition*> PetriRuntime::max_independent_set(
        const std::vector<const Transition*>& candidates) const {

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
 * code, the action is executed. Finally output tokens are produced to output
 * places.
 *
 * @param transition - transition to be fired
 */
void PetriRuntime::fire_transition(const Transition& transition) {
    m_logger.log(m_now_ms, LoggerEventType::TransitionFired, "Transition " + transition.name + " fired");
    consume_input_tokens(transition);

    if (!transition.action_code.empty()) {
        m_evaluator.execute_action(transition.action_code, *this);
    }
    produce_output_tokens(transition);
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
        m_logger.log(m_now_ms, LoggerEventType::TokenChanged, arc.place_name + ": " +
                std::to_string(old_value) + " -> " + std::to_string(new_value));
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
        m_logger.log(m_now_ms, LoggerEventType::TokenChanged, arc.place_name + ": " +
                std::to_string(old_value) + " -> " + std::to_string(new_value));

        // finds output place and executed add-token action if stated
        const Place* place = m_net.find_place(arc.place_name);
        if (place != nullptr && !place->add_token_action.empty()) {
            for (int index = 0; index < arc.weight; ++index) {
                m_evaluator.execute_action(place->add_token_action, *this);
            }
        }
    }
}

/**
 * Method schedules timers for enabled delayed transitions. Checks all
 * transitions in the net and selects those with condition containing
 * delay. If the transition is bound to an event, the event must match the
 * triggering event inargument. The transition must be enabled and mustn't 
 * have a scheduled timer. The delay expression is evaluated and timer
 * is created and stored.
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

        // not enabled skip
        if (!transition_enabled(transition, triggering_event)) {
            continue;
        }

        // already scheduled skip
        if (timer_scheduled(transition.name)) {
            continue;
        }

        // evaluates delay and creates timer
        int delay_ms = m_evaluator.int_evaluation(transition.condition.delay_expression, *this);
        PendingTimer timer;
        timer.transition_name = transition.name;
        timer.scheduled_at_ms = m_now_ms;
        timer.due_at_ms = m_now_ms + delay_ms;
        timer.event_name = transition.condition.event_name;
        m_timers.push(timer);

        // updates last change time and writes new schedule in log
        m_transition_enabled_since[transition.name] = m_now_ms;
        m_logger.log(m_now_ms, LoggerEventType::TimerScheduled, "Timer for transition " + transition.name +
                     " scheduled at " + std::to_string(timer.due_at_ms) + " ms");
    }
}

/**
 * Method checks whether a timer is already scheduled. It creates a copy 
 * of the timer queue and searches through pending timers.
 *
 * @param transition_name - name of the transition to be checked
 * @return true if a timer for the transition is already scheduled otherwise false
 */
bool PetriRuntime::timer_scheduled(const std::string& transition_name) const {
    auto copy = m_timers;
    while (!copy.empty()) {
        if (copy.top().transition_name == transition_name) {
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
        m_logger.log(m_now_ms, LoggerEventType::TimerExpired, "Timer expired for transition " + 
                timer.transition_name);

        // transition exists check
        const Transition* transition = find_transition_or_null(timer.transition_name);
        if (transition == nullptr) {
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
                        + transition->name + " because guard is false");
                }
            } 
            catch (...) {
                m_logger.log(m_now_ms, LoggerEventType::TimerIgnored, "Timer ignored for transition " + 
                    transition->name + " because guard evaluation failed");
            }
        } 
        else {
            m_logger.log(m_now_ms, LoggerEventType::TimerIgnored, "Timer ignored for transition " + 
                transition->name + " because there are not enough tokens");
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