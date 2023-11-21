/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  SimpleParser.hpp
 *  @brief Common praser functionality with custom plugins for variables and functions.
 *  @note Status: ALPHA
 * 
 *  Developer TODO:
 *  - Make ${ ... } actually work
 *  - Setup operator RegEx to be built dynamically
 *  - Allow new operators to be added externally
 *  - Setup LVALUES as a type, and allow assignment
 *  - Add in a type system (String, double, vectors, etc.)
 */

#ifndef EMP_SIMPLE_PARSER_HPP
#define EMP_SIMPLE_PARSER_HPP

#include <cmath>
#include <string>

#include "../base/error.hpp"
#include "../compiler/Lexer.hpp"

#include "SimpleLexer.hpp"

namespace emp {
  class SimpleParser {

    using pos_t = emp::TokenStream::Iterator;

    bool verbose = false;

    using value_fun_t = std::function<double(emp::DataMap &)>;
    struct ValueType {
      enum type_t { ERROR=0, VALUE, FUNCTION };

      type_t type;
      double value;
      value_fun_t fun;

      ValueType() : type(ERROR) {}
      ValueType(const ValueType &) = default;
      ValueType(double in_val) : type(VALUE), value(in_val) { }
      ValueType(value_fun_t in_fun) : type(FUNCTION), fun(in_fun) { }

      ValueType & operator=(const ValueType &) = default;
      ValueType & operator=(double in_val) { type = VALUE; value = in_val; return *this; }
      ValueType & operator=(value_fun_t in_fun) { type = FUNCTION; fun = in_fun; return *this; }

      value_fun_t AsFun() {
        if (type==FUNCTION) return fun; else return [v=value](emp::DataMap &){ return v; };
      }
    };

    struct BinaryOperator {
      using fun_t = std::function<double(double,double)>;
      size_t prec;
      fun_t fun;
      void Set(size_t in_prec, fun_t in_fun) { prec = in_prec; fun = in_fun; }
    };

    struct Function {
      using fun0_t = std::function<double()>;
      using fun1_t = std::function<double(double)>;
      using fun2_t = std::function<double(double,double)>;
      using fun3_t = std::function<double(double,double,double)>;

      size_t num_args = 0;
      fun0_t fun0; fun1_t fun1; fun2_t fun2; fun3_t fun3;

      void Set0(fun0_t in_fun) { num_args = 0; fun0 = in_fun; }
      void Set1(fun1_t in_fun) { num_args = 1; fun1 = in_fun; }
      void Set2(fun2_t in_fun) { num_args = 2; fun2 = in_fun; }
      void Set3(fun3_t in_fun) { num_args = 3; fun3 = in_fun; }
    };

    // --------- MEMBER VARIABLES -----------
    SimpleLexer lexer;
    std::unordered_map<std::string, std::function<double(double)>> unary_ops;
    std::unordered_map<std::string, BinaryOperator> binary_ops;
    std::unordered_map<std::string, Function> functions;
    size_t error_count = 0;

    using error_fun_t = std::function<void(const std::string &)>;
    error_fun_t error_fun =
     [](const std::string & msg){ std::cerr << "ERROR: " << msg << std::endl; };

    template<typename... Ts>
    ValueType AddError(Ts &&... args) {
      error_fun( emp::to_string(args...); );
      ++error_count;
      return ValueType();
    }

  public:
    SimpleParser(bool use_defaults=true) {
      if (use_defaults) {
        AddDefaultOperators();
        AddDefaultFunctions();
      }
    }

    bool HasErrors() const { return error_count; }
    size_t NumErrors() const { return error_count; }

    error_fun_t GetErrorFun() const { return error_fun; }
    void SetErrorFun(error_fun_t in_fun) { error_fun = in_fun; }

    // Add a unary operator
    void AddOp(const std::string & op, std::function<double(double)> fun) {
      unary_ops[op] = fun;
    }

    void AddDefaultOperators() {
            // Setup the unary operators for the parser.
      AddOp("+", [](double x) { return x; });
      AddOp("-", [](double x) { return -x; };
      AddOp("!", [](double x) { return (double) (x==0.0); };

      // Setup the default binary operators for the parser.
      size_t prec = 0;  // Precedence level of each operator...
      binary_ops["||"].Set( ++prec, [](double x, double y){ return (x!=0.0)||(y!=0.0); } );
      binary_ops["&&"].Set( ++prec, [](double x, double y){ return (x!=0.0)&&(y!=0.0); } );
      binary_ops["=="].Set( ++prec, [](double x, double y){ return x == y; } );
      binary_ops["!="].Set(   prec, [](double x, double y){ return x != y; } );
      binary_ops["<"] .Set( ++prec, [](double x, double y){ return x < y; } );
      binary_ops["<="].Set(   prec, [](double x, double y){ return x <= y; } );
      binary_ops[">"] .Set(   prec, [](double x, double y){ return x > y; } );
      binary_ops[">="].Set(   prec, [](double x, double y){ return x >= y; } );
      binary_ops["+"] .Set( ++prec, [](double x, double y){ return x + y; } );
      binary_ops["-"] .Set(   prec, [](double x, double y){ return x - y; } );
      binary_ops["*"] .Set( ++prec, [](double x, double y){ return x * y; } );
      binary_ops["/"] .Set(   prec, [](double x, double y){ return x / y; } );
      binary_ops["%"] .Set(   prec, [](double x, double y){ return emp::Mod(x, y); } );
      binary_ops["**"].Set( ++prec, [](double x, double y){ return emp::Pow(x, y); } );
      binary_ops["%%"].Set(   prec, [](double x, double y){ return emp::Log(x, y); } );
    }

    void AddDefaultFunctions() {
      // Setup the default functions.
      functions["ABS"].Set1( [](double x){ return std::abs(x); } );
      functions["EXP"].Set1( [](double x){ return emp::Pow(emp::E, x); } );
      functions["LOG"].Set1( [](double x){ return std::log(x); } );
      functions["LOG2"].Set1( [](double x){ return std::log2(x); } );
      functions["LOG10"].Set1( [](double x){ return std::log10(x); } );

      functions["SQRT"].Set1( [](double x){ return std::sqrt(x); } );
      functions["CBRT"].Set1( [](double x){ return std::cbrt(x); } );

      functions["SIN"].Set1( [](double x){ return std::sin(x); } );
      functions["COS"].Set1( [](double x){ return std::cos(x); } );
      functions["TAN"].Set1( [](double x){ return std::tan(x); } );
      functions["ASIN"].Set1( [](double x){ return std::asin(x); } );
      functions["ACOS"].Set1( [](double x){ return std::acos(x); } );
      functions["ATAN"].Set1( [](double x){ return std::atan(x); } );
      functions["SINH"].Set1( [](double x){ return std::sinh(x); } );
      functions["COSH"].Set1( [](double x){ return std::cosh(x); } );
      functions["TANH"].Set1( [](double x){ return std::tanh(x); } );
      functions["ASINH"].Set1( [](double x){ return std::asinh(x); } );
      functions["ACOSH"].Set1( [](double x){ return std::acosh(x); } );
      functions["ATANH"].Set1( [](double x){ return std::atanh(x); } );

      functions["CEIL"].Set1( [](double x){ return std::ceil(x); } );
      functions["FLOOR"].Set1( [](double x){ return std::floor(x); } );
      functions["ROUND"].Set1( [](double x){ return std::round(x); } );

      functions["ISINF"].Set1( [](double x){ return std::isinf(x); } );
      functions["ISNAN"].Set1( [](double x){ return std::isnan(x); } );

      // Default 2-input functions
      functions["HYPOT"].Set2( [](double x, double y){ return std::hypot(x,y); } );
      functions["EXP"].Set2( [](double x, double y){ return emp::Pow(x,y); } );
      functions["LOG"].Set2( [](double x, double y){ return emp::Log(x,y); } );
      functions["MIN"].Set2( [](double x, double y){ return (x<y) ? x : y; } );
      functions["MAX"].Set2( [](double x, double y){ return (x>y) ? x : y; } );
      functions["POW"].Set2( [](double x, double y){ return emp::Pow(x,y); } );

      // Default 3-input functions.
      functions["IF"].Set3( [](double x, double y, double z){ return (x!=0.0) ? y : z; } );
      functions["CLAMP"].Set3( [](double x, double y, double z){ return (x<y) ? y : (x>z) ? z : x; } );
      functions["TO_SCALE"].Set3( [](double x, double y, double z){ return (z-y)*x+y; } );
      functions["FROM_SCALE"].Set3( [](double x, double y, double z){ return (x-y) / (z-y); } );
    }

    /// Helpers for parsing.
    ValueType ParseValue(const DataMap & dm, pos_t & pos) {
      if constexpr (verbose) {
        std::cout << "ParseValue at position " << pos.GetIndex() << " : " << pos->lexeme << std::endl;
      }

      // Deal with any unary operators...
      if (emp::Has(unary_ops, pos->lexeme)) {
        if constexpr (verbose) std::cout << "Found UNARY OP: " << pos->lexeme << std::endl;
        auto op = unary_ops[pos->lexeme];
        ++pos;
        ValueType val = ParseValue(dm, pos);
        if (val.type == ValueType::VALUE) { return op(val.value); }
        else { return (value_fun_t) [fun=val.fun,op](emp::DataMap & dm){ return op(fun(dm)); }; }
      }

      // If we have parentheses, process the contents
      if (pos->lexeme == "(") {
        if constexpr (verbose) std::cout << "Found: OPEN PAREN" << std::endl;
        ++pos;
        ValueType val = ParseMath(dm, pos);
        if (pos->lexeme != ")") return AddError("Expected ')', but found '", pos->lexeme, "'.");
        ++pos;
        return val;
      }

      // If this is a value, set it and return.
      if (lexer.IsNumber(*pos)) {
        double result = emp::from_string<double>(pos->lexeme);
        ++pos;
        return result;
      }

      // Otherwise it should be and identifier!
      const std::string & name = pos->lexeme;
      ++pos;

      // If it is followed by a parenthesis, it should be a function.
      const bool is_fun = (pos.IsValid() && pos->lexeme == "(");

      if (is_fun) {
        if (!emp::Has(functions, name)) return AddError("Call to unknown function '", name,"'.");
        ++pos;
        emp::vector<ValueType> args;
        while(pos->lexeme != ")") {
          args.push_back(ParseMath(dm, pos));
          if (pos->lexeme == ",") ++pos;
        }
        ++pos;
        
        // Now build the function based on its argument count.
        value_fun_t out_fun;
        switch (args.size()) {
        case 0:
          if (!functions[name].fun0) AddError("Function '", name, "' requires arguments.");
          out_fun = [fun=functions[name].fun0](emp::DataMap & dm) { return fun(); };
          break;
        case 1:
          if (!functions[name].fun1) AddError("Function '", name, "' cannot have 1 arguments.");
          out_fun = [fun=functions[name].fun1,arg0=args[0].AsFun()](emp::DataMap & dm) {
            return fun(arg0(dm));
          };
          break;
        case 2:
          if (!functions[name].fun2) AddError("Function '", name, "' cannot have 2 arguments.");
          out_fun = [fun=functions[name].fun2,arg0=args[0].AsFun(),arg1=args[1].AsFun()](emp::DataMap & dm) {
            return fun(arg0(dm), arg1(dm));
          };
          break;
        case 3:
          if (!functions[name].fun3) AddError("Function '", name, "' cannot have 3 arguments.");
          out_fun = [fun=functions[name].fun3,arg0=args[0].AsFun(),arg1=args[1].AsFun(),arg2=args[2].AsFun()](emp::DataMap & dm) {
            return fun(arg0(dm), arg1(dm), arg2(dm));
          };
          break;
        default:
          AddError("Too many arguments for function '", name, "'.");
        }
        return out_fun;
      }

      // This must be a DataMap entry name.
      if (!dm.HasName(name)) AddError("Unknown data map entry '", name, "'.");
      size_t id = dm.GetID(name);
      return (value_fun_t) [id](emp::DataMap & dm){ return dm.GetAsDouble(id); };
    }

    ValueType ParseMath(const DataMap & dm, pos_t & pos, size_t prec_limit=0) {
      ValueType val1 = ParseValue(dm, pos);

      if constexpr (verbose) {
        if (pos.IsValid()) {
          std::cout << "ParseMath at " << pos.GetIndex() << " : " << pos->lexeme << std::endl;
        } else std::cout << "PROCESSED!" << std::endl;
      }

      while (pos.IsValid() && pos->lexeme != ")" && pos->lexeme != ",") {
        if constexpr (verbose) { std::cout << "...Scanning for op... [" << pos->lexeme << "]" << std::endl;   }

        // If we have an operator, act on it!
        if (Has(binary_ops, pos->lexeme)) {
          const BinaryOperator & op = binary_ops[pos->lexeme];
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

        else AddError("Operator '", pos->lexeme, "' NOT found!");
      }

      // @CAO Make sure there's not a illegal lexeme here.

      return val1;
    }

    /// Parse a function description that will take a DataMap and return the results.
    /// For example, if the string "foo * 2 + bar" is passed in, a function will be returned
    /// that takes a datamap (of the example type) loads in the values of "foo" and "bar", and
    /// returns the result of the above equation.

    value_fun_t BuildMathFunction(const DataMap & dm, const std::string & expression) {
      emp::TokenStream tokens = lexer.Tokenize(expression);
      if constexpr (verbose) tokens.Print();
      pos_t pos = tokens.begin();
      ValueType val = ParseMath(dm, pos);

      // If this value is fixed, turn it into a function.
      if (val.type == ValueType::VALUE) {
        return [out=val.value](emp::DataMap &){ return out; };
      }

      // Otherwise return the function produced.
      #ifdef NDEBUG
        return val.fun;
      #else
        // If we are in debug mode, save the original datamap and double-check compatability.
        return [fun=val.fun,&orig_layout=dm.GetLayout()](emp::DataMap & dm){
          emp_assert(dm.HasLayout(orig_layout));
          return fun(dm);
      };
      #endif
    }

  };
}

#endif
