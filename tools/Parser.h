//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A general-purpose, fast parser.
//
//  Development notes:
//  * Should we change Parser to a template that takes in the type for the lexer?

#ifndef EMP_PARSER_H
#define EMP_PARSER_H

#include "Lexer.h"
#include "vector.h"

namespace emp {

  struct ParseSymbol {
    std::string name;
    emp::vector< emp::vector<int> > patterns;
    int id;
  };

  class Parser {
  private:
    Lexer & lexer;                  // Default input lexer.
    emp::vector<ParseSymbol> rules;   // Set of rules that make up this grammar.
    int cur_rule_id;                // Which id should the next new rule get?

    void BuildRule(emp::vector<int> & new_rule) { ; }
    template <typename T, typename... EXTRAS>
    void BuildRule(emp::vector<int> & new_rule, T && arg, EXTRAS... extras) {
      new_rule.push_back( GetID(std::forward<T>(arg)) );
      BuildRule(new_rule, std::forward<EXTRAS>(extras)...);
    }
  public:
    Parser(Lexer & in_lexer) : lexer(in_lexer), cur_rule_id(in_lexer.MaxTokenID()) { ; }
    ~Parser() { ; }

    Lexer & GetLexer() { return lexer; }

    // Simple conversions to find an ID...
    int GetID(int id) const { return id; }
    int GetID(const std::string & name) const {
      // @CAO Check existing rules.
      // @CAO Else, check lexer
      // @CAO Else, add rule to declaration list
      return 0;
    }

    template <typename... STATES>
    int AddRule(const std::string & name, STATES... states) {
      // @CAO See if this name already exists.
      ParseSymbol new_rule;
      new_rule.name = name;
      new_rule.id = cur_rule_id++;
      const auto pos = new_rule.patterns.size();
      new_rule.patterns.resize(pos+1);
      BuildRule(new_rule.patterns[pos], states...);
      return new_rule.id;
    }

  };

}

#endif
