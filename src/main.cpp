// autor: Jurišinová Daniela (xjurisd00)

#include "parser.hpp"
#include "runtime.hpp"
#include "validate.hpp"
#include <string>
#include <iostream>
#include <sstream>

/**
 * Function writes current state of the Petrinet
 *
 * @param runtime - the runtime which state is printed
 */
static void print_state(const PetriRuntime& runtime) {
    auto snapshot = runtime.snapshot();

    // place and tokens print
    std::cout << "Marking:\n";
    for (const auto& [place, tokens] : snapshot.marking) {
        std::cout << "\t" << place << " = " << tokens << "\n";
    }

    // variables and values print
    std::cout << "Variables:\n";
    for (const auto& [name, variable] : snapshot.variables) {
        std::cout << "\t" << name << " = " << variable.value << "\n";
    }

    // timers and transitions print
    std::cout << "Pending timers:\n";
    for (const auto& timer : snapshot.pending_timers) {
        std::cout << "\t" << timer.transition_name << " due at " << timer.due_at_ms << " ms\n";
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: petri_core <file.pn>\n";
        return 1;
    }

    // if parse, validation or simulation fails then exception branch is activated
    try {
        Parser parser;
        PetriNet net = parser.parse_file(argv[1]);
        Validator validator;
        ValidationResult validation = validator.validate(net);

        // printing validation result
        for (const auto& message : validation.messages) {
            if (message.severity == ValidationSeverity::Error) {
                std::cerr << "ERROR: ";
            } 
            else {
                std::cerr << "WARNING: ";
            }
            std::cerr << message.message << "\n";
        }

        // if there is error than program ends
        if (!validation.is_ok()) {
            return 2;
        }

        PetriRuntime runtime(net);
        runtime.initialize();

        // valid commands usage print
        std::cout << "Commands:\n";
        std::cout << "\tinput <name> <value>\n";
        std::cout << "\ttick <ms>\n";
        std::cout << "\tstate\n";
        std::cout << "\tquit\n";
        std::string line;

        // interactive program mode
        while (std::cout << "> " && std::getline(std::cin, line)) {
            std::stringstream stream(line);
            std::string command;
            stream >> command;

            // command insert input value in Petrinet
            if (command == "input") {
                std::string name;
                std::string value;
                stream >> name >> value;
                runtime.inject_input(name, value);
            } 
            // command time skip
            else if (command == "tick") {
                int ms = 0;
                stream >> ms;
                runtime.advance_time(ms);
            } 
            // command state print
            else if (command == "state") {
                print_state(runtime);
            } 
            // command program end
            else if (command == "quit") {
                break;
            } 
            // invalid empty command
            else if (!command.empty()) {
                std::cout << "Unknown command\n";
            }
        }
        return 0;
    } 
    catch (const std::exception& except) {
        std::cerr << "Fatal error: " << except.what() << "\n";
        return 3;
    }
}