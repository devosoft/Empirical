/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2024.
 *
 *  @file _TokenStream.hpp
 *  @brief A series of tokens produced from a lexer.
 *  @note Status: BETA
 */

#ifndef EMP_COMPILER_TOKEN_STREAM_HPP_INCLUDE
#define EMP_COMPILER_TOKEN_STREAM_HPP_INCLUDE

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
    TokenStream(const String & in_name) : name(in_name) { }
    TokenStream(const TokenStream &) = default;
    TokenStream(TokenStream &&) = default;
    TokenStream(const emp::vector<Token> & in_tokens, const String & in_name)
    : name(in_name), tokens(in_tokens) { }

    TokenStream & operator=(const TokenStream &) = default;
    TokenStream & operator=(TokenStream &&) = default;

    class Iterator {
    private:
      emp::Ptr<const TokenStream> ts;
      size_t pos;

    public:
      Iterator(const Iterator &) = default;
      Iterator(const TokenStream & in_ts, size_t in_pos) : ts(&in_ts), pos(in_pos) { }
      Iterator & operator=(const Iterator &) = default;

      const TokenStream & GetTokenStream() const { return *ts; }
      size_t GetIndex() const { return pos; }
      emp::Ptr<const Token> ToPtr() const { return ts->GetPtr(pos); }

      Token operator*() const { return ts->tokens[pos]; }
      const Token * operator->() const { return &(ts->tokens[pos]); }

      bool operator==(const Iterator & in) const { return ToPtr() == in.ToPtr(); }
      bool operator!=(const Iterator & in) const { return ToPtr() != in.ToPtr(); }
      bool operator< (const Iterator & in) const { return ToPtr() <  in.ToPtr(); }
      bool operator<=(const Iterator & in) const { return ToPtr() <= in.ToPtr(); }
      bool operator> (const Iterator & in) const { return ToPtr() >  in.ToPtr(); }
      bool operator>=(const Iterator & in) const { return ToPtr() >= in.ToPtr(); }

      Iterator & operator++() { ++pos; return *this; }
      Iterator operator++(int) { Iterator old(*this); ++pos; return old; }
      Iterator & operator--() { --pos; return *this; }
      Iterator operator--(int) { Iterator old(*this); --pos; return old; }

      bool IsValid() const { return pos < ts->size(); }
      bool AtEnd() const { return pos == ts->size(); }

      operator bool() const { return IsValid(); }
    };

    size_t size() const { return tokens.size(); }
    const Token & Get(size_t pos) const { return tokens[pos]; }
    emp::Ptr<const Token> GetPtr(size_t pos) const { return &(tokens.data()[pos]); }
    const String & GetName() const { return name; }
    Iterator begin() const { return Iterator(*this, 0); }
    Iterator end() const { return Iterator(*this, tokens.size()); }
    const Token & back() const { return tokens.back(); }

    void push_back(const Token & in) { tokens.push_back(in); }

    void Print(std::ostream & os=std::cout) const {
      for (auto x : tokens) {
        os << " [" << x.lexeme << "]";
      }
      os << std::endl;
    }
  };

}

#endif // #ifndef EMP_COMPILER_TOKEN_STREAM_HPP_INCLUDE
