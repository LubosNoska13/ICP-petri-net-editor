// autor: Jurišinová Daniela (xjurisd00)

#ifndef PARSER_HPP
#define PARSER_HPP

#include "model.hpp"
#include <string>

/**
 * @file parser.hpp
 * @brief Parser and writer for readable text format of Petri Net
 */

class Parser {
public:
    PetriNet parse_file(const std::string& path) const;
    PetriNet parse_string(const std::string& content) const;
};

class Writer {
public:
    void write_file(const PetriNet& net, const std::string& path) const;
    std::string write_string(const PetriNet& net) const;
};

#endif