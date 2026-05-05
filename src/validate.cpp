// autor: Jurišinová Daniela (xjurisd00)

#include "validate.hpp"
#include <set>

/**
 * Method which checks if the validation result had no error
 *
 * @return true if Petrinet is valid otherwise false
 */
bool ValidationResult::is_ok() const {

}

/**
 * Method which checks if the validation result had any errors.
 * Basically helper method for the ok result method.
 *
 * @return true if Petrinet has at least one error otherwise fales
 */
bool ValidationResult::has_errors() const {

}

/**
 * Method to add error message to result of the validation
 *
 * @param message - the text description of the error 
 */
void ValidationResult::add_error(const std::string& message) {

}

/**
 * Method to add warning message to result of the validation 
 *
 * @param message - the text description of the warning
 */
void ValidationResult::add_warning(const std::string& message) {

}

/**
 * Method performs validation proccess of the whole Petrinet. uses
 * methods to checks names, places, transitions and events a searches
 * for errors and warnings to add to the result.
 *
 * @param net - Petrinet to be checked
 * @return the result with all found errors and warnings
 */
ValidationResult Validator::validate(const PetriNet& net) const {

}

/**
 * Helper method to validate names used in Petrinet and searches for
 * duplicates and other conflicts
 *
 * @param net - Petrinet which names are to be checked
 * @param result - result of the partial validation with stored errors and warnings
 */
void Validator::validate_names(const PetriNet& net, ValidationResult& result) const {

}

/**
 * Helper method to validate places used in Petrinet and searches for 
 * definitions, identifiers etc.
 *
 * @param net - Petrinet which places are to be checked
 * @param result - result of the partial validation with stored errors and warnings
 */
void Validator::validate_places(const PetriNet& net, ValidationResult& result) const {

}

/**
 * Helper method to validate transitions used in Petrinet and checks it's
 * definitions, input/output arcs etc.
 *
 * @param net - Petrinet which transitions are to be checked
 * @param result - result of the partial validation with stored errors and warnings
 */
void Validator::validate_transitions(const PetriNet& net, ValidationResult& result) const {

}

/**
 * Helper method to validate events and their definitions, if there is
 * a transitions to which they refer etc.
 *
 * @param net - Petrinet which events are to be checked
 * @param result - result of the aprtial validation with stored errors and warnings
 */
void Validator::validate_events(const PetriNet& net, ValidationResult& result) const {

}