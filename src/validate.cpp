// autor: Jurišinová Daniela (xjurisd00)

#include "validate.hpp"
#include <set>

/**
 * Method which checks if the validation result had no error
 *
 * @return true if Petrinet is valid otherwise false
 */
bool ValidationResult::is_ok() const {
    return !has_errors();
}

/**
 * Method which checks if the validation result had any errors.
 * Basically helper method for the ok result method.
 *
 * @return true if Petrinet has at least one error otherwise fales
 */
bool ValidationResult::has_errors() const {
    for (const auto& msg : messages) {
        if (msg.severity == ValidationSeverity::Error) {
            return true;
        }
    }
    return false;
}

/**
 * Method to add error message to result of the validation
 *
 * @param message - the text description of the error 
 */
void ValidationResult::add_error(const std::string& message) {
    messages.push_back({ValidationSeverity::Error, message});
}

/**
 * Method to add warning message to result of the validation 
 *
 * @param message - the text description of the warning
 */
void ValidationResult::add_warning(const std::string& message) {
    messages.push_back({ValidationSeverity::Warning, message});
}

/**
 * Method performs validation proccess of the whole Petrinet. uses
 * methods to checks names, places, transitions and events a searches
 * for errors and warnings to add to the result.
 *
 * @param net - Petrinet to be checked
 * @return the result with all found errors and warnings
 */
ValidationResult Validator::validate(const PetriNet& net) const {
    ValidationResult result;
    if (net.name.empty()) {
        result.add_error("Petrinet name is empty");
    }

    validate_names(net, result);
    validate_places(net, result);
    validate_transitions(net, result);
    validate_events(net, result);
    return result;
}

/**
 * Helper method to validate names used in Petrinet. Checks for duplicity
 * or absence of name in places and transition in net.
 *
 * @param net - Petrinet which names are to be checked
 * @param result - result of the partial validation with stored errors and warnings
 */
void Validator::validate_names(const PetriNet& net, ValidationResult& result) const {
    std::set<std::string> place_names;
    std::set<std::string> transition_names;

    // for every place in Petrinet name absence and name duplicity check
    for (const auto& place : net.places) {
        if (place.name.empty()) {
            result.add_error("Place name not defined");
        }
        if (!place_names.insert(place.name).second) {
            result.add_error("Duplicate name of the places: " + place.name);
        }
    }

    // for every transition in Petrinet name absence and name duplicity check
    for (const auto& transition : net.transitions) {
        if (transition.name.empty()) {
            result.add_error("Transition name not defined");
        }
        if (!transition_names.insert(transition.name).second) {
            result.add_error("Duplicate name of the transitions: " + transition.name);
        }
    }
}

/**
 * Helper method to validate places used in Petrinet. Checks if there are
 * places in the first place or if they have negative amount of initial
 * tokens or if there is at least one initial token of any place.
 *
 * @param net - Petrinet which places are to be checked
 * @param result - result of the partial validation with stored errors and warnings
 */
void Validator::validate_places(const PetriNet& net, ValidationResult& result) const {
    int total_initial_tokens = 0;
    // negative initial token check and sum of all initial tokens for each place
    for (const auto& place : net.places) {
        if (place.initial_tokens < 0) {
            result.add_error("Place with negative token count: " + place.name);
        }
        total_initial_tokens += place.initial_tokens;
    }

    // any place or initial token check
    if (net.places.empty()) {
        result.add_warning("Petrinet with no places");
    }
    if (total_initial_tokens == 0) {
        result.add_warning("Petrinet with no initial tokens");
    }
}

/**
 * Helper method to validate transitions used in Petrinet. For each transition
 * checks if it has at least one input arc and then checks all its input arcs if 
 * their weight and place_name is valid (then analogicallz checks all its output arcs)
 *
 * @param net - Petrinet which transitions are to be checked
 * @param result - result of the partial validation with stored errors and warnings
 */
void Validator::validate_transitions(const PetriNet& net, ValidationResult& result) const {
    for (const auto& transition : net.transitions) {
        // at least one input arc check
        if (transition.input_arcs.empty()) {
            result.add_error("Transition with no input arcs: " + transition.name);
        }

        // all input arcs weight and place_name validity check
        for (const auto& arc : transition.input_arcs) {
            if (arc.weight < 1) {
                result.add_error("Transition with input arc with invalid weight (<1): " + transition.name);
            }
            if (net.find_place(arc.place_name) == nullptr) {
                result.add_error("Transition " + transition.name + "references unknown input place: " 
                        + arc.place_name);
            }
        }

        // all output arcs weight and place_name validity check
        for (const auto& arc : transition.output_arcs) {
            if (arc.weight < 1) {
                result.add_error("Transition with output arc with invalid weight (<1): " + transition.name);
            }
            if (net.find_place(arc.place_name) == nullptr) {
                result.add_error("Transition " + transition.name + "references unknown output place: " 
                        + arc.place_name);
            }
        }
    }    
}

/**
 * Helper method to validate events of the Petrinet. Continuosly checks every transition
 * if the name of the event of transition condition is in valid inputs of Petrinet
 *
 * @param net - Petrinet which events are to be checked
 * @param result - result of the aprtial validation with stored errors and warnings
 */
void Validator::validate_events(const PetriNet& net, ValidationResult& result) const {
    for (const auto& transition : net.transitions) {
        const auto& event_name = transition.condition.event_name;
        if (!event_name.empty() && !net.has_input(event_name)) {
            result.add_error("Transition " + transition.name + " uses unknown input event " + event_name);
        }
    }
}