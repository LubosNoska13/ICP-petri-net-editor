// autor: Jurišinová Daniela (xjurisd00)

#include "model.hpp"
#include <algorithm>

/**
 * Method checks if transition condition has defined event
 *
 * @return false if event is empty/not defined otherwise true
 */
bool TransitionCondition::has_event() const {
    return !event_name.empty();
}

/**
 * Method checks if transition condition has defined guard (condition
 * that has to be true for transition execution)
 *
 * @return false if guard is empty/not defined otherwise true
 */
bool TransitionCondition::has_guard() const {
    return !guard_expression.empty();
}

/**
 * Method checks if transition condition has defined delay expression
 *
 * @return false if delay is empty/not defined otherwise true
 */
bool TransitionCondition::has_delay() const {
    return !delay_expression.empty();
}

/**
 * Method finds place defined in Petri Net by it's name
 *
 * constant variant of the method
 * 
 * @param name - the name of the place we're looking for
 * @return Pointer to the place if there is place with that name and 
 *          Null pointer otherwise
 */
const Place* PetriNet::find_place(const std::string& name) const {
    auto found_place = std::find_if(places.begin(), places.end(), 
            [&](const Place& place) {return place.name == name; });
    if (found_place == places.end()) {
        return nullptr;
    }
    else {
        return &(*found_place);
    }
}

/**
 * Method finds place defined in Petri Net by it's name
 * 
 * @param name - the name of the place we're looking for
 * @return Pointer to the place if there is place with that name and 
 *          Null pointer otherwise
 */
Place* PetriNet::find_place(const std::string& name) {
    auto found_place = std::find_if(places.begin(), places.end(), 
            [&](const Place& place) {return place.name == name; });
    if (found_place == places.end()) {
        return nullptr;
    }
    else {
        return &(*found_place);
    }
}

/**
 * Method finds transition defined in Petri Net by it's name
 *
 * constant variant of the method
 * 
 * @param name - the name of the transition we're looking for
 * @return Pointer to the transition if there is transition with that name and 
 *          Null pointer otherwise
 */
const Transition* PetriNet::find_transition(const std::string& name) const {
    auto found_transition = std::find_if(transitions.begin(), transitions.end(),
            [&](const Transition& transition) {return transition.name == name; });
    if (found_transition == transitions.end()) {
        return nullptr;
    }
    else {
        return &(*found_transition);
    }
}

/**
 * Method finds transition defined in Petri Net by it's name
 * 
 * @param name - the name of the transition we're looking for
 * @return Pointer to the transition if there is transition with that name and 
 *          Null pointer otherwise
 */
Transition* PetriNet::find_transition(const std::string& name) {
    auto found_transition = std::find_if(transitions.begin(), transitions.end(),
            [&](const Transition& transition) {return transition.name == name; });
    if (found_transition == transitions.end()) {
        return nullptr;
    }
    else {
        return &(*found_transition);
    }
}

/**
 * Method checks if input with specific name is defines in Petri Net
 * 
 * @param name - the name of the input we're looking for
 * @return true if the input exists and false otherwise
 */
bool PetriNet::has_input(const std::string& name) const {
    return std::any_of(inputs.begin(), inputs.end(), 
            [&](const Input& input) {return input.name == name; });
}

/**
 * Method checks if output with specific name is defines in Petri Net
 * 
 * @param name - the name of the output we're looking for
 * @return true if the output exists and false otherwise
 */
bool PetriNet::has_output(const std::string& name) const {
    return std::any_of(outputs.begin(), outputs.end(), 
            [&](const Output& output) {return output.name == name; });
}

/**
 * Method checks if variable with specific name is defines in Petri Net
 * 
 * @param name - the name of the variable we're looking for
 * @return true if the variable exists and false otherwise
 */
bool PetriNet::has_variable(const std::string& name) const {
    return std::any_of(variables.begin(), variables.end(), 
            [&](const Variable& variable) {return variable.name == name; });
}