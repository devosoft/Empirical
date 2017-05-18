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
    File(const File &) = default;
    File(File &&) = default;
    ~File() { ; }

    File & operator=(const File &) = default;
    File & operator=(File &&) = default;

    size_t GetNumLines() { return lines.size(); }

    std::string & operator[](size_t pos) { return lines[pos]; }
    const std::string & operator[](size_t pos) const { return lines[pos]; }

    bool operator==(const File in) { return lines == in.lines; }
    bool operator!=(const File in) { return lines != in.lines; }

    void LoadLine(istream & input) {
      lines.emplace_back("");
      std::getline(input, lines.back());
    }

    void Load(istream & input) {
      while (!input.eof()) {
	LoadLine(input);
      }
    }

    void Load(const std::string & filename) {
      ifstream file(filename);
      Load(file);
      file.close();
    }

    void Write(ostream & output) {
      for (std::string & cur_line : lines) {
	output << lines << '\n';
      }
    }

    void Write(const std::string & filename) {
      ofstream file(filename);
      Write(file);
      file.close();
    }
  };

}

#endif
