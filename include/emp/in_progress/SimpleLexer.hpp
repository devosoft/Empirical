/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file SimpleLexer.hpp
 *  @brief Lexer with common functionality already filled in.
 *  @note  Status: ALPHA
 */

#ifndef EMP_IN_PROGRESS_SIMPLELEXER_HPP_INCLUDE
#define EMP_IN_PROGRESS_SIMPLELEXER_HPP_INCLUDE

#include <cmath>
#include <string>

#include "../base/error.hpp"
#include "../compiler/Lexer.hpp"

namespace emp {

  using namespace std::string_literals;

  class SimpleLexer : public emp::Lexer {
  private:
    int token_identifier;   ///< Token id for identifiers
    int token_number;       ///< Token id for literal numbers
    int token_string;       ///< Token id for literal strings
    int token_char;         ///< Token id for literal characters
    int token_external;     ///< Token id for strings to be evaluated externally.
    int token_symbol;       ///< Token id for other symbols

  public:
    SimpleLexer() {
      // Whitespace and comments should always be dismissed (top priority)
      IgnoreToken("Whitespace", "[ \t\n\r]+");
      IgnoreToken("//-Comments", "//.*");
      IgnoreToken("/*...*/-Comments", "/[*]([^*]|([*]+[^*/]))*[*]+/");

      // Meaningful tokens have next priority.

      // An indentifier must begin with a letter, underscore, or dot, and followed by
      // more of the same OR numbers or brackets.
      token_identifier = AddToken("Identifier", "[a-zA-Z_.][a-zA-Z0-9_.[\\]]*");

      // A literal number must begin with a digit; it can have any number of digits in it and
      // optionally a decimal point.
      token_number = AddToken("Literal Number", "[0-9]+(\\.[0-9]+)?");

      // A string must begin and end with a quote and can have an escaped quote in the middle.
      token_string = AddToken("Literal String", "\\\"([^\"\\\\]|\\\\.)*\\\"");

      // A literal char must begin and end with a single quote.  It will always be treated as
      // its ascii value.
      token_char = AddToken("Literal Character", "'([^'\n\\\\]|\\\\.)+'");

      // Setup a RegEx that can detect up to 4-deep nested parentheses.
      const std::string no_parens = "[^()\n\r]*";
      const std::string open = "\"(\"";
      const std::string close = "\")\"";
      const std::string matched_parens = open + no_parens + close;
      const std::string multi_parens = no_parens + "("s + matched_parens + no_parens + ")*"s;
      const std::string nested_parens2 = open + multi_parens + close;
      const std::string multi_nested2 = no_parens + "("s + nested_parens2 + no_parens + ")*"s;
      const std::string nested_parens3 = open + multi_nested2 + close;
      const std::string multi_nested3 = no_parens + "("s + nested_parens3 + no_parens + ")*"s;
      const std::string nested_parens4 = open + multi_nested3 + close;
      const std::string multi_nested4 = no_parens + "("s + nested_parens4 + no_parens + ")*"s;

      // An external value should be evaluated in a provided function.  If no such function
      // exists, using it will be an error.
      token_external = AddToken("External Evaluation", "\"$(\""s + multi_nested4 + "\")\""s);

      // Symbols should have least priority.  They include any solitary character not listed
      // above, or pre-specified multi-character groups.
      token_symbol = AddToken("Symbol", ".|\"==\"|\"!=\"|\"<=\"|\">=\"|\"&&\"|\"||\"|\"**\"|\"%%\"");
    }

    bool IsID(const emp::Token token) const noexcept { return token.token_id == token_identifier; }
    bool IsNumber(const emp::Token token) const noexcept { return token.token_id == token_number; }
    bool IsString(const emp::Token token) const noexcept { return token.token_id == token_string; }
    bool IsChar(const emp::Token token) const noexcept { return token.token_id == token_char; }
    bool IsSymbol(const emp::Token token) const noexcept { return token.token_id == token_symbol; }
  };
}

#endif // #ifndef EMP_IN_PROGRESS_SIMPLELEXER_HPP_INCLUDE
