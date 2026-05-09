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

// stable identifier used by GUI
using ModelId = std::uint64_t;

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
    ModelId id = 0;
    std::string name;
    int initial_tokens = 0;
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
    ModelId id = 0;
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
    // editor helpers
    static bool valid_id(const std::string& name);
    Place& add_place(const std::string& name, int initial_tokens = 0, 
            const std::string& add_token_action = "");
    bool remove_place(const std::string& name);
    bool rename_place(const std::string& old_name, const std::string& new_name);
    Transition& add_transition(const std::string& name, 
            const TransitionCondition& condition = TransitionCondition{},
            const std::string& action_code = "", std::optional<int> priority = std::nullopt);
    bool remove_transition(const std::string& name);
    bool add_input_arc(const std::string& transition_name, const std::string& place_name,
            int weight = 1);
    bool add_output_arc(const std::string& transition_name, const std::string& place_name,
            int weight = 1);
    // serialization/ownership helpers
    std::string to_text() const;
    void clear();
private:
    ModelId m_next_place_id = 1;
    ModelId m_next_transition_id = 1;

    ModelId allocate_place_id();
    ModelId allocate_transition_id();
    static void validate_id(const std::string& name, const std::string& what);
    static void validate_weight(int weight);
};

#endif