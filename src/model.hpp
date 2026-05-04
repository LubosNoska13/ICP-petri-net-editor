// autor: Jurišinová Daniela (xjurisd00)

#ifndef MODEL_HPP
#define MODEL_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <optional>

/**
 * @file model.hpp
 * @brief Module contains static parts of network and helpers
 */

struct Variable {
    std::string type;
    std::string name;
    std::string initializer;
};

struct Input {
    std::string name;
};

struct Output {
    std::string name;
};

struct Place {
    std::string name;
    int initialTokens = 0;
    std::string add_token_action;
};

struct Arc {
    std::string place_name;
    int weight = 1;
};

struct TransitionCondition {
    std::string event_name;
    std::string guard_expression;
    std::string delay_expression;

    // existence control methods
    bool has_event() const;
    bool has_guard() const;
    bool has_delay() const;
};

struct Transition {
    std::string name;
    std::vector<Arc> input_arcs;
    std::vector<Arc> output_arcs;
    TransitionCondition condition;
    // code executed when transition fires 
    std::string action_code;
    // note: lower value means higher priority
    int priority = 0;
};

/**
 * Statical definition of Petrinet ()
 */
class PetriNet {
public:
    std::string name;
    std::string comment;
    std::vector<Input> inputs;
    std::vector<Output> outputs;
    std::vector<Variable> variables;
    std::vector<Place> places;
    std::vector<Transition> transitions;

    // search by name methods
    const Place* find_place(const std::string& name) const;
    Place* find_place(const std::string& name);
    const Transition* find_transition(const std::string& name) const;
    Transition* find_transition(const std::string& name);
    // existence control methods
    bool has_input(const std::string& name) const;
    bool has_output(const std::string& name) const;
    bool has_variable(const std::string& name) const;
};

#endif