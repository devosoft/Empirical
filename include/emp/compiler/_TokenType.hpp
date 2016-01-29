/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2024.
 *
 *  @file _TokenType.hpp
 *  @brief Information about a particular type of token (helper struct for Lexer.hpp)
 *  @note Status: BETA
 */

#ifndef EMP_COMPILER_TOKEN_TYPE_HPP_INCLUDE
#define EMP_COMPILER_TOKEN_TYPE_HPP_INCLUDE

#include "../tools/String.hpp"

#include "RegEx.hpp"

namespace emp {

  /// Information about an individual TYPE of token to be processed within a Lexer.
  struct TokenType {
    String name;      ///< Name of this token type.
    String desc;      ///< More detailed description of this token type.
    RegEx regex;      ///< Pattern to describe token type.
    int id;           ///< Unique id for token.
    bool save_lexeme; ///< Preserve the lexeme for this token?
    bool save_token;  ///< Keep token at all? (Whitespace and comments are often discarded).

    // Default constructor produces an error token.
    TokenType() : name(""), desc("Unable to parse input!"), regex(""),
                  id(-1), save_lexeme(true), save_token(true) { }
    TokenType(const String & _name, const String & _regex, int _id,
              bool _save_l=true, bool _save_t=true, const String & _desc="")
      : name(_name), desc(_desc), regex(_regex), id(_id), save_lexeme(_save_l), save_token(_save_t) { }
    TokenType(const TokenType &) = default;
    TokenType(TokenType &&) = default;
    TokenType & operator=(const TokenType &) = default;
    TokenType & operator=(TokenType &&) = default;

    /// Print out the status of this token (for debugging)
    void Print(std::ostream & os=std::cout) const {
      os << "Name:" << name
         << "  RegEx:" << regex.AsString()
         << "  ID:" << id
         << "  save_lexeme:" << save_lexeme
         << "  save_token:" << save_token
         << std::endl;
    }
  };

}

#endif // #ifndef EMP_COMPILER_TOKEN_TYPE_HPP_INCLUDE
