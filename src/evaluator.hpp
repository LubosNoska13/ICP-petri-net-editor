// autor: Jurišinová Daniela (xjurisd00)
#ifndef EVALUATOR_HPP
#define EVALUATOR_HPP

#include "model.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <functional>

/**
 * @file evaluator.hpp
 * @brief Evaluator for guards and actions
 */

struct OutputEvent {
    std::string name;
    std::string value;
};

class EvaluationInterface {
public:
    virtual ~EvaluationInterface() = default;
    virtual bool input_defined(const std::string& name) const = 0;
    virtual std::string input_value(const std::string& name) const = 0;
    virtual int token_count(const std::string& place_name) const = 0;
    virtual int64_t ms_elapsed(const std::string& object_name) const = 0;
    virtual int64_t ms_now() const = 0;
    virtual bool has_variable(const std::string& name) const = 0;
    virtual int variable_value(const std::string& name) const = 0;
    virtual void set_variable_value(const std::string& name, int value) = 0;
    virtual void emit_output(const std::string& name, const std::string& value) = 0;
};

class Evaluator {
public:
    bool guard_evaluation(const std::string& expression, EvaluationInterface& interface) const;
    int int_evaluation(const std::string& expression, EvaluationInterface& interface) const;
    void execute_action(const std::string& code, EvaluationInterface& interface) const;
private:
    bool comparison_evaluation(const std::string& expression, EvaluationInterface& interface) const;
    bool defined_evaluation(const std::string& expression, EvaluationInterface& interface) const;
    void execute_statement(const std::string& statement, EvaluationInterface& interface) const;
    static std::string trim(const std:: string& string);
    static std::vector<std::string> split_statements(const std::string& code);
    std::string remove_brackets(const std::string& expression) const;
    std::vector<std::string> split_by_operator(const std::string& expression,
            const std::string& operat) const;
};

#endif