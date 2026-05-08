// autor: Jurišinová Daniela (xjurisd00)

#include "model.hpp"
#include "evaluator.hpp"

#include <algorithm>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <stdexcept>
#include <limits>
#include <utility>

/**
* Method creates integer evaluation value
*
* @param value - integer value to store
* @return EvalValue containing integer value
*/
EvalValue EvalValue::integer(int64_t value) {
    EvalValue result;
    result.type = Type::Int;
    result.int_value = value;
    result.bool_value = value != 0;
    return result;
}

/**
* Method creates boolean evaluation value
*
* @param value - boolean value to store
* @return EvalValue containing boolean value
*/
EvalValue EvalValue::boolean(bool value) {
    EvalValue result;
    result.type = Type::Bool;
    result.bool_value = value;
    result.int_value = value ? 1 : 0;
    return result;
}

/**
* Method creates string evaluation value
*
* @param value - string value to store
* @return EvalValue containing string value
*/
EvalValue EvalValue::string(std::string value) {
    EvalValue result;
    result.type = Type::String;
    result.string_value = std::move(value);
    return result;
}

/**
* Method converts evaluation value to bool
*
* @return bool form of the stored value
*/
bool EvalValue::as_bool() const {
    // int to bool
    if (type == Type::Int) {
        return int_value != 0;
    }
    // string to bool
    if (type == Type::String) {
        if (string_value == "true") {
            return true;
        }

        if (string_value == "false") {
            return false;
        }
        throw std::runtime_error("Cannot convert string to bool");
    }
    return bool_value;
}

/**
* Method converts evaluation value to integer
*
* @return integer form of the stored value
*/
int64_t EvalValue::as_int() const {
    // string to int
    if (type == Type::String) {
        char* end = nullptr;
        long long value = std::strtoll(string_value.c_str(), &end, 10);
        // int validity check
        if (end == string_value.c_str() || *end != '\0') {
            throw std::runtime_error("String is not a valid integer: " + string_value);
        }
        return static_cast<int64_t>(value);
    }
    // bool to int
    if (type == Type::Bool) {
        if (bool_value) {
            return 1;
        } else {
            return 0;
        }
    }
    return int_value;
}

/**
* Method converts evaluation value to string representation.
*
* @return string representation of the stored value
*/
std::string EvalValue::as_string() const {
    // bool to string
    if (type == Type::Bool) {
        return bool_value ? "true" : "false";
    }
    if (type == Type::Int) {
        return std::to_string(int_value);
    }
    return string_value;
}

// helper functions and classes for this file

namespace {

class ExpressionParser {
public:
    // constructor
    ExpressionParser(std::string text, EvaluationInterface& interface): 
        m_text(std::move(text)), m_interface(interface) {}
    /**
    * Method parses whole expression and checks for unexpected remaining tokens.
    *
    * @return evaluated expression value
    */
    EvalValue parse() {
        EvalValue value = parse_or();
        skip_whitespaces();
        // unexpected token check
        if (!end_of_expression()) {
            throw std::runtime_error("Unexpected token near: " + m_text.substr(m_pos));
        }
        return value;
    }

private:
    std::string m_text;
    EvaluationInterface& m_interface;
    std::size_t m_pos = 0;

    /**
    * Method checks whether parser reached end of expression.
    *
    * @return true if there are no more characters otherwise false
    */
    bool end_of_expression() const { return m_pos >= m_text.size(); }

    /**
    * Method skips whitespace characters at current parser position
    */
    void skip_whitespaces() {
        while (!end_of_expression() && std::isspace(static_cast<unsigned char>(m_text[m_pos]))) {
            ++m_pos;
        }
    }

    /**
    * Method tries to consume given token from current parser position
    *
    * @param token - token to consume
    * @return true if token was consumed otherwise false
    */
    bool consume(const std::string& token) {
        skip_whitespaces();
        if (m_text.compare(m_pos, token.size(), token) == 0) {
            m_pos += token.size();
            return true;
        }
        return false;
    }

    /**
    * Method requires given token at current parser position.
    *
    * @param token - expected token
    */
    void expect(const std::string& token) {
        if (!consume(token)) {
            throw std::runtime_error("Expected '" + token + "'");
        }
    }

    /**
    * Method parses logical OR expression
    *
    * @return evaluated OR expression value
    */
    EvalValue parse_or() {
        EvalValue left = parse_and();
        while (consume("||")) {
            EvalValue right = parse_and();
            left = EvalValue::boolean(left.as_bool() || right.as_bool());
        }
        return left;
    }

    /**
    * Method parses logical AND expression
    *
    * @return evaluated AND expression value
    */
    EvalValue parse_and() {
        EvalValue left = parse_equality();
        while (consume("&&")) {
            EvalValue right = parse_equality();
            left = EvalValue::boolean(left.as_bool() && right.as_bool());
        }
        return left;
    }

    /**
    * Method parses == and != expression
    *
    * @return evaluated equality expression value
    */
    EvalValue parse_equality() {
        EvalValue left = parse_relational();
        while (true) {
            if (consume("==")) {
                EvalValue right = parse_relational();
                left = EvalValue::boolean(compare(left, right, "=="));
            } 
            else if (consume("!=")) {
                EvalValue right = parse_relational();
                left = EvalValue::boolean(compare(left, right, "!="));
            } 
            else {
                return left;
            }
        }
    }

    /**
    * Method parses comparison expression
    *
    * @return evaluated relational expression value
    */
    EvalValue parse_relational() {
        EvalValue left = parse_additive();
        while (true) {
            if (consume(">=")) {
                EvalValue right = parse_additive();
                left = EvalValue::boolean(compare(left, right, ">="));
            } 
            else if (consume("<=")) {
                EvalValue right = parse_additive();
                left = EvalValue::boolean(compare(left, right, "<="));
            } 
            else if (consume(">")) {
                EvalValue right = parse_additive();
                left = EvalValue::boolean(compare(left, right, ">"));
            } 
            else if (consume("<")) {
                EvalValue right = parse_additive();
                left = EvalValue::boolean(compare(left, right, "<"));
            } 
            else {
                return left;
            }
        }
    }

    /**
    * Method parses + and - operators (plus, minus, concatenation)
    *
    * @return evaluated additive expression value
    */
    EvalValue parse_additive() {
        EvalValue left = parse_multiplicative();

        while (true) {
            // + operator
            if (consume("+")) {
                EvalValue right = parse_multiplicative();
                // string can concatenate
                if (left.type == EvalValue::Type::String || right.type == EvalValue::Type::String) {
                    left = EvalValue::string(left.as_string() + right.as_string());
                } 
                // int can sum
                else {
                    left = EvalValue::integer(left.as_int() + right.as_int());
                }
            }
            // - operator
            else if (consume("-")) {
                EvalValue right = parse_multiplicative();
                left = EvalValue::integer(left.as_int() - right.as_int());
            } 
            else {
                return left;
            }
        }
    }

    /**
    * Method parses multiplication, division and modulo expression
    *
    * @return evaluated multiplicative expression value
    */
    EvalValue parse_multiplicative() {
        EvalValue left = parse_unary();
        while (true) {
            // * operator
            if (consume("*")) {
                EvalValue right = parse_unary();
                left = EvalValue::integer(left.as_int() * right.as_int());
            } 
            // / operator
            else if (consume("/")) {
                EvalValue right = parse_unary();
                int64_t divisor = right.as_int();
                // division bz 0 check
                if (divisor == 0) {
                    throw std::runtime_error("Division by zero");
                }
                left = EvalValue::integer(left.as_int() / divisor);
            } 
            // modulor operator
            else if (consume("%")) {
                EvalValue right = parse_unary();
                int64_t divisor = right.as_int();
                // division by zero check
                if (divisor == 0) {
                    throw std::runtime_error("Modulo by zero");
                }
                left = EvalValue::integer(left.as_int() % divisor);
            } 
            else {
                return left;
            }
        }
    }

    /**
    * Method parses unary operators and primary expression
    *
    * @return evaluated unary expression value
    */
    EvalValue parse_unary() {
        // negation operator
        if (consume("!")) {
            return EvalValue::boolean(!parse_unary().as_bool());
        }
        // math negation
        if (consume("-")) {
            return EvalValue::integer(-parse_unary().as_int());
        }
        // unarz plus
        if (consume("+")) {
            return EvalValue::integer(parse_unary().as_int());
        }
        return parse_primary();
    }

    /**
    * Method parses primary expression such as literal, identifier, function call or parentheses.
    *
    * @return evaluated primary expression value
    */
    EvalValue parse_primary() {
        skip_whitespaces();
        // "(expression)"
        if (consume("(")) {
            EvalValue value = parse_or();
            expect(")");
            return value;
        }
        // string literal
        if (!end_of_expression() && m_text[m_pos] == '"') {
            return EvalValue::string(parse_string_literal());
        }
        // number literal
        if (!end_of_expression() && std::isdigit(static_cast<unsigned char>(m_text[m_pos]))) {
            return parse_number();
        }
        if (!end_of_expression() && (std::isalpha(static_cast<unsigned char>(m_text[m_pos])) || m_text[m_pos] == '_')) {
            std::string ident = parse_identifier();
            // bool constant
            if (ident == "true") {
                return EvalValue::boolean(true);
            }
            if (ident == "false") {
                return EvalValue::boolean(false);
            }

            // function call check
            if (consume("(")) {
                return parse_function_call(ident);
            }
            // variable check
            if (m_interface.has_variable(ident)) {
                return m_interface.variable_value(ident);
            }
            throw std::runtime_error("Unknown identifier: " + ident);
        }
        throw std::runtime_error("Expected expression near: " + m_text.substr(m_pos));
    }

    /**
    * Method parses integer number literal
    *
    * @return integer EvalValue parsed from expression
    */
    EvalValue parse_number() {
        skip_whitespaces();
        std::size_t start = m_pos;
        while (!end_of_expression() && std::isdigit(static_cast<unsigned char>(m_text[m_pos]))) {
            ++m_pos;
        }
        return EvalValue::integer(std::stoll(m_text.substr(start, m_pos - start)));
    }

    /**
    * Method parses identifier from current parser position.
    *
    * @return parsed identifier name
    */
    std::string parse_identifier() {
        skip_whitespaces();
        std::size_t start = m_pos;
        while (!end_of_expression() && (std::isalnum(static_cast<unsigned char>(m_text[m_pos])) || m_text[m_pos] == '_')) {
            ++m_pos;
        }
        return m_text.substr(start, m_pos - start);
    }

    /**
    * Method parses string literal with escape sequences.
    *
    * @return parsed string literal value
    */
    std::string parse_string_literal() {
        skip_whitespaces();
        expect("\"");
        std::string value;

        while (!end_of_expression()) {
            char character = m_text[m_pos++];
            // closing quote check
            if (character == '"') {
                return value;
            }
            // processes end of squence
            if (character == '\\') {
                // no end in sequence check
                if (end_of_expression()) {
                    throw std::runtime_error("Unterminated escape sequence in string literal");
                }
                char escaped = m_text[m_pos++];
                switch (escaped) {
                    case 'n': value.push_back('\n'); break;
                    case 't': value.push_back('\t'); break;
                    case 'r': value.push_back('\r'); break;
                    case '\\': value.push_back('\\'); break;
                    case '"': value.push_back('"'); break;
                    default: value.push_back(escaped); break;
                }
            } else {
                value.push_back(character);
            }
        }
        throw std::runtime_error("Unterminated string literal");
    }

    /**
    * Method parses and evaluates supported function call.
    *
    * @param name - called function name
    * @return value returned by called function
    */
    EvalValue parse_function_call(const std::string& name) {
        if (name == "valueof") {
            std::string argument = parse_required_string_arg(name);
            return EvalValue::string(m_interface.input_value(argument));
        }
        if (name == "defined") {
            std::string argument = parse_required_string_arg(name);
            return EvalValue::boolean(m_interface.input_defined(argument));
        }
        if (name == "tokens") {
            std::string argument = parse_required_string_arg(name);
            return EvalValue::integer(m_interface.token_count(argument));
        }
        if (name == "elapsed") {
            std::string argument = parse_required_string_arg(name);
            return EvalValue::integer(m_interface.ms_elapsed(argument));
        }
        if (name == "atoi") {
            EvalValue argument = parse_or();
            expect(")");
            return EvalValue::integer(std::atoi(argument.as_string().c_str()));
        }
        if (name == "now") {
            expect(")");
            return EvalValue::integer(m_interface.ms_now());
        }
        throw std::runtime_error("Unknown function: " + name);
    }

    /**
    * Method parses required string literal argument of function.
    *
    * @param function_name - name of function whose argument is parsed
    * @return parsed string argument
    */
    std::string parse_required_string_arg(const std::string& function_name) {
        // argument check
        skip_whitespaces();
        if (end_of_expression() || m_text[m_pos] != '"') {
            throw std::runtime_error(function_name + " expects a string literal argument");
        }
        // parses string argument
        std::string argument = parse_string_literal();
        expect(")");
        return argument;
    }

    /**
    * Method compares two evaluation values using selected operator.
    *
    * @param left - left comparison operand
    * @param right - right comparison operand
    * @param op - comparison operator
    * @return comparison result
    */
    bool compare(const EvalValue& left, const EvalValue& right, const std::string& operat) {
        // string comparison if at least one string operand
        if (left.type == EvalValue::Type::String || right.type == EvalValue::Type::String) {
            const std::string l = left.as_string();
            const std::string r = right.as_string();
            if (operat == "==") return l == r;
            if (operat == "!=") return l != r;
            if (operat == ">=") return l >= r;
            if (operat == "<=") return l <= r;
            if (operat == ">") return l > r;
            if (operat == "<") return l < r;
        }
        const int64_t l = left.as_int();
        const int64_t r = right.as_int();
        if (operat == "==") return l == r;
        if (operat == "!=") return l != r;
        if (operat == ">=") return l >= r;
        if (operat == "<=") return l <= r;
        if (operat == ">") return l > r;
        if (operat == "<") return l < r;
        throw std::runtime_error("Unknown comparison operator: " + operat);
    }
};

/**
* Function finds matching closing character for opening character at given position.
*
* @param text - text where matching character is searched
* @param open_pos - position of opening character
* @param open_char - opening character
* @param close_char - closing character
* @return position of matching closing character
*/
std::size_t find_matching(const std::string& text, std::size_t open_pos, char open_char, char close_char) {
    int depth = 0;
    bool in_string = false;
    bool escape = false;
    for (std::size_t char_index = open_pos; char_index < text.size(); ++char_index) {
        char character = text[char_index];
        
        // ignores special characters inside string
        if (in_string) {
            if (escape) {
                escape = false;
            } else if (character == '\\') {
                escape = true;
            } else if (character == '"') {
                in_string = false;
            }
            continue;
        }

        // string literal start check
        if (character == '"') {
            in_string = true;
            continue;
        }

        // depth update
        if (character == open_char) {
            ++depth;
        } 
        else if (character == close_char) {
            --depth;
            if (depth == 0) {
                return char_index;
            }
            if (depth < 0) {
                break;
            }
        }
    }
    throw std::runtime_error(std::string("Missing matching '") + close_char + "'");
}

/**
* Function checks whether text starts with keyword and keyword is not part of longer identifier
*
* @param text - text where keyword is checked
* @param keyword - keyword to check
* @return true if text starts with standalone keyword otherwise false
*/
bool starts_with_keyword(const std::string& text, const std::string& keyword) {
    if (text.compare(0, keyword.size(), keyword) != 0) {
        return false;
    }
    // not part of identifier check
    return text.size() == keyword.size() || !std::isalnum(static_cast<unsigned char>(text[keyword.size()]));
}

/**
* Function converts value according to declared variable type
*
* @param name - variable name used in error message
* @param declared_type - declared variable type
* @param value - value to convert
* @return converted value with correct evaluation type
*/
EvalValue convert_for_declared_type(const std::string& name, const std::string& declared_type, const EvalValue& value) {
    std::string type = declared_type;
    std::transform(type.begin(), type.end(), type.begin(), [](unsigned char c) { return std::tolower(c); });

    if (type == "string" || type == "std::string") {
        return EvalValue::string(value.as_string());
    }
    if (type == "bool") {
        return EvalValue::boolean(value.as_bool());
    }
    if (type == "int" || type == "long" || type == "int64" || type == "int64_t" || type == "longlong" || type == "longlongint") {
        return EvalValue::integer(value.as_int());
    }
    throw std::runtime_error("Unsupported variable type for '" + name + "': " + declared_type);
}

/**
 * Checks whether the '=' character at the given position represents
 * a standalone assignment operator. Also checks the current nesting depth, 
 * so assignment is accepted only on the top level of the statement.
 *
 * @param statement The statement being analyzed.
 * @param index Position of the character to check.
 * @param depth Current nesting depth, for example inside parentheses.
 * @return true if statement[index] is an assignment operator, false otherwise.
 */
bool is_assignment_operator(const std::string& statement, std::size_t index, int depth) {
    // top level check
    if (statement[index] != '=' || depth != 0) {
        return false;
    }

    const bool has_next = index + 1 < statement.size();
    const bool has_previous = index > 0;

    // == check
    if (has_next && statement[index + 1] == '=') {
        return false;
    }
    // !=, <=, >= check
    if (has_previous) {
        char previous = statement[index - 1];
        if (previous == '!' || previous == '<' || previous == '>') {
            return false;
        }
    }
    return true;
}

} // namespace

/**
* Method evaluates guard expression as boolean value
*
* @param expression - guard expression to evaluate
* @param interface - evaluation interface used for variables and functions
* @return true if guard expression is true or empty otherwise false
*/
bool Evaluator::guard_evaluation(const std::string& expression, EvaluationInterface& interface) const {
    std::string trimmed_expression = trim(expression);
    // empty term is alwazs true
    if (trimmed_expression.empty()) {
        return true;
    }
    return value_evaluation(trimmed_expression, interface).as_bool();
}

/**
* Method evaluates expression as 64-bit integer.
*
* @param expression - expression to evaluate
* @param interface - evaluation interface used for variables and functions
* @return evaluated 64-bit integer value
*/
int64_t Evaluator::int64_evaluation(const std::string& expression, EvaluationInterface& interface) const {
    return value_evaluation(expression, interface).as_int();
}

/**
* Method evaluates expression as integer and checks integer range
*
* @param expression - expression to evaluate
* @param interface - evaluation interface used for variables and functions
* @return evaluated integer value
*/
int Evaluator::int_evaluation(const std::string& expression, EvaluationInterface& interface) const {
    int64_t value = int64_evaluation(expression, interface);

    // int range check
    if (value < static_cast<int64_t>(std::numeric_limits<int>::min()) ||
        value > static_cast<int64_t>(std::numeric_limits<int>::max())) {
        throw std::runtime_error("Integer expression result is outside int range: " + expression);
    }
    return static_cast<int>(value);
}

/**
* Method evaluates expression and returns generic evaluation value.
*
* @param expression - expression to evaluate
* @param interface - evaluation interface used for variables and functions
* @return evaluated value
*/
EvalValue Evaluator::value_evaluation(const std::string& expression, EvaluationInterface& interface) const {
    ExpressionParser parser(trim(expression), interface);
    return parser.parse();
}

/**
* Method executes action code block
*
* @param code - action code to execute
* @param interface - evaluation interface used for variables, functions and outputs
*/
void Evaluator::execute_action(const std::string& code, EvaluationInterface& interface) const {
    std::string cleaned = trim(code);
    // empty action check
    if (cleaned.empty() || cleaned == "{}") {
        return;
    }

    // deteles block brackets
    if (cleaned.front() == '{' && cleaned.back() == '}') {
        cleaned = cleaned.substr(1, cleaned.size() - 2);
    }
    execute_block(cleaned, interface);
}

/**
* Executes sequence of statements in code block.
*
* @param code - code block to execute
* @param interface - evaluation interface used for variables, functions and outputs
*/
void Evaluator::execute_block(const std::string& code, EvaluationInterface& interface) const {
    for (const auto& statement : split_statements(code)) {
        execute_statement(statement, interface);
    }
}

/**
* Executes one action statement.
*
* @param statement - action statement to execute
* @param interface - evaluation interface used for variables, functions and outputs
*/
void Evaluator::execute_statement(const std::string& statement, EvaluationInterface& interface) const {
    std::string trimmed_statement = trim(statement);

    // empty statement check
    if (trimmed_statement.empty()) {
        return;
    }

    // if statement processing
    if (starts_with_keyword(trimmed_statement, "if")) {
        std::size_t condition_open = trimmed_statement.find('(');
        if (condition_open == std::string::npos) {
            throw std::runtime_error("Invalid if statement: " + statement);
        }

        // parses condition
        std::size_t condition_close = find_matching(trimmed_statement, condition_open, '(', ')');
        std::string condition = trimmed_statement.substr(condition_open + 1, condition_close - condition_open - 1);

        // parses block
        std::size_t then_open = trimmed_statement.find('{', condition_close + 1);
        if (then_open == std::string::npos) {
            throw std::runtime_error("If statement without block: " + statement);
        }
        std::size_t then_close = find_matching(trimmed_statement, then_open, '{', '}');
        std::string then_block = trimmed_statement.substr(then_open + 1, then_close - then_open - 1);

        // parses optional else
        std::string else_block;
        std::string rest = trim(trimmed_statement.substr(then_close + 1));
        if (!rest.empty()) {
            // unexpected text check
            if (!starts_with_keyword(rest, "else")) {
                throw std::runtime_error("Unexpected text after if block: " + rest);
            }
            rest = trim(rest.substr(4));
            // nested if check
            if (starts_with_keyword(rest, "if")) {
                else_block = rest;
            } 
            else {
                // else block exists check
                if (rest.empty() || rest.front() != '{') {
                    throw std::runtime_error("Else statement without block: " + statement);
                }
                std::size_t else_close = find_matching(rest, 0, '{', '}');
                else_block = rest.substr(1, else_close - 1);
                if (!trim(rest.substr(else_close + 1)).empty()) {
                    throw std::runtime_error("Unexpected text after else block: " + rest);
                }
            }
        }

        // selects branch of if and executes
        if (guard_evaluation(condition, interface)) {
            execute_block(then_block, interface);
        } 
        else if (!else_block.empty()) {
            execute_statement(else_block, interface);
        }
        return;
    }

    // outupt statement processing
    std::size_t output_pos = trimmed_statement.find("output");
    if (output_pos == 0) {
        std::size_t open = trimmed_statement.find('(');
        if (open == std::string::npos) {
            throw std::runtime_error("Invalid output statement: " + statement);
        }

        // parses brackets
        std::size_t close = find_matching(trimmed_statement, open, '(', ')');
        if (!trim(trimmed_statement.substr(close + 1)).empty()) {
            throw std::runtime_error("Unexpected text after output statement: " + statement);
        }

        // splits arguments bz comma
        std::string args = trimmed_statement.substr(open + 1, close - open - 1);
        std::vector<std::string> parts;
        int depth = 0;
        bool in_string = false;
        bool escape = false;
        std::size_t start = 0;
        for (std::size_t char_index = 0; char_index < args.size(); ++char_index) {
            char character = args[char_index];

            // ignores commas inside string
            if (in_string) {
                if (escape) escape = false;
                else if (character == '\\') escape = true;
                else if (character == '"') in_string = false;
                continue;
            }
            // updates nested depth (brackets or "")
            if (character == '"') in_string = true;
            else if (character == '(') ++depth;
            else if (character == ')') --depth;
            // next argument check
            else if (character == ',' && depth == 0) {
                parts.push_back(trim(args.substr(start, char_index - start)));
                start = char_index + 1;
            }
        }
        parts.push_back(trim(args.substr(start)));

        // argument count and name check
        if (parts.size() != 2 || parts[0].size() < 2 || parts[0].front() != '"' || parts[0].back() != '"') {
            throw std::runtime_error("output expects output(\"name\", expression)");
        }

        // evaluates and emits output
        std::string output_name = parts[0].substr(1, parts[0].size() - 2);
        interface.emit_output(output_name, value_evaluation(parts[1], interface).as_string());
        return;
    }

    // assignment stating processing
    int depth = 0;
    bool in_string = false;
    bool escape = false;
    for (std::size_t char_index = 0; char_index < trimmed_statement.size(); ++char_index) {
        char character = trimmed_statement[char_index];

        // ignores assignment operators inside string
        if (in_string) {
            if (escape) escape = false;
            else if (character == '\\') escape = true;
            else if (character == '"') in_string = false;
            continue;
        }

        // updates nested depth (brackets or "")
        if (character == '"') in_string = true;
        else if (character == '(') ++depth;
        else if (character == ')') --depth;
        // assignment check
        else if (is_assignment_operator(trimmed_statement, char_index, depth)) {
            std::string variable_name = trim(trimmed_statement.substr(0, char_index));
            std::string value_expression = trim(trimmed_statement.substr(char_index + 1));

            // where assings check
            if (!interface.has_variable(variable_name)) {
                throw std::runtime_error("Unknown variable: " + variable_name);
            }

            // assign execute
            EvalValue value = convert_for_declared_type(variable_name, 
                    interface.variable_type(variable_name), value_evaluation(value_expression, interface));
            interface.set_variable_value(variable_name, value);
            return;
        }
    }

    throw std::runtime_error("Unsupported action statement: " + statement);
}

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
* Splits code block into individual statements.
*
* @param code - code block to split
* @return vector of separated statements
*/
std::vector<std::string> Evaluator::split_statements(const std::string& code) {
    std::vector<std::string> result;
    int paren_depth = 0;
    int brace_depth = 0;
    bool in_string = false;
    bool escape = false;
    std::size_t start = 0;

    // searches for top level ; or block endings
    for (std::size_t char_index = 0; char_index < code.size(); ++char_index) {
        char character = code[char_index];

        // ignores separators in string
        if (in_string) {
            if (escape) {
                escape = false;
            } 
            else if (character == '\\') {
                escape = true;
            } 
            else if (character == '"') {
                in_string = false;
            }
            continue;
        }

        // update depth 
        if (character == '"') {
            in_string = true;
        } 
        else if (character == '(') {
            ++paren_depth;
        } 
        else if (character == ')') {
            --paren_depth;
        } 
        else if (character == '{') {
            ++brace_depth;
        } 
        // founds end of block
        else if (character == '}') {
            --brace_depth;

            // top level block ended
            if (paren_depth == 0 && brace_depth == 0) {
                std::size_t next = char_index + 1;
                while (next < code.size() && std::isspace(static_cast<unsigned char>(code[next]))) {
                    ++next;
                }
                // else branch after end of block check
                bool followed_by_else = code.compare(next, 4, "else") == 0 &&
                    (next + 4 == code.size() || !std::isalnum(static_cast<unsigned char>(code[next + 4])));
                if (!followed_by_else) {
                    std::string item = trim(code.substr(start, char_index - start + 1));
                    if (!item.empty()) {
                        result.push_back(item);
                    }
                    start = char_index + 1;
                }
            }
        } 
        // founds end of command
        else if (character == ';') {
            if (paren_depth == 0 && brace_depth == 0) {
                // top level ; separates statements
                std::string item = trim(code.substr(start, char_index - start));
                if (!item.empty()) {
                    result.push_back(item);
                }
                start = char_index + 1;
            }
        }
    }

    // adds statement after last separator 
    std::string item = trim(code.substr(start));
    if (!item.empty()) {
        result.push_back(item);
    }
    return result;
}