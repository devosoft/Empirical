//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A simple lexer for the Empirical configuration language.
//
//
//  Development notes: Initially building the lexer to be language specific, but a
//  more general lexer will be a good addition to Empirical.

#ifndef EMP_TOKEN_H
#define EMP_TOKEN_H

#include <iostream>
#include <string>

#include "../tools/string_utils.h"

namespace emp {

  struct Token {
    std::string lexeme;
    enum Type { NONE=0, ERROR=256, UNKNOWN,                             // 0-255 are ASCII chars
      // These tokens need lexemes to be fully specified.
      INT_LIT, FLOAT_LIT, CHAR_LIT, STRING_LIT, ID,

      // The rest are completely determined by their type.
      WHITESPACE, ENDLINE,
      CASSIGN_ADD, CASSIGN_SUB, CASSIGN_MULT, CASSIGN_DIV, CASSIGN_MOD,
      COMP_EQU, COMP_NEQU, COMP_LESS, COMP_LTE, COMP_GTR, COMP_GTE,
      BOOL_AND, BOOL_OR,

      // Statement commands
      COMMAND_PRINT, COMMAND_INCLUDE,
      COMMAND_IF, COMMAND_ELSE, COMMAND_WHILE, COMMAND_FOREACH, COMMAND_BREAK, COMMAND_CONTINUE,
      COMMAND_FUNCTION, COMMAND_RETURN,

      // Built-in functions.
      FUN_RANDOM,

      // Track how many total token types there are.
      NUM_TYPES };
    int type;

    Token(int in_type=NONE, const std::string & in_lexeme="") : type(in_type), lexeme(in_lexeme) { ; }
    Token(const Token &) = default;
    ~Token() { ; }

    operator bool() const { return (bool) type; }

    std::string TypeName() const {
      // If this type is an ASCII character, indicate it.
      if (type > 0 && type < UNKNOWN) {
        return to_string("ASCII('", to_escaped_string(type), "')");
      }

      // Otherwise lookup the enum.
      switch (type) {
        case NONE: return "NONE";
        case UNKNOWN: return "UNKNOWN";
        case INT_LIT: return "INT_LIT";
        case FLOAT_LIT: return "FLOAT_LIT";
        case CHAR_LIT: return "CHAR_LIT";
        case STRING_LIT: return "STRING_LIT";
        case ID: return "ID";
        case WHITESPACE: return "WHITESPACE";
        case ENDLINE: return "ENDLINE";
        case CASSIGN_ADD: return "CASSIGN_ADD";
        case CASSIGN_SUB: return "CASSIGN_SUB";
        case CASSIGN_MULT: return "CASSIGN_MULT";
        case CASSIGN_DIV: return "CASSIGN_DIV";
        case CASSIGN_MOD: return "CASSIGN_MOD";
        case COMP_EQU: return "COMP_EQU";
        case COMP_NEQU: return "COMP_NEQU";
        case COMP_LESS: return "COMP_LESS";
        case COMP_LTE: return "COMP_LTE";
        case COMP_GTR: return "COMP_GTR";
        case COMP_GTE: return "COMP_GTE";
        case BOOL_AND: return "BOOL_AND";
        case BOOL_OR: return "BOOL_OR";
        case COMMAND_PRINT: return "COMMAND_PRINT";
        case COMMAND_INCLUDE: return "COMMAND_INCLUDE";
        case COMMAND_IF: return "COMMAND_IF";
        case COMMAND_ELSE: return "COMMAND_ELSE";
        case COMMAND_WHILE: return "COMMAND_WHILE";
        case COMMAND_FOREACH: return "COMMAND_FOREACH";
        case COMMAND_BREAK: return "COMMAND_BREAK";
        case COMMAND_CONTINUE: return "COMMAND_CONTINUE";
        case COMMAND_FUNCTION: return "COMMAND_FUNCTION";
        case COMMAND_RETURN: return "COMMAND_RETURN";
        case FUN_RANDOM: return "FUN_RANDOM";
        default:
          return "Error: Unknown Token Type";
      }
    }

    std::string ToString() const { return to_string(TypeName(), '(', lexeme, ')'); }

  };

}

#endif
