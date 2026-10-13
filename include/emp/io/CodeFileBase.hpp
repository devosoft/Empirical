/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/io/CodeFileBase.hpp
 * @brief Shared text-building tools for language-specific code files.
 * @note Status: ALPHA
 */

#pragma once

#ifndef INCLUDE_EMP_IO_CODE_FILE_BASE_HPP_GUARD
#define INCLUDE_EMP_IO_CODE_FILE_BASE_HPP_GUARD

#include <cstddef>
#include <fstream>
#include <ostream>
#include <string>
#include <unordered_map>
#include <utility>

#include "../base/vector.hpp"
#include "../tools/String.hpp"

namespace emp {

  /// Shared line storage and formatting; DERIVED supplies Write(std::ostream &).
  /// This base uses static dispatch to preserve fluent return types. It is not a
  /// runtime-polymorphic interface. Each AddCode argument list builds one line;
  /// callers supply language-specific syntax and manage block indentation.
  template <typename DERIVED>
  class CodeFileBase {
  protected:
    String filename;
    emp::vector<String> header;
    emp::vector<String> code;
    String indent;
    std::unordered_map<String, String> var_map;

    size_t indent_size;
    String comment_prefix;
    String var_symbol;
    String var_quotes;

    CodeFileBase(String filename       = "",
                 size_t indent_size    = 2,
                 String comment_prefix = "// ",
                 String var_symbol     = "$",
                 String var_quotes     = "\"")
      : filename(std::move(filename))
      , indent_size(indent_size)
      , comment_prefix(std::move(comment_prefix))
      , var_symbol(std::move(var_symbol))
      , var_quotes(std::move(var_quotes)) {}

    ~CodeFileBase() = default;

    CodeFileBase(const CodeFileBase &)             = default;
    CodeFileBase(CodeFileBase &&)                  = default;
    CodeFileBase & operator=(const CodeFileBase &) = default;
    CodeFileBase & operator=(CodeFileBase &&)      = default;

    DERIVED & Self() { return static_cast<DERIVED &>(*this); }

    String ReplaceVars(String line) const {
      if (!var_map.empty() && !var_symbol.empty()) {
        line.SetReplaceVars(var_map, var_symbol, StringSyntax(var_quotes));
      }
      return line;
    }

    void WriteHeader(std::ostream & os) const {
      if (header.empty()) { return; }
      for (const auto & line : header) { os << line << '\n'; }
      os << '\n';
    }

    void WriteCode(std::ostream & os, const String & prefix = "") const {
      for (const auto & line : code) {
        if (line.HasNonwhitespace()) { os << prefix << line; }
        os << '\n';
      }
    }

  public:
    /// Clear content, filename, indentation, and variables, retaining formatting settings.
    DERIVED & Clear() {
      filename.clear();
      header.clear();
      code.clear();
      indent.clear();
      var_map.clear();
      return Self();
    }

    DERIVED & SetFilename(String value) {
      filename = std::move(value);
      return Self();
    }

    const String & GetFilename() const { return filename; }

    const emp::vector<String> & GetCode() const { return code; }

    /// Add one header-comment line, using the comment prefix for this language.
    template <typename... Ts>
    DERIVED & AddHeader(Ts &&... args) {
      header.emplace_back(emp::MakeString(comment_prefix, std::forward<Ts>(args)...));
      return Self();
    }

    /// Add one line, applying substitutions immediately, then current indentation.
    template <typename... Ts>
    DERIVED & AddCode(Ts &&... args) {
      code.emplace_back(
        emp::MakeString(indent, ReplaceVars(emp::MakeString(std::forward<Ts>(args)...))));
      return Self();
    }

    /// Each argument is a separate line; this does not open or close a language block.
    template <typename... Ts>
    DERIVED & AddCodeBlock(const String & line1, Ts &&... extras) {
      AddCode(line1);
      (AddCode(std::forward<Ts>(extras)), ...);
      return Self();
    }

    /// Add a line without substitution, even if it contains the configured marker.
    template <typename... Ts>
    DERIVED & AddRawCode(Ts &&... args) {
      code.emplace_back(emp::MakeString(indent, std::forward<Ts>(args)...));
      return Self();
    }

    /// Extend the previous line, starting a line if the file is empty.
    template <typename... Ts>
    DERIVED & AppendCode(Ts &&... args) {
      if (code.empty()) { return AddCode(std::forward<Ts>(args)...); }
      code.back() += ReplaceVars(emp::MakeString(std::forward<Ts>(args)...));
      return Self();
    }

    template <typename... Ts>
    DERIVED & AppendRawCode(Ts &&... args) {
      if (code.empty()) { return AddRawCode(std::forward<Ts>(args)...); }
      code.back() += emp::MakeString(std::forward<Ts>(args)...);
      return Self();
    }

    /// Pad the last stored line, including its indentation, to at least target_size.
    DERIVED & AppendPadding(size_t target_size, char c = ' ') {
      if (code.empty()) { AddRawCode(); }
      if (code.back().size() < target_size) {
        code.back().append(target_size - code.back().size(), c);
      }
      return Self();
    }

    template <typename... Ts>
    DERIVED & AddVarSetting(const String & var, Ts &&... value) {
      var_map[var] = emp::MakeString(std::forward<Ts>(value)...);
      return Self();
    }

    /// Variables use symbol + "{name}". An empty symbol disables substitution.
    DERIVED & SetVarSymbol(String symbol) {
      var_symbol = std::move(symbol);
      return Self();
    }

    /// Quote characters whose contents are skipped during substitution; empty means none.
    /// This only scans the current line or appended fragment, not full language syntax.
    DERIVED & SetVarQuotes(String quotes) {
      var_quotes = std::move(quotes);
      return Self();
    }

    /// Set the number of spaces used by subsequent no-argument indentation calls.
    DERIVED & SetIndentSize(size_t size) {
      indent_size = size;
      return Self();
    }

    DERIVED & IncIndent(size_t size) {
      indent.append(size, ' ');
      return Self();
    }

    DERIVED & IncIndent() { return IncIndent(indent_size); }

    /// Remove spaces, stopping at zero to avoid unsigned underflow.
    DERIVED & DecIndent(size_t size) {
      indent.resize(size < indent.size() ? indent.size() - size : 0);
      return Self();
    }

    DERIVED & DecIndent() { return DecIndent(indent_size); }

    /// Write to the stored filename. Derived classes expose this with using Base::Write.
    DERIVED & Write() {
      std::ofstream out(filename);
      return Self().Write(out);
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_IO_CODE_FILE_BASE_HPP_GUARD
