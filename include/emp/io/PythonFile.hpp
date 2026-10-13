/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/io/PythonFile.hpp
 * @brief Build Python source files with ordered imports and explicit indentation.
 * @note Status: ALPHA
 */

#pragma once

#ifndef INCLUDE_EMP_IO_PYTHON_FILE_HPP_GUARD
#define INCLUDE_EMP_IO_PYTHON_FILE_HPP_GUARD

#include <algorithm>
#include <ostream>
#include <utility>

#include "../base/vector.hpp"
#include "../tools/String.hpp"

#include "CodeFileBase.hpp"

namespace emp {

  /// Build Python source. Indentation defaults to four spaces; substitutions use
  /// @{name} everywhere, including quoted text. Use AddRawCode to bypass substitution.
  /// Imports are emitted before the body in insertion order, with exact duplicates removed.
  class PythonFile : public CodeFileBase<PythonFile> {
    using base_t = CodeFileBase<PythonFile>;

  protected:
    emp::vector<String> imports;

    PythonFile & AddImport(String line) {
      if (std::find(imports.begin(), imports.end(), line) == imports.end()) {
        imports.emplace_back(std::move(line));
      }
      return *this;
    }

  public:
    PythonFile(String filename = "") : base_t(std::move(filename), 4, "# ", "@", "") {}

    using base_t::Write;

    PythonFile & Clear() {
      base_t::Clear();
      imports.clear();
      return *this;
    }

    const emp::vector<String> & GetImports() const { return imports; }

    /// Add `import module` or `import module as alias`. Names are supplied as Python syntax.
    PythonFile & Import(const String & module, const String & alias = "") {
      if (alias.empty()) { return AddImport(MakeString("import ", module)); }
      return AddImport(MakeString("import ", module, " as ", alias));
    }

    /// Names may include commas, aliases, or '*', e.g., ImportFrom("math", "sqrt as root").
    /// Add future imports before ordinary imports. Put local imports in AddCode instead.
    PythonFile & ImportFrom(const String & module, const String & names) {
      return AddImport(MakeString("from ", module, " import ", names));
    }

    PythonFile & Write(std::ostream & os) {
      WriteHeader(os);
      for (const auto & line : imports) { os << line << '\n'; }
      if (!imports.empty()) { os << '\n'; }
      WriteCode(os);
      os.flush();
      return *this;
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_IO_PYTHON_FILE_HPP_GUARD
