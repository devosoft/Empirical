/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2024.
*/
/**
 *  @file
 *  @brief Generate a C++ file from code.
 *  @note Status: ALPHA
 *
 */

#ifndef EMP_IO_CPP_FILE_HPP_INCLUDE
#define EMP_IO_CPP_FILE_HPP_INCLUDE

#include <fstream>
#include <iostream>
#include <set>

#include "../base/vector.hpp"
#include "../tools/String.hpp"

namespace emp {

  /// A class to maintain files for loading, writing, storing, and easy access to components.
  class CPPFile {
  protected:
    String filename;            // Name of file to write to.
    emp::vector<String> header; // Text for header-comment, if any.
    String include_guard;       // Name of include guards to use, if any.
    std::set<String> includes;  // Set of headers we need to include.
    String name_space;          // Namespace to use, if any.
    emp::vector<String> code;   // One line of code per string.

    String indent;              // Extra indentation before code lines.

  public:
    CPPFile(String filename="") : filename(filename) { }

    CPPFile & SetGuards(String guard_str) {
      include_guard = guard_str;
      return *this;
    }

    const emp::vector<String> & GetCode() const { return code; }
    const std::set<String> & GetIncludes() const { return includes; }

    template <typename... Ts>
    CPPFile & AddCode(Ts &&... args) {
      code.emplace_back( emp::MakeString(indent, std::forward<Ts>(args)...) );
      return *this;
    }

    template <typename... Ts>
    CPPFile & AppendCode(Ts &&... args) {
      code.back() += emp::MakeString(std::forward<Ts>(args)...);
      return *this;
    }

    CPPFile & IncIndent(size_t size=2) {
      indent += std::string(size, ' ');
      return *this;
    }
    CPPFile & DecIndent(size_t size=2) {
      indent = std::string(indent.size() - size, ' ');
      return *this;
    }

    CPPFile & Include(String filename) {
      // If a filename isn't setup as <algorithm> or "my_include.hpp" then add quotes manually.
      if (filename[0] != '"' && filename[0] != '<') {
        filename = MakeString('"', filename, '"');
      }
      includes.insert(filename);
      return *this;
    }

    CPPFile & Write(std::ostream & os) {
      if (header.size()) {
        for (const auto & line : header) os << line << '\n';
        os << '\n';
      }

      if (include_guard.size()) {
        os << "#ifndef " << include_guard << '\n'
           << "#define " << include_guard << '\n'
           << '\n';
      }

      if (includes.size()) {
        for (auto & filename : includes) {
          os << "#include " << filename << '\n';
        }
        os << '\n';
      }

      if (name_space.size()) {
        os << "namespace " << name_space << " {\n";
        for (const auto & line : code) os << "  " << line << '\n';
        os << "} // End of namespace " << name_space << "\n";
      }
      else {
        for (const auto & line : code) os << line << '\n';
      }

      if (include_guard.size()) {
        os << "#endif // #ifndef " << include_guard << "\n";
      }

      os.flush();

      return *this;
    }

    CPPFile & Write() {
      std::ofstream out(filename);
      return Write(out);      
    }
  };

}

#endif // #ifndef EMP_IO_CPP_FILE_HPP_INCLUDE
