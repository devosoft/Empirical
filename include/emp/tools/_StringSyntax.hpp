/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023.
 *
 *  @file StringSyntax.hpp
 *  @brief String helper class to specify syntax for quotes and paren.
 *  @note Status: ALPHA
 *
 *
 *  @todo Set up with regular expressions to handle less trivial syntax structures.
 *  @todo Make constexpr
 *  @todo Make handle non-'char' strings (i.e., use CharT template parameter)
 *
 */

#ifndef EMP_TOOLS_STRING_SYNTAX_HPP_INCLUDE
#define EMP_TOOLS_STRING_SYNTAX_HPP_INCLUDE


#include <string>

#include "../base/array.hpp"
#include "../base/assert.hpp"


namespace emp {

  class StringSyntax {
  private:
    emp::array<char, 128> char_matches;
    uint8_t count = 0;

  public:
    StringSyntax() { char_matches.fill('\0'); }
    StringSyntax(std::string quotes, std::string parens="") : StringSyntax() {
      emp_assert(parens.size() % 2 == 0, "String::StringSyntax must have odd number of paren chars.");
      for (char c : quotes) char_matches[c] = c;
      for (size_t i=0; i < parens.size(); i+=2) char_matches[parens[i]] = parens[i+1];
      count = static_cast<uint8_t>(quotes.size() + parens.size()/2);
    }
    StringSyntax(bool match_quotes, bool match_parens=false) : StringSyntax() {
      if (match_quotes) char_matches['"'] = '"';
      if (match_parens) {
        char_matches['('] = ')';
        char_matches['['] = ']';
        char_matches['{'] = '}';
      }
    }
    StringSyntax(const char * quotes, const char * parens="")
      : StringSyntax(std::string(quotes), std::string(parens)) { }
    StringSyntax & operator=(const StringSyntax &) = default;
    StringSyntax & operator=(StringSyntax &&) = default;
    bool IsQuote(char c) const { return (c > 0) && char_matches[c] && (char_matches[c] == c); }
    bool IsParen(char c) const { return (c > 0) && char_matches[c] && (char_matches[c] != c); }
    char GetMatch(char c) const { return (c >= 0) && char_matches[c]; }
    uint8_t GetCount() const { return count; }
    std::string GetQuotes() const { std::string out; for (uint8_t i=0; i < 128; ++i) if (char_matches[i]==i) out+=(char)i; return out;}

    static StringSyntax None()   { return StringSyntax(); }
    static StringSyntax Quotes() { return StringSyntax("\""); }
    static StringSyntax CharQuotes() { return StringSyntax("'"); }
    static StringSyntax AllQuotes() { return StringSyntax("\"'"); }
    static StringSyntax Parens() { return StringSyntax("",     "()[]{}"); }
    static StringSyntax RParens() { return StringSyntax("",    ")(][}{"); }
    static StringSyntax Full()   { return StringSyntax("\"",   "()[]{}"); }
    static StringSyntax Max()    { return StringSyntax("\"'`", "()[]{}<>"); }
  };

}


#endif // #ifndef EMP_TOOLS_STRING_SYNTAX_HPP_INCLUDE
