// autor: Jurišinová Daniela (xjurisd00)

#ifndef PARSER_HPP
#define PARSER_HPP

#include "model.hpp"
#include <string>
#include <vector>

/**
 * @file parser.hpp
 * @brief Parser and writer for readable text format of Petri Net
 */

struct ParseError {
    int line = 0;
    std::string message;
};

struct ParseResult {
    PetriNet net;
    std::vector<ParseError> errors;

    bool ok() const { return errors.empty(); }
    operator PetriNet() const { return net; }
};

/**
 * Parser of the Petrinet in text format 
 */
class Parser {
public:
    ParseResult parse_file(const std::string& path) const;
    ParseResult parse_string(const std::string& content) const;
};

class Writer {
public:
    void write_file(const PetriNet& net, const std::string& path) const;
    std::string write_string(const PetriNet& net) const;
};

#endif