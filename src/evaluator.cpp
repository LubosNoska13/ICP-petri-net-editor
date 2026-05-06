// autor: Jurišinová Daniela (xjurisd00)

#include "model.hpp"
#include "evaluator.hpp"

#include <algorithm>
#include <regex>
#include <sstream>
#include <cstdlib>
#include <stdexcept>

/**
 * Method evaluates guard expression and returns if transition 
 * should be executed or not. 
 *
 * @param expression - guard expression for evaluation
 * @param interface - interface used to access needed data and procedures
 * @return true if the guard expression is satisfied otherwise false
 */
bool Evaluator::guard_evaluation(const std::string& expression, EvaluationInterface& interface) const {
    // expression adjustment and empty check
    std::string trimmed_expression = trim(expression);
    if (trimmed_expression.empty()) {
        return true;
    }
    trimmed_expression = remove_brackets(trimmed_expression);

    // negation in expression check
    if (!trimmed_expression.empty() && trimmed_expression[0] == '!') {
        std::string negated = trim(trimmed_expression.substr(1));
        return !guard_evaluation(negated, interface);
    }

    // OR in expression check
    auto or_clauses = split_by_operator(trimmed_expression, "||");
    if (or_clauses.size() > 1) {
        for (const auto& clause : or_clauses) {
            if (guard_evaluation(clause, interface)) {
                return true;
            }
        }
        return false;
    }

    // AND in expression check
    auto and_clauses = split_by_operator(trimmed_expression, "&&");
    if (and_clauses.size() > 1) {
        for (const auto& clause : and_clauses) {
            if (!guard_evaluation(clause, interface)) {
                return false;
            }
        }
        return true;
    }

    if (trimmed_expression == "true") {
        return true;
    }
    if (trimmed_expression == "false") {
        return false;
    }
    if (trimmed_expression.find("defined") != std::string::npos) {
        return defined_evaluation(trimmed_expression, interface);
    }
    return comparison_evaluation(trimmed_expression, interface);
}

/**
 * Method evaluates integer expression and then resolves result
 *
 * @param expression - integer expression for evaluation
 * @param interface - interface used to access needed data and procedures
 * @return numeric result of the evaluated expressoin
 */
int Evaluator::int_evaluation(const std::string& expression, EvaluationInterface& interface) const {
    // expression adjustment and empty check
    std::string trimmed_expression = trim(expression);
    if (trimmed_expression.empty()) {
        return 0;
    }

    // expression only integer check
    if (std::regex_match(trimmed_expression, std::regex(R"(-?[0-9]+)"))) {
        return std::stoi(trimmed_expression);
    }
    // expression is variable name check
    if (interface.has_variable(trimmed_expression)) {
        return interface.variable_value(trimmed_expression);
    }
    std::smatch match;
    // atoi in expression check
    if (std::regex_match(trimmed_expression, match, std::regex(R"REGEX(atoi\s*\(\s*valueof\s*\(\s*"([^"]+)"\s*\)\s*\))REGEX"))) {
        const std::string input_name = match[1];
        return std::atoi(interface.input_value(input_name).c_str());
    }
    // tokens in expression check 
    if (std::regex_match(trimmed_expression, match, std::regex(R"REGEX(tokens\s*\(\s*"([^"]+)"\s*\))REGEX"))) {
        return interface.token_count(match[1]);
    }
    // elapsed in expression check
    if (std::regex_match(trimmed_expression, match, std::regex(R"REGEX(elapsed\s*\(\s*"([^"]+)"\s*\))REGEX"))) {
        return static_cast<int>(interface.ms_elapsed(match[1]));
    }
    // now in expression check
    if (trimmed_expression == "now()") {
        return static_cast<int>(interface.ms_now());
    }

    // minus in expression check and evaluation
    auto minus_position = trimmed_expression.find('-');
    if (minus_position != std::string::npos) {
        std::string left = trimmed_expression.substr(0, minus_position);
        std::string right = trimmed_expression.substr(minus_position + 1);
        return int_evaluation(left, interface) - int_evaluation(right, interface);
    }
    // no other known format
    throw std::runtime_error("Unsupported integer expression: " + expression);
}

/**
 * Method executes an action in given code string which may contain more 
 * statements and because that method uses method for statement execution
 *
 * @param code - string containing sequence of actions
 * @param interface - interface used to access needed data and procedures
 */
void Evaluator::execute_action(const std::string& code, EvaluationInterface& interface) const {
    // code adjustment and empty check
    std::string cleaned = trim(code);
    if (cleaned.empty() || cleaned == "{}") {
        return;
    }
    if (cleaned.front() == '{' && cleaned.back() == '}') {
        cleaned = cleaned.substr(1, cleaned.size() - 2);
    }

    std::smatch if_match;
    // type condition check
    if (std::regex_search(cleaned, if_match,
            std::regex(R"REGEX(if\s*\(\s*defined\s*\(\s*"([^"]+)"\s*\)\s*\)\s*\{(.*)\})REGEX"))) {
        // sets input name and code to execute if condition is satisfied
        std::string input_name = if_match[1];
        std::string body = if_match[2];
        if (interface.input_defined(input_name)) {
            execute_action(body, interface);
        }
        return;
    }

    // not condition therefore splits into statements and executes them
    for (const auto& statement : split_statements(cleaned)) {
        execute_statement(statement, interface);
    }
}

/**
 * Method evaluates compraison expressions which may contain integer expression
 * and because that it may need to use integer expression evaluation
 *
 * @param expression - comparison expression for evaluation
 * @param interface - interface used to access needed data and procedures
 * @return true if the comparison is valid otherwise false
 */
bool Evaluator::comparison_evaluation(const std::string& expression, EvaluationInterface& interface) const {
    static const std::vector<std::string> operators = {"==", "!=", ">=", "<=", ">", "<"};

    // continuous check of comparison operators
    for (const auto& operat : operators) {
        auto position = expression.find(operat);
        // evaluation of the expression
        if (position != std::string::npos) {
            int left = int_evaluation(expression.substr(0, position), interface);
            int right = int_evaluation(expression.substr(position + operat.size()), interface);
            if (operat == "==") {
                return left == right;
            }
            if (operat == "!=") {
                return left != right;
            }
            if (operat == ">=") {
                return left >= right;
            }
            if (operat == "<=") {
                return left <= right;
            }
            if (operat == ">") {
                return left > right;
            }
            if (operat == "<") {
                return left < right;
            }
        }
    }
    return int_evaluation(expression, interface) != 0;
}

/**
 * Method checks if given expression or identifier is defined
 *
 * @param expression - expression or identifier that should be checked
 * @param interface - interface used to access needed data and procedures
 * @return true if the expression/indetifier does exists otherwise false
 */
bool Evaluator::defined_evaluation(const std::string& expression, EvaluationInterface& interface) const {
    std::smatch match;
    std::string trimmed_expression = trim(expression);
    if (std::regex_match(trimmed_expression, match,
            std::regex(R"REGEX(defined\s*\(\s*"([^"]+)"\s*\))REGEX"))) {
        return interface.input_defined(match[1]);
    }
    throw std::runtime_error("Unsupported defined expression: " + expression);
}

/**
 * Method executes statements like assignment, update etc. It can modify
 * evaluation context through interface.
 *
 * @param statement - statement to be executed
 * @param interface - interface used to access needed data and procedures
 */
void Evaluator::execute_statement(const std::string& statement, EvaluationInterface& interface) const {
    // statement adjustment and empty check
    std::string trimmed_statement = trim(statement);
    if (trimmed_statement.empty()) {
        return;
    }

    std::smatch match;
    // output(output_name, expression) check
    if (std::regex_match(trimmed_statement, match,
            std::regex(R"REGEX(output\s*\(\s*"([^"]+)"\s*,\s*(.+)\s*\))REGEX"))) {
        std::string output_name = match[1];
        std::string value_expression = trim(match[2]);
        int value = int_evaluation(value_expression, interface);
        interface.emit_output(output_name, std::to_string(value));
        return;
    }

    // variable = expression check
    if (std::regex_match(trimmed_statement, match,
            std::regex(R"(([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.+))"))) {
        std::string variable_name = match[1];
        std::string value_expression = match[2];
        int value = int_evaluation(value_expression, interface);
        interface.set_variable_value(variable_name, value);
        return;
    }
    throw std::runtime_error("Unsupported action statement: " + statement);
}

/**
* Helper function deletes whitespaces from the start and the end 
* of the string
* note: method copied from parser.cpp (autor: Jurišinová Daniela) 
*
* @param string - the string for trimming
* @return trimmed string
*/
std::string Evaluator::trim(const std::string& string) {
    const auto begin = string.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return "";
    }
       
    const auto end = string.find_last_not_of(" \t\r\n");
    return string.substr(begin, end - begin + 1);
}

/**
 * Helper method that splits block of code into elemental statements.
 *
 * @param code - code block with sequence of statements
 * @return vector of separated elemental statements
 */
std::vector<std::string> Evaluator::split_statements(const std::string& code) {
    std::vector<std::string> result;
    std::stringstream stream(code);
    std::string item;

    while (std::getline(stream, item, ';')) {
        item = trim(item);
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    return result;
}

/**
 * Helper method that deletes outer brackets. Used for guard evaluation
 *
 * @param expression - expression for bracket deletion
 * @return string with same expression without outer brackets
 */
std::string Evaluator::remove_brackets(const std::string& expression) const {
    std::string result = trim(expression);
    while ( expression.size() >= 2 && 
            expression.front() == '(' && 
            expression.back() == ')') {
        int depth = 0;
        bool wraps_whole_expression = true;

        // expression continuous check
        for (std::size_t index = 0; index < result.size(); ++index) {
            if (result[index] == '(') {
                ++depth;
            } 
            else if (result[index] == ')') {
                --depth;
                // check if right bracket is really ending bracket
                if (depth == 0 && index != result.size() - 1) {
                    wraps_whole_expression = false;
                    break;
                }
            }

            // check if there are ')(' brackets
            if (depth < 0) {
                wraps_whole_expression = false;
                break;
            }
        }
        // check if the bracket is really pair and end bracket
        if (!wraps_whole_expression || depth != 0) {
            break;
        }
        result = trim(result.substr(1, result.size() - 2));
    }
    return result;
}

/**
 * Helper method that splits expression by selected top-level operator.
 * Used in guard evaluation.
 *
 * @param expression - expression for split
 * @return vector of two clauses by top-level operator
 */
std::vector<std::string> Evaluator::split_by_operator(const std::string& expression,
        const std::string& operat) const {

    std::vector<std::string> result;
    int depth = 0;
    std::size_t start = 0;

    // continuous control of expression
    for (std::size_t index = 0; index < expression.size(); ++index) {
        char character = expression[index];

        // subexpressions check
        if (character == '(') {
            ++depth;
        } 
        else if (character == ')') {
            --depth;
        }

        // check so spliting is only if depth is 0 which means top-level operator
        if (depth == 0 && expression.compare(index, operat.size(), operat) == 0) {
            result.push_back(trim(expression.substr(start, index - start)));
            index += operat.size() - 1;
            start = index + 1;
        }
    }
    result.push_back(trim(expression.substr(start)));
    return result;
}