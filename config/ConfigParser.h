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
    ConfigLexer lexer;
    emp::vector<ParseRule> rules;
    std::map<std::string, int> state_ids;
    int next_state;

  public:
    ConfigParser(std::istream & in_stream)
      : lexer(in_stream), next_state(lexer.GetMaxToken()) { ; }
    ~ConfigParser() { ; }

    // Returns state ID for rule.
    int AddRule(const std::string & state_name, const emp::vector<int> & rhs) {
      auto state_ptr = state_ids.find(state_name);
      int result_id = (state_ptr == state_ids.end()) ? (next_state++) : state_ptr->second;
      rules.emplace_back(result_id, rhs);
      return result_id;
    }
  };

}

#endif
