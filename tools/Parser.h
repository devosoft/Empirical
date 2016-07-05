//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A general-purpose, fast parser.
//
//
//  Development notes:
//  * Make sure to warn if a symbol has no patterns associated with it.
//  * ...or if a symbol has no path to terminals.
//  * ...of if a symbol is never use in another pattern (and is not a start state)
//  * Should we change Parser to a template that takes in the type for the lexer?
//
//  Setup -> and | and || operators on Parse symbol to all do the same thing: take a pattern of
//  either string or int (or ideally mixed??) and add a new rule.
//
//    parser("expression") -> { "literal_int" }
//                         -> { "expression", "+", "expression"}
//                         -> { "expression", "*", "expression"}
//                         -> { "(", "expression", ")"}

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
    Lexer & lexer;                     // Default input lexer.
    emp::vector<ParseSymbol> symbols;  // Set of symbols that make up this grammar.
    int cur_symbol_id;                 // Which id should the next new symbol get?
    int active_pos;                    // Which symbol pos is active?

    void BuildRule(emp::vector<int> & new_rule) { ; }
    template <typename T, typename... EXTRAS>
    void BuildRule(emp::vector<int> & new_rule, T && arg, EXTRAS... extras) {
      new_rule.push_back( GetID(std::forward<T>(arg)) );
      BuildRule(new_rule, std::forward<EXTRAS>(extras)...);
    }

    // Return the position in the symbols vector where this name is found; else return -1.
    int GetSymbolPos(const std::string & name) const {
      for (auto i = 0; i < symbols.size(); i++) {
        if (symbols[i].name == name) return i;
      }
      return -1;
    }

    // Create a new symbol and return its POSITION.
    int AddSymbol(const std::string & name) {
      ParseSymbol new_symbol;
      new_symbol.name = name;
      new_symbol.id = cur_symbol_id++;
      const int out_pos = (int) symbols.size();
      symbols.emplace_back(new_symbol);
      return out_pos;
    }
  public:
    Parser(Lexer & in_lexer) : lexer(in_lexer), cur_symbol_id(in_lexer.MaxTokenID()) { ; }
    ~Parser() { ; }

    Lexer & GetLexer() { return lexer; }

    // Simple conversions to find an ID...
    int GetID(int id) const { return id; }
    int GetID(const std::string & name) {
      int spos = GetSymbolPos(name);       // First check if parse symbol exists.
      if (spos >= 0) return spos;          // ...if so, return it.
      int tid = lexer.GetTokenID(name);    // Otherwise, check for token name.
      if (tid >= 0) return tid;            // ...if so, return id.

      // Else, add symbol to declaration list
      spos = AddSymbol(name);
      return symbols[spos].id;
    }

    Parser & operator()(const std::string & name) {
      active_pos = GetSymbolPos(name);
      if (active_pos == -1) active_pos = AddSymbol(name);
      return *this;
    }

    ParseSymbol & GetParseSymbol(const std::string & name) {
      int pos = GetSymbolPos( name );
      return symbols[pos];
    }

    template <typename... STATES>
    Parser & Rule(STATES... states) {
      emp_assert(active_pos >= 0 && active_pos < (int) symbols.size(), active_pos);

      const auto ppos = symbols[active_pos].patterns.size();
      symbols[active_pos].patterns.resize(ppos+1);
      BuildRule(symbols[active_pos].patterns[ppos], states...);
      return *this;
    }

    template <typename... STATES>
    int AddRule(const std::string & name, STATES... states) {
      const int id = GetID(name);
      const int pos = GetSymbolPos(name);  // @CAO We just did this, so can be faster.

      const auto ppos = symbols[pos].patterns.size();
      symbols[pos].patterns.resize(ppos+1);
      BuildRule(symbols[pos].patterns[ppos], states...);
      return id;
    }

  };

}

#endif
