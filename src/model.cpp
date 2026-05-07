// autor: Jurišinová Daniela (xjurisd00)

#include "model.hpp"
#include <algorithm>
#include <sstream>
#include <stdexcept>

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

/** 
 * Helper function for id validation. Checks if character is in
 * alphabet or _
 *
 * @param character - character to be checked 
 * @return true if character is in alphabet or is _ otherwise false
 */
bool is_alpha_or_underscore(char character) {
    return (character >= 'A' && character <= 'Z') ||
           (character >= 'a' && character <= 'z') ||
           character == '_';
}

/** 
 * Helper function for id validation. Checks if character is number
 * or alphabet szmbol or _
 *
 * @param character - character to be checked 
 * @return true if character is in alphabet or is _ otherwise false
 */
bool is_alnum_or_underscore(char character) {
    return is_alpha_or_underscore(character) ||
           (character >= '0' && character <= '9');
}

/**
 * Method checks if name can be used as a model identifier.
 *
 * @param name - name to be checked
 * @return true if name is not empty and valid id otherwise false
 */
bool PetriNet::valid_id(const std::string& name) {
    // empty argument check
    if (name.empty()) {
        return false;
    }

    // recursive symbols check
    if (!is_alpha_or_underscore(name.front())) {
        return false;
    }
    return std::all_of(name.begin() + 1, name.end(), is_alnum_or_underscore);
}

/**
 * Method creates new place in Petrinet and sets it's features as stated in 
 * arguments
 *
 * @param name - unique name of the new place
 * @param initial_tokens - not negative initial token count of new place 
 * @param add_token_action - optional action of the new place
 * @return reference to new place
 */
Place& PetriNet::add_place(const std::string& name, int initial_tokens, 
        const std::string& add_token_action) {
    
    // name, already exists and valid tokens check
    validate_id(name, "place");
    if (find_place(name) != nullptr) {
        throw std::invalid_argument("Place already exists: " + name);
    }
    if (initial_tokens < 0) {
        throw std::invalid_argument("Place cannot have negative initial tokens: " + name);
    }

    // creates and fills place
    Place place;
    place.id = allocate_place_id();
    place.name = name;
    place.initial_tokens = initial_tokens;
    place.add_token_action = add_token_action;
    places.push_back(place);
    return places.back();
}

/**
 * Method removes place and all arcs that reference it in Petrinet
 *
 * @param name - name of removed place
 * @return true if place was removed otherwise false
 */
bool PetriNet::remove_place(const std::string& name) {
    auto old_size = places.size();
    // deletes place from list
    places.erase(std::remove_if(places.begin(), places.end(),
            [&](const Place& place) { return place.name == name; }), places.end());

    // no deletion check
    if (places.size() == old_size) {
        return false;
    }

    // deletes transition arcs bound to the place
    for (auto& transition : transitions) {
        transition.input_arcs.erase(std::remove_if(transition.input_arcs.begin(), transition.input_arcs.end(),
                [&](const Arc& arc) { return arc.place_name == name; }), transition.input_arcs.end());
        transition.output_arcs.erase(std::remove_if(transition.output_arcs.begin(), transition.output_arcs.end(),
                [&](const Arc& arc) { return arc.place_name == name; }), transition.output_arcs.end());
    }
    return true;
}

/**
 * Method renames place and updates all arcs that referenced the old name.
 *
 * @param old_name - current place name
 * @param new_name - new unique place name
 * @return true if place was renamed otherwise false
 */
bool PetriNet::rename_place(const std::string& old_name, const std::string& new_name) {
    // valid place in argument and unique new name check
    validate_id(new_name, "place");
    Place* place = find_place(old_name);
    if (place == nullptr) {
        return false;
    }
    if (old_name != new_name && find_place(new_name) != nullptr) {
        throw std::invalid_argument("Place already exists: " + new_name);
    }

    // renames place and name in all transition arcs references
    place->name = new_name;
    for (auto& transition : transitions) {
        for (auto& arc : transition.input_arcs) {
            if (arc.place_name == old_name) {
                arc.place_name = new_name;
            }
        }
        for (auto& arc : transition.output_arcs) {
            if (arc.place_name == old_name) {
                arc.place_name = new_name;
            }
        }
    }
    return true;
}

/**
 * Method creates new transition in Petrinet
 *
 * @param name - unique name of the new transition
 * @param condition - new transition enabling condition
 * @param action_code - new transition action code
 * @param priority - optional priority of the new transition
 * @return reference to newly created transition
 */
Transition& PetriNet::add_transition(const std::string& name, const TransitionCondition& condition,
        const std::string& action_code, std::optional<int> priority) {

    // name and already exists check
    validate_id(name, "transition");
    if (find_transition(name) != nullptr) {
        throw std::invalid_argument("Transition already exists: " + name);
    }

    // creates and fills transition with stated features in arguments
    Transition transition;
    transition.id = allocate_transition_id();
    transition.name = name;
    transition.condition = condition;
    transition.action_code = action_code;
    transition.priority = priority.value_or(static_cast<int>(transitions.size()));
    transitions.push_back(transition);
    return transitions.back();
}

/**
 * Method removes transition with name stated in argument
 *
 * @param name - transition name
 * @return true if transition was removed otherwise false
 */
bool PetriNet::remove_transition(const std::string& name) {
    auto old_size = transitions.size();
    transitions.erase(std::remove_if(transitions.begin(), transitions.end(),
            [&](const Transition& transition) { return transition.name == name; }), transitions.end());
    return transitions.size() != old_size;
}

/**
 * Method adds or updates input arc from place to transition in Petrinet
 *
 * @param transition_name - edited transition name
 * @param place_name - source place name
 * @param weight - positive arc weight
 * @return true if transition and place exists and was added/changed otherwise false
 */
bool PetriNet::add_input_arc(const std::string& transition_name, const std::string& place_name,
        int weight) {

    // transition validity check
    validate_weight(weight);
    Transition* transition = find_transition(transition_name);
    if (transition == nullptr || find_place(place_name) == nullptr) {
        return false;
    }

    auto found_arc = std::find_if(transition->input_arcs.begin(), transition->input_arcs.end(),
            [&](const Arc& arc) { return arc.place_name == place_name; });
    // updates weigth
    if (found_arc != transition->input_arcs.end()) {
        found_arc->weight = weight;
    }
    // creates new arc
    else {
        transition->input_arcs.push_back(Arc{place_name, weight});
    }
    return true;
}

/**
 * Method adds or updates output arc from transition to place in Petrinet
 *
 * @param transition_name - edited transition name
 * @param place_name - target place name
 * @param weight - positive arc weight
 * @return true if transition and place exists and was added/changed otherwise false
 */
bool PetriNet::add_output_arc(const std::string& transition_name, const std::string& place_name,
        int weight) {

    // transition validity check
    validate_weight(weight);
    Transition* transition = find_transition(transition_name);
    if (transition == nullptr || find_place(place_name) == nullptr) {
        return false;
    }

    auto found_arc = std::find_if(transition->output_arcs.begin(), transition->output_arcs.end(),
            [&](const Arc& arc) { return arc.place_name == place_name; });
    // updates weight
    if (found_arc != transition->output_arcs.end()) {
        found_arc->weight = weight;
    }
    // creates new arc
    else {
        transition->output_arcs.push_back(Arc{place_name, weight});
    }
    return true;
}

/**
 * Method translates Petrinet into readable text format from assigment
 * in elearning used in parser
 *
 * @return text representation of the Petrinet
 */
std::string PetriNet::to_text() const {
    std::ostringstream out;

    out << "Jméno sítě:\n";
    out << "\t" << name << "\n";

    out << "Komentář:\n";
    out << "\t" << comment << "\n";

    out << "Vstupy:\n";
    for (const auto& input : inputs) {
        out << "\t" << input.name << "\n";
    }

    out << "Výstupy:\n";
    for (const auto& output : outputs) {
        out << "\t" << output.name << "\n";
    }

    out << "Proměnné:\n";
    for (const auto& var : variables) {
        out << "\t" << var.type << " " << var.name << " = " << var.initializer << "\n";
    }

    out << "Místa (počáteční tokeny, volitelně akce):\n";
    for (const auto& place : places) {
        out << "\t" << place.name << " (" << place.initial_tokens << ") : { "
            << place.add_token_action << " }\n";
    }

    out << "Přechody a jejich podmínky:\n\n";
    for (const auto& transition : transitions) {
        out << transition.name << " :\n";
        out << "\tin:\t";
        for (size_t input = 0; input < transition.input_arcs.size(); ++input) {
            if (input > 0) out << ", ";
            out << transition.input_arcs[input].place_name << "*" << transition.input_arcs[input].weight;
        }
        out << "\n";

        out << "\tout:\t";
        for (size_t output = 0; output < transition.output_arcs.size(); ++output) {
            if (output > 0) out << ", ";
            out << transition.output_arcs[output].place_name << "*" << transition.output_arcs[output].weight;
        }
        out << "\n";

        out << "\twhen:\t";
        if (!transition.condition.event_name.empty()) {
            out << transition.condition.event_name << " ";
        }
        if (!transition.condition.guard_expression.empty()) {
            out << "[ " << transition.condition.guard_expression << " ] ";
        }
        if (!transition.condition.delay_expression.empty()) {
            out << "@ " << transition.condition.delay_expression;
        }
        out << "\n";

        out << "\tdo: { " << transition.action_code << " }\n";
    }
    return out.str();
}

/**
 * Method clears all model data and resets stable id counters
 */
void PetriNet::clear() {
    name.clear();
    comment.clear();
    inputs.clear();
    outputs.clear();
    variables.clear();
    places.clear();
    transitions.clear();
    m_next_place_id = 1;
    m_next_transition_id = 1;
}

/**
 * Method allocates next stable place id
 *
 * @return unique id for a new place
 */
ModelId PetriNet::allocate_place_id() {
    return m_next_place_id++;
}

/**
 * Method allocates next stable transition id
 *
 * @return unique id for a new transition
 */
ModelId PetriNet::allocate_transition_id() {
    return m_next_transition_id++;
}

/**
 * Method throws error if identifier is invalid
 *
 * @param name - checked name
 * @param what - object type used in error message
 */
void PetriNet::validate_id(const std::string& name, const std::string& what) {
    if (!valid_id(name)) {
        throw std::invalid_argument("Invalid " + what + " name: " + name);
    }
}

/**
 * Method throws error if arc weight is invalid
 *
 * @param weight - checked arc weight
 */
void PetriNet::validate_weight(int weight) {
    if (weight < 1) {
        throw std::invalid_argument("Arc weight must be at least 1");
    }
}