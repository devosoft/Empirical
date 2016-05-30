//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A simple parser for the Empirical configuration language.
//
//  This parser is being implemented as a pushdown automata.

#ifndef EMP_CONFIG_PARSER_H
#define EMP_CONFIG_PARSER_H

#include <map>
#include <string>

#include "../tools/vector.h"

#include "ConfigLexer.h"

namespace emp {

  struct ParseRule {
    int result_id;
    emp::vector<int> pattern;

    ParseRule(int id, const emp::vector<int> & _p) : result_id(id), pattern(_p) { ; }
  };

  class ConfigParser {
  private:
    ConfigLexer lexer;                      // Lexer to provide token stream.
    emp::vector<ParseRule> rules;           // Vector of all rules linking states to productions.
    std::map<std::string, int> state_ids;   // Map of state names to their IDs.
    int next_state;                         // If we add another state, what ID should we use?

    int StateNameToID(const std::string & name) {
      auto state_ptr = state_ids.find(name);
      if (state_ptr == state_ids.end()) return next_state++;
      return state_ptr->second;
    }
  public:
    ConfigParser(std::istream & in_stream)
      : lexer(in_stream), next_state(lexer.GetMaxToken()) { ; }
    ~ConfigParser() { ; }

    // Returns state ID for rule.
    int AddRule(int state_id, const emp::vector<int> & rhs) {
      rules.emplace_back(state_id, rhs);
      return state_id;
    }

    int AddRule(const std::string & state_name, const emp::vector<int> & rhs) {
      return AddRule( StateNameToID(state_name), rhs );
    }
  };

}

#endif
