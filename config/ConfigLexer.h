//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A simple lexer for the Empirical configuration language.
//
//
//  Development notes: Initially building the lexer to be language specific, but a
//  more general lexer will be a good addition to Empirical.

#ifndef EMP_CONFIG_LEXER_H
#define EMP_CONFIG_LEXER_H

#include <iostream>
#include <map>
#include <string>

#include "Token.h"

namespace emp {

  class ConfigLexer {
  private:
    std::map<std::string, emp::Token> command_map;

    std::istream & is;
    char next_char;
    std::string cur_lexeme;

  public:
    ConfigLexer(std::istream & in_stream) : is(in_stream) {
      command_map["print"] = Token(Token::COMMAND_PRINT);
      command_map["if"] = Token(Token::COMMAND_IF);
      command_map["else"] = Token(Token::COMMAND_ELSE);
      command_map["while"] = Token(Token::COMMAND_WHILE);
      command_map["break"] = Token(Token::COMMAND_BREAK);
      command_map["continue"] = Token(Token::COMMAND_CONTINUE);
      command_map["return"] = Token(Token::COMMAND_RETURN);
      command_map["function"] = Token(Token::COMMAND_FUNCTION);
      command_map["foreach"] = Token(Token::COMMAND_FOREACH);

      // Prime the first character so it's ready to go.
      is.get(next_char);
    }
    ConfigLexer(ConfigLexer &) = delete;
    ~ConfigLexer() { ; }

    emp::Token GetToken() {
      while (next_char > 0) {                      // Keep looping until we find a token or hit EOF.
        if (is_digit(next_char)) {                 // Must be a number
          cur_lexeme.resize(1);
          cur_lexeme[0] = next_char;
          while (is_digit(next_char = is.get())) {
            cur_lexeme.push_back(next_char);
          }
          if (next_char == '.') {                  // Must be floating point.
            cur_lexeme.push_back('.');
            while (is_digit(next_char = is.get())) {
              cur_lexeme.push_back(next_char);
            }
            return Token(Token::FLOAT_LIT, cur_lexeme);
          }
          return Token(Token::INT_LIT, cur_lexeme);
        }
        if (is_idchar(next_char)) {                 // Must be ID or Keyword (number already captured)
          cur_lexeme.resize(1);
          cur_lexeme[0] = next_char;
          while (is_idchar(next_char = is.get())) {
            cur_lexeme.push_back(next_char);
          }

          // Determine if the current lexeme is a command.
          auto map_ptr = command_map.find(cur_lexeme);
          if (map_ptr != command_map.end()) {
            return map_ptr->second;
          }

          return Token(Token::ID, cur_lexeme);
        }

        // Any remaining possibilities are one to two characters.  Advance next_char now.
        char prev_char = next_char;
        next_char = is.get();

        switch (prev_char) {
          case ' ':
          case '\t':
          case '\r':
            break;  // Skip WS; don't return token.
            // return Token(Token::WHITESPACE);
          case '\n':
          case ';':
            return Token(Token::ENDLINE);
          case '+':
            if (next_char == '=') { next_char = is.get(); return Token(Token::CASSIGN_ADD); }
            return Token('+');
          case '-':
            if (next_char == '=') { next_char = is.get(); return Token(Token::CASSIGN_SUB); }
            return Token('-');
          case '*':
            if (next_char == '=') { next_char = is.get(); return Token(Token::CASSIGN_MULT); }
            return Token('*');
          case '/':
            if (next_char == '=') { next_char = is.get(); return Token(Token::CASSIGN_DIV); }
            return Token('/');
          case '%':
            if (next_char == '=') { next_char = is.get(); return Token(Token::CASSIGN_MOD); }
            return Token('%');
          case '=':
            if (next_char == '=') { next_char = is.get(); return Token(Token::COMP_EQU); }
            return Token('=');
          case '<':
            if (next_char == '=') { next_char = is.get(); return Token(Token::COMP_LTE); }
            return Token(Token::COMP_LESS);
          case '>':
            if (next_char == '=') { next_char = is.get(); return Token(Token::COMP_GTE); }
            return Token(Token::COMP_GTR);
          case '!':
            if (next_char == '=') { next_char = is.get(); return Token(Token::COMP_NEQU); }
            return Token('!');
          case '&':
            if (next_char == '&') { next_char = is.get(); return Token(Token::BOOL_AND); }
            return Token('&');
          case '|':
            if (next_char == '|') { next_char = is.get(); return Token(Token::BOOL_OR); }
            return Token('|');
          case '\'':   // Char literal.
            if (next_char == '\\') {
              next_char = is.get();
              switch (next_char) {
                case 'n': next_char = '\n'; break;
                case 'r': next_char = '\r'; break;
                case 't': next_char = '\t'; break;
                case '\\': next_char = '\\'; break;
                case '\'': next_char = '\''; break;
                case '\"': next_char = '\"'; break;
              }
            }
            prev_char = next_char;   // prev_char is now contents.
            next_char = is.get();
            if (next_char != '\'') return Token(Token::ERROR);
            next_char = is.get();
            return Token(Token::CHAR_LIT, std::string(1,prev_char));
          default:
            // std::cout << "[[ Unk_char=" << int(unk_char) << " ]]" << std::endl;
            return Token(Token::UNKNOWN, std::string(1, prev_char));
        }
      }

      return Token(Token::NONE);
    }
  };

}

#endif
