/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/tools/TemplateString.hpp
 * @brief String literal wrapper suitable as a non-type template parameter.
 * @note Status: BETA
 */

#pragma once

#ifndef INCLUDE_EMP_TOOLS_TEMPLATE_STRING_HPP_GUARD
#define INCLUDE_EMP_TOOLS_TEMPLATE_STRING_HPP_GUARD

#include <array>
#include <cstddef>
#include <string_view>

namespace emp {

  /// Structural type for passing string literals as template parameters.
  template <size_t N>
  struct TemplateString {
    std::array<char, N> value{};

    constexpr TemplateString(const char (& in)[N]) {
      for (size_t i = 0; i < N; ++i) { value[i] = in[i]; }
    }

    constexpr const char * data() const { return value.data(); }

    constexpr size_t size() const { return N - 1; }

    constexpr std::string_view AsStringView() const { return {data(), size()}; }
  };

  template <size_t N>
  TemplateString(const char (&)[N]) -> TemplateString<N>;

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_TOOLS_TEMPLATE_STRING_HPP_GUARD
