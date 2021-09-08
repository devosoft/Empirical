/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  DataMapParser.hpp
 *  @brief Useful functions for working with DataMaps and AnnotatedTypes.
 *  @note Status: ALPHA
 */

#ifndef EMP_DATA_MAP_PARSER_HPP
#define EMP_DATA_MAP_PARSER_HPP

#include <string>

#include "../base/error.hpp"
#include "../compiler/Lexer.hpp"

#include "AnnotatedType.hpp"
#include "DataMap.hpp"

namespace emp {
  struct DataMapParser {

    using value_fun_t = std::function<double(emp::DataMap &)>;
    using pos_t = emp::TokenStream::Iterator;

    static constexpr const bool verbose = false;

    class DataMapLexer : public emp::Lexer {
    private:
      int token_identifier;   ///< Token id for identifiers
      int token_number;       ///< Token id for literal numbers
      int token_string;       ///< Token id for literal strings
      int token_char;         ///< Token id for literal characters
      int token_external;     ///< Token id for strings to be evaluated externally.
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

        // Setup a RegEx that can detect up to 4-deep nested parentheses.
        const std::string no_parens = "[^()\n\r]*";
        const std::string open = "\"(\"";
        const std::string close = "\")\"";
        const std::string matched_parens = open + no_parens + close;
        const std::string multi_parens = no_parens + "("s + matched_parens + no_parens + ")*"s;
        const std::string nested_parens2 = open + multi_parens + close;
        const std::string multi_nested2 = no_parens + "("s + nested_parens2 + no_parens + ")*"s;
        const std::string nested_parens3 = open + multi_nested2 + close;
        const std::string multi_nested3 = no_parens + "("s + nested_parens3 + no_parens + ")*"s;
        const std::string nested_parens4 = open + multi_nested3 + close;
        const std::string multi_nested4 = no_parens + "("s + nested_parens4 + no_parens + ")*"s;

        // An external value should be evaluated in a provided function.  If no such function
        // exists, using it will be an error.
        token_external = AddToken("External Evaluation", "\"$(\""s + multi_nested4 + "\")\""s);

        // Symbols should have least priority.  They include any solitary character not listed
        // above, or pre-specified multi-character groups.
        token_symbol = AddToken("Symbol", ".|\"==\"|\"!=\"|\"<=\"|\">=\"|\"&&\"|\"||\"|\"**\"|\"%%\"");
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

      ValueType & operator=(const ValueType &) = default;
      ValueType & operator=(double in_val) { type = VALUE; value = in_val; return *this; }
      ValueType & operator=(value_fun_t in_fun) { type = FUNCTION; fun = in_fun; return *this; }
    };

    struct BinaryOperator {
      using fun_t = std::function<double(double,double)>;
      size_t prec;
      fun_t fun;
      void Set(size_t in_prec, fun_t in_fun) { prec = in_prec; fun = in_fun; }
    };

    // Create map of binary operators to the functions they should call.
    static const std::map<std::string, BinaryOperator> & GetBinaryOperators() {
      static std::map<std::string, BinaryOperator> ops;
      if (ops.size() == 0) {
        size_t prec = 0;  // Precedence level of each operator...
        ops["||"].Set( ++prec, [](double x, double y){ return (x!=0.0)||(y!=0.0); } );
        ops["&&"].Set( ++prec, [](double x, double y){ return (x!=0.0)&&(y!=0.0); } );
        ops["=="].Set( ++prec, [](double x, double y){ return x == y; } );
        ops["!="].Set(   prec, [](double x, double y){ return x != y; } );
        ops["<"] .Set( ++prec, [](double x, double y){ return x < y; } );
        ops["<="].Set(   prec, [](double x, double y){ return x <= y; } );
        ops[">"] .Set(   prec, [](double x, double y){ return x > y; } );
        ops[">="].Set(   prec, [](double x, double y){ return x >= y; } );
        ops["+"] .Set( ++prec, [](double x, double y){ return x + y; } );
        ops["-"] .Set(   prec, [](double x, double y){ return x - y; } );
        ops["*"] .Set( ++prec, [](double x, double y){ return x * y; } );
        ops["/"] .Set(   prec, [](double x, double y){ return x / y; } );
        ops["%"] .Set(   prec, [](double x, double y){ return emp::Mod(x, y); } );
        ops["**"].Set( ++prec, [](double x, double y){ return emp::Pow(x, y); } );
        ops["%%"].Set(   prec, [](double x, double y){ return emp::Log(x, y); } );
      }

      return ops;
    }

    /// Helpers for parsing.
    static ValueType ParseValue(const DataMap & dm, pos_t & pos) {
      if constexpr (verbose) {
        std::cout << "ParseValue at position " << pos.GetIndex() << " : " << pos->lexeme << std::endl;
      }

      // Test if we should NEGATE!
      if (pos->lexeme == "-") {
        if constexpr (verbose) std::cout << "Found: UNARY NEGATION" << std::endl;
        ++pos;
        ValueType val = ParseValue(dm, pos);
        if (val.type == ValueType::VALUE) { return -val.value; }
        else { return (value_fun_t) [fun=val.fun](emp::DataMap & dm){ return -fun(dm); }; }
      }   

      // Test if we should NOT!
      if (pos->lexeme == "!") {
        if constexpr (verbose) std::cout << "Found: UNARY NOT" << std::endl;
        ++pos;
        ValueType val = ParseValue(dm, pos);
        if (val.type == ValueType::VALUE) { return !val.value; }
        else { return (value_fun_t) [fun=val.fun](emp::DataMap & dm){ return !fun(dm); }; }
      }   

      // A unary PLUS does nothing...
      if (pos->lexeme == "+") {
        if constexpr (verbose) std::cout << "Found: UNARY PLUS" << std::endl;
        ++pos; return ParseValue(dm, pos);
      }

      // If we have parentheses, process the contents
      if (pos->lexeme == "(") {
        if constexpr (verbose) std::cout << "Found: OPEN PAREN" << std::endl;
        ++pos;
        ValueType val = ParseMath(dm, pos);
        emp_assert(pos->lexeme == ")");
        ++pos;
        return val;
      }

      // If this is a value, set it and return.
      if (emp::is_digit(pos->lexeme[0])) {
        double result = emp::from_string<double>(pos->lexeme);
        ++pos;
        return result;
      }

      // Otherwise it should be in the datamap!
      emp_assert(dm.HasName(pos->lexeme), pos->lexeme);
      size_t id = dm.GetID(pos->lexeme);
      ++pos;
      return (value_fun_t) [id](emp::DataMap & dm){ return dm.GetAsDouble(id); };
    }

    static ValueType ParseMath(const DataMap & dm, pos_t & pos, size_t prec_limit=0) {
      ValueType val1 = ParseValue(dm, pos);
      auto ops = GetBinaryOperators();

      if constexpr (verbose) {
        if (pos.IsValid()) {
          std::cout << "ParseMath at position " << pos.GetIndex()
                    << " : " << pos->lexeme << std::endl;
        } else {
          std::cout << "PROCESSED!" << std::endl;
        }
      }

      while (pos.IsValid() && pos->lexeme != ")") {
        if constexpr (verbose) {
          std::cout << "...Scanning for operator... [" << pos->lexeme << "]" << std::endl;
        }

        if (Has(ops, pos->lexeme)) {
          const BinaryOperator & op = ops[pos->lexeme];
          if (prec_limit >= op.prec) return val1; // Precedence not allowed; return currnet value.
          ++pos;
          ValueType val2 = ParseMath(dm, pos, op.prec);
          if (val1.type == ValueType::VALUE) {
            if (val2.type == ValueType::VALUE) { val1 = op.fun(val1.value, val2.value); }
            else {
              val1 = (value_fun_t) [val1_num=val1.value,val2_fun=val2.fun,op_fun=op.fun](emp::DataMap & dm){
                return op_fun(val1_num, val2_fun(dm));
              };
            }
          } else {
            if (val2.type == ValueType::VALUE) {
              val1 = (value_fun_t) [val1_fun=val1.fun,val2_num=val2.value,op_fun=op.fun](emp::DataMap & dm){
                return op_fun(val1_fun(dm), val2_num);
              };
            } else {
              val1 = (value_fun_t) [val1_fun=val1.fun,val2_fun=val2.fun,op_fun=op.fun](emp::DataMap & dm){
                return op_fun(val1_fun(dm), val2_fun(dm));
              };
            }
          }
        }

        else emp_error("No operator [", pos->lexeme, "] found!");
      }

      // @CAO Make sure there's not a illegal lexeme here.

      return val1;
    }

    /// Parse a function description that will take a DataMap and return the results.
    /// For example, if the string "foo * 2 + bar" is passed in, a function will be returned
    /// that takes a datamap (of the example type) loads in the values of "foo" and "bar", and
    /// returns the result of the above equation.

    static value_fun_t BuildMathFunction(const DataMap & dm, const std::string & fun_info) {
      emp::TokenStream tokens = GetDataMapLexer().Tokenize(fun_info);
      if constexpr (verbose) tokens.Print();
      pos_t pos = tokens.begin();
      ValueType val = ParseMath(dm, pos);

      // If this value is fixed, turn it into a function.
      if (val.type == ValueType::VALUE) {
        return [out=val.value](emp::DataMap &){ return out; };
      }

      // Otherwise return the function produced.
      return val.fun;
    }

  };
}

#endif
