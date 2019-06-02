/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  EmphaticLexer.h
 *  @brief A Lexer that tokenizes inputs to Emphatic.
 **/

#include "../../source/tools/Lexer.h"

class EmphaticLexer : public emp::Lexer {
private:
  int token_identifier = -1;        ///< Token id for identifiers.
  int token_number = -1;            ///< Token id for literal numbers.
  int token_string = -1;            ///< Token id for literal strings.
  int token_char = -1;              ///< Token id for literal characters.
  int token_symbol = -1;            ///< Token id for other symbols.
  int token_pp = -1;                ///< Token id for pre-processor commands.

public:
  EmphaticLexer() {
    // Whitespace and comments should always be dismissed (top priority)
    IgnoreToken("Whitespace", "[ \t\n\r]+");
    IgnoreToken("//-Comments", "//.*");
    IgnoreToken("/*...*/-Comments", "/[*]([^*]|([*]+[^*/]))*[*]+/");

    // Meaningful tokens have next priority.
    token_identifier = AddToken("Identifier", "[a-zA-Z_][a-zA-Z0-9_]*");
    token_number = AddToken("Literal Number", "[0-9]+(.[0-9]+)?");
    token_string = AddToken("Literal String", "\\\"([^\"\\\\]|\\\\.)*\\\"");
    token_char = AddToken("Literal Character", "'([^'\n\\\\]|\\\\.)+'");  // C++ will throw errors for bad usage.

    // Symbol tokens should have least priority.
    token_symbol = AddToken("Symbol", ".|\"::\"|\"==\"|\"!=\"|\"<=\"|\">=\"|\"->\"|\"&&\"|\"||\"|\"<<\"|\">>\"|\"++\"|\"--\"");

    // Pre-processor tokens should just get echoed.
    token_pp = AddToken("Pre-Processor", "#(.|\"\\\\\\n\")+");
  }

  bool IsID(const emp::Token token) const { return token.token_id == token_identifier; }
  bool IsNumber(const emp::Token token) const { return token.token_id == token_number; }
  bool IsString(const emp::Token token) const { return token.token_id == token_string; }
  bool IsSymbol(const emp::Token token) const { return token.token_id == token_symbol; }
  bool IsPP(const emp::Token token) const { return token.token_id == token_pp; }
};