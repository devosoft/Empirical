//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A simple parser for the Empirical configuration language.
//
//  This parser is being implemented as a pushdown automata.

#ifndef EMP_CONFIG_PARSER_H
#define EMP_CONFIG_PARSER_H

#include <map>
#include <string>

#include "../base/vector.hpp"

#include "ConfigLexer.hpp"

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

    int ToStateID(const std::string & name) {
      auto state_ptr = state_ids.find(name);
      if (state_ptr == state_ids.end()) return next_state++;
      return state_ptr->second;
    }
    int ToStateID(int state_id) const { return state_id; }

    // Take a token name or as state name and convert to its ID.
    int StringToID(const std::string name) {
      // @CAO Check to see if this is a TOKEN name!!!
      return ToStateID(name);
    }

    emp::vector<int> ToRHS(const emp::vector<std::string> & str_rhs) {
      emp::vector<int> rhs(str_rhs.size());
      for (int i = 0; i < (int) str_rhs.size(); i++) {
        rhs[i] = StringToID(str_rhs[i]);
      }
      return rhs;
    }
    emp::vector<int> ToRHS(const std::string & str_rhs) {
      return ToRHS(emp::slice(str_rhs, ' '));
    }
    emp::vector<int> ToRHS(const emp::vector<int> & rhs) { return rhs; }

    int AddRule_impl(int state_id, const emp::vector<int> & rhs) {
      rules.emplace_back(state_id, rhs);
      return state_id;
    }

  public:
    ConfigParser(std::istream & in_stream)
      : lexer(in_stream), next_state(lexer.GetMaxToken()) { ; }
    ~ConfigParser() { ; }

    // Returns state ID for rule.
    template <typename T1, typename T2>
    int AddRule(T1 && state, const emp::vector<int> & rhs) {
      return AddRule_impl( ToStateID(std::forward<T1>(state)), ToRHS(std::forward<T2>(rhs)) );
    }
  };

}

#endif
