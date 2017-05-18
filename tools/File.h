//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  The File object maintains a simple, in-memory file.
//
//
//  Developer notes:
//   We need to modify this code to make sure File can also work with Emscripten, appropriately.
//   Alternatively, we might want to have a more flexible file class that wraps this one.


#ifndef EMP_FILE_H
#define EMP_FILE_H

#include <fstream>
#include <functional>
#include <iostream>
#include <string>

#include "../base/vector.h"
#include "string_utils.h"

namespace emp {

  class File {
  protected:
    emp::vector<std::string> lines;

  public:
    File() { ; }
    File(std::istream & input) { Load(input); }
    File(const std::string & filename) { Load(filename); }
    File(const File &) = default;
    File(File &&) = default;
    ~File() { ; }

    File & operator=(const File &) = default;
    File & operator=(File &&) = default;

    size_t GetNumLines() { return lines.size(); }

    std::string & operator[](size_t pos) { return lines[pos]; }
    const std::string & operator[](size_t pos) const { return lines[pos]; }

    void Append(const std::string & line) { lines.emplace_back(line); }
    void Append(const emp::vector<std::string> & in_lines) {
      size_t start_size = lines.size();
      lines.resize(start_size + in_lines.size());
      for (size_t pos = 0; pos < in_lines.size(); pos++) {
        lines[start_size+pos] = in_lines[pos];
      }
    }
    void Append(const File & in_file) { Append(in_file.lines); }

    bool operator==(const File in) { return lines == in.lines; }
    bool operator!=(const File in) { return lines != in.lines; }

    void LoadLine(std::istream & input) {
      lines.emplace_back("");
      std::getline(input, lines.back());
    }

    void Load(std::istream & input) {
      while (!input.eof()) {
	      LoadLine(input);
      }
    }

    void Load(const std::string & filename) {
      std::ifstream file(filename);
      Load(file);
      file.close();
    }

    void Write(std::ostream & output) {
      for (std::string & cur_line : lines) {
	      output << cur_line << '\n';
      }
    }

    void Write(const std::string & filename) {
      std::ofstream file(filename);
      Write(file);
      file.close();
    }

    /// Apply a string manipulation function to all lines in the file.
    void Apply(const std::function<void(std::string &)> & fun) {
      for (std::string & cur_line : lines) {
        fun(cur_line);
      }
    }

    /// Purge functions that don't meet a certain criterion.
    void KeepIf(const std::function<bool(const std::string &)> & fun) {
      emp::vector<std::string> new_lines;
      for (std::string & cur_line : lines) {
        if (fun(cur_line)) new_lines.emplace_back(cur_line);
      }
      std::swap(lines, new_lines);
    }

    /// Remove all lines that are empty strings.
    void RemoveEmpty() {
      KeepIf( [](const std::string & str){ return (bool) str.size(); } );
    }

    /// Any time multiple whitespaces are next to each other, collapse to a single WS char.
    /// Prefer '\n' if in whitespace collapsed, otherwise use ' '.
    void CompressWhitespace() {
      Apply(compress_whitespace);
      RemoveEmpty();
    }
  };

}

#endif
