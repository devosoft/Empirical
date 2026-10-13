/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/compiler/_CodeGen.hpp
 * @brief Internal helpers for emitting language-specific source templates.
 */

#pragma once

#ifndef INCLUDE_EMP_COMPILER_CODE_GEN_HPP_impl_GUARD
#define INCLUDE_EMP_COMPILER_CODE_GEN_HPP_impl_GUARD

#include <unordered_map>

#include "../tools/String.hpp"

namespace emp::internal {

  /// Expand this template independently of substitution settings on the destination file.
  template <typename FILE_T>
  void WriteCodeTemplate(FILE_T & file,
                         String source,
                         const std::unordered_map<String, String> & vars) {
    source.SetReplaceVars(vars, "@", StringSyntax::None());
    for (const auto & line : source.Slice('\n', StringSyntax::None(), false)) {
      file.AddRawCode(line);
    }
  }

  /// A quoted string literal valid in both Python and JavaScript (ASCII metadata).
  inline String QuoteCodeString(const String & value) {
    String out           = "\"";
    constexpr char hex[] = "0123456789abcdef";
    for (unsigned char c : value) {
      if (c == '"' || c == '\\') {
        out += '\\';
        out += static_cast<char>(c);
      } else if (c < 32 || c == 127) {
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

}  // namespace emp::internal

#endif  // #ifndef INCLUDE_EMP_COMPILER_CODE_GEN_HPP_impl_GUARD

// Local settings for Empecable file checker.
// empecable_words: metadata
