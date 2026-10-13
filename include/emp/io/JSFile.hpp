/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/io/JSFile.hpp
 * @brief Build JavaScript source files with ordered ECMAScript module imports.
 * @note Status: ALPHA
 */

#pragma once

#ifndef INCLUDE_EMP_IO_JSFILE_HPP_GUARD
#define INCLUDE_EMP_IO_JSFILE_HPP_GUARD

#include <algorithm>
#include <ostream>
#include <utility>

#include "../base/vector.hpp"
#include "../tools/String.hpp"

#include "CodeFileBase.hpp"

namespace emp {

  /// Build JavaScript source. Indentation defaults to two spaces; substitutions use
  /// @{name} everywhere, leaving JavaScript ${name} template expressions intact.
  /// Imports are emitted before the body in insertion order, with exact duplicates removed.
  /// CommonJS require calls and other module forms can be supplied through AddCode.
  class JSFile : public CodeFileBase<JSFile> {
    using base_t = CodeFileBase<JSFile>;

  protected:
    emp::vector<String> imports;

    JSFile & AddImport(String line) {
      if (std::find(imports.begin(), imports.end(), line) == imports.end()) {
        imports.emplace_back(std::move(line));
      }
      return *this;
    }

    // Module names are data, not code; quote them as JavaScript string literals.
    static String QuoteModule(const String & module) {
      String out           = "\"";
      constexpr char hex[] = "0123456789abcdef";
      for (unsigned char c : module) {
        if (c == '"' || c == '\\') {
          out += '\\';
          out += static_cast<char>(c);
        } else if (c < 32) {
          out += "\\u00";
          out += hex[c / 16];
          out += hex[c % 16];
        } else {
          out += static_cast<char>(c);
        }
      }
      out += '"';
      return out;
    }

  public:
    JSFile(String filename = "") : base_t(std::move(filename), 2, "// ", "@", "") {}

    using base_t::Write;

    JSFile & Clear() {
      base_t::Clear();
      imports.clear();
      return *this;
    }

    const emp::vector<String> & GetImports() const { return imports; }

    /// Add a side-effect import; the module specifier is supplied without quotes.
    JSFile & Import(const String & module) {
      return AddImport(MakeString("import ", QuoteModule(module), ";"));
    }

    /// Bindings are JavaScript syntax: "helper", "{ readFile as read }", or "* as utils".
    JSFile & ImportFrom(const String & module, const String & bindings) {
      return AddImport(MakeString("import ", bindings, " from ", QuoteModule(module), ";"));
    }

    JSFile & Write(std::ostream & os) {
      WriteHeader(os);
      for (const auto & line : imports) { os << line << '\n'; }
      if (!imports.empty()) { os << '\n'; }
      WriteCode(os);
      os.flush();
      return *this;
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_IO_JSFILE_HPP_GUARD
