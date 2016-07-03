//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A general-purpose, fast parser.
//
//  Development notes:
//  * Make sure to warn if a symbol has no patterns associated with it.
//  * ...or if a symbol has no path to terminals.
//  * ...of if a symbol is never use in another pattern (and is not a start state)
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
    Lexer & lexer;                     // Default input lexer.
    emp::vector<ParseSymbol> symbols;  // Set of symbols that make up this grammar.
    int cur_symbol_id;                 // Which id should the next new symbol get?

    void BuildRule(emp::vector<int> & new_rule) { ; }
    template <typename T, typename... EXTRAS>
    void BuildRule(emp::vector<int> & new_rule, T && arg, EXTRAS... extras) {
      new_rule.push_back( GetID(std::forward<T>(arg)) );
      BuildRule(new_rule, std::forward<EXTRAS>(extras)...);
    }

    int GetSymbolPos(const std::string & name) const {
      for (auto i = 0; i < symbols.size(); i++) {
        if (symbols[i].name == name) return i;
      }
      return -1;
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

      ParseSymbol new_symbol;
      new_symbol.name = name;
      new_symbol.id = cur_symbol_id++;
      symbols.emplace_back(new_symbol);
      return new_symbol.id;
    }

    ParseSymbol & GetParseSymbol(const std::string & name) {
      int pos = GetSymbolPos( name );
      return symbols[pos];
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
