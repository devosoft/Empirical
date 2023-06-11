/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file ConfigLexer.hpp
 *  @brief A simple lexer for the Empirical configuration language.
 *
 *  Development notes: Initially building the lexer to be language specific, but a
 *  more general lexer will be a good addition to Empirical.
 */

#ifndef EMP_IN_PROGRESS_CONFIGLEXER_HPP_INCLUDE
#define EMP_IN_PROGRESS_CONFIGLEXER_HPP_INCLUDE

#include <iostream>
#include <map>
#include <string>

#include "../base/errors.hpp"

#include "Token.h"

namespace emp {

  class ConfigLexer {
  private:
    std::map<std::string, emp::Token> command_map;

    std::istream & is;
    char next_char;
    std::string cur_lexeme;

    struct Pattern {
      std::string name;      // Unique name for this token.
      std::string pattern;   // Regular expression used to identify instances of token.
      int token_id;          // Unique ID for this token.
      bool ignore;           // Should we skip over these tokens when looking for the next one?
    };

    std::map<std::string, Pattern> patterns;
    int next_token_id;

  public:
    ConfigLexer(std::istream & in_stream) : is(in_stream), next_token_id(256) {
      command_map["print"] = Token(Token::COMMAND_PRINT);
      command_map["include"] = Token(Token::COMMAND_INCLUDE);
      command_map["if"] = Token(Token::COMMAND_IF);
      command_map["else"] = Token(Token::COMMAND_ELSE);
      command_map["while"] = Token(Token::COMMAND_WHILE);
      command_map["break"] = Token(Token::COMMAND_BREAK);
      command_map["continue"] = Token(Token::COMMAND_CONTINUE);
      command_map["return"] = Token(Token::COMMAND_RETURN);
      command_map["function"] = Token(Token::COMMAND_FUNCTION);
      command_map["foreach"] = Token(Token::COMMAND_FOREACH);
      command_map["random"] = Token(Token::FUN_RANDOM);

      // Prime the first character so it's ready to go.
      is.get(next_char);

      AddDefaultPatterns();
    }
    ConfigLexer(ConfigLexer &) = delete;
    ~ConfigLexer() { ; }

    int AddPattern(const std::string & name, const std::string & pattern, int id=0, bool ignore=false) {
      if (patterns.find(name) != patterns.end()) {
        emp::LibraryWarning("Attempting to add multiple lexer patterns for '", name, "'.  Ignoring.");
        return -1;
      }
      if (!id) id = next_token_id++;
      else if (id >= next_token_id) next_token_id = id+1;

      patterns[name] = { name, pattern, id, ignore };
      return id;
    }

    void AddDefaultPatterns() {
      AddPattern("WHITESPACE", "[ \t\r]", Token::WHITESPACE, true);
      AddPattern("COMMENT", "#.*", Token::COMMENT, true);
      AddPattern("INT_LIT", "[0-9]+", Token::INT_LIT);
      AddPattern("FLOAT_LIT", "[0-9]+[.]'[0-9]+", Token::FLOAT_LIT);
      AddPattern("CHAR_LIT", "'(.|(\\\\[\\\\'nt]))'", Token::CHAR_LIT);

      AddPattern("STRING_LIT", "[\"](\\\\[nt\"\\]|[^\\\"])*\"", Token::STRING_LIT);
      AddPattern("ID", "[a-zA-Z0-9_]+", Token::ID);

      // The rest are completely determined by their type.
      AddPattern("ENDLINE", "[\n;]", Token::ENDLINE);
      AddPattern("CASSIGN_ADD", "\"+=\"", Token::CASSIGN_ADD);
      AddPattern("CASSIGN_SUB", "\"-=\"", Token::CASSIGN_SUB);
      AddPattern("CASSIGN_MULT", "\"*=\"", Token::CASSIGN_MULT);
      AddPattern("CASSIGN_DIV", "\"/=\"", Token::CASSIGN_DIV);
      AddPattern("CASSIGN_MOD", "\"%=\"", Token::CASSIGN_MOD);
      AddPattern("COMP_EQU", "==", Token::COMP_EQU);
      AddPattern("COMP_NEQU", "!=", Token::COMP_NEQU);
      AddPattern("COMP_LESS", "<", Token::COMP_LESS);
      AddPattern("COMP_LTE", "<=", Token::COMP_LTE);
      AddPattern("COMP_GTR", ">", Token::COMP_GTR);
      AddPattern("COMP_GTE", ">=", Token::COMP_GTE);
      AddPattern("BOOL_AND", "&&", Token::BOOL_AND);
      AddPattern("BOOL_OR", "||", Token::BOOL_OR);

      // Statement commands
      AddPattern("COMMAND_PRINT", "print", Token::COMMAND_PRINT);
      AddPattern("COMMAND_INCLUDE", "include", Token::COMMAND_INCLUDE);
      AddPattern("COMMAND_IF", "if", Token::COMMAND_IF);
      AddPattern("COMMAND_ELSE", "else", Token::COMMAND_ELSE);
      AddPattern("COMMAND_WHILE", "while", Token::COMMAND_WHILE);
      AddPattern("COMMAND_FOREACH", "foreach", Token::COMMAND_FOREACH);
      AddPattern("COMMAND_BREAK", "break", Token::COMMAND_BREAK);
      AddPattern("COMMAND_CONTINUE", "continue", Token::COMMAND_CONTINUE);
      AddPattern("COMMAND_FUNCTION", "function", Token::COMMAND_FUNCTION);
      AddPattern("COMMAND_RETURN", "return", Token::COMMAND_RETURN);

      // Built-in functions.
      AddPattern("FUN_RANDOM", "random", Token::FUN_RANDOM);
    }

    int GetMaxToken() const { return next_token_id; }

    // @CAO For the moment, GetToken is pre-build using the default token set.
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

        // Any remaining possibilities start with a specific chatacter (or small set of options).
        // Advance next_char now for look-ahead.
        char prev_char = next_char;
        next_char = is.get();

        switch (prev_char) {
          case '#':  // Comment.  Delete to end of line.
            while (next_char != '\n') next_char = is.get();
            next_char = is.get();
            break;   // Comments don't return a token.
          case '\"':
            cur_lexeme.resize(0);
            next_char = is.get();     // Get first char of string.
            while (next_char != '\"') { cur_lexeme.push_back(next_char); next_char = is.get(); }
            next_char = is.get();
            return Token(Token::STRING_LIT, cur_lexeme);
          case ' ':
          case '\t':
          case '\r':
            break;   // Skip WS; don't return token.
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
          case '(':
          case ')':
          case ',':
          case '{':
          case '}':
          case '[':
          case ']':
          case '.':
          case ':':
          case '?':
            return Token(prev_char);
          default:
            // std::cout << "[[ Unk_char=" << int(unk_char) << " ]]" << std::endl;
            return Token(Token::UNKNOWN, std::string(1, prev_char));
        }
      }

      return Token(Token::NONE);
    }
  };

}

#endif // #ifndef EMP_IN_PROGRESS_CONFIGLEXER_HPP_INCLUDE
