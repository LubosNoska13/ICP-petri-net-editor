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

struct EvalValue {
    enum class Type {
        Int,
        Bool,
        String
    };

    Type type = Type::Int;
    int64_t int_value = 0;
    bool bool_value = false;
    std::string string_value;

    static EvalValue integer(int64_t value);
    static EvalValue boolean(bool value);
    static EvalValue string(std::string value);

    bool as_bool() const;
    int64_t as_int() const;
    std::string as_string() const;
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
    virtual EvalValue variable_value(const std::string& name) const = 0;
    virtual std::string variable_type(const std::string& name) const = 0;
    virtual void set_variable_value(const std::string& name, const EvalValue& value) = 0;
    virtual void emit_output(const std::string& name, const std::string& value) = 0;
};

class Evaluator {
public:
    bool guard_evaluation(const std::string& expression, EvaluationInterface& interface) const;
    int64_t int64_evaluation(const std::string& expression, EvaluationInterface& interface) const;
    int int_evaluation(const std::string& expression, EvaluationInterface& interface) const;
    EvalValue value_evaluation(const std::string& expression, EvaluationInterface& interface) const;
    void execute_action(const std::string& code, EvaluationInterface& interface) const;
private:
    void execute_block(const std::string& code, EvaluationInterface& interface) const;
    void execute_statement(const std::string& statement, EvaluationInterface& interface) const;
    static std::string trim(const std::string& string);
    static std::vector<std::string> split_statements(const std::string& code);
};

#endif