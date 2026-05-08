// autor: Jurišinová Daniela (xjurisd00)

#include "validate.hpp"
#include <set>
#include <algorithm>
#include <cctype>
#include <map>

// helper functions for this file
namespace {

/**
* Helper function deletes whitespaces from the start and the end 
* of the string
*
* @param string - the string for trimming
* @return trimmed string
*/
std::string trim(const std::string& string) {
    const auto begin = string.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return "";
    }
       
    const auto end = string.find_last_not_of(" \t\r\n");
    return string.substr(begin, end - begin + 1);
}

/**
* Helper function checks whether text is a valid identifier
*
* @param text - the text to check
* @return true if text is a valid identifier otherwise false
*/
bool is_identifier(const std::string& text) {
    return PetriNet::valid_id(text);
}

/**
* Helper function checks whether text represents a zero or positive integer
*
* @param text - the text to check
* @return true if text isn't negative integer otherwise false otherwise
*/
bool is_non_negative_integer(const std::string& text) {
    const std::string number = trim(text);
    return !number.empty() && std::all_of(number.begin(), number.end(),
            [](unsigned char character) { return std::isdigit(character) != 0; });
}

/**
* Helper function checks whether character is not a word character
*
* @param character - the character to check
* @return true if character is not alphanumeric and is not underscore otherwise false
*/
bool is_non_word_char(char character) {
    return !std::isalnum(static_cast<unsigned char>(character)) && character != '_';
}

/**
* Helper function checks whether function name starts at the given position
* and is not part of another identifier
*
* @param code - the code where the function name is searched
* @param position - the position where the function name should start
* @param name - the function name to check
* @return true if function name is found at position in argument otherwise false otherwise
*/
bool is_function_name_at(const std::string& code, std::size_t position, const std::string& name) {
    // name fits into code check
    if (position + name.size() > code.size()) {
        return false;
    }
    // name at the position check
    if (code.compare(position, name.size(), name) != 0) {
        return false;
    }
    // invalid character before name check
    if (position > 0 && !is_non_word_char(code[position - 1])) {
        return false;
    }
    const std::size_t after_name = position + name.size();
    return after_name >= code.size() || is_non_word_char(code[after_name]);
}

/**
* Helper function skips whitespace characters from the given position
*
* @param code - the code where whitespaces are skipped
* @param position - the starting position
* @return position of the first no whitespace character
*/
std::size_t skip_spaces(const std::string& code, std::size_t position) {
    while (position < code.size() && std::isspace(static_cast<unsigned char>(code[position]))) {
        ++position;
    }
    return position;
}

/**
* Helper function reads a string argument from a function call
*
* @param code - the code containing the function call
* @param function_position - the position where the function name starts
* @param function_name - the name of the function
* @param argument - variable where the read argument is stored
* @return true if string argument was successfully read otherwise false
*/
bool read_string_argument(const std::string& code, std::size_t function_position,
        const std::string& function_name, std::string& argument) {
       
    std::size_t position = function_position + function_name.size();
    position = skip_spaces(code, position);
    // () after function name check
    if (position >= code.size() || code[position] != '(') {
        return false;
    }
    ++position;
    position = skip_spaces(code, position);

    // argument "" check
    if (position >= code.size() || code[position] != '"') {
        return false;
    }
    ++position;

    std::string value;
    while (position < code.size()) {
        const char character = code[position];
        // sets end of sequence 
        if (character == '\\') {
            if (position + 1 < code.size()) {
                value.push_back(code[position + 1]);
                position += 2;
                continue;
            }
            return false;
        }
        // sets end of argument
        if (character == '"') {
            argument = value;
            return true;
        }
        value.push_back(character);
        ++position;
    }
    // no end '"' found
    return false;
}

/**
* Helper function finds all string arguments of calls to the given function
*
* @param code - the code where function calls are searched
* @param function_name - the name of the function to search for
* @return vector of found string arguments
*/
std::vector<std::string> find_string_arguments(const std::string& code, const std::string& function_name) {
    std::vector<std::string> arguments;
    std::size_t position = 0;

    // checks all function name positions in code
    while ((position = code.find(function_name, position)) != std::string::npos) {
        // function name check
        if (is_function_name_at(code, position, function_name)) {
            std::string argument;
            // reads argument
            if (read_string_argument(code, position, function_name, argument)) {
                arguments.push_back(argument);
            }
        }
        position += function_name.size();
    }
    return arguments;
}

/**
* Helper function checks whether delay expression is numeric or an existing variable
*
* @param net - the Petri net containing declared variables
* @param expression - the delay expression to check
* @return true if expression is empty, no negative integer, or existing variable otherwise false
*/
bool is_numeric_delay_or_existing_variable(const PetriNet& net, const std::string& expression) {
    const std::string delay = trim(expression);
    if (delay.empty()) {
        return true;
    }
    if (is_non_negative_integer(delay)) {
        return true;
    }
    return is_identifier(delay) && net.has_variable(delay);
}

/**
* Helper function checks whether output() calls use existing outputs
*
* @param net - the Petri net containing defined outputs
* @param result - validation result where errors are stored
* @param code - the code where output() calls are checked
* @param context - description of the checked context
*/
void check_output_calls(const PetriNet& net, ValidationResult& result, const std::string& code, 
            const std::string& context) {
    
    for (const auto& output_name : find_string_arguments(code, "output")) {
        if (!net.has_output(output_name)) {
            result.add_error(context + " uses unknown output: " + output_name);
        }
    }
}

/**
* Helper function checks whether tokens() calls use existing places
*
* @param net - the Petri net containing defined places
* @param result - validation result where errors are stored
* @param code - the code where tokens() calls are checked
* @param context - description of the checked context
*/
void check_tokens_calls(const PetriNet& net, ValidationResult& result, const std::string& code, 
            const std::string& context) {
    
    for (const auto& place_name : find_string_arguments(code, "tokens")) {
        if (net.find_place(place_name) == nullptr) {
            result.add_error(context + " uses tokens() with unknown place: " + place_name);
        }
    }
}

/**
* Helper function checks whether elapsed() calls use existing places or transitions
*
* @param net - the Petri net containing defined places and transitions
* @param result - validation result where errors are stored
* @param code - the code where elapsed() calls are checked
* @param context - description of the checked context
*/
void check_elapsed_calls(const PetriNet& net, ValidationResult& result, const std::string& code, 
            const std::string& context) {
    
    for (const auto& object_name : find_string_arguments(code, "elapsed")) {
        if (net.find_place(object_name) == nullptr && net.find_transition(object_name) == nullptr) {
            result.add_error(context + " uses elapsed() with unknown place or transition: " + object_name);
        }
    }
}

/**
* Helper function checks references used in inscription code
*
* @param net - the Petri net containing defined objects
* @param result - validation result where errors are stored
* @param code - the inscription code to check
* @param context - description of the checked context
*/
void check_inscription_references(const PetriNet& net, ValidationResult& result, const std::string& code, 
            const std::string& context) {
    
    check_output_calls(net, result, code, context);
    check_tokens_calls(net, result, code, context);
    check_elapsed_calls(net, result, code, context);
}

/**
* Helper function adds a name occurrence with its description
*
* @param names - map storing names and their occurrence descriptions
* @param name - the name to add
* @param description - description of the occurrence
*/
void add_name_occurrence(std::map<std::string, std::vector<std::string>>& names, const std::string& name, 
        const std::string& description) {
    
    if (!name.empty()) {
        names[name].push_back(description);
    }
}

/**
* Helper function joins descriptions into one comma-separated string
*
* @param descriptions - vector of descriptions to join
* @return joined descriptions
*/
std::string join_descriptions(const std::vector<std::string>& descriptions) {
    std::string joined;
    for (std::size_t description_index = 0; description_index < descriptions.size(); ++description_index) {
        if (description_index > 0) {
            joined += ", ";
        }
        joined += descriptions[description_index];
    }
    return joined;
}

}

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
    validate_inscriptions(net, result);
    validate_reachability(net, result);
    return result;
}

/**
 * Method for GUI live validation after every edit
 *
 * @param net - Petrinet to be checked
 * @return the result with all found errors and warnings
 */
ValidationResult Validator::validate_live(const PetriNet& net) const {
    return validate(net);
}

/**
 * Helper method to validate names used in Petrinet. Checks for duplicity
 * or absence of name in places and transition in net.
 *
 * @param net - Petrinet which names are to be checked
 * @param result - result of the partial validation with stored errors and warnings
 */
void Validator::validate_names(const PetriNet& net, ValidationResult& result) const {
    std::map<std::string, std::vector<std::string>> all_names;
    std::set<std::string> place_names;
    std::set<std::string> transition_names;
    std::set<std::string> input_names;
    std::set<std::string> output_names;
    std::set<std::string> variable_names;

    // for every place in Petrinet name absence, validity and name duplicity check
    for (const auto& place : net.places) {
        if (place.name.empty()) {
            result.add_error("Place name not defined");
        }
        else if (!PetriNet::valid_id(place.name)) {
            result.add_error("Invalid place name: " + place.name);
        }
        if (!place_names.insert(place.name).second) {
            result.add_error("Duplicate name of the places: " + place.name);
        }
        add_name_occurrence(all_names, place.name, "place");
    }

    // for every transition in Petrinet name absence, validity and name duplicity check
    for (const auto& transition : net.transitions) {
        if (transition.name.empty()) {
            result.add_error("Transition name not defined");
        }
        else if (!PetriNet::valid_id(transition.name)) {
            result.add_error("Invalid transition name: " + transition.name);
        }
        if (!transition_names.insert(transition.name).second) {
            result.add_error("Duplicate name of the transitions: " + transition.name);
        }
        add_name_occurrence(all_names, transition.name, "transition");
    }

    // for every input in Petrinet name absence, validity and name duplicity check
    for (const auto& input : net.inputs) {
        if (input.name.empty()) {
            result.add_error("Input name not defined");
        }
        else if (!PetriNet::valid_id(input.name)) {
            result.add_error("Invalid input name: " + input.name);
        }
        if (!input_names.insert(input.name).second) {
            result.add_error("Duplicate name of the inputs: " + input.name);
        }
        add_name_occurrence(all_names, input.name, "input");
    }

    // for every output in Petrinet name absence, validity and name duplicity check
    for (const auto& output : net.outputs) {
        if (output.name.empty()) {
            result.add_error("Output name not defined");
        }
        else if (!PetriNet::valid_id(output.name)) {
            result.add_error("Invalid output name: " + output.name);
        }
        if (!output_names.insert(output.name).second) {
            result.add_error("Duplicate name of the outputs: " + output.name);
        }
        add_name_occurrence(all_names, output.name, "output");
    }

    // for every variable in Petrinet name absence, validity and name duplicity check
    for (const auto& variable : net.variables) {
        if (variable.name.empty()) {
            result.add_error("Variable name not defined");
        }
        else if (!PetriNet::valid_id(variable.name)) {
            result.add_error("Invalid variable name: " + variable.name);
        }
        if (!variable_names.insert(variable.name).second) {
            result.add_error("Duplicate name of the variables: " + variable.name);
        }
        add_name_occurrence(all_names, variable.name, "variable");
    }

    // name collision via different object types
    for (const auto& item : all_names) {
        if (item.second.size() > 1) {
            result.add_error("Name collision for '" + item.first + "' between: " + join_descriptions(item.second));
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
        // should have at least one output check
        if (transition.output_arcs.empty()) {
            result.add_warning("Transition with no output arcs: " + transition.name);
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

/**
 * Helper method to validate names used inside guards, actions and delay expressions.
 * It checks output(), tokens(), elapsed() references and validates transition delay.
 *
 * @param net - Petrinet which inscriptions are to be checked
 * @param result - result of the partial validation with stored errors and warnings
 */
void Validator::validate_inscriptions(const PetriNet& net, ValidationResult& result) const {
    // place token and action check
    for (const auto& place : net.places) {
        check_inscription_references(net, result, place.add_token_action,
                "Place action of " + place.name);
    }

    // transition guard, delay and action check
    for (const auto& transition : net.transitions) {
        check_inscription_references(net, result, transition.condition.guard_expression,
                "Guard of transition " + transition.name);
        check_inscription_references(net, result, transition.condition.delay_expression,
                "Delay of transition " + transition.name);
        check_inscription_references(net, result, transition.action_code,
                "Action of transition " + transition.name);

        // valid delaz check
        if (!is_numeric_delay_or_existing_variable(net, transition.condition.delay_expression)) {
            result.add_error("Transition " + transition.name + " has invalid delay expression: " + 
                transition.condition.delay_expression + " (expected non-negative number or existing variable)");
        }
    }
}

/**
 * Helper method to warn about graph parts that are not reachable from initial marking.
 * This is a static approximation and ignores guards and input events.
 *
 * @param net - Petrinet which graph reachability is to be checked
 * @param result - result of the partial validation with stored errors and warnings
 */
void Validator::validate_reachability(const PetriNet& net, ValidationResult& result) const {
    std::set<std::string> reachable_places;
    std::set<std::string> potentially_fireable_transitions;

    // sets reachable palces
    for (const auto& place : net.places) {
        if (place.initial_tokens > 0) {
            reachable_places.insert(place.name);
        }
    }

    bool changed = true;
    while (changed) {
        changed = false;
        // propagates valid transitions and ignores no fireable transitions
        for (const auto& transition : net.transitions) {
            if (transition.input_arcs.empty()) {
                continue;
            }

            // transition input places reachable check
            bool all_inputs_reachable = true;
            for (const auto& arc : transition.input_arcs) {
                if (net.find_place(arc.place_name) == nullptr
                        || reachable_places.find(arc.place_name) == reachable_places.end()) {
                    all_inputs_reachable = false;
                    break;
                }
            }

            if (!all_inputs_reachable) {
                continue;
            }

            // sets validity for fireing
            if (potentially_fireable_transitions.insert(transition.name).second) {
                changed = true;
            }

            // propagates reachability to output places
            for (const auto& arc : transition.output_arcs) {
                if (net.find_place(arc.place_name) != nullptr
                        && reachable_places.insert(arc.place_name).second) {
                    changed = true;
                }
            }
        }
    }

    // sets warning about unreachable places
    for (const auto& place : net.places) {
        if (reachable_places.find(place.name) == reachable_places.end()) {
            result.add_warning("Unreachable place from initial marking: " + place.name);
        }
    }

    // sets warning about never fireable transitions
    for (const auto& transition : net.transitions) {
        if (potentially_fireable_transitions.find(transition.name) == potentially_fireable_transitions.end()) {
            result.add_warning("Transition can never fire from initial marking: " + transition.name);
        }
    }
}