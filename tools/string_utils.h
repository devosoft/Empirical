#ifndef EMP_STRING_UTILS_H
#define EMP_STRING_UTILS_H

//////////////////////////////////////////////////////////////////////////////
//
//  This file contains a set of simple functions to manipulate strings.
//

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace emp {

  std::string to_escaped_string(char value) {
    // Start by quickly returning a string if it's easy.
    std::stringstream ss;
    if ( (value >= 40 && value < 91) || value > 96) {
      ss << value;
      return ss.str();
    }
    switch (value) {
    case '\\':
      return "\\\\";
      break;
    case '\'':
      return "\\\'";
      break;
    case '\"':
      return "\\\"";
      break;
    case '\n':
      return "\\n";
      break;
    case '\r':
      return "\\r";
      break;
    case '\t':
      return "\\t";
      break;
    default:
      ss << value;
      return ss.str();
    };
  }
  std::string to_escaped_string(std::string value) {
    std::stringstream ss;
    for (char c : value) { ss << to_escaped_string(c); }
    return ss.str();
  }



  // The to_literal function set will take a value and convert it to a C++ literal.
  template <typename LIT_TYPE> std::string to_literal(const LIT_TYPE & value) {
    return std::to_string(value);
  }
  std::string to_literal(char value) {
    std::stringstream ss;
    ss << "'" << to_escaped_string(value) << "'";
    return ss.str();
  }
  std::string to_literal(const std::string & value) {
    // Add quotes to the ends and convert each character.
    std::stringstream ss;
    ss << "\"";
    for (char c : value) {
      ss << to_escaped_string(c);
    }
    ss << "\"";
    return ss.str();
  }


  bool is_whitespace(char test_char) {
    return (test_char == ' ' || test_char == '\n' || test_char == '\r' || test_char == '\t');
  }

  bool is_upper_letter(char test_char) {
    return (test_char >= 'A' && test_char <= 'Z');
  }

  bool is_lower_letter(char test_char) {
    return (test_char >= 'a' && test_char <= 'z');
  }

  bool is_letter(char test_char) {
    return is_upper_letter(test_char) || is_lower_letter(test_char);
  }

  bool is_digit(char test_char) {
    return (test_char >= '0' && test_char <= '9');
  }

  // Pop a segment from the beginning of a string as another string, shortening original.
  std::string string_pop_fixed(std::string & in_string, std::size_t end_pos, int delim_size=0) {
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

  // Get a segment from the beginning of a string as another string, leaving original untouched.
  std::string string_get_range(const std::string & in_string, std::size_t start_pos,
                               std::size_t end_pos) {
    if (end_pos == std::string::npos) end_pos = in_string.size() - start_pos;
    return in_string.substr(start_pos, end_pos);
  }

  std::string string_pop(std::string & in_string, const char delim) {
    return string_pop_fixed(in_string, in_string.find(delim), 1);
  }

  std::string string_get(const std::string & in_string, const char delim, int start_pos=0) {
    return string_get_range(in_string, start_pos, in_string.find(delim, start_pos));
  }

  std::string string_pop(std::string & in_string, const std::string & delim_set) {
    return string_pop_fixed(in_string, in_string.find_first_of(delim_set), 1);
  }

  std::string string_get(const std::string & in_string, const std::string & delim_set, int start_pos=0) {
    return string_get_range(in_string, start_pos, in_string.find_first_of(delim_set, start_pos));
  }

  std::string string_pop_word(std::string & in_string) {
    // Whitespace = ' ' '\n' '\r' or '\t'
    return string_pop(in_string, " \n\r\t");
  }

  std::string string_get_word(const std::string & in_string, int start_pos=0) {
    // Whitespace = ' ' '\n' '\r' or '\t'
    return string_get(in_string, " \n\r\t", start_pos);
  }

  std::string string_pop_line(std::string & in_string) {
    return string_pop(in_string, '\n');
  }

  std::string string_get_line(const std::string & in_string, int start_pos=0) {
    return string_get(in_string, '\n', start_pos);
  }

  // Tricks for dealing with whitespace.
  std::string left_justify(std::string & in_string) {
    return string_pop(in_string, in_string.find_first_not_of(" \n\r\t"));
  }

  void right_justify(std::string & in_string) {
    while (is_whitespace(in_string.back())) in_string.pop_back();
  }

  void compress_whitespace(std::string & in_string) {
    const int strlen = (int) in_string.size();
    bool last_whitespace = true;
    int next_char = 0;

    for (int i = 0; i < strlen; i++) {
      if (is_whitespace(in_string[i])) {  // This char is whitespace
        if (last_whitespace) continue;
        in_string[next_char++] = ' ';
        last_whitespace = true;
      }
      else {  // Not whitespace
        in_string[next_char++] = in_string[i];
        last_whitespace = false;
      }
    }

    in_string.resize(next_char);
  }

  void remove_whitespace(std::string & in_string) {
    const int strlen = (int) in_string.size();
    int next_char = 0;

    for (int i = 0; i < strlen; i++) {
      if (is_whitespace(in_string[i])) continue;
      in_string[next_char++] = in_string[i];
    }

    in_string.resize(next_char);
  }

  // Cut up a string based on a deliminator.
  void slice_string(const std::string & in_string, std::vector<std::string> & out_set,
                    char delim='\n') {
    int test_size = (int) in_string.size();

    // Count produced strings
    int out_count = 0;
    int pos = 0;
    while (pos < test_size) {
      while (pos < test_size && in_string[pos] != delim) pos++;
      pos++; // Skip over deliminator
      out_count++;
    }

    // And copy over the strings
    out_set.resize(out_count);
    pos = 0;
    int string_id = 0;
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

};

#endif
