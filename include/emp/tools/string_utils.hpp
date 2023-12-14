/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2016-2023.
*/
/**
 *  @file
 *  @brief Simple functions to manipulate strings.
 *  Status: RELEASE
 */

#ifndef EMP_TOOLS_STRING_UTILS_HPP_INCLUDE
#define EMP_TOOLS_STRING_UTILS_HPP_INCLUDE


#include <algorithm>
#include <cctype>
#include <cstdio>
#include <functional>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <regex>
#include <sstream>
#include <stddef.h>
#include <string>
#include <string_view>
#include <unordered_set>

#include "../base/array.hpp"
#include "../base/assert.hpp"
#include "../base/notify.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../meta/reflection.hpp"
#include "../meta/type_traits.hpp"

#include "char_utils.hpp"

namespace emp {

  /// Return a const reference to an empty string.  This function is useful to implement other
  /// functions that need to return a const reference for efficiency, but also need a null response.
  static inline const std::string & empty_string() {
    static std::string empty = "";
    return empty;
  }

  /// Count the number of times a specific character appears in a string
  /// (a clean shortcut to std::count)
  static inline size_t count(const std::string & str, char c) {
    return (size_t) std::count(str.begin(), str.end(), c);
  }

  /// Test if an input string is properly formatted as a literal character.
  static inline bool is_literal_char(const std::string & value);

  /// Test if an input string is properly formatted as a literal string.
  static inline bool is_literal_string(const std::string & value,
                                       const std::string & quote_marks="\"");

  /// Test if an input string is properly formatted as a literal string.
  static inline std::string diagnose_literal_string(const std::string & value,
                                                    const std::string & quote_marks="\"");

  /// Determine if a string is composed only of a set of characters (represented as a string)
  static inline bool is_composed_of(const std::string & test_str, const std::string & char_set) {
    for (char x : test_str) if (!is_one_of(x, char_set)) return false;
    return true;
  }

  /// Determine if there is whitespace anywhere in a string.
  inline bool has_whitespace(const std::string & str) { return WhitespaceCharSet().HasAny(str); }
  inline bool has_nonwhitespace(const std::string & str) { return !WhitespaceCharSet().HasOnly(str); }
  inline bool has_upper_letter(const std::string & str) { return UpperCharSet().HasAny(str); }
  inline bool has_lower_letter(const std::string & str) { return LowerCharSet().HasAny(str); }
  inline bool has_letter(const std::string & str) { return LetterCharSet().HasAny(str); }
  inline bool has_digit(const std::string & str) { return DigitCharSet().HasAny(str); }
  inline bool has_alphanumeric(const std::string & str) { return AlphanumericCharSet().HasAny(str); }

  inline bool has_char_at(const std::string & str, char c, size_t pos) {
    return (pos < str.size()) && (str[pos] == c);
  }
  inline bool has_one_of_at(const std::string & str, const std::string & opts, size_t pos) {
    return (pos < str.size()) && is_one_of(str[pos], opts);
  }
  inline bool has_digit_at(const std::string & str, size_t pos) { return DigitCharSet().HasAt(str, pos); }
  inline bool has_letter_at(const std::string & str, size_t pos) { return LetterCharSet().HasAt(str, pos); }

  /// Determine if there are only lowercase letters in a string.
  inline bool is_lower(const std::string & str) {
    if (str.size() == 0) return false;   // If string is empty, there are NO letters.
    return LowerCharSet().Has(str);      // Otherwise return false if any character is not lower.
  }

  /// Determine if there are only uppercase letters in a string.
  inline bool is_upper(const std::string & str) {
    if (str.size() == 0) return false;   // If string is empty, there are NO letters.
    return UpperCharSet().Has(str);      // Otherwise return false if any character is not upper.
  }

  /// Determine if there are only digits in a string.
  inline bool is_digits(const std::string & str) {
    if (str.size() == 0) return false;   // If string is empty, there are NO digits.
    return DigitCharSet().Has(str);      // Otherwise return false if any character is not a digit.
  }

  /// Determine if this string represents a proper number.
  inline bool is_number(const std::string & str) {
    if (str.size() == 0) return false;             // If string is empty, not a number!
    size_t pos = 0;
    if (has_one_of_at(str, "+-", pos)) ++pos;      // skip leading +/-
    while (has_digit_at(str, pos)) ++pos;          // Any number of digits (zero is okay)
    if (has_char_at(str, '.', pos)) {              // If there's a DECIMAL PLACE, look for more digits.
      ++pos;                                       // Skip over the dot.
      if (!has_digit_at(str, pos++)) return false; // Must have at least one digit after '.'
      while (has_digit_at(str, pos)) ++pos;        // Any number of digits.
    }
    if (has_one_of_at(str, "eE", pos)) {           // If there's an e... SCIENTIFIC NOTATION
      ++pos;                                       // Skip over the e.
      if (has_one_of_at(str, "+-", pos)) ++pos;    // skip leading +/-
      if (!has_digit_at(str, pos++)) return false; // Must have at least one digit after 'e'
      while (has_digit_at(str, pos)) ++pos;        // Allow for MORE digits.
    }
    // If we've made it to the end of the string AND there was at least one digit, success!
    return (pos == str.size()) && has_digit(str);
  }

  /// Determine if string is only letters or digits.
  inline bool is_alphanumeric(const std::string & str) {
    if (str.size() == 0) return false;      // If string is empty, there are NO characters.
    return AlphanumericCharSet().Has(str);  // Otherwise return false if any character is not a digit.
  }

  inline bool is_whitespace(const std::string & str) {
    if (str.size() == 0) return true; // Empty string is just whitespace.
    return WhitespaceCharSet().Has(str);
  }

  /// Determine if string is only letters, digits, or underscore ('_').
  inline bool is_identifier(const std::string & str) {
    if (str.size() == 0) return false;   // If string is empty, there are NO characters.
    if (is_digit(str[0])) return false;  // Identifiers cannot begin with a number.
    return IDCharSet().Has(str);         // Otherwise return false if any character is not a digit.
  }

  /// Determine if a specified set of characters appears anywhere in a string.
  static inline bool has_one_of(const std::string & str, const std::string & char_set) {
    for (char c : str) if (is_one_of(c, char_set)) return true;
    return false;
  }

  /// For a string to be valid, each character must pass at least one provided function.
  template <typename... FUNS>
  static inline bool is_valid(const std::string & str, FUNS... funs) {
    for (char x : str) if ( !is_valid(x, funs...) ) return false;
    return true;
  }

  /// Test if a string has a given prefix.
  inline bool has_prefix(const std::string & in_string, const std::string & prefix) {
    if (prefix.size() > in_string.size()) return false;
    for (size_t i = 0; i < prefix.size(); ++i) {
      if (in_string[i] != prefix[i]) return false;
    }
    return true;
  }

  // Given the start position of a quote, find where it ends.
  static inline size_t find_quote_match(std::string_view in_string, const size_t start_pos=0, char mark='"') {
    // A literal string must begin and end with a double quote and contain only valid characters.
    if (in_string.size() < start_pos+2) return start_pos;
    if (in_string[start_pos] != mark) return start_pos;

    // Search for the close-quote.
    for (size_t pos = start_pos + 1; pos < in_string.size(); ++pos) {
      // If we have a backslash, does not end on this or next char.
      if (in_string[pos] == '\\') {
        ++pos;
        continue;
      }
      // If we found the close-quote, pop to here.
      if (in_string[pos] == mark) {
        return pos;
      }
    }

    // If we made it here without a close-quote, no full quote is available!
    return start_pos;
  }

  static inline size_t find_paren_match(std::string_view in_string, const size_t start_pos=0,
                                        const char open='(', const char close=')',
                                        const bool skip_quotes=true) {
    if (in_string[start_pos] != open) return start_pos;
    size_t open_count = 1;
    for (size_t pos = start_pos + 1; pos < in_string.size(); ++pos) {
      if (in_string[pos] == open) ++open_count;
      else if (in_string[pos] == close) {
        --open_count;
        if (open_count == 0) return pos;
      }
      else if (in_string[pos] == '"' && skip_quotes) {
        pos = emp::find_quote_match(in_string, pos);
      }
      else if (in_string[pos] == '\'' && skip_quotes) {
        pos = emp::find_quote_match(in_string, pos, '\'');
      }
    }

    return start_pos;
  }

  // A version of string::find() that can skip over quotes.
  static inline size_t find(std::string_view in_string, std::string target, size_t start_pos,
                            bool skip_quotes=false, bool skip_parens=false,
                            bool skip_braces=false, bool skip_brackets=false) {
    size_t found_pos = in_string.find(target, start_pos);
    if (!skip_quotes && !skip_parens && !skip_braces && !skip_brackets) return found_pos;

    // Make sure we are not in a quote and/or parens; adjust as needed!
    for (size_t scan_pos=0;
         scan_pos < found_pos && found_pos != std::string::npos;
         scan_pos++)
    {
      // Skip quotes, if needed...
      if (skip_quotes && (in_string[scan_pos] == '"' || in_string[scan_pos] == '\'')) {
        scan_pos = find_quote_match(in_string, scan_pos, in_string[scan_pos]);
        if (found_pos < scan_pos) found_pos = in_string.find(target, scan_pos);
      }
      else if (skip_parens && in_string[scan_pos] == '(') {
        scan_pos = find_paren_match(in_string, scan_pos, '(', ')', skip_quotes);
        if (found_pos < scan_pos) found_pos = in_string.find(target, scan_pos);
      }
      else if (skip_braces && in_string[scan_pos] == '{') {
        scan_pos = find_paren_match(in_string, scan_pos, '{', '}', skip_quotes);
        if (found_pos < scan_pos) found_pos = in_string.find(target, scan_pos);
      }
      else if (skip_brackets && in_string[scan_pos] == '[') {
        scan_pos = find_paren_match(in_string, scan_pos, '[', ']', skip_quotes);
        if (found_pos < scan_pos) found_pos = in_string.find(target, scan_pos);
      }
    }

    return found_pos;
  }

  static inline void find_all(
    std::string_view in_string, char target, emp::vector<size_t> & results,
    const bool skip_quotes=false, bool skip_parens=false, bool skip_braces=false,
    bool skip_brackets=false
  ) {
    results.resize(0);
    for (size_t pos=0; pos < in_string.size(); pos++) {
      if (in_string[pos] == target) results.push_back(pos);
      // See if we need to skip over a section...
      switch (in_string[pos]) {
        case '"':
        case '\'':
          if (skip_quotes) pos = find_quote_match(in_string, pos, in_string[pos]);
          break;
        case '(':
          if (skip_parens) pos = find_paren_match(in_string, pos, '(', ')', skip_quotes);
          break;
        case '{':
          if (skip_braces) pos = find_paren_match(in_string, pos, '{', '}', skip_quotes);
          break;
        case '[':
          if (skip_brackets) pos = find_paren_match(in_string, pos, '[', ']', skip_quotes);
          break;
      }
    }
  }

  static inline emp::vector<size_t> find_all(
    std::string_view in_string, char target, bool skip_quotes=false,
    bool skip_parens=false, bool skip_braces=false, bool skip_brackets=false
  ) {
    emp::vector<size_t> out;
    find_all(in_string, target, out, skip_quotes, skip_parens, skip_braces, skip_brackets);
    return out;
  }

  /// Return the first position found for any of a set of substring tests
  /// (or std::string::npos if none are found).
  template <typename... Ts>
  static inline size_t find_any_of_from(
    const std::string & test_str,
    size_t start_pos,
    std::string test1,
    Ts... tests)
  {
    size_t pos1 = test_str.find(test1, start_pos);
    if constexpr (sizeof...(Ts) == 0) return pos1;
    else {
      size_t pos2 = find_any_of_from(test_str, start_pos, tests...);
      if (pos1 == std::string::npos) return pos2;
      if (pos2 == std::string::npos) return pos1;
      return std::min(pos1, pos2);
    }
  }

  template <typename T, typename... Ts>
  static inline size_t find_any_of(const std::string & test_str, T test1, Ts... tests)
  {
    // If an offset is provided, use it.
    if constexpr (std::is_arithmetic_v<T>) {
      return find_any_of_from(test_str, test1, std::forward<Ts>(tests)...);
    } else {
      return find_any_of_from(test_str, 0, test1, tests...);
    }
  }

  // Find an identifier.  A key here is that the found string should NOT have an alphanumeric
  // character or '_' immediately before it or after it.
  static inline size_t find_id(
    std::string_view in_string, std::string target, size_t start_pos,
    bool skip_quotes=true, bool skip_parens=false, bool skip_braces=false, bool skip_brackets=false
  ) {
    size_t pos = emp::find(in_string, target, start_pos, skip_quotes, skip_parens, skip_braces, skip_brackets);
    while (pos != std::string::npos) {
      bool before_ok = (pos == 0) || !is_idchar(in_string[pos-1]);
      size_t after_pos = pos+target.size();
      bool after_ok = (after_pos == in_string.size()) || !is_idchar(in_string[after_pos]);
      if (before_ok && after_ok) return pos;

      pos = emp::find(in_string, target, pos+target.size(), skip_quotes, skip_parens, skip_braces, skip_brackets);
    }

    return std::string::npos;
  }

  // Search for a non-whitespace character.
  static inline size_t find_non_whitespace(std::string_view in_string, size_t pos) {
    while (pos < in_string.size()) {
      if (!is_whitespace(in_string[pos])) return pos;
      ++pos;
    }
    return std::string::npos;
  }

  /// Convert a single character to one that uses a proper escape sequence (in a string) if needed.
  [[nodiscard]] static inline std::string to_escaped_string(char value);

  /// Convert a full string to one that uses proper escape sequences, as needed.
  [[nodiscard]] static inline std::string to_escaped_string(const std::string & value) {
    std::stringstream ss;
    for (char c : value) { ss << to_escaped_string(c); }
    return ss.str();
  }

  /// Take a string and replace reserved HTML characters with character entities
  [[nodiscard]] inline std::string to_web_safe_string(const std::string & value) {
    std::string web_safe = value;
    std::regex apm("[&]");
    std::regex open_brace("[<]");
    std::regex close_brace("[>]");
    std::regex single_quote("[']");
    std::regex double_quote("[\"]");

    web_safe = std::regex_replace(web_safe, apm, "&amp");
    web_safe = std::regex_replace(web_safe, open_brace, "&lt");
    web_safe = std::regex_replace(web_safe, close_brace, "&gt");
    web_safe = std::regex_replace(web_safe, single_quote, "&apos");
    web_safe = std::regex_replace(web_safe, double_quote, "&quot");

    return web_safe;
  }


  /// Returns url encoding of value.
  /// See https://en.wikipedia.org/wiki/Percent-encoding
  // adapted from https://stackoverflow.com/a/17708801
  template<bool encode_space=false>
  [[nodiscard]] std::string url_encode(const std::string &value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (const auto c : value) {

      // If encoding space, replace with +
      if ( encode_space && c == ' ' ) escaped << '+';
      // Keep alphanumeric and other accepted characters intact
      else if (
        std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~'
      ) escaped << c;
      // Any other characters are percent-encoded
      else {
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << (static_cast<int>(c) & 0x000000FF);
        escaped << std::nouppercase;
      }

    }

    return escaped.str();
  }

  /// Returns url decoding of string.
  /// See https://en.wikipedia.org/wiki/Percent-encoding
  // adapted from https://stackoverflow.com/a/29962178
  template<bool decode_plus=false>
  [[nodiscard]] std::string url_decode(const std::string& str){
    std::string res;

    for (size_t i{}; i < str.size(); ++i) {
      if (str[i] == '%') {
        int hex_code;
        std::sscanf(str.substr(i + 1, 2).c_str(), "%x", &hex_code);
        res += static_cast<char>(hex_code);
        i += 2;
      } else {
        res += ( decode_plus && str[i] == '+' ) ? ' ' : str[i];
      }
    }

    return res;
  }

  /// Take a value and convert it to a C++-style literal.
  template <typename T>
  inline
  typename std::enable_if<!emp::IsIterable<T>::value, std::string>::type to_literal(const T & value) {
    return std::to_string(value);
  }

  /// Take a char and convert it to a C++-style literal.
  [[nodiscard]] static inline std::string to_literal(char value) {
    std::stringstream ss;
    ss << "'" << to_escaped_string(value) << "'";
    return ss.str();
  }

  /// Take a string or iterable and convert it to a C++-style literal.
  // This is the version for string. The version for an iterable is below.
  [[nodiscard]] static inline std::string to_literal(const std::string & value) {
    // Add quotes to the ends and convert each character.
    std::stringstream ss;
    ss << "\"";
    for (char c : value) {
      ss << to_escaped_string(c);
    }
    ss << "\"";
    return ss.str();
  }

  #ifndef DOXYGEN_SHOULD_SKIP_THIS

  /// Take any iterable value and convert it to a C++-style literal.
  template <typename T>
  inline
  typename std::enable_if<emp::IsIterable<T>::value, std::string>::type to_literal(const T & value) {
    std::stringstream ss;
    ss << "{ ";
    for (auto iter = std::begin( value ); iter != std::end( value ); ++iter) {
      if (iter != std::begin( value )) ss << " ";
      ss << emp::to_literal< std::decay_t<decltype(*iter)> >( *iter );
    }
    ss << " }";

    return ss.str();
  }

  #endif  // #ifndef DOXYGEN_SHOULD_SKIP_THIS


  /// Convert a literal character representation to an actual string.
  /// (i.e., 'A', ';', or '\n')
  [[nodiscard]] static inline char from_literal_char(const std::string & value) {
    emp_assert(is_literal_char(value));
    // Given the assert, we can assume the string DOES contain a literal representation,
    // and we just need to convert it.

    if (value.size() == 3) return value[1];
    if (value.size() == 4) {
      switch (value[2]) {
        case 'n': return '\n';   // Newline
        case 'r': return '\r';   // Return
        case 't': return '\t';   // Tab
        case '0': return '\0';   // Empty (character 0)
        case '\\': return '\\';  // Backslash
        case '\'': return '\'';  // Single quote
      }
    }

    // @CAO: Need to add special types of numerical escapes here (e.g., ascii codes!)

    // Problem!
    return '0';
  }


  /// Convert a literal string representation to an actual string.
  [[nodiscard]] static inline std::string from_literal_string(
    const std::string & value,
    [[maybe_unused]] const std::string & quote_marks="\""
  );

  /// Convert a string to all uppercase.
  [[nodiscard]] static inline std::string to_upper(std::string value) {
    constexpr int char_shift = 'a' - 'A';
    for (auto & x : value) {
      if (x >= 'a' && x <= 'z') x = (char) (x - char_shift);
    }
    return value;
  }

  /// Convert a string to all lowercase.
  [[nodiscard]] static inline std::string to_lower(std::string value) {
    constexpr int char_shift = 'a' - 'A';
    for (auto & x : value) {
      if (x >= 'A' && x <= 'Z') x = (char) (x + char_shift);
    }
    return value;
  }

  /// Make first letter of each word upper case
  [[nodiscard]] static inline std::string to_titlecase(std::string value) {
    constexpr int char_shift = 'a' - 'A';
    bool next_upper = true;
    for (size_t i = 0; i < value.size(); i++) {
      if (next_upper && value[i] >= 'a' && value[i] <= 'z') {
        value[i] = (char) (value[i] - char_shift);
      } else if (!next_upper && value[i] >= 'A' && value[i] <= 'Z') {
        value[i] = (char) (value[i] + char_shift);
      }

      next_upper = (value[i] == ' ');
    }
    return value;
  }

  /// Convert an integer to a roman numeral string.
  [[nodiscard]] static inline std::string to_roman_numeral(int val, const std::string & prefix="") {
    std::string ret_string(prefix);
    if (val < 0) ret_string += to_roman_numeral(-val, "-");
    else if (val > 3999) { ; } // Out of bounds; return a blank;
    else if (val >= 1000) ret_string += to_roman_numeral(val - 1000, "M");
    else if (val >= 900) ret_string += to_roman_numeral(val - 900, "CM");
    else if (val >= 500) ret_string += to_roman_numeral(val - 500, "D");
    else if (val >= 400) ret_string += to_roman_numeral(val - 400, "CD");
    else if (val >= 100) ret_string += to_roman_numeral(val - 100, "C");
    else if (val >= 90) ret_string += to_roman_numeral(val - 90, "XC");
    else if (val >= 50) ret_string += to_roman_numeral(val - 50, "L");
    else if (val >= 40) ret_string += to_roman_numeral(val - 40, "XL");
    else if (val >= 10) ret_string += to_roman_numeral(val - 10, "X");
    else if (val == 9) ret_string += "IX";
    else if (val >= 5) ret_string += to_roman_numeral(val - 5, "V");
    else if (val == 4) ret_string += "IV";
    else if (val > 0) ret_string += to_roman_numeral(val - 1, "I");

    // else we already have it exactly and don't need to return anything.
    return ret_string;
  }

  /// Remove whitespace from the beginning or end of a string.
  static inline void trim_whitespace(std::string & in_str) {
    size_t start_count=0;
    while (start_count < in_str.size() && is_whitespace(in_str[start_count])) ++start_count;
    if (start_count) in_str.erase(0, start_count);

    size_t new_size = in_str.size();
    while (new_size > 0 && is_whitespace(in_str[new_size-1])) --new_size;
    in_str.resize(new_size);
  }

  /// Every time one or more whitespace characters appear replace them with a single space.
  static inline void compress_whitespace(std::string & in_string) {
    const size_t strlen = in_string.size();
    bool last_whitespace = true;          // Remove whitespace from beginning of line.
    size_t pos = 0;

    for (size_t i = 0; i < strlen; i++) {
      if (is_whitespace(in_string[i])) {  // This char is whitespace
        if (last_whitespace) continue;
        in_string[pos++] = ' ';
        last_whitespace = true;
      }
      else {  // Not whitespace
        in_string[pos++] = in_string[i];
        last_whitespace = false;
      }
    }

    if (pos && last_whitespace) pos--;   // If the end of the line is whitespace, remove it.

    in_string.resize(pos);
  }

  /// Remove all whitespace from anywhere within a string.
  static inline void remove_whitespace(std::string & in_string) {
    const size_t strlen = in_string.size();
    size_t pos = 0;

    for (size_t i = 0; i < strlen; i++) {
      if (is_whitespace(in_string[i])) continue;
      in_string[pos++] = in_string[i];
    }

    in_string.resize(pos);
  }

  /// Remove all characters from a string except letters, numbers, and whitespace.
  static inline void remove_punctuation(std::string & in_string) {
    const size_t strlen = in_string.size();
    size_t pos = 0;

    for (size_t i = 0; i < strlen; i++) {
      const char cur_char = in_string[i];
      if (is_alphanumeric(cur_char) || is_whitespace(cur_char)) {
        in_string[pos++] = cur_char;
      }
    }

    in_string.resize(pos);
  }

  /// Remove instances of characters from file.
  static inline void remove_chars(std::string & in_string, std::string chars) {
    size_t cur_pos = 0;
    for (size_t i = 0; i < in_string.size(); i++) {
      if (is_one_of(in_string[i], chars)) continue;
      in_string[cur_pos++] = in_string[i];
    }
    in_string.resize(cur_pos);
  }

  /// Make a string safe(r)
  [[nodiscard]] static inline std::string slugify(const std::string & in_string) {
    //TODO handle complicated unicode strings
    std::string res = to_lower(in_string);
    remove_punctuation(res);
    compress_whitespace(res);
    std::transform(res.begin(), res.end(), res.begin(), [](char ch) {
      return (ch == ' ') ? '-' : ch;
    });
    return res;
  }

  // -------- Functions that operate on VECTORS of strings --------

  using string_vec_t = emp::vector<std::string>;

  [[nodiscard]] static inline std::string
  combine_strings(const string_vec_t & strings, std::string spacer=" ") {
    // If there are no input strings, return an empty string.
    if (strings.size() == 0) { return ""; }

    // If there is one string provided, return it by itself.
    if (strings.size() == 1) { return strings[0]; }

    // If there is more than one, separate with spaces.
    std::string out_string = strings[0];
    for (size_t i = 1; i < strings.size(); ++i) {
      out_string += spacer;
      out_string += strings[i];
    }
    return out_string;
  }

 /// Convert a vector of strings to an English list, such as "one, two, three, and four."
  [[nodiscard]] static inline std::string to_english_list(const string_vec_t & strings) {
    // If there are no input strings, return an empty string.
    if (strings.size() == 0) { return ""; }

    // If there is one string provided, return it by itself.
    if (strings.size() == 1) { return strings[0]; }

    // If two strings are provided, link them by an "and".
    if (strings.size() == 2) { return strings[0] + " and " + strings[1]; }

    // If MORE than two strings are provided, list the first n-1 followed by commas, ending
    // with an "and" before the final one.
    std::string out_str;
    for (size_t i = 0; i < strings.size(); i++) {
      if (i) {
        out_str += ", ";
        if (i == strings.size()-1) out_str += "and ";
      }
      out_str += strings[i];
    }

    return out_str;
  }


  /// Transform all strings in a vector.
  [[nodiscard]] static inline string_vec_t
  transform_strings(const string_vec_t & in_strings,
                    std::function<std::string(const std::string &)> fun) {
    string_vec_t out_strings(in_strings.size());
    for (size_t i = 0; i < in_strings.size(); i++) {
      out_strings[i] = fun(in_strings[i]);
    }
    return out_strings;
  }

  /// Put all strings provided in quotes (Like 'this'), pre- and post-fixing another string if
  /// provided.
  [[nodiscard]] static inline string_vec_t
  quote_strings(const string_vec_t & in_strings, const std::string quote="'") {
    return transform_strings(in_strings, [quote](const std::string & str) {
      return quote + str + quote;
    });
  }

  /// Pre-pend and post-pend specified sequences to all strings provided.
  [[nodiscard]] static inline string_vec_t
  quote_strings(const string_vec_t & in_strings,
                const std::string open_quote,
                const std::string close_quote) {
    return transform_strings(in_strings, [open_quote, close_quote](const std::string & str) {
      return open_quote + str + close_quote;
    });
  }

  /// Take a vector of strings, put them in quotes, and then transform it into an English list.
  [[nodiscard]] static inline std::string to_quoted_list(const string_vec_t & in_strings,
                                                         const std::string quote="'") {
    return to_english_list(quote_strings(in_strings, quote));
  }

  static inline bool string_pop_if_char(std::string & in_string, char c)
  {
    if (in_string.size() && in_string[0] == c) {
      in_string.erase(0,1);
      return true;
    }
    return false;
  }

  /// Pop a segment from the beginning of a string as another string, shortening original.
  static inline std::string
  string_pop_fixed(std::string & in_string, std::size_t end_pos, size_t delim_size=0)
  {
    if (end_pos == 0) return "";                   // Not popping anything!

    std::string out_string = "";
    if (end_pos >= in_string.size()) {            // Popping whole string.
      out_string = in_string;
      in_string = "";
    }
    else {
      out_string = in_string.substr(0, end_pos);  // Copy up to the deliminator for ouput
      in_string.erase(0, end_pos + delim_size);   // Delete output string AND deliminator
    }

    return out_string;
  }

  /// Get a segment from the beginning of a string as another string, leaving original untouched.
  [[nodiscard]] static inline std::string
  string_get_range(const std::string & in_string, std::size_t start_pos, std::size_t end_pos) {
    emp_assert(start_pos <= in_string.size());
    if (end_pos == std::string::npos) end_pos = in_string.size();
    emp_assert(end_pos <= in_string.size());
    return in_string.substr(start_pos, end_pos - start_pos);
  }

  /// Remove a prefix of the input string (up to a specified delimeter) and return it.  If the
  /// delimeter is not found, return the entire input string and clear it.
  inline std::string string_pop(std::string & in_string, const char delim=' ') {
    return string_pop_fixed(in_string, in_string.find(delim), 1);
  }

  /// Return a prefix of the input string (up to a specified delimeter), but do not modify it.
  /// If the delimeter is not found, return the entire input string.
  [[nodiscard]] inline std::string
  string_get(const std::string & in_string, const char delim=' ', size_t start_pos=0) {
    return string_get_range(in_string, start_pos, in_string.find(delim, start_pos));
  }

  /// Remove a prefix of the input string (up to any of a specified set of delimeters) and
  /// return it.  If the delimeter is not found, return the entire input string and clear it.
  inline std::string string_pop(std::string & in_string, const std::string & delim_set) {
    return string_pop_fixed(in_string, in_string.find_first_of(delim_set), 1);
  }

  /// Return a prefix of the input string (up to any of a specified set of delimeters), but do not
  /// modify it. If the delimeter is not found, return the entire input string.
  [[nodiscard]] inline std::string
  string_get(const std::string & in_string, const std::string & delim_set, size_t start_pos=0) {
    emp_assert(start_pos <= in_string.size());
    return string_get_range(in_string, start_pos, in_string.find_first_of(delim_set, start_pos));
  }

  inline std::string
  string_pop_to(std::string & in_string, const std::string & delim=" ", size_t start_pos=0,
                bool skip_quotes=false, bool skip_parens=false,
                bool skip_braces=false, bool skip_brackets=false) {
    const size_t found_pos =
      emp::find(in_string, delim, start_pos, skip_quotes, skip_parens, skip_braces, skip_brackets);
    return string_pop_fixed(in_string, found_pos, delim.size());
  }

  [[nodiscard]] inline std::string
  string_get_to(const std::string & in_string, const std::string & delim=" ", size_t start_pos=0) {
    return string_get_range(in_string, start_pos, in_string.find(delim, start_pos));
  }

  /// Remove a prefix of a string, up to the first whitespace, and return it.
  inline std::string string_pop_word(std::string & in_string) {
    // Whitespace = ' ' '\n' '\r' or '\t'
    return string_pop(in_string, " \n\r\t");
  }

  /// Return a prefix of a string, up to the first whitespace (do not modify the original string)
  [[nodiscard]] inline std::string
  string_get_word(const std::string & in_string, size_t start_pos=0) {
    // Whitespace = ' ' '\n' '\r' or '\t'
    return string_get(in_string, " \n\r\t", start_pos);
  }

  /// Remove a prefix of a string, up to the first newline, and return it.
  inline std::string string_pop_line(std::string & in_string) {
    return string_pop(in_string, '\n');
  }

  /// Return a prefix of a string, up to the first newline (do not modify the original string)
  [[nodiscard]] inline std::string
  string_get_line(const std::string & in_string, size_t start_pos=0) {
    return string_get(in_string, '\n', start_pos);
  }

  inline std::string string_pop_quote(std::string & in_string) {
    const size_t end_pos = emp::find_quote_match(in_string);
    return end_pos ? string_pop_fixed(in_string, end_pos+1) : "";
  }

  inline size_t string_uint_size(const std::string & in_string) {
    size_t uint_size = 0;
    for (char c : in_string) {
      if (is_digit(c)) uint_size++;
      else break;
    }
    return uint_size;
  }

  inline unsigned long long string_pop_uint(std::string & in_string) {
    const size_t uint_size = string_uint_size(in_string);
    std::string out_uint = string_pop_fixed(in_string, uint_size);
    return std::stoull(out_uint);
  }

  inline unsigned long long string_get_uint(const std::string & in_string) {
    const size_t uint_size = string_uint_size(in_string);
    std::string out_uint = string_get_range(in_string, 0, uint_size);
    return std::stoull(out_uint);
  }

  /// Remove all whitespace at the beginning of a string.  Return the whitespace removed.
  inline std::string left_justify(std::string & in_string) {
    return string_pop_fixed(in_string, in_string.find_first_not_of(" \n\r\t"));
  }

  /// Remove all whitespace at the end of a string.
  inline void right_justify(std::string & in_string) {
    // @CAO *very* inefficient at the moment.
    while (is_whitespace(in_string.back())) in_string.pop_back();
  }

  /// Remove all whitespace at both the beginning and the end of a string.
  inline void justify(std::string & in_string) {
    left_justify(in_string);
    right_justify(in_string);
  }

  #ifndef DOXYGEN_SHOULD_SKIP_THIS

  /// Apply sprintf-like formatting to a string.
  template<typename... Args>
  std::string format_string( const std::string& format, Args... args ) {
    // See https://en.cppreference.com/w/cpp/io/c/fprintf.
    // Adapted from https://stackoverflow.com/a/26221725.

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-security"

    // Extra space for '\0'
    const size_t size = static_cast<size_t>(std::snprintf(nullptr, 0, format.c_str(), args...) + 1);

    emp::vector<char> buf( size );
    std::snprintf( buf.data(), size, format.c_str(), args... );

     // We don't want the '\0' inside
    return std::string( buf.data(), buf.data() + size - 1 );

    #pragma GCC diagnostic pop

  }

  #endif  // #ifndef DOXYGEN_SHOULD_SKIP_THIS

  /// Find any instances of ${X} and replace with dictionary lookup of X.
  template <typename MAP_T>
  [[nodiscard]] std::string replace_vars( const std::string& in_string, const MAP_T & var_map );

  /// Find any instance of MACRO_NAME(ARGS) and call replace it with fun(ARGS).
  template <typename FUN_T>
  [[nodiscard]] std::string replace_macro( const std::string & str, std::string macro_name,
                                           FUN_T && fun, bool skip_quotes=true );

  /// Provide a string_view on a given string
  static inline std::string_view view_string(const std::string_view & str) {
    return std::string_view(str);
  }

  /// Provide a string_view on a string from a given starting point.
  static inline std::string_view view_string(const std::string_view & str, size_t start) {
    emp_assert(start <= str.size());
    return std::string_view(str.data()+start);
  }

  /// Provide a string_view on a string from a starting point with a given size.
  static inline std::string_view view_string(const std::string_view & str,
                                             size_t start,
                                             size_t npos) {
    emp_assert(start + npos <= str.size());
    return std::string_view(str.data()+start, npos);
  }

  /// Provide a string_view on a string from the beginning to a given size.
  static inline std::string_view view_string_front(const std::string_view & str,
                                                   size_t npos) {
    emp_assert(npos <= str.size());
    return std::string_view(str.data(), npos);
  }

  /// Provide a string_view on a string from a starting point with a given size.
  static inline std::string_view view_string_back(const std::string_view & str,
                                                  size_t npos) {
    emp_assert(npos <= str.size());
    return std::string_view(str.data()+(str.size()-npos), npos);
  }

  /// Provide a string_view on a string from a starting point to an ending point.
  static inline std::string_view view_string_range(const std::string_view & str,
                                                   size_t start,
                                                   size_t end) {
    emp_assert(start <= end);
    emp_assert(end <= str.size());
    return std::string_view(str.data()+start, end - start);
  }

  /// Return a view of the prefix of the input string up to a specified delimeter.
  /// If the delimeter is not found, return the entire input string.
  static inline std::string_view view_string_to(const std::string_view & in_string,
                                                const char delim,
                                                size_t start_pos=0) {
    const size_t in_size = in_string.size();
    size_t end_pos = start_pos;
    while (end_pos < in_size && in_string[end_pos] != delim) end_pos++;
    return view_string_range(in_string, start_pos, end_pos);
  }

  inline std::string pad_front(const std::string & in_str, char padding, size_t target_size) {
    if (in_str.size() >= target_size) return in_str;
    const size_t pad_size = target_size - in_str.size();
    return std::string(pad_size, padding) + in_str;
  }

  inline std::string pad_back(const std::string & in_str, char padding, size_t target_size) {
    if (in_str.size() >= target_size) return in_str;
    const size_t pad_size = target_size - in_str.size();
    return in_str + std::string(pad_size, padding);
  }

  /// Concatenate n copies of a string.
  inline std::string repeat( const std::string& value, const size_t n ) {
    const emp::vector<std::string> repeated( n, value );
    return std::accumulate(
      std::begin(repeated), std::end(repeated), std::string{}
    );
  }

  /// Cut up a string based on the provided delimiter; fill them in to the provided vector.
  /// @param in_string string to be sliced
  /// @param out_set destination
  /// @param delim delimiter to split on
  /// @param max_split defines the maximum number of splits
  /// @param keep_quotes Should quoted text be kept together?
  /// @param keep_parens Should parentheses ('(' and ')') be kept together?
  /// @param keep_braces Should braces ('{' and '}') be kept together?
  /// @param keep_brackets Should brackets ('[' and ']') be kept together?
  static inline void slice(
    const std::string_view & in_string,
    emp::vector<std::string> & out_set,
    const char delim='\n',
    const size_t max_split=std::numeric_limits<size_t>::max(),
    const bool keep_quotes=false,
    const bool keep_parens=false,
    const bool keep_braces=false,
    const bool keep_brackets=false
  );

  /// Slice a string without passing in result vector (may be less efficient).
  /// @param in_string string to be sliced
  /// @param delim delimiter to split on
  /// @param max_split defines the maximum number of splits
  /// @param keep_quotes Should quoted text be kept together?
  /// @param keep_parens Should parentheses ('(' and ')') be kept together?
  /// @param keep_braces Should braces ('{' and '}') be kept together?
  /// @param keep_brackets Should brackets ('[' and ']') be kept together?
  static inline emp::vector<std::string> slice(
    const std::string_view & in_string,
    const char delim='\n',
    const size_t max_split=std::numeric_limits<size_t>::max(),
    const bool keep_quotes=false,
    const bool keep_parens=false,
    const bool keep_braces=false,
    const bool keep_brackets=false
  ) {
    emp::vector<std::string> result;
    slice(in_string, result, delim, max_split, keep_quotes, keep_parens, keep_braces, keep_brackets);
    return result;
  }

  /// Create a set of string_views based on the provided delimiter; fill them in to the provided vector.
  /// @param in_string string to be sliced
  /// @param out_set destination vector
  /// @param delim delimiter to split on
  /// @param keep_quotes Should quoted text be kept together?
  /// @param keep_parens Should parentheses ('(' and ')') be kept together?
  /// @param keep_braces Should braces ('{' and '}') be kept together?
  /// @param keep_brackets Should brackets ('[' and ']') be kept together?
  static inline void view_slices(
    const std::string_view & in_string,
    emp::vector<std::string_view> & out_set,
    char delim='\n',
    const bool keep_quotes=false,
    const bool keep_parens=false,
    const bool keep_braces=false,
    const bool keep_brackets=false
  ) {
    out_set.resize(0);
    size_t start_pos = 0;
    for (size_t pos=0; pos < in_string.size(); pos++) {
      if (keep_quotes && (in_string[pos] == '"' || in_string[pos] == '\'')) {
        pos = find_quote_match(in_string, pos, in_string[pos]);
      }
      else if (keep_parens && in_string[pos] == '(') {
        pos = find_paren_match(in_string, pos, '(', ')', keep_quotes);
      }
      else if (keep_braces && in_string[pos] == '{') {
        pos = find_paren_match(in_string, pos, '{', '}', keep_quotes);
      }
      else if (keep_brackets && in_string[pos] == '[') {
        pos = find_paren_match(in_string, pos, '[', ']', keep_quotes);
      }
      else if (in_string[pos] == delim) {  // Hit an end point!
        out_set.push_back( view_string_range(in_string, start_pos, pos) );
        start_pos = pos+1;
      }
    }

    // Include the final segment.
    out_set.push_back( view_string_range(in_string, start_pos, in_string.size()) );
  }

  /// Slice a string without passing in result vector (may be less efficient).
  static inline emp::vector<std::string_view> view_slices(
    const std::string_view & in_string,
    char delim='\n',
    const bool keep_quotes=false,
    const bool keep_parens=false,
    const bool keep_braces=false,
    const bool keep_brackets=false
  ) {
    emp::vector<std::string_view> result;
    view_slices(in_string, result, delim, keep_quotes, keep_parens, keep_braces, keep_brackets);
    return result;
  }

  /// Slice a string without passing in result vector (may be less efficient).
  /// @param in_string string to be sliced
  /// @param delim delimiter to split on (default ',')
  /// @param assign separator for left and right side of assignment (default: "=")
  /// @param max_split defines the maximum number of splits (default, no max)
  /// @param keep_quotes Should quoted text be kept together? (default: no)
  /// @param trim_whitespace Should extra whitespace around delim or assign be ignored?
  static inline std::map<std::string,std::string> slice_assign(
    const std::string_view & in_string,
    const char delim=',',
    std::string assign_op="=",
    const size_t max_split=std::numeric_limits<size_t>::max(),
    const bool trim_whitespace=true,
    const bool keep_quotes=true,
    const bool keep_parens=true,
    const bool keep_braces=true,
    const bool keep_brackets=true
  ) {
    auto assign_set = emp::slice(in_string, delim, max_split, keep_quotes, keep_parens, keep_braces, keep_brackets);
    std::map<std::string,std::string> result_map;
    for (auto setting : assign_set) {
      // Skip blank settings (especially at the end).
      if (emp::is_whitespace(setting)) continue;

      // Remove any extra spaces around parsed values.
      std::string var_name = emp::string_pop_to(setting, assign_op);
      if (trim_whitespace) {
        emp::trim_whitespace(var_name);
        emp::trim_whitespace(setting);
      }
      if (setting.size() == 0) {
        std::stringstream msg;
        msg << "No assignment found in slice_assign(): " << in_string;
        abort();
        emp::notify::Exception("emp::string_utils::slice_assign::missing_assign",
                               msg.str(), setting);
      }
      result_map[var_name] = setting;
    }
    return result_map;
  }

  static inline emp::vector<std::string_view> ViewCSV( const std::string_view & in_string ) {
    return view_slices(in_string, ',', true);
  }

  /// View a section of a string with the properly matching nested blocks.
  /// For example if ((abc(de))f(ghi)) would return "(abc(de))f(ghi)" at 0, "de" at 5, or
  /// "" at 2 (since there is no start!)
  static inline std::string_view ViewNestedBlock(std::string_view str,
                                                 const std::string symbols="()",
                                                 size_t start=0) {
    // Test if we are not starting at the beginning of a block, return empty.
    if (str[start] != symbols[0]) return emp::view_string(str, 0, 0);

    size_t depth = 0;
    size_t stop = start;
    while (++stop < str.size()) {
      if (str[stop] == symbols[0]) depth++;
      else if (str[stop] == symbols[1]) {
        if (depth == 0) break;
        depth--;
      }
    }

    return emp::view_string(str, start+1, stop-start-1);
  }

  // The next functions are not efficient, but they will take any number of inputs and
  // dynamically convert them all into a single, concatenated string.

  /// Setup emp::ToString declarations for built-in types.
  template <typename T, size_t N> inline std::string ToString(const emp::array<T,N> & container);
  template <typename T, typename... Ts>
  inline std::string ToString(const emp::vector<T, Ts...> & container);

  #ifndef DOXYGEN_SHOULD_SKIP_THIS

  namespace internal {
    // If the item passed in has a ToString(), always use it.
    template <typename T>
    decltype(std::declval<T>().ToString()) to_stream_item(const T & in, bool) {
      return in.ToString();
    }

    // Otherwise, if emp::ToString(x) is defined for x, use it.
    template <typename T>
    auto to_stream_item(const T & in, int) -> decltype(emp::ToString(in)) {
      return emp::ToString(in);
    }

    // If neither works, just assume stream operator will handle things...
    // @CAO: Technically we can detect this to give a more informative error...
    template <typename T> const T & to_stream_item(const T & in, ...) { return in; }

  }

  #endif // #ifndef DOXYGEN_SHOULD_SKIP_THIS


  /// This function does its best to convert any type to a string. Accepts any number of
  /// arguments and returns a single concatenated string. Conversions attempted for an
  /// object 'x' include (in order):
  /// - Call a x.ToString()
  /// - Call appropriate emp::ToString(x) overload
  /// - Pass x through stringstream
  template <typename... Ts>
  inline std::string to_string(const Ts &... values) {
    std::stringstream ss;
    (ss << ... << internal::to_stream_item(values, true));
    return ss.str();
  }

  /// Overload of to_string() string arguments to be directly returned.
  inline const std::string & to_string(const std::string & value) {
    return value;
  }

  /// Setup emp::ToString to work on arrays.
  template <typename T, size_t N>
  inline std::string ToString(const emp::array<T,N> & container) {
    std::stringstream ss;
    ss << "[ ";
    for (const auto & el : container) {
      ss << to_string(el);
      ss << " ";
    }
    ss << "]";
    return ss.str();
  }

  /// Setup emp::ToString to work on vectors.
  template <typename T, typename... Ts>
  inline std::string ToString(const emp::vector<T, Ts...> & container) {
    std::stringstream ss;
    ss << "[ ";
    for (const auto & el : container) {
      ss << to_string(el);
      ss << " ";
    }
    ss << "]";
    return ss.str();
  }

  /// This function tries to convert a string into any type you're looking for...  You just
  /// need to specify the out type as the template argument.
  template <typename T>
  inline T from_string(const std::string & str) {
    std::stringstream ss;
    ss << str;
    T out_val;
    ss >> out_val;
    return out_val;
  }

  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {
    static inline void _from_string(std::stringstream &) { ; }

    template <typename T, typename... Ts>
    void _from_string(std::stringstream & ss, T & arg1, Ts... extra_args) {
      ss >> arg1;
      _from_string(ss, extra_args...);
    }
  }
  #endif // DOXYGEN_SHOULD_SKIP_THIS

  /// The from_string() function can also take multiple args instead of a return.
  template <typename... Ts>
  inline void from_string(const std::string & str, Ts &... args) {
    std::stringstream ss;
    ss << str;
    internal::_from_string(ss, args...);
  }

  /// The from_strings() function takes a vector of strings and converts them into a vector
  /// of the appropriate type.
  template <typename T>
  inline emp::vector<T> from_strings(const emp::vector<std::string> & string_v) {
    emp::vector<T> vals(string_v.size());
    for (size_t i = 0; i < string_v.size(); i++) {
      vals[i] = from_string<T>(string_v[i]);
    }
    return vals;
  }

  /// This function tries to convert a string_view into any other type...  You must
  /// need to specify the out type as the template argument.
  template <typename T>
  inline T from_string(std::string_view str) {
    std::stringstream ss;
    ss << str;
    T out_val;
    ss >> out_val;
    return out_val;
  }

  /// This function returns values from a container as a single string separated
  /// by a given delimeter.
  /// @param container is any standard-interface container holding objects to be joined.
  /// @param join_str optional delimeter
  /// @return merged string of all values
  template <typename CONTAINER_T>
  inline std::string join(const CONTAINER_T & container, std::string join_str="") {
    if (container.size() == 0) return "";
    if (container.size() == 1) return to_string(container.front());

    std::stringstream out;
    for (auto it = container.begin(); it != container.end(); ++it) {
      if (it != container.begin()) out << join_str;
      out << to_string(*it);
    }

    return out.str();
  }


  // Some ANSI helper functions.
  inline constexpr char ANSI_ESC() { return (char) 27; }
  inline std::string ANSI_Reset() { return "\033[0m"; }
  inline std::string ANSI_Bold() { return "\033[1m"; }
  inline std::string ANSI_Faint() { return "\033[2m"; }
  inline std::string ANSI_Italic() { return "\033[3m"; }
  inline std::string ANSI_Underline() { return "\033[4m"; }
  inline std::string ANSI_SlowBlink() { return "\033[5m"; }
  inline std::string ANSI_Blink() { return "\033[6m"; }
  inline std::string ANSI_Reverse() { return "\033[7m"; }
  inline std::string ANSI_Strike() { return "\033[9m"; }

  inline std::string ANSI_NoBold() { return "\033[22m"; }
  inline std::string ANSI_NoItalic() { return "\033[23m"; }
  inline std::string ANSI_NoUnderline() { return "\033[24m"; }
  inline std::string ANSI_NoBlink() { return "\033[25m"; }
  inline std::string ANSI_NoReverse() { return "\033[27m"; }

  inline std::string ANSI_Black() { return "\033[30m"; }
  inline std::string ANSI_Red() { return "\033[31m"; }
  inline std::string ANSI_Green() { return "\033[32m"; }
  inline std::string ANSI_Yellow() { return "\033[33m"; }
  inline std::string ANSI_Blue() { return "\033[34m"; }
  inline std::string ANSI_Magenta() { return "\033[35m"; }
  inline std::string ANSI_Cyan() { return "\033[36m"; }
  inline std::string ANSI_White() { return "\033[37m"; }
  inline std::string ANSI_DefaultColor() { return "\033[39m"; }

  inline std::string ANSI_BlackBG() { return "\033[40m"; }
  inline std::string ANSI_RedBG() { return "\033[41m"; }
  inline std::string ANSI_GreenBG() { return "\033[42m"; }
  inline std::string ANSI_YellowBG() { return "\033[43m"; }
  inline std::string ANSI_BlueBG() { return "\033[44m"; }
  inline std::string ANSI_MagentaBG() { return "\033[45m"; }
  inline std::string ANSI_CyanBG() { return "\033[46m"; }
  inline std::string ANSI_WhiteBG() { return "\033[47m"; }
  inline std::string ANSI_DefaultBGColor() { return "\033[49m"; }

  inline std::string ANSI_BrightBlack() { return "\033[30m"; }
  inline std::string ANSI_BrightRed() { return "\033[31m"; }
  inline std::string ANSI_BrightGreen() { return "\033[32m"; }
  inline std::string ANSI_BrightYellow() { return "\033[33m"; }
  inline std::string ANSI_BrightBlue() { return "\033[34m"; }
  inline std::string ANSI_BrightMagenta() { return "\033[35m"; }
  inline std::string ANSI_BrightCyan() { return "\033[36m"; }
  inline std::string ANSI_BrightWhite() { return "\033[37m"; }

  inline std::string ANSI_BrightBlackBG() { return "\033[40m"; }
  inline std::string ANSI_BrightRedBG() { return "\033[41m"; }
  inline std::string ANSI_BrightGreenBG() { return "\033[42m"; }
  inline std::string ANSI_BrightYellowBG() { return "\033[43m"; }
  inline std::string ANSI_BrightBlueBG() { return "\033[44m"; }
  inline std::string ANSI_BrightMagentaBG() { return "\033[45m"; }
  inline std::string ANSI_BrightCyanBG() { return "\033[46m"; }
  inline std::string ANSI_BrightWhiteBG() { return "\033[47m"; }

  /// Make a string appear bold when printed to the command line.
  inline std::string to_ansi_bold(const std::string & _in) {
    return ANSI_Bold() + _in + ANSI_NoBold();
  }

  /// Make a string appear italics when printed to the command line.
  inline std::string to_ansi_italic(const std::string & _in) {
    return ANSI_Italic() + _in + ANSI_NoItalic();
  }

  /// Make a string appear underline when printed to the command line.
  inline std::string to_ansi_underline(const std::string & _in) {
    return ANSI_Underline() + _in + ANSI_NoUnderline();
  }

  /// Make a string appear blink when printed to the command line.
  inline std::string to_ansi_blink(const std::string & _in) {
    return ANSI_Blink() + _in + ANSI_NoBlink();
  }

  /// Make a string appear reverse when printed to the command line.
  inline std::string to_ansi_reverse(const std::string & _in) {
    return ANSI_Reverse() + _in + ANSI_NoReverse();
  }

  //////////////////////////////////////////////////////
  //  Implementations of larger functions (>25 lines)
  //////////////////////////////////////////////////////

  /// Test if an input string is properly formatted as a literal character.
  static inline bool is_literal_char(const std::string & value) {
    // A literal char must begin with a single quote, contain a representation of a single
    // character, and end with a single quote.
    if (value.size() < 3) return false;
    if (value[0] != '\'' || value.back() != '\'') return false;

    // If there's only a single character in the quotes, it's USUALLY legal.
    if (value.size() == 3) {
      switch (value[1]) {
        case '\'':         // Can't be a single quote (must be escaped!)
        case '\\':         // Can't be a backslash (must be followed by something!)
          return false;
        default:
          return true;
      }
    }

    // If there are more characters, must be an escape sequence.
    if (value.size() == 4) {
      if (value[1] != '\\') return false;

      // Identify legal escape sequences.
      // @CAO Need more here!
      switch (value[2]) {
        case 'n':   // Newline
        case 'r':   // Return
        case 't':   // Tab
        case '0':   // Empty (character 0)
        case '\\':  // Backslash
        case '\'':  // Single quote
          return true;
        default:
          return false;
      }
    }

    // @CAO: Need to add special types of numerical escapes here (e.g., ascii codes!)

    // If we made it here without a problem, it must be correct!
    return true;
  }


  /// Test if an input string is properly formatted as a literal string.
  static inline bool is_literal_string(const std::string & value,
                                       const std::string & quote_marks) {
    if (value.size() < 2) return false;               // Two short to contain even quote marks!
    char quote = value[0];
    if (!is_one_of(quote, quote_marks)) return false; // Must be working with allowed quote mark.
    if (value.back() != quote) return false;          // Must use same quote at front and back.

    // Are all of the characters valid?
    for (size_t pos = 1; pos < value.size() - 1; pos++) {
      if (value[pos] == quote) return false;          // Cannot have a raw quote in the middle.
      if (value[pos] == '\\') {                       // Allow escaped characters...
        if (pos == value.size()-2) return false;      // Backslash must have char to escape.

        // Move to the next char and make sure it is legal to be escaped.
        // @CAO Expand on options!
        pos++;
        switch (value[pos]) {
          case 'b':   // Backspace
          case 'f':   // Form feed
          case 'n':   // Newline
          case 'r':   // Return
          case 't':   // Tab
          case 'v':   // Vertical tab.
          case '0':   // Empty (character 0)
          case '\\':  // Backslash
          case '"':   // Double quote
          case '\'':  // Single quote
          case '`':   // Back quote
            continue;
          default:
            return false;
        }
      }
    }

    // @CAO: Need to check special types of numerical escapes (e.g., ascii codes!)

    // If we made it here without a problem, it must be correct!
    return true;
  }


  /// Test if an input string is properly formatted as a literal string.
  static inline std::string diagnose_literal_string(const std::string & value,
                                                    const std::string & quote_marks) {
    // A literal string must begin and end with a double quote and contain only valid characters.
    if (value.size() < 2) return "Too short!";
    char quote = value[0];
    if (!is_one_of(quote, quote_marks)) return "Must begin an end in quotes.";
    if (value.back() != quote) return "Begin and end quotes must match.";

    // Are all of the characters valid?
    for (size_t pos = 1; pos < value.size() - 1; pos++) {
      if (value[pos] == quote) return "Has a floating quote.";
      if (value[pos] == '\\') {
        if (pos == value.size()-2) return "Cannot escape the final quote.";  // Backslash must have char to escape.

        // Move to the next char and make sure it's legal to be escaped.
        // @CAO Expand on options!
        pos++;
        switch (value[pos]) {
          case 'b':   // Backspace
          case 'f':   // Form feed
          case 'n':   // Newline
          case 'r':   // Return
          case 't':   // Tab
          case 'v':   // Vertical tab.
          case '0':   // Empty (character 0)
          case '\\':  // Backslash
          case '"':   // Double quote
          case '\'':  // Single quote
          case '`':   // Back quote
            continue;
          default:
            return "Unknown escape charater.";
        }
      }
    }

    // @CAO: Need to check special types of numerical escapes (e.g., ascii codes!)

    // If we made it here without a problem, it must be correct!
    return "Good!";
  }


  /// Convert a single character to one that uses a proper escape sequence (in a string) if needed.
  static inline std::string to_escaped_string(char value) {
    // Start by quickly returning a string if it's easy.
    std::stringstream ss;
    if ( (value >= 40 && value < 91) || (value > 96 && value < 127)) {
      ss << value;
      return ss.str();
    }
    switch (value) {
    case '\0': return "\\0";
    case 1: return "\\001";
    case 2: return "\\002";
    case 3: return "\\003";
    case 4: return "\\004";
    case 5: return "\\005";
    case 6: return "\\006";
    case '\a': return "\\a";  // case  7 (audible bell)
    case '\b': return "\\b";  // case  8 (backspace)
    case '\t': return "\\t";  // case  9 (tab)
    case '\n': return "\\n";  // case 10 (newline)
    case '\v': return "\\v";  // case 11 (vertical tab)
    case '\f': return "\\f";  // case 12 (form feed - new page)
    case '\r': return "\\r";  // case 13 (carriage return)
    case 14: return "\\016";
    case 15: return "\\017";
    case 16: return "\\020";
    case 17: return "\\021";
    case 18: return "\\022";
    case 19: return "\\023";
    case 20: return "\\024";
    case 21: return "\\025";
    case 22: return "\\026";
    case 23: return "\\027";
    case 24: return "\\030";
    case 25: return "\\031";
    case 26: return "\\032";
    case 27: return "\\033";  // case 27 (ESC), sometimes \e
    case 28: return "\\034";
    case 29: return "\\035";
    case 30: return "\\036";
    case 31: return "\\037";

    case '\"': return "\\\"";  // case 34
    case '\'': return "\\\'";  // case 39
    case '\\': return "\\\\";  // case 92
    case 127: return "\\177";  // (delete)

    // case '\?': return "\\\?";
    default:
      ss << value;
      return ss.str();
    };
  }


  /// Convert a literal string representation to an actual string.
  static inline std::string from_literal_string(
    const std::string & value,
    [[maybe_unused]] const std::string & quote_marks)
  {
    emp_assert(is_literal_string(value, quote_marks),
               value, diagnose_literal_string(value, quote_marks));
    // Given the assert, assume string DOES contain a literal string representation.

    std::string out_string;
    out_string.reserve(value.size()-2);  // Make a guess on final size.

    for (size_t pos = 1; pos < value.size() - 1; pos++) {
      // If we don't have an escaped character, just move it over.
      if (value[pos] != '\\') {
        out_string.push_back(value[pos]);
        continue;
      }

      // If we do have an escape character, convert it.
      pos++;

      switch (value[pos]) {
        case 'b': out_string.push_back('\b'); break;   // Backspace
        case 'f': out_string.push_back('\f'); break;   // Form feed
        case 'n': out_string.push_back('\n'); break;   // Newline
        case 'r': out_string.push_back('\r'); break;   // Return
        case 't': out_string.push_back('\t'); break;   // Tab
        case 'v': out_string.push_back('\v'); break;   // Vertical tab
        case '0': out_string.push_back('\0'); break;   // Empty (character 0)
        case '\\': out_string.push_back('\\'); break;  // Backslash
        case '"': out_string.push_back('"'); break;    // Double quote
        case '\'': out_string.push_back('\''); break;  // Single quote
        default:
          emp_assert(false, "unknown escape char used; probably need to update converter!");
      }
    }

    return out_string;
  }


  /// Find any instances of ${X} and replace with dictionary lookup of X.
  template <typename MAP_T>
  [[nodiscard]] std::string replace_vars( const std::string& in_string, const MAP_T & var_map ) {
    std::string result = in_string;

    // Seek out instances of "${" to indicate the start of pre-processing.
    for (size_t i = 0; i < result.size(); ++i) {
      if (result[i] != '$') continue;   // Replacement tag must start with a '$'.
      if (result.size() <= i+2) break;  // Not enough room for a replacement tag.
      if (result[i+1] == '$') {         // Compress two $$ into one $
        result.erase(i,1);
        continue;
      }
      if (result[i+1] != '{') continue; // Eval must be surrounded by braces.

      // If we made it this far, we have a starting match!
      size_t end_pos = emp::find_paren_match(result, i+1, '{', '}', false);
      if (end_pos == i+1) {
        emp::notify::Exception("emp::string_utils::replace_vars::missing_close",
                               "No close brace found in string_utils::replace_vars()",
                               result);
        return result; // Stop where we are... No end brace found!
      }

      std::string key = result.substr(i+2, end_pos-i-2);
      auto replacement_it = var_map.find(key);
      if (replacement_it == var_map.end()) {
        emp::notify::Exception("emp::string_utils::replace_vars::missing_var",
                               emp::to_string("Lookup variable not found in var_map (key=", key, ")"),
                               key);
        return result; // Stop here; variable could not be replaced.
      }
      result.replace(i, end_pos-i+1, replacement_it->second);   // Put into place.
      i += replacement_it->second.size();                       // Continue at the next position...
    }

    return result;
  }

  /// @brief Find any instance of MACRO_NAME(ARGS) and replace it with fun(ARGS).
  /// @param in_string String to perform macro replacement.
  /// @param macro_name Name of the macro to look for.
  /// @param macro_fun Function to call with contents of macro.  Params are macro_args (string), line_num (size_t), and hit_num (size_t)
  /// @param skip_quotes Should we skip quotes when looking for macro?
  /// @return Processed version of in_string with macros replaced.
  template <typename FUN_T>
  [[nodiscard]] std::string replace_macro(
    const std::string & in_string,
    std::string macro_name,
    FUN_T && macro_fun,
    bool skip_quotes
  ) {
    std::stringstream out;

    // We need to identify the comparator and divide up arguments in macro.
    size_t macro_count = 0;     // Count of the number of hits for this macro.
    size_t line_num = 0;        // Line number where current macro hit was found.
    size_t macro_end = 0;
    for (size_t macro_pos = emp::find_id(in_string, macro_name, 0, skip_quotes);
         macro_pos != std::string::npos;
         macro_pos = emp::find_id(in_string, macro_name, macro_end, skip_quotes))
    {
      // Output everything from the end of the last macro hit to the beginning of this one.
      std::string code_segment = in_string.substr(macro_end, macro_pos-macro_end);
      line_num += emp::count(code_segment, '\n');
      out << code_segment;

      // Make sure this macro is okay.
      macro_pos = emp::find_non_whitespace(in_string, macro_pos+macro_name.size());
      if (in_string[macro_pos] != '(') {
        emp::notify::Warning("Line ", line_num, ": Invalid MACRO instance of '", macro_name,
          "' - found ", in_string[macro_pos], "instead of '('.");
        macro_end = macro_pos;
        continue;
      }

      // Isolate this macro instance and call the conversion function.
      macro_end = emp::find_paren_match(in_string, macro_pos);
      const std::string macro_body = in_string.substr(macro_pos+1, macro_end-macro_pos-1);
      macro_end += 2;  // Advance the end past the ");" at the end of the macro.

      out << macro_fun(macro_body, line_num, macro_count);

      // Find the next macro instance and loop starting from the end of this one.
      macro_count++;
    }

    // Grab the rest of the in_string and output the processed string.
    out << in_string.substr(macro_end);
    return out.str();
  }

  /// Advance a position in a string, respecting quotes, parens, braces, and brackets as indicated
  /// @param in_string string being stepped through
  /// @param pos Position to advance from
  /// @param keep_quotes Should quoted text be treated as a unit?
  /// @param keep_parens Should parens be treated as a unit?
  /// @param keep_braces Should braces be treated as a unit?
  /// @param keep_brackets Should brackets be treated as a unit?
  static inline size_t next_pos(
    std::string_view in_string, size_t pos,
    bool keep_quotes=false, bool keep_parens=false,
    bool keep_braces=false, bool keep_brackets=false)
  {
    // See if we need to skip a whole segment
    const char c = in_string[pos];
    switch (c) {
    case '"': case '\'':
      if (keep_quotes) pos = find_quote_match(in_string, pos, c);
      break;
    case '(':
      if (keep_parens) pos = find_paren_match(in_string, pos, c, ')', keep_quotes);
      break;
    case '[':
      if (keep_brackets) pos = find_paren_match(in_string, pos, c, ']', keep_quotes);
      break;
    case '{':
      if (keep_braces) pos = find_paren_match(in_string, pos, c, '}', keep_quotes);
      break;
    }

    return pos + 1;
  }

  /// Cut up a string based on the provided delimiter; fill them in to the provided vector.
  /// @param in_string string to be sliced
  /// @param out_set destination
  /// @param delim delimiter to split on
  /// @param max_split defines the maximum number of splits
  /// @param keep_quotes Should quoted text be treated as a unit?
  /// @param keep_parens Should parens be treated as a unit?
  /// @param keep_braces Should braces be treated as a unit?
  /// @param keep_brackets Should brackets be treated as a unit?
  static inline void slice(
    const std::string_view & in_string,
    emp::vector<std::string> & out_set,
    const char delim,
    const size_t max_split,
    const bool keep_quotes,
    const bool keep_parens,
    const bool keep_braces,
    const bool keep_brackets
  ) {
    out_set.resize(0);
    if (in_string.empty() || max_split == 0) return; // Nothing to set!

    size_t pos = 0;           // Position currently being scanned.
    size_t start_pos = 0;     // Start position for the next segment.
    while (pos < in_string.size() && out_set.size()+1 < max_split) {
      // Find the end of the current segment.
      while (pos < in_string.size() && in_string[pos] != delim) {
        pos = next_pos(in_string, pos, keep_quotes, keep_parens, keep_braces, keep_brackets);
      }

      // Record the current segment.
      out_set.emplace_back( in_string.substr(start_pos, pos-start_pos) );

      start_pos = ++pos;  // Skip deliminator and record next start.
    }

    // If there are any segments left, put them all at the end.
    if (start_pos < in_string.size()) {
      out_set.emplace_back( in_string.substr(start_pos, in_string.size()-start_pos) );
    }
  }
}

#endif // #ifndef EMP_TOOLS_STRING_UTILS_HPP_INCLUDE
