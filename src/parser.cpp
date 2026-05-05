// autor: Jurišinová Daniela (xjurisd00)

#include "parser.hpp"
#include <regex>
#include <sstream>
#include <fstream>
#include <stdexcept>

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
 * Helper function checks if the string starts with the prefix
 *
 * @param string - the string we're checking
 * @param prefix - the prefix that we search for
 * @return true if the string starts with the prefix otherwise false
 */
bool starts_with(const std::string& string, const std::string& prefix) {
    return string.rfind(prefix, 0) == 0;
}

/**
 * Helper function creates the vector of arcs after "translating"
 * the list of arcs from text
 *
 * @param text - the text with arc list
 * @return vector with arcs from the text or error if arc has invalid format
 */
std::vector<Arc> parse_arc_list(const std::string& text) {
    std::vector<Arc> arcs;
    std::stringstream input_stream(text);
    std::string item;

    while (std::getline(input_stream, item, ',')) {
        item = trim(item);
        if (item.empty()) {
            continue;
        }

        // valid format of arc check
        std::smatch match;
        if (!std::regex_match(item, match, std::regex(R"(([A-Za-z_][A-Za-z0-9_]*)\s*\*\s*([0-9]+))"))) {
            throw std::runtime_error("Invalid arc format: " + item);
        }

        // creating arc and putting it into vector
        Arc arc;
        arc.place_name = match[1];
        arc.weight = std::stoi(match[2]);
        arcs.push_back(arc);
    }
    return arcs;
}

/**
 * Helper function evaluates text condition for transition and 
 * sets it's event_name, delaz_expression and guard_expression
 *
 * @param text - text from with the condition is evaluated
 * @return the condition for the transition
 */
TransitionCondition parse_when(const std::string& text) {
    TransitionCondition condition;
    std::string string = trim(text);
    
    // finds delay and evaluates it 
    auto at_position = string.find('@');
    if (at_position != std::string::npos) {
        condition.delay_expression = trim(string.substr(at_position + 1));
        string = trim(string.substr(0, at_position));
    }

    // finds guard condition and evaluates it
    auto left_bound = string.find('[');
    auto right_bound = string.rfind(']');
    if (left_bound != std::string::npos && right_bound != std::string::npos && right_bound > left_bound) {
        condition.event_name = trim(string.substr(0, left_bound));
        condition.guard_expression = trim(string.substr(left_bound + 1, right_bound - left_bound - 1));
    }
    else {
        condition.event_name = trim(string);
    }
    return condition;
}

}

/**
 * Method loads file from path in argument and on it's content calls 
 * parse_string function
 *
 * @param path - the path to the file with Petrinet
 * @return Petrinet from the file or error if load error occures
 */
PetriNet Parser::parse_file(const std::string& path) const {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open file" + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return parse_string(buffer.str());
}

/**
 * Method reads file with text format of Petrinet and fills Petrinet info
 * if the format of each element in file is correct
 *
 * @param content - the text format of Petrinet 
 * @return Petrinet structure filled by content of the file
 */
PetriNet Parser::parse_string(const std::string& content) const {
    PetriNet net;
    enum class Section {
        None,
        Name,
        Comment,
        Inputs,
        Outputs,
        Variables,
        Places,
        Transitions
    };

    // sets variables for file processing
    Section section = Section::None;
    std::stringstream input_stream(content);
    std::string line;
    Transition* current_transition = nullptr;
    int priority_counter = 0;

    // reading file bz lines and processing each line
    while (std::getline(input_stream, line)) {
        line = trim(line);

        // comments and empty lines are ignored
        if (line.empty()) {
            continue;
        }
        if (starts_with(line, "#")) {
            continue;
        }

        // sets section by line content
        if (    line == "Jméno sítě:" || 
                line == "Jmeno site:" || 
                line == "Name:" ||
                line == "Meno siete:") {
            section = Section::Name;
            continue;
        }
        if (    line == "Komentář:" ||
                line == "Komentar:" ||
                line == "Comment:" ||
                line == "Komentár:") {
            section = Section::Comment;
            continue;
        }
        if (    line == "Vstupy:" || 
                line == "Inputs:") {
            section = Section::Inputs;
            continue;
        }
        if (    line == "Výstupy:" || 
                line == "Vystupy:" || 
                line == "Outputs:") {
            section = Section::Outputs;
            continue;
        }
        if (    line == "Proměnné:" || 
                line == "Promenne:" || 
                line == "Variables:" ||
                line == "Premenné:" ||
                line == "Premenne:") {
            section = Section::Variables;
            continue;
        }
        if (    starts_with(line, "Místa") ||
                starts_with(line, "Mista") ||
                starts_with(line, "Places") ||
                starts_with(line, "Miesta")) {
            section = Section::Places;
            continue;
        }
        if (    starts_with(line, "Přechody") ||
                starts_with(line, "Prechody") ||
                starts_with(line, "Transitions")) {
            section = Section::Transitions;
            continue;
        }

        // fills info into Petrinet depending on set section
        switch (section) {
            case Section::Name:
                net.name = line;
                break;
            case Section::Comment:
                if (!net.comment.empty()) {
                    net.comment += "\n";
                }
                net.comment += line;
                break;
            case Section::Inputs:
                net.inputs.push_back(Input{line});
                break;
            case Section::Outputs:
                net.outputs.push_back(Output{line});
                break;
            // before filling info checks for valid format otherwise ignored
            case Section::Variables: {
                std::smatch match;
                if (std::regex_match(line, match,
                        std::regex(R"(([A-Za-z_][A-Za-z0-9_]*)\s+([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.+))"))) {
                    Variable var;
                    var.type = match[1];
                    var.name = match[2];
                    var.initializer = trim(match[3]);
                    net.variables.push_back(var);
                }
                break;
            }
            // before filling info checks for valid format otherwise ignored
            case Section::Places: {
                std::smatch match;
                if (std::regex_match(line, match,
                        std::regex(R"(([A-Za-z_][A-Za-z0-9_]*)\s*\(\s*([0-9]+)\s*\)\s*:\s*\{(.*)\})"))) {
                    Place place;
                    place.name = match[1];
                    place.initial_tokens = std::stoi(match[2]);
                    place.add_token_action = trim(match[3]);
                    net.places.push_back(place);
                }
                break;
            }
            // creates transition if needed and fills arcs, condition and action
            case Section::Transitions: {
                std::smatch transition_header;
                // valid transition, creates new transition nad fills info
                if (std::regex_match(line, transition_header,
                        std::regex(R"(([A-Za-z_][A-Za-z0-9_]*)\s*:)"))) {
                    Transition transition;
                    transition.name = transition_header[1];
                    transition.priority = priority_counter++;
                    net.transitions.push_back(transition);
                    current_transition = &net.transitions.back();
                    break;
                }
                // no existing transition to fill info
                if (current_transition == nullptr) {
                    break;
                }
                // fills info like arcs, condition and action
                if (starts_with(line, "in:")) {
                    current_transition->input_arcs = parse_arc_list(line.substr(3));
                } 
                else if (starts_with(line, "out:")) {
                    current_transition->output_arcs = parse_arc_list(line.substr(4));
                } 
                else if (starts_with(line, "when:")) {
                    current_transition->condition = parse_when(line.substr(5));
                } 
                else if (starts_with(line, "do:")) {
                    std::string action = trim(line.substr(3));
                    if (!action.empty() && action.front() == '{' && action.back() == '}') {
                        action = action.substr(1, action.size() - 2);
                    }
                    current_transition->action_code = trim(action);
                }
                break;
            }
            case Section::None:
                break;
        }
    }
    return net;
}

/**
 * Method creates the file if it doesn't exists and checks for error.
 * Then method calls other function to write Petrinet into file.
 *
 * @param net - Petrinet to be translated and writen in it's text format
 * @param path - the path to the file where Petrinet will be writen
 */
void Writer::write_file(const PetriNet& net, const std::string& path) const {
    std::ofstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot write file: " + path);
    }
    file << write_string(net);
}

/**
 * Method translates Petrinet into string with the valid interpretation
 * of the Petrinet (name, comment,...).
 *
 * @param net - Petrinet to be interpreted 
 * @return string with the text interpretation of the Petrinet in argumnet
 */
std::string Writer::write_string(const PetriNet& net) const {
    std::ostringstream out;

    out << "Jméno sítě:\n";
    out << "\t" << net.name << "\n";

    out << "Komentář:\n";
    out << "\t" << net.comment << "\n";

    out << "Vstupy:\n";
    for (const auto& input : net.inputs) {
        out << "\t" << input.name << "\n";
    }

    out << "Výstupy:\n";
    for (const auto& output : net.outputs) {
        out << "\t" << output.name << "\n";
    }

    out << "Proměnné:\n";
    for (const auto& var : net.variables) {
        out << "\t" << var.type << " " << var.name << " = " << var.initializer << "\n";
    }

    out << "Místa (počáteční tokeny, volitelně akce):\n";
    for (const auto& place : net.places) {
        out << "\t" << place.name << " (" << place.initial_tokens << ") : { "
            << place.add_token_action << " }\n";
    }

    out << "Přechody a jejich podmínky:\n\n";
    for (const auto& transition : net.transitions) {
        out << transition.name << " :\n";
        // in:
        out << "\tin:\t";
        for (size_t input = 0; input < transition.input_arcs.size(); ++input) {
            if (input > 0) out << ", ";
            out << transition.input_arcs[input].place_name << "*" << transition.input_arcs[input].weight;
        }
        out << "\n";
        // out:
        out << "\tout:\t";
        for (size_t output = 0; output < transition.output_arcs.size(); ++output) {
            if (output > 0) out << ", ";
            out << transition.output_arcs[output].place_name << "*" << transition.output_arcs[output].weight;
        }
        out << "\n";
        // when:
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
        // do:
        out << "\tdo: { " << transition.action_code << " }\n";
    }
    return out.str();
}