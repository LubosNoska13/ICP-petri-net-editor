// autor: Jurišinová Daniela (xjurisd00)

#include "runtime.hpp"
#include <cstdlib>
#include <stdexcept> 

/**
 * Constructor of class PetriRuntime
 */
PetriRuntime::PetriRuntime(PetriNet net) 
    : m_net(std::move(net)) {
}

/**
 * Method for initialization of the Petrinet state. Sets initial 
 * marking, variables, inputs, outputs and timers.
 */
void PetriRuntime::initialize() {

}

/**
 * Method injects input value into runtime
 *
 * @param name - name of the input
 * @param value - value assigned to input
 */
void PetriRuntime::inject_input(const std::string& name, const std::string& value) {

}

/**
 * Method advances internal runtime clock
 *
 * @param delta_ms - time to be added
 */
void PetriRuntime::advance_time(int64_t delta_ms) {

}

/**
 * Method runs Petrinet stabiliyation proccess
 *
 * @param triggering_event - optional event that triggers stabilization
 */
void PetriRuntime::run_stabilization(const std::optional<std::string>& triggering_event) {

}

/**
 * Method creates a snapshot of the current state of Petrinet
 *
 * @return snapshot with current state of runtime
 */
StateSnapshot PetriRuntime::snapshot() const {

}

/**
 * Method returns runtime logger
 * note: constant variant of the method
 *
 * @return reference to the logger
 */
const Logger& PetriRuntime::logger() const {

}

/**
 * Method returns runtime logger
 *
 * @return references to the logger
 */
Logger& PetriRuntime::logger() {

}

/**
 * Method checks if input in argument is defined
 *
 * @param name - name of the input to be checked
 * @return true if the input with name exists otherwise false
 */
bool PetriRuntime::input_defined(const std::string& name) const {

}

/**
 * Method returns value of the input
 *
 * @param name - name of the input with value is checked
 * @return current value of the input  
 */
std::string PetriRuntime::input_value(const std::string& name) const {

}

/**
 * Method returns the number of the tokens in place in argument
 *
 * @param place_name - name of the place which tokens are checked
 * @return count of the tokens stored in that place
 */
int PetriRuntime::token_count(const std::string& place_name) const {

}

/**
 * Method returns elapsed time for a runtime object in argument
 *
 * @param object_name - name of the object which elapsed time is checked
 * @return elapsed time of the objext
 */
int64_t PetriRuntime::ms_elapsed(const std::string& object_name) const {

}

/**
 * Method returns current time in runtime
 * 
 * @return currenct time 
 */
int64_t PetriRuntime::ms_now() const {

}

/**
 * Method checks if variable in argument exists
 *
 * @param name - name of the variable to be checked
 * @return true if variable exists otherwise false
 */
bool PetriRuntime::has_variable(const std::string& name) const {

}

/**
 * Method checks variable value
 *
 * @param name - name of the variable which value is checked
 * @return value of the variable
 */
int PetriRuntime::variable_value(const std::string& name) const {

}

/**
 * Method sets value of the variable in argument
 *
 * @param name - name of the variable to be set
 * @param value - value which is gonna be value of the variable
 */
void PetriRuntime::set_variable_value(const std::string& name, int value) {

}

/**
 * Method emits output value from the runtime
 *
 * @param name - name of the output
 * @param value - value to be emitted in the output
 */
void PetriRuntime::emit_output(const std::string& name, const std::string& value) {

}