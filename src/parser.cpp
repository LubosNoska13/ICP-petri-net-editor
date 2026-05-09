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
 * Helper function adds error into parser diagnosis
 *
 * @param result - the result of the diagnosis
 * @param line - the line where the message will be written
 * @param message - text of the message to be added
 */
void add_error(ParseResult& result, int line, const std::string& message) {
    result.errors.push_back(ParseError{line, message});
}

/**
 * Helper function removes comments with # or // outside quotes "" or ''
 *
 * @param line - the line where the comment is deleted
 * @return string with copy of the line without the inline comment
 */
std::string strip_inline_comment(const std::string& line) {
    bool in_string = false;
    bool in_char = false;
    bool escape = false;

    for (std::size_t char_index = 0; char_index < line.size(); ++char_index) {
        const char character = line[char_index];
        // ignoring character after '\' 
        if (escape) {
            escape = false;
            continue;
        }
        // sets found '\'
        if ((in_string || in_char) && character == '\\') {
            escape = true;
            continue;
        }
        // sets/ends string if '"' is not in apostrofs
        if (!in_char && character == '"') {
            in_string = !in_string;
            continue;
        }
        // sets/ends char if '\'' is not in string
        if (!in_string && character == '\'') {
            in_char = !in_char;
            continue;
        }
        // deletes comment
        if (!in_string && !in_char) {
            if (character == '#') {
                return line.substr(0, char_index);
            }
            if (character == '/' && char_index + 1 < line.size() && line[char_index + 1] == '/') {
                return line.substr(0, char_index);
            }
        }
    }
    return line;
}

/**
 * Helper function finds a character outside quotes "" or '', and outside square brackets []
 *
 * @param text - the text where the character is searched
 * @param searched - the character that is searched
 * @return position of the found character, or std::string::npos if the character is not found
 */
std::size_t find_top_level_char(const std::string& text, char searched) {
    bool in_string = false;
    bool in_char = false;
    bool escape = false;
    int square_depth = 0;

    for (std::size_t char_index = 0; char_index < text.size(); ++char_index) {
        const char character = text[char_index];
        // ignoring character after '\'
        if (escape) {
            escape = false;
            continue;
        }
        // sets found '\'
        if ((in_string || in_char) && character == '\\') {
            escape = true;
            continue;
        }
        // sets/ends string if '"' is not in apostrophes
        if (!in_char && character == '"') {
            in_string = !in_string;
            continue;
        }
        // sets/ends char if '\'' is not in string
        if (!in_string && character == '\'') {
            in_char = !in_char;
            continue;
        }
        // skips characters inside string or char
        if (in_string || in_char) {
            continue;
        }
        // increases square bracket depth
        if (character == '[') {
            ++square_depth;
            continue;
        }
        // decreases square bracket depth
        if (character == ']' && square_depth > 0) {
            --square_depth;
            continue;
        }
        // returns searched character only if it is outside square brackets
        if (character == searched && square_depth == 0) {
            return char_index;
        }
    }
    return std::string::npos;
}

/**
 * Helper function finds matching square bracket pair [] outside quotes "" or ''
 *
 * @param text - the text where the square bracket pair is searched
 * @param left - reference where the position of the left square bracket is stored
 * @param right - reference where the position of the right square bracket is stored
 * @return true if matching square bracket pair is found, false otherwise
 */
bool find_guard_bounds(const std::string& text, std::size_t& left, std::size_t& right) {
    bool in_string = false;
    bool in_char = false;
    bool escape = false;
    int depth = 0;
    left = std::string::npos;
    right = std::string::npos;

    for (std::size_t char_index = 0; char_index < text.size(); ++char_index) {
        const char character = text[char_index];
        // ignoring character after '\'
        if (escape) {
            escape = false;
            continue;
        }
        // sets found '\'
        if ((in_string || in_char) && character == '\\') {
            escape = true;
            continue;
        }
        // sets/ends string if '"' is not in apostrophes
        if (!in_char && character == '"') {
            in_string = !in_string;
            continue;
        }
        // sets/ends char if '\'' is not in string
        if (!in_string && character == '\'') {
            in_char = !in_char;
            continue;
        }
        // skips characters inside string or char
        if (in_string || in_char) {
            continue;
        }
        // finds left square bracket and increases depth
        if (character == '[') {
            if (depth == 0) {
                left = char_index;
            }
            ++depth;
        }
        // finds right square bracket and decreases depth
        else if (character == ']') {
            --depth;
            // matching right square bracket found
            if (depth == 0) {
                right = char_index;
                return true;
            }
            // right square bracket before left one check
            if (depth < 0) {
                return false;
            }
        }
    }
    return false;
}


/**
 * Helper function creates the vector of arcs after "translating"
 * the list of arcs from text
 *
 * @param text - the text with arc list
 * @param line_number - current source line number
 * @param result - parse result where diagnostics are stored
 * @return vector with arcs from the text or error if arc has invalid format
 */
std::vector<Arc> parse_arc_list(const std::string& text, int line_number, ParseResult& result) {
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
            add_error(result, line_number, "Invalid arc format: " + item);
            continue;
        }

        // creating arc and putting it into vector
        Arc arc;
        arc.place_name = match[1];
        arc.weight = std::stoi(match[2]);
        arcs.push_back(arc);
        // weigth validitz check
        if (arc.weight < 1) {
            add_error(result, line_number, "Arc weight must be >= 1: " + item);
            continue;
        }
    }
    return arcs;
}

/**
 * Helper function evaluates transition condition and 
 * sets it's event_name, delaz_expression and guard_expression
 *
 * @param text - text from which the transition condition is parsed
 * @param line_number - source line number used for error reporting
 * @param result - parse result object where parsing errors are stored
 * @return parsed transition condition
 */
TransitionCondition parse_when(const std::string& text, int line_number, ParseResult& result) {
    TransitionCondition condition;
    std::string string = trim(text);
    
    // finds delay (string, guard @ ignored) and evaluates it 
    auto at_position = find_top_level_char(string, '@');
    if (at_position != std::string::npos) {
        condition.delay_expression = trim(string.substr(at_position + 1));
        string = trim(string.substr(0, at_position));
        // empty delay check
        if (condition.delay_expression.empty()) {
            add_error(result, line_number, "Missing delay expression after @");
        }
    }

    std::size_t left_bound = std::string::npos;
    std::size_t right_bound = std::string::npos;
    // finds and sets event_name before [] and guard in []
    if (find_guard_bounds(string, left_bound, right_bound)) {
        condition.event_name = trim(string.substr(0, left_bound));
        condition.guard_expression = trim(string.substr(left_bound + 1, right_bound - left_bound - 1));
        // invalid text after [] check
        if (!trim(string.substr(right_bound + 1)).empty()) {
            add_error(result, line_number, "Unexpected text after guard condition");
        }
    }
    // checks invalid format and sets event_name
    else {
        if (string.find('[') != std::string::npos || string.find(']') != std::string::npos) {
            add_error(result, line_number, "Invalid guard condition brackets");
        }
        condition.event_name = trim(string);
    }
    return condition;
}

/**
 * Function collects a braced block that can span multiple lines. Braces inside
 * strings, character literals and comments are ignored. The text inside the
 * outer braces is returned without the braces
 *
 * @param lines - source lines with their original line numbers
 * @param index - current line index; updated to the line where the block ends
 * @param first_text - text of the first line where the opening brace is expected
 * @param block - output string containing the collected block without outer braces
 * @param result - parse result where parser diagnostics are stored
 * @param what - description of the parsed construct used in error messages
 * @return true if the braced block was collected successfully, false otherwise
 */
bool collect_braced_block(const std::vector<std::pair<int, std::string>>& lines, std::size_t& index, 
        const std::string& first_text, std::string& block, ParseResult& result, const std::string& what) {
    
    // valid start check
    std::size_t brace_position = first_text.find('{');
    if (brace_position == std::string::npos) {
        add_error(result, lines[index].first, "Missing opening { for " + what);
        return false;
    }

    // initializes output block and parser state variables
    block.clear();
    int depth = 0;
    bool started = false;
    bool finished = false;
    bool in_string = false;
    bool in_char = false;
    bool in_line_comment = false;
    bool in_block_comment = false;
    bool escape = false;

    // processes lines until matching closing brace is found
    for (std::size_t line_index = index; line_index < lines.size(); ++line_index) {
        const int line_number = lines[line_index].first;
        const std::string current = (line_index == index) ? first_text : lines[line_index].second;
        const std::size_t start = (line_index == index) ? brace_position : 0;

        // preserves newlines inside collected block
        if (line_index > index) {
            block += '\n';
            in_line_comment = false;
        }

        // processes current line character by character
        for (std::size_t char_index = start; char_index < current.size(); ++char_index) {
            const char character = current[char_index];

            // checks for unexpected text after the outer closing brace
            if (finished) {
                std::string rest = trim(strip_inline_comment(current.substr(char_index)));
                if (!rest.empty()) {
                    add_error(result, line_number, "Unexpected text after closing } of " + what);
                }
                index = line_index;
                return true;
            }
            // ignores brace handling inside line comments
            if (in_line_comment) {
                if (started) {
                    block += character;
                }
                continue;
            }
            // ignores brace handling inside block comments
            if (in_block_comment) {
                if (started) {
                    block += character;
                }
                if (character == '*' && char_index + 1 < current.size() && current[char_index + 1] == '/') {
                    if (started) {
                        block += current[char_index + 1];
                    }
                    ++char_index;
                    in_block_comment = false;
                }
                continue;
            }
            // handles escaped characters inside strings and character literals
            if (escape) {
                if (started) {
                    block += character;
                }
                escape = false;
                continue;
            }
            // detects escape sequence inside string or character literal
            if ((in_string || in_char) && character == '\\') {
                if (started) {
                    block += character;
                }
                escape = true;
                continue;
            }
            // detects start of line comment
            if (!in_string && !in_char && character == '/' && char_index + 1 < current.size() && current[char_index + 1] == '/') {
                if (started) {
                    block += character;
                    block += current[char_index + 1];
                }
                ++char_index;
                in_line_comment = true;
                continue;
            }
            // detects start of block comment
            if (!in_string && !in_char && character == '/' && char_index + 1 < current.size() && current[char_index + 1] == '*') {
                if (started) {
                    block += character;
                    block += current[char_index + 1];
                }
                ++char_index;
                in_block_comment = true;
                continue;
            }
            // toggles string literal state
            if (!in_char && character == '"') {
                if (started) {
                    block += character;
                }
                in_string = !in_string;
                continue;
            }
            // toggles character literal state
            if (!in_string && character == '\'') {
                if (started) {
                    block += character;
                }
                in_char = !in_char;
                continue;
            }
            // handles opening brace and increases nesting depth
            if (!in_string && !in_char && character == '{') {
                if (started) {
                    block += character;
                }
                else {
                    started = true;
                }
                ++depth;
                continue;
            }
            // handles closing brace and decreases nesting depth
            if (!in_string && !in_char && character == '}') {
                --depth;
                // outer closing brace found
                if (depth == 0) {
                    finished = true;
                    continue;
                }
                // closing brace without matching opening brace
                if (depth < 0) {
                    add_error(result, line_number, "Unexpected } in " + what);
                    index = line_index;
                    return false;
                }
                block += character;
                continue;
            }
            // collects character inside the outer braces
            if (started) {
                block += character;
            }
        }
        // returns successfully after whole block was collecte
        if (finished) {
            index = line_index;
            return true;
        }
    }
    // reports missing closing brace
    add_error(result, lines[index].first, "Unterminated braced block for " + what);
    index = lines.size() - 1;
    return false;
}

}

/**
 * Method loads file from path in argument and on it's content calls 
 * parse_string function
 *
 * @param path - the path to the file with Petrinet
 * @return parse result with Petrinet and parser diagnostics
 */
ParseResult Parser::parse_file(const std::string& path) const {
    std::ifstream file(path);
    if (!file) {
        ParseResult result;
        add_error(result, 0, "Cannot open file: " + path);
        return result;
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
 * @return parse result with Petrinet and parser diagnostics
 */
ParseResult Parser::parse_string(const std::string& content) const {
    ParseResult result;
    PetriNet& net = result.net;
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
    // lines list loading
    std::vector<std::pair<int, std::string>> lines;
    std::stringstream input_stream(content);
    std::string original_line;
    int line_number = 1;
    while (std::getline(input_stream, original_line)) {
        lines.push_back({line_number++, original_line});
    }

    // sets variables for file processing
    Section section = Section::None;
    Transition* current_transition = nullptr;
    int priority_counter = 0;

    // reading file bz lines and processing each line
    for (std::size_t line_index = 0; line_index < lines.size(); ++line_index) {
        const int current_line_number = lines[line_index].first;
        std::string raw_line = lines[line_index].second;
        std::string line = trim(strip_inline_comment(raw_line));

        // comments and empty lines are ignored
        if (line.empty()) {
            continue;
        }

        // sets section by line content
        if (    line == "Jméno sítě:" || 
                line == "Jmeno site:" || 
                line == "Name:" ||
                line == "Meno siete:") {
            section = Section::Name;
            current_transition = nullptr;
            continue;
        }
        if (    line == "Komentář:" ||
                line == "Komentar:" ||
                line == "Comment:" ||
                line == "Komentár:") {
            section = Section::Comment;
            current_transition = nullptr;
            continue;
        }
        if (    line == "Vstupy:" || 
                line == "Inputs:") {
            section = Section::Inputs;
            current_transition = nullptr;
            continue;
        }
        if (    line == "Výstupy:" || 
                line == "Vystupy:" || 
                line == "Outputs:") {
            section = Section::Outputs;
            current_transition = nullptr;
            continue;
        }
        if (    line == "Proměnné:" || 
                line == "Promenne:" || 
                line == "Variables:" ||
                line == "Premenné:" ||
                line == "Premenne:") {
            section = Section::Variables;
            current_transition = nullptr;
            continue;
        }
        if (    starts_with(line, "Místa") ||
                starts_with(line, "Mista") ||
                starts_with(line, "Places") ||
                starts_with(line, "Miesta")) {
            section = Section::Places;
            current_transition = nullptr;
            continue;
        }
        if (    starts_with(line, "Přechody") ||
                starts_with(line, "Prechody") ||
                starts_with(line, "Transitions")) {
            section = Section::Transitions;
            current_transition = nullptr;
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
                // name check
                if (!PetriNet::valid_id(line)) {
                    add_error(result, current_line_number, "Invalid input name: " + line);
                }
                else {
                    net.inputs.push_back(Input{line});
                }
                break;
            case Section::Outputs:
                // name chcek
                if (!PetriNet::valid_id(line)) {
                    add_error(result, current_line_number, "Invalid output name: " + line);
                }
                else {
                    net.outputs.push_back(Output{line});
                }
                break;
            // before filling info checks for valid format otherwise ignored
            case Section::Variables: {
                std::smatch match;
                if (std::regex_match(line, match,
                        std::regex(R"(([A-Za-z_][A-Za-z0-9_:<>]*)\s+([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.+))"))) {
                    Variable var;
                    var.type = match[1];
                    var.name = match[2];
                    var.initializer = trim(match[3]);
                    net.variables.push_back(var);
                }
                else {
                    add_error(result, current_line_number, "Invalid variable format: " + line);
                }
                break;
            }
            // before filling info checks for valid format otherwise ignored
            case Section::Places: {
                std::smatch match;
                if (std::regex_match(line, match,
                        std::regex(R"(([A-Za-z_][A-Za-z0-9_]*)\s*\(\s*([0-9]+)\s*\)\s*:\s*(.*))"))) {
                    Place place;
                    place.name = match[1];
                    place.initial_tokens = std::stoi(match[2]);
                    std::string action;
                    std::string action_text = match[3];
                    if (collect_braced_block(lines, line_index, action_text, action, result, "place action")) {
                        place.add_token_action = trim(action);
                    }
                    net.places.push_back(place);
                }
                else {
                    add_error(result, current_line_number, "Invalid place format or unknown line: " + line);
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
                    add_error(result, current_line_number, "Transition property before transition header: " + line);
                    break;
                }
                // fills info like arcs, condition and action
                if (starts_with(line, "in:")) {
                    current_transition->input_arcs = parse_arc_list(line.substr(3), current_line_number, result);
                } 
                else if (starts_with(line, "out:")) {
                    current_transition->output_arcs = parse_arc_list(line.substr(4), current_line_number, result);
                } 
                else if (starts_with(line, "when:")) {
                    current_transition->condition = parse_when(line.substr(5), current_line_number, result);
                } 
                else if (starts_with(line, "do:")) {
                    std::string action;
                    std::string action_text = raw_line.substr(raw_line.find("do:") + 3);
                    if (collect_braced_block(lines, line_index, action_text, action, result, "transition action")) {
                        current_transition->action_code = trim(action);
                    }
                }
                else {
                    add_error(result, current_line_number, "Unknown transition line: " + line);
                }
                break;
            }
            case Section::None:
                add_error(result, current_line_number, "Line outside any section: " + line);
                break;
        }
    }
    return result;
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
        out << "\t" << place.name << " (" << place.initial_tokens << ") :";
        if (trim(place.add_token_action).empty()) {
            out << " { }\n";
        }
        else {
            out << " {\n" << place.add_token_action << "\n\t}\n";
        }
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
        if (trim(transition.action_code).empty()) {
            out << "\tdo: { }\n";
        }
        else {
            out << "\tdo: {\n" << transition.action_code << "\n\t}\n";
        }
    }
    return out.str();
}