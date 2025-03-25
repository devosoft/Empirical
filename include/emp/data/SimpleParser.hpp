/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2021-2024
*/
/**
 *  @file
 *  @brief Parser to convert function descriptions to lambdas using maps for variable lookup.
 *  @note Status: ALPHA
 *
 *  A fully functional parser that will convert a string-description of a function to a C++
 *  lambda.  A map-typed object should be passed in to provide values associated with variables.
 *  Allowed map types include std::map<emp::String,T>, std::unordered_map<emp::String,T>,
 *  emp::DataMap, and (soon) derivations from emp::AnnotatedType.  For standard maps, T must be
 *  convertible to emp::Datum.
 *
 *  Developer TODO:
 *  - Setup operator RegEx to be built dynamically
 *  - Setup LVALUES as a type, and allow assignment
 *  - Allow types other than Datum (string and double)?
 */

#ifndef EMP_DATA_SIMPLEPARSER_HPP_INCLUDE
#define EMP_DATA_SIMPLEPARSER_HPP_INCLUDE

#include <cmath>
#include <map>
#include <string>
#include <unordered_map>

#include "../base/notify.hpp"
#include "../compiler/Lexer.hpp"
#include "../compiler/regex_utils.hpp"
#include "../data/Datum.hpp"
#include "../datastructs/ra_map.hpp"
#include "../math/Random.hpp"
#include "../meta/meta.hpp"
#include "../tools/String.hpp"

#include "AnnotatedType.hpp"
#include "DataMap.hpp"

namespace emp {

  class SimpleParser {
  private:

    template <typename ARG_T>
    struct ValueType {
      using fun_t = std::function<emp::Datum(ARG_T)>;
      enum type_t { ERROR=0, VALUE, FUNCTION };

      type_t type = ERROR;
      emp::Datum value;
      fun_t fun;

      ValueType() = default;
      ValueType(const ValueType &) = default;
      ValueType(ValueType &&) = default;
      ValueType(double in_val) : type(VALUE), value(in_val) { }
      ValueType(emp::String in_val) : type(VALUE), value(in_val) { }
      ValueType(emp::Datum in_val) : type(VALUE), value(in_val) { }
      ValueType(fun_t in_fun) : type(FUNCTION), fun(in_fun) { }

      ValueType & operator=(const ValueType &) = default;
      ValueType & operator=(emp::Datum in_val) { type = VALUE; value = in_val; return *this; }
      ValueType & operator=(double in_val) { type = VALUE; value = in_val; return *this; }
      ValueType & operator=(const emp::String & in_val) { type = VALUE; value = in_val; return *this; }
      ValueType & operator=(fun_t in_fun) { type = FUNCTION; fun = in_fun; return *this; }

      fun_t AsFunction() {
        if (type==FUNCTION) return fun;
        else return [v=value](ARG_T){ return v; };
      }
    };

    template <typename MAP_T, typename DUMMY_T=int>
    struct SymbolTable {
      using arg_t = const MAP_T &;
      using fun_t = std::function<emp::Datum(arg_t)>;
      using value_t = ValueType<arg_t>;

      SymbolTable() = default;
      SymbolTable(arg_t) { }

      static_assert( std::same_as<typename MAP_T::key_type, emp::String>,
                    "Any map type used by the parser must have a key type of emp::String");

      static fun_t MakeDatumAccessor(const emp::String & name) {
        return [name](arg_t symbol_vals){
          auto val_it = symbol_vals.find(name);
          emp_assert(val_it != symbol_vals.end());
          return emp::Datum(val_it->second);
        };
      }

      /// By default, let the value handle its own conversion to a function.
      auto AsFunction(ValueType<arg_t> & val) const { return val.AsFunction(); }
    };

    template <typename VALUE_T, typename DUMMY_T>
    struct SymbolTable<emp::ra_map<emp::String,VALUE_T>, DUMMY_T> {
      using map_t = emp::ra_map<emp::String,VALUE_T>;
      using arg_t = const map_t &;
      using fun_t = std::function<emp::Datum(arg_t)>;
      using value_t = ValueType<arg_t>;

      const typename map_t::layout_t & layout;

      SymbolTable(const emp::ra_map<emp::String,VALUE_T> & in_map)
        : layout(in_map.GetLayout()) { }

      fun_t MakeDatumAccessor(const emp::String & name) const {
        emp_assert(layout.find(name) != layout.end());
        size_t id = layout.find(name)->second;
        #ifdef NDEBUG
        return [id](arg_t symbol_vals){
        #else
        return [id,name](arg_t symbol_vals){          // Keep name in debug mode to check id.
          emp_assert(symbol_vals.GetID(name) == id);
        #endif
          return emp::Datum(symbol_vals.AtID(id));
        };
      }

      /// By default, let the value handle its own conversion to a function.
      auto AsFunction(ValueType<arg_t> & val) const {
        // @CAO: Could check layout correctness in debug mode.
        return val.AsFunction();
      }
    };

    /// Specialty implementation for DataLayouts.
    template <typename DUMMY_T>
    struct SymbolTable<emp::DataLayout, DUMMY_T> {
      using arg_t = const emp::DataMap &;
      using fun_t = std::function<emp::Datum(arg_t)>;
      using value_t = ValueType<arg_t>;

      const emp::DataLayout & layout;

      SymbolTable(const emp::DataLayout & in_layout) : layout(in_layout) { }

      auto MakeDatumAccessor(const emp::String & name) const {
        return emp::DataMap::MakeDatumAccessor(layout, name);
      }

      auto AsFunction(ValueType<arg_t> & val) const {
        #ifdef NDEBUG
          return val.AsFunction();
        #else
          // If we are in debug mode, add wrapper to ensure DataMap with has correct layout.
          return [fun=val.AsFunction(),layout_ptr=&layout](arg_t dm) {
            emp_assert(dm.HasLayout(*layout_ptr));
            return fun(dm);
          };
        #endif
      }

    };

    /// Special DataMap implementation that just converts to underlying layout.
    template <typename DUMMY_T>
    struct SymbolTable<emp::DataMap, DUMMY_T> : public SymbolTable<emp::DataLayout> {
      SymbolTable(const emp::DataMap & dm) : SymbolTable<emp::DataLayout>(dm.GetLayout()) { }
    };


    using pos_t = emp::TokenStream::Iterator;

    static constexpr const bool verbose = false;

    class MapLexer : public emp::Lexer {
    private:
      int token_identifier;   ///< Token id for identifiers
      int token_number;       ///< Token id for literal numbers
      int token_string;       ///< Token id for literal strings
      int token_char;         ///< Token id for literal characters
      int token_external;     ///< Token id for an external value that was passed in
      int token_symbol;       ///< Token id for other symbols

    public:
      MapLexer() {
        // Whitespace and comments should always be dismissed (top priority)
        IgnoreToken("Whitespace", "[ \t\n\r]+");
        IgnoreToken("//-Comments", "//.*");
        IgnoreToken("/*...*/-Comments", "/[*]([^*]|([*]+[^*/]))*[*]+/");

        // Meaningful tokens have next priority.

        // An identifier must begin with a letter, underscore, or dot, and followed by
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

        // An external value that was passed in will be a dollar sign ('$') followed by the
        // position of the value to be used (e.g., '$3').
        token_external = AddToken("External Value", "[$][0-9]+");

        // Symbols should have least priority.  They include any solitary character not listed
        // above, or pre-specified multi-character groups.
        token_symbol = AddToken("Symbol", ".|\"==\"|\"!=\"|\"<=\"|\">=\"|\"~==\"|\"~!=\"|\"~<\"|\"~>\"|\"~<=\"|\"~>=\"|\"&&\"|\"||\"|\"**\"|\"%%\"");
      }

      bool IsID(const emp::Token & token) const noexcept { return token.id == token_identifier; }
      bool IsNumber(const emp::Token & token) const noexcept { return token.id == token_number; }
      bool IsString(const emp::Token & token) const noexcept { return token.id == token_string; }
      bool IsChar(const emp::Token & token) const noexcept { return token.id == token_char; }
      bool IsExternal(const emp::Token & token) const noexcept { return token.id == token_external; }
      bool IsSymbol(const emp::Token & token) const noexcept { return token.id == token_symbol; }
    };

    struct BinaryOperator {
      using fun_t = std::function<emp::Datum(emp::Datum,emp::Datum)>;
      size_t prec;
      fun_t fun;
      void Set(size_t in_prec, fun_t in_fun) { prec = in_prec; fun = in_fun; }
    };

    struct Function {
      using fun0_t = std::function<emp::Datum()>;
      using fun1_t = std::function<emp::Datum(emp::Datum)>;
      using fun2_t = std::function<emp::Datum(emp::Datum,emp::Datum)>;
      using fun3_t = std::function<emp::Datum(emp::Datum,emp::Datum,emp::Datum)>;

      size_t num_args = 0;
      fun0_t fun0; fun1_t fun1; fun2_t fun2; fun3_t fun3;

      void Set0(fun0_t in_fun) { num_args = 0; fun0 = in_fun; }
      void Set1(fun1_t in_fun) { num_args = 1; fun1 = in_fun; }
      void Set2(fun2_t in_fun) { num_args = 2; fun2 = in_fun; }
      void Set3(fun3_t in_fun) { num_args = 3; fun3 = in_fun; }
    };

    // --------- MEMBER VARIABLES -----------
    MapLexer lexer;

    // Operators and functions that should be used when parsing.
    std::unordered_map<emp::String, std::function<emp::Datum(emp::Datum)>> unary_ops;
    std::unordered_map<emp::String, BinaryOperator> binary_ops;
    std::unordered_map<emp::String, Function> functions;
    emp::vector<emp::Datum> external_vals;

    // The set of data map entries accessed when the last function was parsed.
    std::set<emp::String> var_names;

    // Track the number of errors and the function to call when errors occur.
    template<typename... Ts>
    size_t ParseError(Ts &&... args) {
      emp::notify::Exception("SimpleParser::PARSE_ERROR", emp::to_string(args...), this);
      return 1;
    }

  public:
    SimpleParser(bool use_defaults=true) {
      if (use_defaults) {
        AddDefaultOperators();
        AddDefaultFunctions();
      }
    }

    /// Construct with a random number generator to automatically include random functions.
    SimpleParser(bool use_defaults, emp::Random & random) : SimpleParser(use_defaults)
    { AddRandomFunctions(random); }

    /// Get the set of variable names that the most recently generated function used.
    const std::set<emp::String> & GetNamesUsed() const { return var_names; }

    /// Get the set of names used in the provided equation.
    const std::set<emp::String> & GetNamesUsed(const emp::String & expression) {
      var_names.clear();
      emp::TokenStream tokens = lexer.Tokenize(expression, emp::String("Expression: ") + expression);
      for (emp::Token token : tokens) {
        if (lexer.IsID(token) && !emp::Has(functions, token.lexeme)) {
          var_names.insert(token.lexeme);
        }
      }
      return var_names;
    }


    /// Add a unary operator
    void AddOp(const emp::String & op, std::function<emp::Datum(emp::Datum)> fun) {
      unary_ops[op] = fun;
    }

    /// Add a binary operator
    void AddOp(const emp::String & op, size_t prec,
               std::function<emp::Datum(emp::Datum,emp::Datum)> fun) {
      binary_ops[op].Set(prec, fun);
    }


    static int ApproxCompare(double x, double y) {
      static constexpr double APPROX_FRACTION = 8192.0;
      double margin = y / APPROX_FRACTION;
      if (x < y - margin) return -1;
      if (x > y + margin) return 1;
      return 0;
    }

    void AddDefaultOperators() {
      // Setup the unary operators for the parser.
      AddOp("+", [](emp::Datum x) { return x; });
      AddOp("-", [](emp::Datum x) { return -x; });
      AddOp("!", [](emp::Datum x) { return !x; });


      // Setup the default binary operators for the parser.
      size_t prec = 0;  // Precedence level of each operator...
      AddOp("||", ++prec, [](emp::Datum x, emp::Datum y){ return (x!=0.0)||(y!=0.0); } );
      AddOp("&&", ++prec, [](emp::Datum x, emp::Datum y){ return (x!=0.0)&&(y!=0.0); } );
      AddOp("==", ++prec, [](emp::Datum x, emp::Datum y){ return x == y; } );
      AddOp("!=",   prec, [](emp::Datum x, emp::Datum y){ return x != y; } );
      AddOp("~==",  prec, [](emp::Datum x, emp::Datum y){ return ApproxCompare(x,y) == 0; } );
      AddOp("~!=",  prec, [](emp::Datum x, emp::Datum y){ return ApproxCompare(x,y) != 0; } );
      AddOp("<",  ++prec, [](emp::Datum x, emp::Datum y){ return x < y; } );
      AddOp("<=",   prec, [](emp::Datum x, emp::Datum y){ return x <= y; } );
      AddOp(">",    prec, [](emp::Datum x, emp::Datum y){ return x > y; } );
      AddOp(">=",   prec, [](emp::Datum x, emp::Datum y){ return x >= y; } );
      AddOp("~<",   prec, [](emp::Datum x, emp::Datum y){ return ApproxCompare(x,y) == -1; } );
      AddOp("~<=",  prec, [](emp::Datum x, emp::Datum y){ return ApproxCompare(x,y) != 1; } );
      AddOp("~>",   prec, [](emp::Datum x, emp::Datum y){ return ApproxCompare(x,y) == 1; } );
      AddOp("~>=",  prec, [](emp::Datum x, emp::Datum y){ return ApproxCompare(x,y) != -1; } );
      AddOp("+",  ++prec, [](emp::Datum x, emp::Datum y){ return x + y; } );
      AddOp("-",    prec, [](emp::Datum x, emp::Datum y){ return x - y; } );
      AddOp("*",  ++prec, [](emp::Datum x, emp::Datum y){ return x * y; } );
      AddOp("/",    prec, [](emp::Datum x, emp::Datum y){ return x / y; } );
      AddOp("%",    prec, [](emp::Datum x, emp::Datum y){ return emp::Mod(x, y); } );
      AddOp("**", ++prec, [](emp::Datum x, emp::Datum y){ return emp::Pow(x, y); } );
      AddOp("%%",   prec, [](emp::Datum x, emp::Datum y){ return emp::Log(x, y); } );
    }

    void AddDefaultFunctions() {
      // Setup the default functions.
      functions["ABS"].Set1( [](emp::Datum x){ return std::abs(x); } );
      functions["EXP"].Set1( [](emp::Datum x){ return emp::Pow(emp::E, x); } );
      functions["LOG"].Set1( [](emp::Datum x){ return std::log(x); } );
      functions["LOG2"].Set1( [](emp::Datum x){ return std::log2(x); } );
      functions["LOG10"].Set1( [](emp::Datum x){ return std::log10(x); } );

      functions["SQRT"].Set1( [](emp::Datum x){ return std::sqrt(x); } );
      functions["CBRT"].Set1( [](emp::Datum x){ return std::cbrt(x); } );

      functions["SIN"].Set1( [](emp::Datum x){ return std::sin(x); } );
      functions["COS"].Set1( [](emp::Datum x){ return std::cos(x); } );
      functions["TAN"].Set1( [](emp::Datum x){ return std::tan(x); } );
      functions["ASIN"].Set1( [](emp::Datum x){ return std::asin(x); } );
      functions["ACOS"].Set1( [](emp::Datum x){ return std::acos(x); } );
      functions["ATAN"].Set1( [](emp::Datum x){ return std::atan(x); } );
      functions["SINH"].Set1( [](emp::Datum x){ return std::sinh(x); } );
      functions["COSH"].Set1( [](emp::Datum x){ return std::cosh(x); } );
      functions["TANH"].Set1( [](emp::Datum x){ return std::tanh(x); } );
      functions["ASINH"].Set1( [](emp::Datum x){ return std::asinh(x); } );
      functions["ACOSH"].Set1( [](emp::Datum x){ return std::acosh(x); } );
      functions["ATANH"].Set1( [](emp::Datum x){ return std::atanh(x); } );

      functions["CEIL"].Set1( [](emp::Datum x){ return std::ceil(x); } );
      functions["FLOOR"].Set1( [](emp::Datum x){ return std::floor(x); } );
      functions["ROUND"].Set1( [](emp::Datum x){ return std::round(x); } );

      functions["ISINF"].Set1( [](emp::Datum x){ return std::isinf(x); } );
      functions["ISNAN"].Set1( [](emp::Datum x){ return std::isnan(x); } );

      // Default 2-input functions
      functions["HYPOT"].Set2( [](emp::Datum x, emp::Datum y){ return std::hypot(x,y); } );
      functions["EXP"].Set2( [](emp::Datum x, emp::Datum y){ return emp::Pow(x,y); } );
      functions["LOG"].Set2( [](emp::Datum x, emp::Datum y){ return emp::Log(x,y); } );
      functions["MIN"].Set2( [](emp::Datum x, emp::Datum y){ return (x<y) ? x : y; } );
      functions["MAX"].Set2( [](emp::Datum x, emp::Datum y){ return (x>y) ? x : y; } );
      functions["POW"].Set2( [](emp::Datum x, emp::Datum y){ return emp::Pow(x,y); } );

      // Default 3-input functions.
      functions["IF"].Set3( [](emp::Datum x, emp::Datum y, emp::Datum z){
        return (x!=0.0) ? y : z;
      } );
      functions["CLAMP"].Set3( [](emp::Datum x, emp::Datum y, emp::Datum z){
        return (x<y) ? y : (x>z) ? z : x;
      } );
      functions["TO_SCALE"].Set3( [](emp::Datum x, emp::Datum y, emp::Datum z){
        return (z-y)*x+y;
      } );
      functions["FROM_SCALE"].Set3( [](emp::Datum x, emp::Datum y, emp::Datum z){
        return (x-y) / (z-y);
      } );
    }

    void AddRandomFunctions(Random & random) {
      functions["RAND"].Set0( [&random](){ return random.GetDouble(); } );
      functions["RAND"].Set1( [&random](emp::Datum x){ return random.GetDouble(x); } );
      functions["RAND"].Set2( [&random](emp::Datum x, emp::Datum y){ return random.GetDouble(x,y); } );
    }

    /// Helpers for parsing.
    template <typename SYMBOLS_T>
    typename SYMBOLS_T::value_t ParseValue(const SYMBOLS_T & symbols, pos_t & pos) {
      if constexpr (verbose) {
        std::cout << "ParseValue at position " << pos.GetIndex() << " : " << pos->lexeme << std::endl;
      }

      using arg_t = typename SYMBOLS_T::arg_t;
      using fun_t = typename SYMBOLS_T::fun_t;
      using value_t = typename SYMBOLS_T::value_t;

      // Deal with any unary operators...
      if (emp::Has(unary_ops, pos->lexeme)) {
        if constexpr (verbose) std::cout << "Found UNARY OP: " << pos->lexeme << std::endl;
        auto op = unary_ops[pos->lexeme];
        ++pos;
        value_t val = ParseValue(symbols, pos);
        if (val.type == value_t::VALUE) { return op(val.value); }
        else {
          return static_cast<typename value_t::fun_t>(
            [fun=val.fun,op](arg_t arg){ return op(fun(arg)); }
          );
        }
      }

      // If we have parentheses, process the contents
      if (pos->lexeme == "(") {
        if constexpr (verbose) std::cout << "Found: OPEN PAREN" << std::endl;
        ++pos;
        value_t val = ParseMath(symbols, pos);
        if (pos->lexeme != ")") return ParseError("Expected ')', but found '", pos->lexeme, "'.");
        ++pos;
        return val;
      }

      // If this is a value, set it and return.
      if (lexer.IsNumber(*pos)) {
        double result = emp::from_string<double>(pos->lexeme);
        ++pos;
        return result;
      }

      // Similar for an external value
      if (lexer.IsExternal(*pos)) {
        size_t id = emp::from_string<size_t>(pos->lexeme.substr(1));
        ++pos;
        if (id >= external_vals.size()) {
          ParseError("Invalid access into external variable (\"$", id, "\"): Does not exist.");
        }
        return external_vals[id];
      }

      // Otherwise it should be and identifier!
      const emp::String & name = pos->lexeme;
      ++pos;

      // If it is followed by a parenthesis, it should be a function.
      const bool is_fun = (pos.IsValid() && pos->lexeme == "(");

      if (is_fun) {
        if (!emp::Has(functions, name)) return ParseError("Call to unknown function '", name,"'.");
        ++pos;
        emp::vector<value_t> args;
        while(pos->lexeme != ")") {
          args.push_back(ParseMath(symbols, pos));
          if (pos->lexeme == ",") ++pos;
        }
        ++pos;

        // Now build the function based on its argument count.
        fun_t out_fun;
        switch (args.size()) {
        case 0:
          if (!functions[name].fun0) ParseError("Function '", name, "' requires arguments.");
          out_fun = [fun=functions[name].fun0](arg_t /*sym_arg*/) { return fun(); };
          break;
        case 1:
          if (!functions[name].fun1) ParseError("Function '", name, "' cannot have 1 arguments.");
          out_fun = [fun=functions[name].fun1,arg0=args[0].AsFunction()](arg_t sym_arg) {
            return fun(arg0(sym_arg));
          };
          break;
        case 2:
          if (!functions[name].fun2) ParseError("Function '", name, "' cannot have 2 arguments.");
          out_fun = [fun=functions[name].fun2,
                     arg0=args[0].AsFunction(),
                     arg1=args[1].AsFunction()](arg_t sym_arg) {
            return fun(arg0(sym_arg), arg1(sym_arg));
          };
          break;
        case 3:
          if (!functions[name].fun3) ParseError("Function '", name, "' cannot have 3 arguments.");
          out_fun = [fun=functions[name].fun3,
                     arg0=args[0].AsFunction(),
                     arg1=args[1].AsFunction(),
                     arg2=args[2].AsFunction()](arg_t sym_arg) {
            return fun(arg0(sym_arg), arg1(sym_arg), arg2(sym_arg));
          };
          break;
        default:
          ParseError("Too many arguments (", args.size(), ") for function '", name, "'.");
        }
        return out_fun;
      }

      var_names.insert(name);                 // Store this name in the list of those used.
      return symbols.MakeDatumAccessor(name); // Return an accessor for this name.
    }

    template <typename SYMBOLS_T>
    typename SYMBOLS_T::value_t ParseMath(const SYMBOLS_T & symbols, pos_t & pos, size_t prec_limit=0) {
      using value_t = typename SYMBOLS_T::value_t;
      using arg_t = typename SYMBOLS_T::arg_t;
      value_t val1 = ParseValue(symbols, pos);

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
          if (prec_limit >= op.prec) return val1; // Precedence not allowed; return current value.
          ++pos;
          value_t val2 = ParseMath(symbols, pos, op.prec);
          if (val1.type == value_t::VALUE) {
            if (val2.type == value_t::VALUE) { val1 = op.fun(val1.value, val2.value); }
            else {
              val1 = [val1_num=val1.value,val2_fun=val2.fun,op_fun=op.fun](arg_t symbol_vals){
                return op_fun(val1_num, val2_fun(symbol_vals));
              };
            }
          } else {
            if (val2.type == value_t::VALUE) {
              val1 = [val1_fun=val1.fun,val2_num=val2.value,op_fun=op.fun](arg_t symbol_vals){
                return op_fun(val1_fun(symbol_vals), val2_num);
              };
            } else {
              val1 = [val1_fun=val1.fun,val2_fun=val2.fun,op_fun=op.fun](arg_t symbol_vals){
                return op_fun(val1_fun(symbol_vals), val2_fun(symbol_vals));
              };
            }
          }
        }

        else ParseError("Operator '", pos->lexeme, "' NOT found!");
      }

      // @CAO Make sure there's not a illegal lexeme here.

      return val1;
    }

    /// Take a set of variables and use them to replace $0, $1, etc. in any function.
    template <typename T1, typename... Ts>
    void SetupStaticValues(T1 arg1, Ts... args) {
      // If we have a vector of incoming values, make sure it is valid and then just pass it along.
      if constexpr (sizeof...(Ts) == 0 && (emp::is_emp_vector<T1>() || emp::is_std_vector<T1>())) {
        using value_t = typename T1::value_type;
        static_assert(std::is_same<value_t, emp::Datum>(),
          "If BuildMathFunction is provided a vector, it must contain only emp::Datum.");
        external_vals = arg1;
        return;
      }

      else {
        // Otherwise convert all args to emp::Datum.
        external_vals = emp::vector<emp::Datum>{
          static_cast<emp::Datum>(arg1),
          static_cast<emp::Datum>(args)...
        };
      }
    }

    /// If there are no input args, just clear external values.
    void SetupStaticValues() { external_vals.resize(0); }

    /// Parse a function description that will take a map and return the results.
    /// For example, if the string "foo * 2 + bar" is passed in, a function will be returned
    /// that takes a map (of the proper type) loads in the values of "foo" and "bar", and
    /// returns the result of the above equation.

    template <typename MAP_T, typename... EXTRA_Ts>
    auto BuildMathFunction(
      const MAP_T & symbol_map,   ///< The map or layout to use, specifying variables.
      std::string_view expression,     ///< The primary expression to convert.
      EXTRA_Ts... extra_args      ///< Extra value arguments (accessed as $1, $2, etc.)
    ) {
      // If we have incoming values, store them appropriately.
      SetupStaticValues(extra_args...);

      using value_t = typename SymbolTable<MAP_T>::value_t;
      SymbolTable<MAP_T> symbol_table(symbol_map);

      // Tokenize the expression.
      emp::TokenStream tokens = lexer.Tokenize(expression, emp::String("Expression: ") + expression);
      if constexpr (verbose) tokens.Print();
      var_names.clear();    // Reset the names used from data map.
      pos_t pos = tokens.begin();
      value_t val = ParseMath(symbol_table, pos);

      // Return the value as a function.
      return symbol_table.AsFunction(val);
    }


    /// Generate a temporary math function and immediately run it on the provided arguments.
    /// @param symbol_map The map containing the required variables.
    /// @param expression The mathematical expression to be run on the data map.
    /// @param extras Any extra values to fill in a $0, $1, etc.
    template <typename MAP_T, typename... ARG_Ts>
    emp::Datum RunMathFunction(const MAP_T & symbol_map, ARG_Ts... args) {
      auto fun = BuildMathFunction(symbol_map, std::forward<ARG_Ts>(args)...);
      return fun(symbol_map);
    }

  };

}

#endif // #ifndef EMP_DATA_SIMPLEPARSER_HPP_INCLUDE
