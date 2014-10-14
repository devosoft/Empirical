#ifndef EMP_STRING_UTILS_H
#define EMP_STRING_UTILS_H

////////////////////////////////////////////////////////////////////
//  This file contains a set of simple functions to 

#include <iostream>
#include <string>
#include <vector>

namespace emp {

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
  std::string string_pop(std::string & in_string, std::size_t end_pos) {
    std::string out_string = "";
    if (end_pos == std::string::npos) {
      out_string = in_string;
      in_string = "";
    }
    else {
      out_string = in_string.substr(0, end_pos);  // Copy up to the deliminator for ouput
      in_string.erase(0, end_pos+1);              // Delete output string AND deliminator
    }

    return out_string;
  }

  std::string string_pop(std::string & in_string, const char delim) {
    return string_pop(in_string, in_string.find(delim));
  }

  std::string string_pop(std::string & in_string, const std::string & delim_set) {
    return string_pop(in_string, in_string.find_first_of(delim_set));
  }

  std::string string_pop_word(std::string & in_string) {
    // Whitespace = ' ' '\n' '\r' or '\t'
    return string_pop(in_string, " \n\r\t");
  }

  std::string string_pop_line(std::string & in_string) {
    return string_pop(in_string, '\n');
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
