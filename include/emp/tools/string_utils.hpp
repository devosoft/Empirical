/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2021.
 *
 *  @file string_utils.hpp
 *  @brief Simple functions to manipulate strings.
 *  @note Status: RELEASE
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
#include <memory>
#include <numeric>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>

#include "../base/array.hpp"
#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../meta/reflection.hpp"
#include "../meta/type_traits.hpp"

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

  /// Convert a single chararcter to one that uses a proper escape sequence (in a string) if needed.
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

  /// Convert a full string to one that uses proper escape sequences, as needed.
  static inline std::string to_escaped_string(const std::string & value) {
    std::stringstream ss;
    for (char c : value) { ss << to_escaped_string(c); }
    return ss.str();
  }

  /// Take a string and replace reserved HTML characters with character entities
  inline std::string to_web_safe_string(const std::string & value) {
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
  std::string url_encode(const std::string &value) {
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
  std::string url_decode(const std::string& str){
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
  static inline std::string to_literal(char value) {
    std::stringstream ss;
    ss << "'" << to_escaped_string(value) << "'";
    return ss.str();
  }

  /// Take a string or iterable and convert it to a C++-style literal.
  // This is the version for string. The version for an iterable is below.
  static inline std::string to_literal(const std::string & value) {
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

  #endif

  /// Test if an input string is properly formated as a literal character.
  static inline char is_literal_char(const std::string & value) {
    // A literal char must beging with a single quote, contain a representation of a single
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

  /// Convert a literal character representation to an actual string.
  /// (i.e., 'A', ';', or '\n')
  static inline char from_literal_char(const std::string & value) {
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

  /// Test if an input string is properly formated as a literal string.
  static inline char is_literal_string(const std::string & value) {
    // A literal string must begin and end with a double quote and contain only valid characters.
    if (value.size() < 2) return false;
    if (value[0] != '"' || value.back() != '"') return false;

    // Are all of the characters valid?
    for (size_t pos = 1; pos < value.size() - 1; pos++) {
      if (value[pos] == '"') return false;  // Cannot have a raw double-quote in the middle.
      if (value[pos] == '\\') {
        if (pos == value.size()-2) return false;  // Backslash must have char to escape.

        // Move to the next char and make sure it's legal to be escaped.
        // @CAO Expand on options!
        pos++;
        switch (value[pos]) {
          case 'n':   // Newline
          case 'r':   // Return
          case 't':   // Tab
          case '0':   // Empty (character 0)
          case '\\':  // Backslash
          case '\'':  // Single quote
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

  /// Concatenate n copies of a string.
  inline std::string repeat( const std::string& value, const size_t n ) {
    const emp::vector<std::string> repeated( n, value );
    return std::accumulate(
      std::begin(repeated), std::end(repeated), std::string{}
    );
  }


  /// Convert a literal string representation to an actual string.
  static inline std::string from_literal_string(const std::string & value) {
    emp_assert(is_literal_string(value));
    // Given the assert, we can assume the string DOES contain a literal representation,
    // and we just need to convert it.

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
        case 'n': out_string.push_back('\n'); break;   // Newline
        case 'r': out_string.push_back('\r'); break;   // Return
        case 't': out_string.push_back('\t'); break;   // Tab
        case '0': out_string.push_back('\0'); break;   // Empty (character 0)
        case '\\': out_string.push_back('\\'); break;  // Backslash
        case '\'': out_string.push_back('\''); break;  // Single quote
        default:
          emp_assert(false, "unknown escape char used; probably need to update converter!");
      }
    }

    return out_string;
  }


  /// Convert a string to all uppercase.
  static inline std::string to_upper(std::string value) {
    constexpr int char_shift = 'a' - 'A';
    for (auto & x : value) {
      if (x >= 'a' && x <= 'z') x = (char) (x - char_shift);
    }
    return value;
  }

  /// Convert a string to all lowercase.
  static inline std::string to_lower(std::string value) {
    constexpr int char_shift = 'a' - 'A';
    for (auto & x : value) {
      if (x >= 'A' && x <= 'Z') x = (char) (x + char_shift);
    }
    return value;
  }

  /// Make first letter of each word upper case
  static inline std::string to_titlecase(std::string value) {
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
  static inline std::string to_roman_numeral(int val, const std::string & prefix="") {
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


  /// Determine if a character is whitespace.
  inline bool is_whitespace(char test_char) {
    return (test_char == ' ' || test_char == '\n' || test_char == '\r' || test_char == '\t');
  }

  /// Determine if a character is an uppercase letter.
  inline bool is_upper_letter(char test_char) {
    return (test_char >= 'A' && test_char <= 'Z');
  }

  /// Determine if a character is a lowercase letter.
  inline bool is_lower_letter(char test_char) {
    return (test_char >= 'a' && test_char <= 'z');
  }

  /// Determine if a character is a letter of any kind.
  inline bool is_letter(char test_char) {
    return is_upper_letter(test_char) || is_lower_letter(test_char);
  }

  /// Determine if a character is a digit.
  inline bool is_digit(char test_char) {
    return (test_char >= '0' && test_char <= '9');
  }

  /// Determine if a character is a letter or digit.
  inline bool is_alphanumeric(char test_char) {
    return is_letter(test_char) || is_digit(test_char);
  }

  /// Determine if a character is a letter, digit, or underscore.
  inline bool is_idchar(char test_char) {
    return is_alphanumeric(test_char) || test_char == '_';
  }

  /// Determine if a character is in a set of characters (represented as a string)
  static inline bool is_one_of(char test_char, const std::string & char_set) {
    for (char x : char_set) if (test_char == x) return true;
    return false;
  }

  /// Determine if a string is composed only of a set of characters (represented as a string)
  static inline bool is_composed_of(const std::string & test_str, const std::string & char_set) {
    for (char x : test_str) if (!is_one_of(x, char_set)) return false;
    return true;
  }

  /// Determine if there is whitespace anywhere in a string.
  inline bool has_whitespace(const std::string & test_str) {
    for (char c : test_str) if (is_whitespace(c)) return true;
    return false;
  }

  /// Determine if there are any uppercase letters in a string.
  inline bool has_upper_letter(const std::string & test_str) {
    for (char c : test_str) if (is_upper_letter(c)) return true;
    return false;
  }

  /// Determine if there are any lowercase letters in a string.
  inline bool has_lower_letter(const std::string & test_str) {
    for (char c : test_str) if (is_lower_letter(c)) return true;
    return false;
  }

  /// Determine if there are any letters in a string.
  inline bool has_letter(const std::string & test_str) {
    for (char c : test_str) if (is_letter(c)) return true;
    return false;
  }

  /// Determine if there are any digits in a string.
  inline bool has_digit(const std::string & test_str) {
    for (char c : test_str) if (is_digit(c)) return true;
    return false;
  }

  /// Determine if there are only digits in a string.
  inline bool is_digits(const std::string & test_str) {
    // If string is empty, there are no digits.
    if (test_str.size() == 0) return false;

    // Otherwise return false if any character is not a digit.
    for (char c : test_str) if (!is_digit(c)) return false;
    return true;
  }

  /// Determine if there are any letters or digits anywhere in a string.
  inline bool has_alphanumeric(const std::string & test_str) {
    for (char c : test_str) if (is_alphanumeric(c)) return true;
    return false;
  }

  /// Determine if there are any letters or digits anywhere in a string.
  inline bool is_alphanumeric(const std::string & test_str) {
    for (char c : test_str) if (!is_alphanumeric(c)) return false;
    return true;
  }

  /// Determine if there are any letters, digit, or underscores anywhere in a string.
  inline bool has_idchar(const std::string & test_str) {
    for (char c : test_str) if (is_idchar(c)) return true;
    return false;
  }

  /// Determine if a specified set of characters appears anywhere in a string.
  static inline bool has_one_of(const std::string & test_str, const std::string & char_set) {
    for (char c : test_str) if (is_one_of(c, char_set)) return true;
    return false;
  }


  /// If no functions are provided to is_valid(), always return false as base case.
  inline bool is_valid(char /* test_char */ ) { return false; }

  /// Determine if a character passes any of the test functions provided.
  template <typename... FUNS>
  inline bool is_valid(char test_char, std::function<bool(char)> fun1, FUNS... funs) {
    return fun1(test_char) || is_valid(test_char, funs...);
  }

  /// For a string to be valid, each character must pass at least one provided function.
  template <typename... FUNS>
  static inline bool is_valid(const std::string & test_str, FUNS... funs) {
    for (char x : test_str) if ( !is_valid(x, funs...) ) return false;
    return true;
  }


  /// Pop a segment from the beginning of a string as another string, shortening original.
  static inline std::string string_pop_fixed(std::string & in_string, std::size_t end_pos, size_t delim_size=0)
  {
    std::string out_string = "";
    if (end_pos == 0);                        // Not popping anything!
    else if (end_pos == std::string::npos) {  // Popping whole string.
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
  static inline std::string string_get_range(const std::string & in_string, std::size_t start_pos,
                                             std::size_t end_pos) {
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
  inline std::string string_get(const std::string & in_string, const char delim=' ', size_t start_pos=0) {
    return string_get_range(in_string, start_pos, in_string.find(delim, start_pos));
  }

  /// Remove a prefix of the input string (up to any of a specified set of delimeters) and
  /// return it.  If the delimeter is not found, return the entire input string and clear it.
  inline std::string string_pop(std::string & in_string, const std::string & delim_set) {
    return string_pop_fixed(in_string, in_string.find_first_of(delim_set), 1);
  }

  /// Return a prefix of the input string (up to any of a specified set of delimeters), but do not
  /// modify it. If the delimeter is not found, return the entire input string.
  inline std::string string_get(const std::string & in_string, const std::string & delim_set, size_t start_pos=0) {
    emp_assert(start_pos <= in_string.size());
    return string_get_range(in_string, start_pos, in_string.find_first_of(delim_set, start_pos));
  }

  /// Remove a prefix of a string, up to the first whitespace, and return it.
  inline std::string string_pop_word(std::string & in_string) {
    // Whitespace = ' ' '\n' '\r' or '\t'
    return string_pop(in_string, " \n\r\t");
  }

  /// Return a prefix of a string, up to the first whitespace (do not modify the original string)
  inline std::string string_get_word(const std::string & in_string, size_t start_pos=0) {
    // Whitespace = ' ' '\n' '\r' or '\t'
    return string_get(in_string, " \n\r\t", start_pos);
  }

  /// Test if a string has a given prefix.
  inline bool has_prefix(const std::string & in_string, const std::string & prefix) {
    if (prefix.size() > in_string.size()) return false;
    for (size_t i = 0; i < prefix.size(); ++i) {
      if (in_string[i] != prefix[i]) return false;
    }
    return true;
  }

  /// Remove a prefix of a string, up to the first newline, and return it.
  inline std::string string_pop_line(std::string & in_string) {
    return string_pop(in_string, '\n');
  }

  /// Return a prefix of a string, up to the first newline (do not modify the original string)
  inline std::string string_get_line(const std::string & in_string, size_t start_pos=0) {
    return string_get(in_string, '\n', start_pos);
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

  /// Remove instances of characters from file.
  static inline void remove_chars(std::string & in_string, std::string chars) {
    size_t cur_pos = 0;
    for (size_t i = 0; i < in_string.size(); i++) {
      if (is_one_of(in_string[i], chars)) continue;
      in_string[cur_pos++] = in_string[i];
    }
    in_string.resize(cur_pos);
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

  /// Make a string safe(r)
  static inline std::string slugify(const std::string & in_string) {
    //TODO handle complicated unicode strings
    std::string res = to_lower(in_string);
    remove_punctuation(res);
    compress_whitespace(res);
    std::transform(res.begin(), res.end(), res.begin(), [](char ch) {
      return (ch == ' ') ? '-' : ch;
    });
    return res;
  }

  /// Provide a string_view on a given string
  static inline std::string_view view_string(const std::string_view & str) {
    return std::string_view(str);
  }

  /// Provide a string_view on a string from a given starting point.
  static inline std::string_view view_string(const std::string_view & str, size_t start) {
    emp_assert(start <= str.size());
    return str.substr(start, str.size() - start);
  }

  /// Provide a string_view on a string from a starting point with a given size.
  static inline std::string_view view_string(const std::string_view & str,
                                             size_t start,
                                             size_t npos) {
    emp_assert(start + npos <= str.size());
    return str.substr(start, npos);
  }

  /// Provide a string_view on a string from the beginning to a given size.
  static inline std::string_view view_string_front(const std::string_view & str,
                                                   size_t npos) {
    emp_assert(npos <= str.size());
    return str.substr(0, npos);
  }

  /// Provide a string_view on a string from a starting point with a given size.
  static inline std::string_view view_string_back(const std::string_view & str,
                                                  size_t npos) {
    emp_assert(npos <= str.size());
    return str.substr(str.size() - npos, npos);
  }

  /// Provide a string_view on a string from a starting point to an ending point.
  static inline std::string_view view_string_range(const std::string_view & str,
                                                   size_t start,
                                                   size_t end) {
    emp_assert(start <= end);
    emp_assert(end <= str.size());
    return str.substr(start, end - start);
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

  /// Cut up a string based on the provided delimiter; fill them in to the provided vector.
  /// @param in_string operand
  /// @param out_set destination
  /// @param delim delimiter to split on
  /// @param max_split defines the maximum number of splits
  static inline void slice(
    const std::string_view & in_string,
    emp::vector<std::string> & out_set,
    const char delim='\n',
    const size_t max_split=std::numeric_limits<size_t>::max()
  ) {
    const size_t test_size = in_string.size();

    // Count produced strings
    size_t out_count = 0;
    size_t pos = 0;
    while (pos < test_size && out_count <= max_split) {
      while (pos < test_size && in_string[pos] != delim) pos++;
      pos++; // Skip over deliminator
      out_count++;  // Increment for each delim plus once at the end (so once if no delims).
    }

    // And copy over the strings
    out_set.resize(out_count);
    pos = 0;
    size_t string_id = 0;
    while (pos < test_size) {
      out_set[string_id] = "";
      while (
        pos < test_size
        && (in_string[pos] != delim || string_id == out_count - 1)
      ) {
        out_set[string_id] += in_string[pos];
        pos++;
      }
      pos++;        // Skip over any final deliminator
      string_id++;  // Move to the next sub-string.
    }

  }

  /// Slice a string without passing in result vector (may be less efficient).
  /// @param in_string operand
  /// @param delim delimiter to split on
  /// @param max_split defines the maximum number of splits
  static inline emp::vector<std::string> slice(
    const std::string_view & in_string,
    const char delim='\n',
    const size_t max_split=std::numeric_limits<size_t>::max()
  ) {
    emp::vector<std::string> result;
    slice(in_string, result, delim, max_split);
    return result;
  }

  /// Create a set of string_views based on the provided delimiter; fill them in to the provided vector.
  static inline void view_slices(const std::string_view & in_string,
                                 emp::vector<std::string_view> & out_set,
                                 char delim='\n') {
    const size_t in_size = in_string.size();
    out_set.resize(0);

    size_t pos = 0;
    while (pos < in_size) {
      out_set.push_back( view_string_to(in_string, delim, pos) );
      pos += out_set.back().size() + 1;
    }

  }

  /// Slice a string without passing in result vector (may be less efficient).
  static inline emp::vector<std::string_view> view_slices(const std::string_view & in_string,
                                                          char delim='\n') {
    emp::vector<std::string_view> result;
    view_slices(in_string, result, delim);
    return result;
  }

  #ifndef DOXYGEN_SHOULD_SKIP_THIS

  // The next functions are not efficient, but they will take any number of inputs and
  // dynamically convert them all into a single, concatanated string.

  /// Setup emp::ToString declarations for built-in types.
  template <typename T, size_t N> inline std::string ToString(const emp::array<T,N> & container);
  template <typename T, typename... Ts>
  inline std::string ToString(const emp::vector<T, Ts...> & container);

  /// Join a container of strings with a delimiter.
  /// Adapted fromhttps://stackoverflow.com/questions/5288396/c-ostream-out-manipulation/5289170#5289170
  template <typename Range, typename Value = typename Range::value_type>
  std::string join_on(
    Range const& elements,
    const char *const delimiter
  ) {
    std::ostringstream os;
    auto b = std::begin(elements), e = std::end(elements);

    if (b != e) {
        std::copy(b, std::prev(e), std::ostream_iterator<Value>(os, delimiter));
        b = std::prev(e);
    }
    if (b != e) {
        os << *b;
    }

    return os.str();
  }


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

  #endif // DOXYGEN_SHOULD_SKIP_THIS


  /// This function does its very best to convert anything it gets to a string. Takes any number
  /// of arguments and returns a single string containing all of them concatenated.  Any objects
  /// that can go through a stringstream, have a ToString() memember function, or are defined to
  /// be passed into emp::ToString(x) will work correctly.
  template <typename... Ts>
  inline std::string to_string(const Ts &... values) {
    std::stringstream ss;
    (ss << ... << internal::to_stream_item(values, true));
    return ss.str();
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

  /// The from_strings() function takes a vector of strings and convets them into a vector
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

  /**
   * This function returns the values in a vector as a string separated
   * by a given delimeter.
   *
   * @param v a vector
   * @param join_str delimeter
   * @return string of vector values
   */
  template <typename T>
  inline std::string join(const emp::vector<T> & v, std::string join_str) {

    if (v.size() == 0) {
      return "";
    } else if (v.size() == 1) {
      return to_string(v[0]);
    } else {
      std::stringstream res;
      res << v[0];
      for (size_t i = 1; i < v.size(); i++) {
        res << join_str;
        res << to_string(v[i]);
      }
      return res.str();
    }
  }


  // -------- Functions that operate on VECTORS of strings --------

  using string_vec_t = emp::vector<std::string>;

  /// Convert a vector of strings to an English list, such as "one, two, three, and four."
  static inline std::string to_english_list(const string_vec_t & strings) {
    // If there are no input strings, return an empty string.
    if (strings.size() == 0) { return ""; }

    // If there is one string provided, return it by itself.
    if (strings.size() == 1) { return strings[0]; }

    // If two strings are provided, link them by an "and".
    if (strings.size() == 2) { return to_string(strings[0], " and ", strings[1]); }

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
  static inline string_vec_t transform_strings(const string_vec_t & in_strings,
                                               std::function<std::string(const std::string &)> fun) {
    string_vec_t out_strings(in_strings.size());
    for (size_t i = 0; i < in_strings.size(); i++) {
      out_strings[i] = fun(in_strings[i]);
    }
    return out_strings;
  }

  /// Put all strings provided in quotes (Like 'this'), pre- and post-fixing another string if
  /// provided.
  static inline string_vec_t quote_strings(const string_vec_t & in_strings,
                                           const std::string quote="'") {
    return transform_strings(in_strings, [quote](const std::string & str) {
      return to_string(quote, str, quote);
    });
  }

  /// Pre-pend and post-pend specified sequences to all strings provided.
  static inline string_vec_t quote_strings(const string_vec_t & in_strings,
                                           const std::string open_quote,
                                           const std::string close_quote) {
    return transform_strings(in_strings, [open_quote, close_quote](const std::string & str) {
      return to_string(open_quote, str, close_quote);
    });
  }

  /// Take a vector of strings, put them in quotes, and then transform it into an English list.
  static inline std::string to_quoted_list(const string_vec_t & in_strings,
                                           const std::string quote="'") {
    return to_english_list(quote_strings(in_strings, quote));
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


  /// Apply sprintf-like formatting to a string.
  /// See https://en.cppreference.com/w/cpp/io/c/fprintf.
  /// Adapted from https://stackoverflow.com/a/26221725.
  template<typename... Args>
  std::string format_string( const std::string& format, Args... args ) {

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-security"

    // Extra space for '\0'
    const size_t size = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
    emp_assert( size >= 0 );

    emp::vector<char> buf( size );
    std::snprintf( buf.data(), size, format.c_str(), args... );

     // We don't want the '\0' inside
    return std::string( buf.data(), buf.data() + size - 1 );

    #pragma GCC diagnostic pop

  }

}

#endif // #ifndef EMP_TOOLS_STRING_UTILS_HPP_INCLUDE
