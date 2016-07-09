//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A general-purpose, fast parser.
//
//
//  Development notes:
//  * Patterns should include functions that are called when that point of rule is triggered.
//  * Make sure to warn if a symbol has no patterns associated with it.
//  * ...or if a symbol has no path to terminals.
//  * ...of if a symbol is never use in another pattern (and is not a start state)
//  * Should we change Parser to a template that takes in the type for the lexer?
//
//  Setup -> and | and || operators on Parse symbol to all do the same thing: take a pattern of
//  either string or int (or ideally mixed??) and add a new rule.
//
//    parser("expression") -> { "literal_int" }
//                         |  { "expression", "+", "expression"}
//                         |  { "expression", "*", "expression"}
//                         |  { "(", "expression", ")"}

#ifndef EMP_PARSER_H
#define EMP_PARSER_H

#include "Lexer.h"
#include "vector.h"

namespace emp {

  // A single symbol in a grammer including the patterns that generate it.
  struct ParseSymbol {
    std::string name;
    emp::vector< emp::vector<int> > patterns;
    int id;

    emp::vector<int> first;   // What tokens can begin this symbol?
    emp::vector<int> follow;  // What tokens can come after this symbol?
    bool nullable;            // Can this symbol be converted to nothing?

    ParseSymbol()
     : first(Lexer::MaxTokenID(), -1), follow(Lexer::MaxTokenID(), -1), nullable(false) { ; }
  };

  // A single node in a parse tree.
  struct ParseNode {
    int symbol_id;
    int rule_pos;
    emp::vector< ParseNode * > children;
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
      if (spos >= 0) return symbols[spos].id;          // ...if so, return it.
      int tid = lexer.GetTokenID(name);    // Otherwise, check for token name.
      if (tid >= 0) return tid;            // ...if so, return id.

      // Else, add symbol to declaration list
      spos = AddSymbol(name);
      return symbols[spos].id;
    }

    std::string GetName(int symbol_id) const {
      if (symbol_id < lexer.MaxTokenID()) return lexer.GetTokenName(symbol_id);
      const int spos = symbol_id - lexer.MaxTokenID();
      return symbols[spos].name;
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

    // Use the currently active symbol and attach a rule to it.
    template <typename... STATES>
    Parser & Rule(STATES... states) {
      emp_assert(active_pos >= 0 && active_pos < (int) symbols.size(), active_pos);

      const auto ppos = symbols[active_pos].patterns.size();
      symbols[active_pos].patterns.resize(ppos+1);
      BuildRule(symbols[active_pos].patterns[ppos], states...);
      if (symbols[active_pos].patterns[ppos].size() == 0) symbols[active_pos].nullable = true;
      return *this;
    }

    // Specify the name of the symbol and add a rule to it, returning the symbol id.
    template <typename... STATES>
    int AddRule(const std::string & name, STATES... states) {
      const int id = GetID(name);
      active_pos = GetSymbolPos(name);  // @CAO We just did this, so can be faster.
      Rule(std::forward<STATES>(states)...);
      return id;
    }

    // Scan through the current grammar and try to spot any problems.
    bool Validate(std::ostream & os=std::cerr) {
      bool valid = true;
      return valid;
    }

    void Process(std::istream & is, bool test_valid=true) {
      if (test_valid) Validate();
    }

    void Print(std::ostream & os=std::cout) const {
      os << symbols.size() << " parser symbols available." << std::endl;
      for (const auto & s : symbols) {
        os << "symbol '" << s.name << "' (id " << s.id << ") has "
           << s.patterns.size() << " patterns." << std::endl;
        for (const auto & p : s.patterns) {
          os << " ";
          if (p.size() == 0) os << " [empty]";
          for (int x : p) os << " " << GetName(x) << "(" << x << ")";
          os << std::endl;
        }
      }
    }

  };

}

#endif
