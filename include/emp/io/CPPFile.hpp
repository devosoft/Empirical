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

#include <ostream>
#include <set>
#include <utility>

#include "../tools/String.hpp"

#include "CodeFileBase.hpp"

namespace emp {

  /// Build C++ source with includes, optional guards, and an optional namespace.
  class CPPFile : public CodeFileBase<CPPFile> {
    using base_t = CodeFileBase<CPPFile>;

  protected:
    String include_guard;       // Name of include guards to use, if any.
    std::set<String> includes;  // Set of headers we need to include.
    String name_space;          // Namespace to use, if any.

  public:
    CPPFile(String filename = "") : base_t(std::move(filename)) {}

    using base_t::Write;

    CPPFile & Clear() {
      base_t::Clear();
      include_guard.clear();
      includes.clear();
      name_space.clear();
      return *this;
    }

    CPPFile & SetGuards(String guard_str) {
      include_guard = guard_str;
      return *this;
    }

    CPPFile & SetNamespace(String ns) {
      name_space = ns;
      return *this;
    }

    const std::set<String> & GetIncludes() const { return includes; }

    CPPFile & Include(String filename) {
      if (filename.empty()) { return *this; }
      // If a filename isn't setup as <algorithm> or "my_include.hpp" then add quotes manually.
      if (filename[0] != '"' && filename[0] != '<') { filename = MakeString('"', filename, '"'); }
      includes.insert(filename);
      return *this;
    }

    CPPFile & Write(std::ostream & os) {
      WriteHeader(os);

      if (include_guard.size()) {
        os << "#ifndef " << include_guard << '\n' << "#define " << include_guard << '\n' << '\n';
      }

      if (includes.size()) {
        for (auto & filename : includes) { os << "#include " << filename << '\n'; }
        os << '\n';
      }

      if (name_space.size()) { os << "namespace " << name_space << " {\n"; }
      // Preserve the existing two-space output prefix, including without a namespace.
      WriteCode(os, "  ");
      if (name_space.size()) { os << "} // End of namespace " << name_space << "\n"; }


      if (include_guard.size()) { os << "#endif // #ifndef " << include_guard << "\n"; }

      os.flush();

      return *this;
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_IO_CPPFILE_HPP_GUARD
