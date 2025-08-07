/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2024 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/compiler/_Token.hpp
 * @brief An individual token produced from a lexer.
 * @note Status: BETA
 */

#pragma once

#ifndef INCLUDE_EMP_COMPILER_TOKEN_HPP_impl_GUARD
#define INCLUDE_EMP_COMPILER_TOKEN_HPP_impl_GUARD

#include <vector>

#include "../tools/String.hpp"

namespace emp {

  /// Information about a token instance from an input stream.
  struct Token {
    int id;          ///< Which type of token is this?
    String lexeme;   ///< Sequence matched by this token (or empty if not saved)
    size_t line_id;  ///< Which line did this token start on?

    Token(int _id, const String & str = "", size_t _line = 0)
      : id(_id), lexeme(str), line_id(_line) {
      ;
    }

    Token(const Token &)             = default;
    Token(Token &&)                  = default;
    Token & operator=(const Token &) = default;
    Token & operator=(Token &&)      = default;

    operator int() const { return id; }  // Convert to token ID if used as an int.

    operator const std::string &() const { return lexeme; }  // Convert to lexeme if used as string.

    template <std::same_as<int>... Ts>
    [[nodiscard]] bool IsOneOf(int test_id, Ts... args) const {
      if (id == test_id) { return true; }
      if constexpr (sizeof...(args) > 0) {
        return IsOneOf(args...);
      } else {
        return false;
      }
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_COMPILER_TOKEN_HPP_impl_GUARD
