/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2023
*/
/**
 *  @file
 *  @brief String helper class to specify syntax for quotes and paren.
 *  @note Status: ALPHA
 *
 *
 *  @todo Set up with regular expressions to handle less trivial syntax structures.
 *  @todo Make constexpr
 *  @todo Make handle non-'char' strings (i.e., use CharT template parameter)
 *
 */

#ifndef EMP_TOOLS__STRINGSYNTAX_HPP_INCLUDE
#define EMP_TOOLS__STRINGSYNTAX_HPP_INCLUDE


#include <sstream>
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
      for (char c : quotes) GetMatch(c) = c;
      for (size_t i=0; i < parens.size(); i+=2) GetMatch(parens[i]) = parens[i+1];
      count = static_cast<uint8_t>(quotes.size() + parens.size()/2);
    }
    StringSyntax(bool match_quotes, bool match_parens=false) : StringSyntax() {
      if (match_quotes) GetMatch('"') = '"';
      if (match_parens) {
        GetMatch('(') = ')';
        GetMatch('[') = ']';
        GetMatch('{') = '}';
      }
    }
    StringSyntax(const char * quotes, const char * parens="")
      : StringSyntax(std::string(quotes), std::string(parens)) { }

    StringSyntax & operator=(const StringSyntax &) = default;
    StringSyntax & operator=(StringSyntax &&) = default;

    char & GetMatch(char c) { emp_assert(c >= 0); return char_matches[static_cast<size_t>(c)]; }
    char GetMatch(char c) const { return (c >= 0) ? char_matches[static_cast<size_t>(c)] : 0; }
    bool IsQuote(char c) const { return (c > 0) && GetMatch(c) && (GetMatch(c) == c); }
    bool IsParen(char c) const { return (c > 0) && GetMatch(c) && (GetMatch(c) != c); }
    uint8_t GetCount() const { return count; }

    std::string GetQuotes() const {
      std::string out;
      for (char i = 0; i < 127; ++i) {
        if (GetMatch(i) == i) out+= static_cast<char>(i);
      }
      return out;
    }

    std::string AsString() const {
      std::stringstream ss;
      for (char c = 0; c < 127; ++c) {
        emp_assert(static_cast<size_t>(c) < 128, (int) c, static_cast<size_t>(c));
        if (GetMatch(c)) {
          ss << "['" << c << "'->'" << GetMatch(c) << "']";
        }
      }
      return ss.str();
    }

    static StringSyntax None()       { return StringSyntax(); }
    static StringSyntax Quotes()     { return StringSyntax("\""); }
    static StringSyntax CharQuotes() { return StringSyntax("'"); }
    static StringSyntax AllQuotes()  { return StringSyntax("\"'"); }
    static StringSyntax Parens()     { return StringSyntax("",     "()[]{}"); }
    static StringSyntax RParens()    { return StringSyntax("",    ")(][}{"); }
    static StringSyntax Full()       { return StringSyntax("\"",   "()[]{}"); }
    static StringSyntax Max()        { return StringSyntax("\"'`", "()[]{}<>"); }
  };

}


#endif // #ifndef EMP_TOOLS__STRINGSYNTAX_HPP_INCLUDE
