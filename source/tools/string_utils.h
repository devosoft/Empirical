/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2019
 *
 *  @file string_utils.h
 *  @brief Simple functions to manipulate strings.
 *  @note Status: RELEASE
 */


#ifndef EMP_STRING_UTILS_H
#define EMP_STRING_UTILS_H

#include <functional>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>

#include "../base/Ptr.h"
#include "../base/vector.h"
#include "../meta/reflection.h"
#include "../meta/StringType.h"

namespace emp {

  /// Return a const reference to an empty string.  This function is useful to implement other
  /// functions that need to return a const reference for efficiency, but also need a null response.

  static inline const std::string & empty_string() {
    static std::string empty = "";
    return empty;
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


  /// Take a value and convert it to a C++-style literal.
  template <typename LIT_TYPE>
  inline std::string to_literal(const LIT_TYPE & value) {
    return std::to_string(value);
  }

  /// Take a char and convert it to a C++-style literal.
  static inline std::string to_literal(char value) {
    std::stringstream ss;
    ss << "'" << to_escaped_string(value) << "'";
    return ss.str();
  }

  /// Take a string and convert it to a C++-style literal.
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

  // Convert an integer to a roman numeral string.
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

  /// Determine if there are any letters or digits anywhere in a string.
  inline bool has_alphanumeric(const std::string & test_str) {
    for (char c : test_str) if (is_alphanumeric(c)) return true;
    return false;
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
  inline bool is_valid(char test_char) { return false; }

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
    if (end_pos == std::string::npos) end_pos = in_string.size() - start_pos;
    return in_string.substr(start_pos, end_pos);
  }

  /// Remove a prefix of the input string (up to a specified delimeter) and return it.  If the
  /// delimeter is not found, return the entire input string and clear it.
  inline std::string string_pop(std::string & in_string, const char delim=' ') {
    return string_pop_fixed(in_string, in_string.find(delim), 1);
  }

  /// Return a prefix of the input string (up to a specified delimeter), but do not modify it.
  /// If the delimeter is not found, return the entire input string.
  inline std::string string_get(const std::string & in_string, const char delim, size_t start_pos=0) {
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


  /// Provide a string_view on a given string
  static inline std::string_view view_string(const std::string & str) {
    return std::string_view(str);
  }

  /// Provide a string_view on a string from a given starting point.
  static inline std::string_view view_string(const std::string & str, size_t start) {
    emp_assert(start <= str.size());
    return std::string_view(str.c_str() + start, str.size() - start);
  }

  /// Provide a string_view on a string from a starting point with a given size.
  static inline std::string_view view_string(const std::string & str, size_t start, size_t npos) {
    emp_assert(start + npos <= str.size());
    return std::string_view(str.c_str() + start, npos);
  }

  /// Provide a string_view on a string from the beginning to a given size.
  static inline std::string_view view_string_front(const std::string & str, size_t npos) {
    emp_assert(npos <= str.size());
    return std::string_view(str.c_str(), npos);
  }

  /// Provide a string_view on a string from a starting point with a given size.
  static inline std::string_view view_string_back(const std::string & str, size_t npos) {
    emp_assert(npos <= str.size());
    return std::string_view(str.c_str() + str.size() - npos, npos);
  }

  /// Provide a string_view on a string from a starting point to an ending point.
  static inline std::string_view view_string_range(const std::string & str, size_t start, size_t end) {
    emp_assert(start <= end);
    emp_assert(end <= str.size());
    return std::string_view(str.c_str() + start, end - start);
  }

  /// Return a view of the prefix of the input string up to a specified delimeter.
  /// If the delimeter is not found, return the entire input string.
  static inline std::string_view view_string_to(const std::string & in_string, const char delim, size_t start_pos=0) {
    const size_t in_size = in_string.size();
    size_t end_pos = start_pos;
    while (end_pos < in_size && in_string[end_pos] != delim) end_pos++;    
    return view_string_range(in_string, start_pos, end_pos);
  }

  /// Cut up a string based on the provided delimitor; fill them in to the provided vector.
  static inline void slice(const std::string & in_string, emp::vector<std::string> & out_set,
                           char delim='\n') {
    const size_t test_size = in_string.size();

    // Count produced strings
    size_t out_count = 0;
    size_t pos = 0;
    while (pos < test_size) {
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
      while (pos < test_size && in_string[pos] != delim) {
        out_set[string_id] += in_string[pos];
        pos++;
      }
      pos++;        // Skip over any final deliminator
      string_id++;  // Move to the next sub-string.
    }

  }

  /// Slice a string without passing in result vector (may be less efficient).
  static inline emp::vector<std::string> slice(const std::string & in_string, char delim='\n') {
    emp::vector<std::string> result;
    slice(in_string, result, delim);
    return result;
  }

  /// Create a set of string_views based on the provided delimitor; fill them in to the provided vector.
  static inline void view_slices(const std::string & in_string, emp::vector<std::string_view> & out_set,
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
  static inline emp::vector<std::string_view> view_slices(const std::string & in_string, char delim='\n') {
    emp::vector<std::string_view> result;
    view_slices(in_string, result, delim);
    return result;
  }

  /// @cond TEMPLATES

  // The next functions are not efficient, but they will take any number of inputs and
  // dynamically convert them all into a single, concatanated strings or stringstreams.

  namespace internal {
    inline void append_sstream(std::stringstream & ss) { (void) ss; }

    template <typename TYPE, typename... OTHER_TYPES>
    static void append_sstream(std::stringstream & ss, TYPE value, OTHER_TYPES... other_values) {
      ss << value;
      append_sstream(ss, other_values...);
    }

    // Give mutliple implmentations of to_string_impl... if we can append quickly, do so!!
    template <typename T1, typename T2, typename... EXTRA_TYPES>
    inline std::string to_string_impl(int, T1 val1, T2 val2, EXTRA_TYPES... extra_values) {
      std::stringstream ss;
      append_sstream(ss, val1, val2, extra_values...);
      return ss.str();
    }

    // If std::to_string knows how to handle the case use it!
    template <typename T>
    inline auto to_string_impl(bool, T val) -> decltype(std::to_string(val))
    { return std::to_string(val); }

    // If there's another single POD entry, we can convert it manually and pass the result back.
    inline std::string to_string_impl(bool, const std::string & s) { return s; }
    inline std::string to_string_impl(bool, char c) { return std::string(1,c); }
    inline std::string to_string_impl(bool, unsigned char c) { return std::string(1,(char)c); }
    inline std::string to_string_impl(bool, char* str) { return std::string(str); }

    // Operate on std::containers
    template <typename T>
    inline typename emp::sfinae_decoy<std::string, typename T::value_type>
    to_string_impl(bool, T container) {
      std::stringstream ss;
      ss << "[ ";
      for (const auto & el : container) {
        ss << to_string_impl(true, el);
        ss << " ";
      }
      ss << "]";
      return ss.str();
    }
  }

  /// @endcond

  /// This function does its very best to convert everything it's to a string. Takes any number
  /// of arguments and returns a single string containing all of them concatenated. Objects can be
  /// any normal (POD) data type, container, or anything that can be passed into a stringstream.
  template <typename... ALL_TYPES>
  inline std::string to_string(ALL_TYPES &&... all_values) {
    return internal::to_string_impl(true, std::forward<ALL_TYPES>(all_values)...);
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

  namespace internal {
    static inline void _from_string(std::stringstream &) { ; }

    template <typename T, typename... Ts>
    void _from_string(std::stringstream & ss, T & arg1, Ts... extra_args) {
      ss >> arg1;
      _from_string(ss, extra_args...);
    }
  }

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

}

#endif
