// autor: Jurišinová Daniela (xjurisd00)

#ifndef VALIDATE_HPP
#define VALIDATE_HPP

#include "model.hpp"
#include <string>
#include <vector>

/**
 * @file validate.hpp
 * @brief Validation of the static features of Petrinet
 */

enum class ValidationSeverity {
    Error,
    Warning
};

struct ValidationMessage {
    ValidationSeverity severity;
    std::string message;
};

struct ValidationResult {
    std::vector<ValidationMessage> messages;

    bool is_ok() const;
    bool has_errors() const;
    void add_error(const std::string& message);
    void add_warning(const std::string& message);
};

class Validator {
public:
    ValidationResult validate(const PetriNet& net) const;
    ValidationResult validate_live(const PetriNet& net) const;

private:
    void validate_names(const PetriNet& net, ValidationResult& result) const;
    void validate_places(const PetriNet& net, ValidationResult& result) const;
    void validate_transitions(const PetriNet& net, ValidationResult& result) const;
    void validate_events(const PetriNet& net, ValidationResult& result) const;
    void validate_inscriptions(const PetriNet& net, ValidationResult& result) const;
    void validate_reachability(const PetriNet& net, ValidationResult& result) const;
};

#endif