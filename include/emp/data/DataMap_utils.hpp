/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  DataMap_utils.hpp
 *  @brief Useful functions for working with DataMaps and AnnotatedTypes.
 *  @note Status: ALPHA
 */

#ifndef EMP_DATA_MAP_UTILS_HPP
#define EMP_DATA_MAP_UTILS_HPP

#include <string>

#include "AnnotatedType.hpp"
#include "DataMap.hpp"
#include "../compiler/Lexer.hpp"

namespace emp {
namespace DataMapUtils {

  using value_fun_t = std::function<double(emp::DataMap &)>;
  using pos_t = emp::TokenStream::Iterator;

  // Precendence levels (low to high):
  enum class Prec {
    UNKNOWN=0,
    // ASSIGN,     // Assingment: x = y
    OR,         // Or: x || y
    AND,        // And: x && y
    EQUALITY,   // Equality tests: x == y, x != y
    INEQUALITY, // Inquality tests: x < y, x <= y, x > y, x >= y
    PLUS_MINUS, // Add & substract: x + y, x - y
    MULTIPLY,   // Muliply, divide, and mod: x * y, x / y, x % y
    POW_LOG,    // Power and log: x ** y, x %% y
    UNARY,      // Unary operations:  -x, +x, !x
    // INC_DEC,    // Post-or Pre- increment, decrement: x++, ++x, x--, --x
    PARENS,     // Parentheses: (x)
    NUM_PREC    // Total number of precendence levels.
  };

  class DataMapLexer : public emp::Lexer {
  private:
    int token_identifier;   ///< Token id for identifiers
    int token_number;       ///< Token id for literal numbers
    int token_string;       ///< Token id for literal strings
    int token_char;         ///< Token id for literal characters
    int token_symbol;       ///< Token id for other symbols

  public:
    DataMapLexer() {
      // Whitespace and comments should always be dismissed (top priority)
      IgnoreToken("Whitespace", "[ \t\n\r]+");
      IgnoreToken("//-Comments", "//.*");
      IgnoreToken("/*...*/-Comments", "/[*]([^*]|([*]+[^*/]))*[*]+/");

      // Meaningful tokens have next priority.

      // An indentifier must begin with a letter, underscore, or dot, and followed by
      // more of the same OR numbers or brackets.
      token_identifier = AddToken("Identifier", "[a-zA-Z_.][a-zA-Z0-9_.[\\]]*");

      // A literal number must begin with a digit; it can have any number of digits in it and
      // optionally a decimal point.
      token_number = AddToken("Literal Number", "[0-9]+(\\.[0-9]+)?");

      // A string must begin and end with a quote and can have an escaped quote in the middle.
      token_string = AddToken("Literal String", "\\\"([^\"\\\\]|\\\\.)*\\\"");

      // A literal char must begin and end with a single quote.  It will always be treated as
      // its ascii value.
      token_char = AddToken("Literal Character", "'([^'\n\\\\]|\\\\.)+'");

      // Symbols should have least priority.  They include any solitary character not listed
      // above, or pre-specified multi-character groups.
      token_symbol = AddToken("Symbol", ".|\"==\"|\"!=\"|\"<=\"|\">=\"|\"&&\"|\"||\"|\"++\"|\"--\"");
    }

    bool IsID(const emp::Token token) const noexcept { return token.token_id == token_identifier; }
    bool IsNumber(const emp::Token token) const noexcept { return token.token_id == token_number; }
    bool IsString(const emp::Token token) const noexcept { return token.token_id == token_string; }
    bool IsChar(const emp::Token token) const noexcept { return token.token_id == token_char; }
    bool IsSymbol(const emp::Token token) const noexcept { return token.token_id == token_symbol; }
  };

  static const DataMapLexer & GetDataMapLexer() {
    static DataMapLexer dm_lexer;
    return dm_lexer;
  }

  struct ValueType {
    enum type_t { VALUE, FUNCTION };
    type_t type;
    double value;
    value_fun_t fun;

    ValueType(double in_val) : type(VALUE), value(in_val) { }
    ValueType(value_fun_t in_fun) : type(FUNCTION), fun(in_fun) { }
  };

  /// Helpers for parsing.
  ValueType ParseValue(const DataMap & dm, pos_t & pos) {
    // Test if we should NEGATE!
    if (pos->lexeme == "-") {
      pos++;
      ValueType val = ParseValue(dm, pos);
      if (val.type == ValueType::VALUE) { return -val.value; }
      else { return (value_fun_t) [fun=val.fun](emp::DataMap & dm){ return -fun(dm); }; }
    }   

    // Test if we should NOT!
    if (pos->lexeme == "!") {
      pos++;
      ValueType val = ParseValue(dm, pos);
      if (val.type == ValueType::VALUE) { return !val.value; }
      else { return (value_fun_t) [fun=val.fun](emp::DataMap & dm){ return !fun(dm); }; }
    }   

    // A unary PLUS does nothing...
    if (pos->lexeme == "+") { pos++; return ParseValue(dm, pos); }

    // If we have parentheses, process the contents
    if (pos->lexeme == "(") {
      pos++;
      ValueType val = ParseMath(dm, pos);
      // @CAO: Should make sure the close is another paren...
      pos++;
      return val;
    }

    // If this is a value, set it and return.
    if (emp::is_digit(pos->lexeme[0])) {
      return emp::from_string<double>(pos->lexeme);
    }

    // Otherwise it should be in the datamap!
    emp_assert(dm.HasName(pos->lexeme));
    size_t id = dm.GetID(pos->lexeme);
    return (value_fun_t) [id](emp::DataMap & dm){ return dm.GetAsDouble(id); };
  }

  ValueType ParseMath(const DataMap & dm, pos_t & pos, Prec min_prec=Prec::UNKNOWN) {
    ValueType val1 = ParseValue(dm, pos);

    if (!pos.IsValid() || pos->lexeme == ")") return val1;

    // Exponetiation
    if (pos->lexeme == "**") {
      if (min_prec > Prec::POW_LOG) return val1;
      pos++;
      ValueType val2 = ParseValue(dm, pos);
      if (val1.type == ValueType::VALUE) {
        if (val2.type == ValueType::VALUE) { return emp::Pow(val1.value, val2.value); }
        else {
          return (value_fun_t) [base=val1.value,exp_fun=val2.fun](emp::DataMap & dm){
            return emp::Pow(base, exp_fun(dm));
          };
        }
      } else {
        if (val2.type == ValueType::VALUE) {
          return (value_fun_t) [base_fun=val1.fun,exp=val2.value](emp::DataMap & dm){
            return emp::Pow(base_fun(dm), exp);
          };
        } else {
          return (value_fun_t) [base_fun=val1.fun,exp_fun=val2.fun](emp::DataMap & dm){
            return emp::Pow(base_fun(dm), exp_fun(dm));
          };
        }
      }
    }

    // Log!
    if (pos->lexeme == "%%") {
      if (min_prec > Prec::POW_LOG) return val1;
      pos++;
      ValueType val2 = ParseValue(dm, pos);
      if (val1.type == ValueType::VALUE) {
        if (val2.type == ValueType::VALUE) { return emp::Log(val1.value, val2.value); }
        else {
          return (value_fun_t) [base=val1.value,exp_fun=val2.fun](emp::DataMap & dm){
            return emp::Log(base, exp_fun(dm));
          };
        }
      } else {
        if (val2.type == ValueType::VALUE) {
          return (value_fun_t) [base_fun=val1.fun,exp=val2.value](emp::DataMap & dm){
            return emp::Log(base_fun(dm), exp);
          };
        } else {
          return (value_fun_t) [base_fun=val1.fun,exp_fun=val2.fun](emp::DataMap & dm){
            return emp::Log(base_fun(dm), exp_fun(dm));
          };
        }
      }
    }

    // Multiply!
    if (pos->lexeme == "*") {
      if (min_prec > Prec::MULTIPLY) return val1;
      pos++;
      ValueType val2 = ParseValue(dm, pos);
      if (val1.type == ValueType::VALUE) {
        if (val2.type == ValueType::VALUE) { return val1.value * val2.value; }
        else {
          return (value_fun_t) [base=val1.value,exp_fun=val2.fun](emp::DataMap & dm){
            return base * exp_fun(dm);
          };
        }
      } else {
        if (val2.type == ValueType::VALUE) {
          return (value_fun_t) [base_fun=val1.fun,exp=val2.value](emp::DataMap & dm){
            return base_fun(dm) * exp;
          };
        } else {
          return (value_fun_t) [base_fun=val1.fun,exp_fun=val2.fun](emp::DataMap & dm){
            return base_fun(dm) * exp_fun(dm);
          };
        }
      }
    }

    // Divide!
    if (pos->lexeme == "/") {
      if (min_prec > Prec::MULTIPLY) return val1;
      pos++;
      ValueType val2 = ParseValue(dm, pos);
      if (val1.type == ValueType::VALUE) {
        if (val2.type == ValueType::VALUE) { return val1.value / val2.value; }
        else {
          return (value_fun_t) [base=val1.value,exp_fun=val2.fun](emp::DataMap & dm){
            return base / exp_fun(dm);
          };
        }
      } else {
        if (val2.type == ValueType::VALUE) {
          return (value_fun_t) [base_fun=val1.fun,exp=val2.value](emp::DataMap & dm){
            return base_fun(dm) / exp;
          };
        } else {
          return (value_fun_t) [base_fun=val1.fun,exp_fun=val2.fun](emp::DataMap & dm){
            return base_fun(dm) / exp_fun(dm);
          };
        }
      }
    }

    // Remainder!
    if (pos->lexeme == "%") {
      if (min_prec > Prec::MULTIPLY) return val1;
      pos++;
      ValueType val2 = ParseValue(dm, pos);
      if (val1.type == ValueType::VALUE) {
        if (val2.type == ValueType::VALUE) { return emp::Mod(val1.value, val2.value); }
        else {
          return (value_fun_t) [base=val1.value,exp_fun=val2.fun](emp::DataMap & dm){
            return emp::Mod(base, exp_fun(dm));
          };
        }
      } else {
        if (val2.type == ValueType::VALUE) {
          return (value_fun_t) [base_fun=val1.fun,exp=val2.value](emp::DataMap & dm){
            return emp::Mod(base_fun(dm), exp);
          };
        } else {
          return (value_fun_t) [base_fun=val1.fun,exp_fun=val2.fun](emp::DataMap & dm){
            return emp::Mod(base_fun(dm), exp_fun(dm));
          };
        }
      }
    }

    // Addition!
    if (pos->lexeme == "*") {
      if (min_prec > Prec::PLUS_MINUS) return val1;
      pos++;
      ValueType val2 = ParseValue(dm, pos);
      if (val1.type == ValueType::VALUE) {
        if (val2.type == ValueType::VALUE) { return val1.value + val2.value; }
        else {
          return (value_fun_t) [base=val1.value,exp_fun=val2.fun](emp::DataMap & dm){
            return base + exp_fun(dm);
          };
        }
      } else {
        if (val2.type == ValueType::VALUE) {
          return (value_fun_t) [base_fun=val1.fun,exp=val2.value](emp::DataMap & dm){
            return base_fun(dm) + exp;
          };
        } else {
          return (value_fun_t) [base_fun=val1.fun,exp_fun=val2.fun](emp::DataMap & dm){
            return base_fun(dm) + exp_fun(dm);
          };
        }
      }
    }

    // Subtraction!
    if (pos->lexeme == "-") {
      if (min_prec > Prec::PLUS_MINUS) return val1;
      pos++;
      ValueType val2 = ParseValue(dm, pos);
      if (val1.type == ValueType::VALUE) {
        if (val2.type == ValueType::VALUE) { return val1.value - val2.value; }
        else {
          return (value_fun_t) [base=val1.value,exp_fun=val2.fun](emp::DataMap & dm){
            return base - exp_fun(dm);
          };
        }
      } else {
        if (val2.type == ValueType::VALUE) {
          return (value_fun_t) [base_fun=val1.fun,exp=val2.value](emp::DataMap & dm){
            return base_fun(dm) - exp;
          };
        } else {
          return (value_fun_t) [base_fun=val1.fun,exp_fun=val2.fun](emp::DataMap & dm){
            return base_fun(dm) - exp_fun(dm);
          };
        }
      }
    }

    // TODO:
    // INEQUALITY, // Inquality tests: x < y, x <= y, x > y, x >= y
    // EQUALITY,   // Equality tests: x == y, x != y
    // AND,        // And: x && y
    // OR,         // Or: x || y

  }

  /// Parse a function description that will take a DataMap and return the results.
  /// For example, if the string "foo * 2 + bar" is passed in, a function will be returned
  /// that takes a datamap (of the example type) loads in the values of "foo" and "bar", and
  /// returns the result of the above equation.

  auto BuildMathFunction(const DataMap & dm, const std::string & fun_info) {
    emp::TokenStream tokens = GetDataMapLexer().Tokenize(fun_info);
    pos_t pos = tokens.begin();
  }

}
}

#endif
