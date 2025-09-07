/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2024-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/compiler/_TokenStream.hpp
 * @brief A series of tokens produced from a lexer.
 * @note Status: BETA
 */

#pragma once

#ifndef INCLUDE_EMP_COMPILER_TOKEN_STREAM_HPP_impl_GUARD
#define INCLUDE_EMP_COMPILER_TOKEN_STREAM_HPP_impl_GUARD

#include <vector>

#include "../base/Ptr.hpp"
#include "../tools/String.hpp"

#include "_Token.hpp"

namespace emp {

  class TokenStream {
  private:
    String name = "";
    emp::vector<Token> tokens;

  public:
    TokenStream(const String & in_name) : name(in_name) {}

    TokenStream(const TokenStream &) = default;
    TokenStream(TokenStream &&)      = default;

    TokenStream(const emp::vector<Token> & in_tokens, const String & in_name)
      : name(in_name), tokens(in_tokens) {}

    TokenStream & operator=(const TokenStream &) = default;
    TokenStream & operator=(TokenStream &&)      = default;

    [[nodiscard]] static const Token & GetEOF() {
      static Token eof_token{0, "", 0};
      return eof_token;
    }

    class Iterator {
    private:
      emp::Ptr<const TokenStream> ts;
      size_t pos;

    public:
      Iterator(const Iterator &) = default;

      Iterator(const TokenStream & in_ts, size_t in_pos) : ts(&in_ts), pos(in_pos) {}

      Iterator & operator=(const Iterator &) = default;

      [[nodiscard]] const TokenStream & GetTokenStream() const { return *ts; }

      [[nodiscard]] size_t GetIndex() const { return pos; }

      [[nodiscard]] emp::Ptr<const Token> ToPtr() const { return ts->GetPtr(pos); }

      [[nodiscard]] Token operator*() const { return ts->tokens[pos]; }

      [[nodiscard]] const Token * operator->() const { return &(ts->tokens[pos]); }

      [[nodiscard]] bool operator==(const Iterator & in) const { return ToPtr() == in.ToPtr(); }

      [[nodiscard]] bool operator!=(const Iterator & in) const { return ToPtr() != in.ToPtr(); }

      [[nodiscard]] bool operator<(const Iterator & in) const { return ToPtr() < in.ToPtr(); }

      [[nodiscard]] bool operator<=(const Iterator & in) const { return ToPtr() <= in.ToPtr(); }

      [[nodiscard]] bool operator>(const Iterator & in) const { return ToPtr() > in.ToPtr(); }

      [[nodiscard]] bool operator>=(const Iterator & in) const { return ToPtr() >= in.ToPtr(); }

      Iterator & operator++() {
        ++pos;
        return *this;
      }

      Iterator operator++(int) {
        Iterator old(*this);
        ++pos;
        return old;
      }

      Iterator & operator--() {
        --pos;
        return *this;
      }

      Iterator operator--(int) {
        Iterator old(*this);
        --pos;
        return old;
      }

      [[nodiscard]] bool IsValid() const { return pos < ts->size(); }

      [[nodiscard]] bool AtEnd() const { return pos == ts->size(); }

      operator bool() const { return IsValid(); }

      // Test if there are ANY tokens remaining.
      [[nodiscard]] bool Any() const { return !AtEnd(); }

      // Test if there are NO tokens remaining.
      [[nodiscard]] bool None() const { return AtEnd(); }

      // Get the current (or upcoming) token, but don't remove it from the queue.
      [[nodiscard]] const Token & Peek(size_t skip_count = 0) const {
        if (pos + skip_count >= ts->tokens.size()) { return ts->GetEOF(); }
        return ts->tokens[pos];
      }

      // Test if the current token is a specific type.
      [[nodiscard]] bool Is(int id, size_t skip_count = 0) const { return Peek(skip_count) == id; }

      // Get the current token, removing it from the queue.
      const Token & Use() {
        if (None()) { return ts->GetEOF(); }
        return ts->tokens[pos++];
      }

      // Use the current token if it is the expected type; otherwise error.
      // (Use provided error if available, otherwise use default error)
      template <typename... Ts>
      const Token & Use(int id, Ts &&... message) {
        if (!Is(id)) { notify::Error(std::forward<Ts>(message)...); }
        return Use();
      }

      // If the current token is one of the provided ids, use it an return the ID used.
      // Otherwise, don't use it and return 0.
      template <typename... Ts>
      const Token & UseIf(int id, Ts... args) {
        if (Is(id)) { return Use(); }
        return UseIf(args...);
      }

      // Base case for UseIf
      const Token & UseIf() { return ts->GetEOF(); }

      // Rewind one or more tokens.
      bool Rewind(size_t steps = 1) {
        if (pos >= steps) {
          pos -= steps;
          return true;
        } else {
          pos = 0;
          return false;
        }
      }
    };

    [[nodiscard]] size_t size() const { return tokens.size(); }

    [[nodiscard]] const Token & Get(size_t pos) const { return tokens[pos]; }

    [[nodiscard]] emp::Ptr<const Token> GetPtr(size_t pos) const { return &(tokens.data()[pos]); }

    [[nodiscard]] const String & GetName() const { return name; }

    [[nodiscard]] Iterator begin() const { return Iterator(*this, 0); }

    [[nodiscard]] Iterator end() const { return Iterator(*this, tokens.size()); }

    [[nodiscard]] const Token & back() const { return tokens.back(); }

    void push_back(const Token & in) { tokens.push_back(in); }

    void Print(std::ostream & os = std::cout) const {
      for (const auto & x : tokens) { os << " [" << x.lexeme << "]"; }
      os << std::endl;
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_COMPILER_TOKEN_STREAM_HPP_impl_GUARD
