// autor: Jurišinová Daniela (xjurisd00)

#include "model.hpp"
#include "evaluator.hpp"

/**
 * Method evaluates guard expression and returns if transition 
 * should be executed or not
 *
 * @param expression - guard expression for evaluation
 * @param interface - interface used to access needed data and procedures
 * @return true if the guard expression is satisfied otherwise false
 */
bool Evaluator::guard_evaluation(const std::string& expression, EvaluationInterface& interface) const {

}

/**
 * Method evaluates integer expression and then resolves result
 *
 * @param expression - integer expression for evaluation
 * @param interface - interface used to access needed data and procedures
 * @return numeric result of the evaluated expressoin
 */
int Evaluator::int_evaluation(const std::string& expression, EvaluationInterface& interface) const {

}

/**
 * Method executes an action in given code string which may contain more 
 * statements and because that method uses method for statement execution
 *
 * @param code - string containing sequence of actions
 * @param interface - interface used to access needed data and procedures
 */
void Evaluator::execute_action(const std::string& code, EvaluationInterface& interface) const {

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

}

/**
 * Method checks if given expression or identifier is defined
 *
 * @param expression - expression or identifier that should be checked
 * @param interface - interface used to access needed data and procedures
 * @return true if the expression/indetifier does exists otherwise false
 */
bool Evaluator::defined_evaluation(const std::string& expression, EvaluationInterface& interface) const {

}

/**
 * Method executes statements like assignment, update etc. It can modify
 * evaluation context through interface.
 *
 * @param statement - statement to be executed
 * @param interface - interface used to access needed data and procedures
 */
void Evaluator::execute_statement(const std::string& statement, EvaluationInterface& interface) const {

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

}

