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
#include <iostream>
#include <string>

#include "../base/vector.h"

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
  };

}

#endif
