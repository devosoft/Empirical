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
    std::string & front() { return lines.front(); }
    const std::string & front() const { return lines.front(); }
    std::string & back() { return lines.back(); }
    const std::string & back() const { return lines.back(); }

    File & Append(const std::string & line) { lines.emplace_back(line); return *this; }
    File & Append(const emp::vector<std::string> & in_lines) {
      size_t start_size = lines.size();
      lines.resize(start_size + in_lines.size());
      for (size_t pos = 0; pos < in_lines.size(); pos++) {
        lines[start_size+pos] = in_lines[pos];
      }
      return *this;
    }
    File & Append(const File & in_file) { return Append(in_file.lines); }

    template <typename T>
    File & operator+=(T && in) { Append( std::forward<T>(in) ); }

    bool operator==(const File in) { return lines == in.lines; }
    bool operator!=(const File in) { return lines != in.lines; }

    File & LoadLine(std::istream & input) {
      lines.emplace_back("");
      std::getline(input, lines.back());
      return *this;
    }

    File & Load(std::istream & input) {
      while (!input.eof()) {
	      LoadLine(input);
      }
      return *this;
    }

    File & Load(const std::string & filename) {
      std::ifstream file(filename);
      Load(file);
      file.close();
      return *this;
    }

    File & Write(std::ostream & output) {
      for (std::string & cur_line : lines) {
	      output << cur_line << '\n';
      }
      return *this;
    }

    File & Write(const std::string & filename) {
      std::ofstream file(filename);
      Write(file);
      file.close();
      return *this;
    }

    /// Apply a string manipulation function to all lines in the file.
    File & Apply(const std::function<void(std::string &)> & fun) {
      for (std::string & cur_line : lines) {
        fun(cur_line);
      }
      return *this;
    }

    /// Purge functions that don't meet a certain criterion.
    File & KeepIf(const std::function<bool(const std::string &)> & fun) {
      emp::vector<std::string> new_lines;
      for (std::string & cur_line : lines) {
        if (fun(cur_line)) new_lines.emplace_back(cur_line);
      }
      std::swap(lines, new_lines);
      return *this;
    }

    /// Remove all lines that are empty strings.
    File & RemoveEmpty() {
      return KeepIf( [](const std::string & str){ return (bool) str.size(); } );
    }

    /// Any time multiple whitespaces are next to each other, collapse to a single WS char.
    /// Prefer '\n' if in whitespace collapsed, otherwise use ' '.
    File & CompressWhitespace() {
      Apply(compress_whitespace);
      RemoveEmpty();
      return *this;
    }

    /// Delete all whitespace; by default keep newlines.
    File & RemoveWhitespace(bool keep_newlines=true) {
      Apply(compress_whitespace);
      RemoveEmpty();
      if (!keep_newlines) {
        std::string all_lines;
        for (const std::string & cur_line : lines){
          all_lines += cur_line;
        }
        lines.resize(1);
        std::swap(lines[0], all_lines);
      }
      return *this;
    }

    /// A technique to remove all comments in a file.
    File & RemoveComments(const std::string & marker) {
      Apply( [marker](std::string & str) {
        size_t pos = str.find(marker);
        if (pos !=std::string::npos) str.resize( pos );
      } );
      return *this;
    }
  };

}

#endif
