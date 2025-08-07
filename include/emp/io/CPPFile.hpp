/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2024 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/io/CPPFile.hpp
 * @brief Generate a C++ file from code.
 * @note Status: ALPHA
 *
 */

#pragma once

#ifndef INCLUDE_EMP_IO_CPPFILE_HPP_GUARD
#define INCLUDE_EMP_IO_CPPFILE_HPP_GUARD

#include <fstream>
#include <iostream>
#include <set>
#include <unordered_map>

#include "../base/vector.hpp"
#include "../tools/String.hpp"

namespace emp {

  /// A class to maintain files for loading, writing, storing, and easy access to components.
  class CPPFile {
  protected:
    String filename;             // Name of file to write to.
    emp::vector<String> header;  // Text for header-comment, if any.
    String include_guard;        // Name of include guards to use, if any.
    std::set<String> includes;   // Set of headers we need to include.
    String name_space;           // Namespace to use, if any.
    emp::vector<String> code;    // One line of code per string.

    String indent;  // Extra indentation before code lines.

    // Track values for variables to use as code is added.
    std::unordered_map<emp::String, emp::String> var_map;

  public:
    CPPFile(String filename = "") : filename(filename) {}

    void Clear() {
      filename.clear();
      header.resize(0);
      include_guard.clear();
      includes.clear();
      name_space.clear();
      code.resize(0);
      indent.clear();
    }

    CPPFile & SetGuards(String guard_str) {
      include_guard = guard_str;
      return *this;
    }

    CPPFile & SetNamespace(String ns) {
      name_space = ns;
      return *this;
    }

    const emp::vector<String> & GetCode() const { return code; }

    const std::set<String> & GetIncludes() const { return includes; }

    // Add a single line of code, merging together everything passed in.
    template <typename... Ts>
    CPPFile & AddCode(Ts &&... args) {
      emp::String line = emp::MakeString(indent, std::forward<Ts>(args)...);
      if (var_map.size() > 0) { line.SetReplaceVars(var_map, "$", StringSyntax::Quotes()); }
      code.emplace_back(line);
      return *this;
    }

    // Add on several lines of code; each argument should be a single line.
    template <typename... Ts>
    CPPFile & AddCodeBlock(const emp::String & line1, Ts &&... extras) {
      AddCode(line1);
      if constexpr (sizeof...(extras) > 0) { AddCodeBlock(extras...); }
      return *this;
    }

    // Extend the previous line of code.
    template <typename... Ts>
    CPPFile & AppendCode(Ts &&... args) {
      emp::String line = emp::MakeString(std::forward<Ts>(args)...);
      if (var_map.size() > 0) { line.SetReplaceVars(var_map, "$", StringSyntax::Quotes()); }
      code.back() += line;
      return *this;
    }

    /// Add spaces (or other character) to make current line at least a specified length.
    /// (usually used to align code or comments)
    CPPFile & AppendPadding(size_t target_size, char c = ' ') {
      const size_t cur_size = code.back().size();
      if (cur_size < target_size) { code.back() += emp::String(target_size - cur_size, c); }
      return *this;
    }

    template <typename... Ts>
    CPPFile & AddVarSetting(const emp::String & var, Ts &&... value) {
      var_map[var] = emp::MakeString(std::forward<Ts>(value)...);
      return *this;
    }

    CPPFile & IncIndent(size_t size = 2) {
      indent += std::string(size, ' ');
      return *this;
    }

    CPPFile & DecIndent(size_t size = 2) {
      indent = std::string(indent.size() - size, ' ');
      return *this;
    }

    CPPFile & Include(String filename) {
      // If a filename isn't setup as <algorithm> or "my_include.hpp" then add quotes manually.
      if (filename[0] != '"' && filename[0] != '<') { filename = MakeString('"', filename, '"'); }
      includes.insert(filename);
      return *this;
    }

    CPPFile & Write(std::ostream & os) {
      if (header.size()) {
        for (const auto & line : header) { os << line << '\n'; }
        os << '\n';
      }

      if (include_guard.size()) {
        os << "#ifndef " << include_guard << '\n' << "#define " << include_guard << '\n' << '\n';
      }

      if (includes.size()) {
        for (auto & filename : includes) { os << "#include " << filename << '\n'; }
        os << '\n';
      }

      if (name_space.size()) { os << "namespace " << name_space << " {\n"; }
      for (const auto & line : code) {
        if (line.HasNonwhitespace()) { os << "  " << line; }
        os << '\n';
      }
      if (name_space.size()) { os << "} // End of namespace " << name_space << "\n"; }


      if (include_guard.size()) { os << "#endif // #ifndef " << include_guard << "\n"; }

      os.flush();

      return *this;
    }

    CPPFile & Write() {
      std::ofstream out(filename);
      return Write(out);
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_IO_CPPFILE_HPP_GUARD
