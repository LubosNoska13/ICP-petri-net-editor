// autor: Jurišinová Daniela (xjurisd00)

#include "parser.hpp"
#include "runtime.hpp"
#include "validate.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include <memory>
#include <fstream>
#include <algorithm>


// helper functions for this file
namespace {

/**
* Helper function deletes whitespaces from the start and the end 
* of the string
*
* @param string - the string for trimming
* @return trimmed string
*/
std::string trim(const std::string& value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return "";
    }
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

/**
 * The function gets textual representation of a value stored in an EvalValue object
 *
 * @param value The value to be converted to a string
 * @return The string representation of the value
 */
std::string eval_to_string(const EvalValue& value) {
    return value.as_string();
}

/**
 * The function prints all inputs and outputs stored in the snapshot. For 
 * each input or output, it prints its name, value, and the time of the
 * last update. If the value is not defined, <undefined> is printed.
 *
 * @param snapshot The current state snapshot of the runtime environment
 */
void print_inputs_outputs(const StateSnapshot& snapshot) {
    // continously checks inputs
    std::cout << "Inputs:\n";
    for (const auto& [name, input] : snapshot.inputs) {
        std::cout << "\t" << name << " = ";
        // exists check
        if (input.defined) {
            std::cout << input.value << " (at " << input.last_update_ms << " ms)";
        }
        else {
            std::cout << "<undefined>";
        }
        std::cout << "\n";
    }

    // continuously checks outputs
    std::cout << "Outputs:\n";
    for (const auto& [name, output] : snapshot.outputs) {
        std::cout << "\t" << name << " = ";
        // exists check
        if (output.defined) {
            std::cout << output.value << " (at " << output.last_update_ms << " ms)";
        }
        else {
            std::cout << "<undefined>";
        }
        std::cout << "\n";
    }
}

/**
 * The function prints the list of transitions that are enabled in the current
 * state. If no transition is enabled, <none> is printed.
 *
 * @param snapshot The current state snapshot of the runtime environment
 */
void print_enabled(const StateSnapshot& snapshot) {
    std::cout << "Enabled transitions:\n";
    // empty list check
    if (snapshot.enabled_transitions.empty()) {
        std::cout << "\t<none>\n";
        return;
    }
    for (const auto& transition : snapshot.enabled_transitions) {
        std::cout << "\t" << transition << "\n";
    }
}

/**
 * The function prints all outputs together with their values and last update
 * times. If an output has not been defined yet, <undefined> is printed.
 *
 * @param snapshot The current state snapshot of the runtime environment
 */
void print_outputs(const StateSnapshot& snapshot) {
    std::cout << "Last outputs:\n";
    for (const auto& [name, output] : snapshot.outputs) {
        std::cout << "\t" << name << " = ";
        // defined output check
        if (output.defined) {
            std::cout << output.value << " (at " << output.last_update_ms << " ms)";
        }
        else {
            std::cout << "<undefined>";
        }
        std::cout << "\n";
    }
}

/**
 * The function prints the list of timers that are currently scheduled. For 
 * each timer prints the transition name, event name, guard expression, delay 
 * expression, computed delay, scheduling time, and due time
 *
 * @param snapshot The current state snapshot of the runtime environment
 */
void print_timers(const StateSnapshot& snapshot) {
    std::cout << "Pending timers:\n";
    // empty timers check
    if (snapshot.pending_timers.empty()) {
        std::cout << "\t<none>\n";
        return;
    }
    for (const auto& timer : snapshot.pending_timers) {
        std::cout << "\t" << timer.transition_name
                  << " event='" << timer.event_name << "'"
                  << " guard='" << timer.guard_expression << "'"
                  << " delay_expr='" << timer.delay_expression << "'"
                  << " delay=" << timer.delay_ms << " ms"
                  << " scheduled_at=" << timer.scheduled_at_ms << " ms"
                  << " due_at=" << timer.due_at_ms << " ms\n";
    }
}

/**
 * The function obtains a snapshot of the current runtime state and prints
 * the current runtime time, marking of places, enabled transitions, variables,
 * inputs, outputs, and pending timers
 *
 * @param runtime The Petrinet runtime environment
 */
void print_state(const PetriRuntime& runtime) {
    auto snapshot = runtime.snapshot();
    std::cout << "Runtime time: " << snapshot.now_ms << " ms\n";
    std::cout << "Marking:\n";
    for (const auto& [place, tokens] : snapshot.marking) {
        std::cout << "\t" << place << " = " << tokens << "\n";
    }
    print_enabled(snapshot);
    std::cout << "Variables:\n";
    for (const auto& [name, variable] : snapshot.variables) {
        std::cout << "\t" << name << " : " << variable.type << " = "
                  << eval_to_string(variable.value) << "\n";
    }
    print_inputs_outputs(snapshot);
    print_timers(snapshot);
}

/**
 * The function gets the logger and prints its content in text form
 *
 * @param runtime The Petrinet runtime environment
 */
void print_log(const PetriRuntime& runtime) {
    std::cout << runtime.logger().export_text();
}

/**
 * The function serves as a simple help message for the user. It prints
 * the commands that can be entered while the program is running.
 */
void print_help() {
    std::cout << "Commands:\n";
    std::cout << "\tload <file.pn>          load and initialize another network\n";
    std::cout << "\tsave [file.pn]          save current network\n";
    std::cout << "\tinput <name> <value>    inject input; value may contain spaces\n";
    std::cout << "\ttick <ms>               advance runtime time\n";
    std::cout << "\tstep                    process due timers at current time\n";
    std::cout << "\tstate                   print monitor state\n";
    std::cout << "\tenabled                 print enabled transitions\n";
    std::cout << "\toutputs                 print last outputs\n";
    std::cout << "\tlog                     print event log\n";
    std::cout << "\thelp                    print commands\n";
    std::cout << "\tquit                    exit\n";
}

/**
 * The function runs the validator on the given Petri net. All validation
 * messages are printed to the standard error output. The function returns
 * whether the net is valid and contains no validation errors.
 *
 * @param net the Petrinet to be validated
 * @return true If the Petri net is valid or false if validation errors were found
 */
bool validate_or_print(const PetriNet& net) {
    Validator validator;
    ValidationResult validation = validator.validate(net);
    for (const auto& message : validation.messages) {
        std::cerr << (message.severity == ValidationSeverity::Error ? "ERROR: " : "WARNING: ")
                  << message.message << "\n";
    }
    return validation.is_ok();
}

/**
 * The function parses a filewith a Petrinet definition, reports syntax
 * errors if parsing fails, validates the parsed net, and creates a runtime
 * environment if the net is valid. The runtime is initialized before it is
 * returned
 *
 * @param path path to the file containing the Petri net
 * @param loaded_net reference where the loaded Petri net will be stored
 * @return a unique pointer to the created runtime environment
 * @return nullptr If parsing or validation fails
 */
std::unique_ptr<PetriRuntime> load_runtime(const std::string& path, PetriNet& loaded_net) {
    Parser parser;
    ParseResult parse_result = parser.parse_file(path);

    // prints parsing errors if failed
    if (!parse_result.ok()) {
        for (const auto& error : parse_result.errors) {
            std::cerr << "PARSE ERROR line " << error.line << ": " << error.message << "\n";
        }
        return nullptr;
    }

    // validate and creates if net is ok 
    if (!validate_or_print(parse_result.net)) {
        return nullptr;
    }

    // creates and initializes runtime enviroment
    loaded_net = parse_result.net;
    auto runtime = std::make_unique<PetriRuntime>(loaded_net);
    runtime->initialize(true);
    return runtime;
}

} 

int main(int argc, char** argv) {
    std::string current_path;
    PetriNet current_net;
    std::unique_ptr<PetriRuntime> runtime;

    try {
        // loads and initializes Petrinet if file ok
        if (argc >= 2) {
            current_path = argv[1];
            runtime = load_runtime(current_path, current_net);
            if (!runtime) {
                return 2;
            }
        }
        else {
            std::cout << "No .pn file loaded yet. Use: load <file.pn>\n";
        }

        print_help();
        std::string line;

        // interactive enviroment
        while (std::cout << "> " && std::getline(std::cin, line)) {
            std::stringstream stream(line);
            std::string command;
            stream >> command;

            // distincts command from user
            // first checks validity of petrinet if not loading
            if (command == "load") {
                // load another Petrinet with stated path
                std::string path;
                stream >> path;
                // empty path check
                if (path.empty()) {
                    std::cout << "Usage: load <file.pn>\n";
                    continue;
                }

                // tries running in temporary if ok then replaces old Petrinet
                PetriNet new_net;
                auto new_runtime = load_runtime(path, new_net);
                if (new_runtime) {
                    current_path = path;
                    current_net = new_net;
                    runtime = std::move(new_runtime);
                    std::cout << "Loaded: " << path << "\n";
                }
            }
            else if (command == "save") {
                if (!runtime) {
                    std::cout << "No network loaded\n";
                    continue;
                }

                // if no file then rewrites original file or error occres
                std::string path;
                stream >> path;
                if (path.empty()) {
                    path = current_path;
                }
                if (path.empty()) {
                    std::cout << "Usage: save <file.pn>\n";
                    continue;
                }

                // writes Petrinet and saves new path
                Writer writer;
                writer.write_file(current_net, path);
                current_path = path;
                std::cout << "Saved: " << path << "\n";
            }
            else if (command == "input") {
                if (!runtime) {
                    std::cout << "No network loaded\n";
                    continue;
                }

                // reads name and value
                std::string name;
                stream >> name;
                std::string value;
                std::getline(stream, value);
                value = trim(value);

                // valid name check
                if (name.empty()) {
                    std::cout << "Usage: input <name> <value>\n";
                    continue;
                }
                runtime->inject_input(name, value);
            }
            else if (command == "tick") {
                if (!runtime) {
                    std::cout << "No network loaded\n";
                    continue;
                }

                int ms = 0;
                stream >> ms;
                runtime->advance_time(ms);
            }
            else if (command == "step") {
                if (!runtime) {
                    std::cout << "No network loaded\n";
                    continue;
                }
                runtime->step();
            }
            else if (command == "state") {
                if (!runtime) {
                    std::cout << "No network loaded\n";
                    continue;
                }
                print_state(*runtime);
            }
            else if (command == "enabled") {
                if (!runtime) {
                    std::cout << "No network loaded\n";
                    continue;
                }
                print_enabled(runtime->snapshot());
            }
            else if (command == "outputs") {
                if (!runtime) {
                    std::cout << "No network loaded\n";
                    continue;
                }
                print_outputs(runtime->snapshot());
            }
            else if (command == "log") {
                if (!runtime) {
                    std::cout << "No network loaded\n";
                    continue;
                }
                print_log(*runtime);
            }
            else if (command == "help") {
                print_help();
            }
            else if (command == "quit") {
                break;
            }
            else if (!command.empty()) {
                std::cout << "Unknown command. Type 'help'.\n";
            }
        }
        return 0;
    }
    catch (const std::exception& except) {
        std::cerr << "Fatal error: " << except.what() << "\n";
        return 3;
    }
}
