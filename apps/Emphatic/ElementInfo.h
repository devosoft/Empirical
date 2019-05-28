/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ElementInfo.h
 *  @brief Information about C++ elements (variables, functions, typedefs, etc) that are loaded in.
 **/

#include <string>

#include "../../source/base/Ptr.h"
#include "../../source/base/vector.h"
#include "../../source/tools/string_utils.h"

/// Parameter in a function or template definition
struct ParamInfo {
  std::string type;
  std::string name;
};

/// Info for a variable or function
struct ElementInfo {
  enum ElementType { NONE=0, TYPEDEF, VARIABLE, FUNCTION };
  ElementType element_type = NONE;    ///< What type of element are we describing?

  std::string type;                   ///< Type of variable, return type of function, or assigned type of using.
  std::string name;                   ///< Element name.
  emp::vector<ParamInfo> params;      ///< Full set of function parameters
  std::set<std::string> attributes;   ///< const, noexcept, etc.
  std::string default_code;           ///< Variable initialization or function body.
  std::string special_value;          ///< "default", "delete", or "required" (for concepts)

  bool IsTypedef() const { return element_type == TYPEDEF; }
  bool IsVariable() const { return element_type == VARIABLE; }
  bool IsFunction() const { return element_type == FUNCTION; }

  bool IsRequired() const { return special_value == "required"; }
  bool IsDefault() const { return special_value == "default"; }
  bool IsDeleted() const { return special_value == "delete"; }

  void SetTypedef() { element_type = TYPEDEF; }
  void SetVariable() { element_type = VARIABLE; }
  void SetFunction() { element_type = FUNCTION; }

  /// List out all of the parameters for this function.
  std::string ParamString() const {
    emp_assert(IsFunction());
    std::string out_str;
    for (size_t i = 0; i < params.size(); i++) {
      if (i) out_str += ", ";
      out_str += emp::to_string(params[i].type, " ", params[i].name);
    }
    return out_str;
  }

  /// Create fake values for the parameters for this function.
  std::string DeclvalArgString() const {
    emp_assert(IsFunction());
    std::string out_str;
    for (size_t i = 0; i < params.size(); i++) {
      if (i) out_str += ", ";
      out_str += emp::to_string("std::declval<", params[i].type, ">()");
    }
    return out_str;
  }

  /// List out all attributes for this function.
  std::string AttributeString() const {
    emp_assert(IsFunction());
    std::string out_str;
    for (const auto & x : attributes) {
      out_str += " ";
      out_str += x;
    }
    return out_str;
  }

  /// Convert the inputs to a function to arguments to another function.
  std::string ArgString() const {
    emp_assert(IsFunction());
    std::string out_str;
    for (size_t i = 0; i < params.size(); i++) {
      if (i) out_str += ", ";
      out_str += params[i].name;
    }
    return out_str;
  }

  /// Print this element as the Emphatic C++ code that would have generated it.
  void PrintEcho(std::ostream & os, const std::string & prefix) const {
    if (IsTypedef()) {
      os << prefix << "using " << name << " = " << default_code << "\n";
    }
    else if (IsVariable()) {
      os << prefix << type << " " << name;
      if (default_code.size()) os << " = " << default_code << "\n";
      else os << ";\n";
    }
    else if (IsFunction()) {
      os << prefix << type << " " << name << "(" << ParamString() << ") " << AttributeString();
      if (IsRequired()) os << " = required;\n";
      else if (IsDefault()) os << " = default;\n";
      else os << " {\n" << prefix << "  " << default_code << "\n" << prefix << "}\n";
    }
  }

  /// Print this element as the converted C++ code for the base class.
  void PrintConceptBase(std::ostream & os, const std::string & prefix) const {
    // Note: Typedefs and variables do not need to be represented in the base class.
    if (IsFunction()) {
      os << prefix << "virtual " << type << " " << name << "(" << ParamString() << ") "
         << AttributeString() << " = 0;\n";
    }
  }

  /// Print this element as the converted C++ code for the derived class that does reflection.
  void PrintConceptDerived(std::ostream & os, const std::string & prefix) const {
    if (IsTypedef()) {
      // Build type collector.
      os << prefix << "template <typename T> using member_t_" << name
         << " = typename T::" << name << ";\n";

      // Build constexpr HasFun_* to determine if function exists.
      os << prefix << "static constexpr bool HasType_" << name << "() {\n"
         << prefix << "  return emp::test_type<member_t_" << name << ", WRAPPED_T>();\n"
         << prefix << "}\n";

      // Setup a static assert to ensure required types are present.
      if (IsRequired()) {
        os << prefix << "  " << "static_assert( HasType_" << name
                     << "(), \"\\n\\n  ** Error: concept instance missing required type '"
                     << name << "' **\\n\");\n";
        os << prefix << "using " << name << " = typename WRAPPED_T::" << name << ";\n";
      }
      else {
        // Use a typepack with a filter to identify whether we have a wrapped class with
        // the appropriate type defined or do we need to use the default.
        os << prefix << "using " << name << " = "
                     << "typename emp::TypePack<WRAPPED_T>::template wrap<member_t_"
                     << name << ">::template push_back<" << default_code << ">::first_t;\n";
      }
    }
    else if (IsVariable()) {
      os << prefix << type << " " << name;
      if (default_code.size()) os << " " << default_code << "\n";
      else os << ";\n";
    }
    else if (IsFunction()) {
      // Build return-type checker.
      os << prefix << "template <typename T>\n"
         << prefix << "using return_t_" << name
                   << " = decltype( std::declval<T>()." << name
                   << "( " << DeclvalArgString() << " ) );\n";

      // Build constexpr HasFun_* to determine if function exists.
      os << prefix << "static constexpr bool HasFun_" << name << "() {\n"
         << prefix << "  return emp::test_type<return_t_" << name << ", WRAPPED_T>();\n"
         << prefix << "}\n";

      // Build function to call.
      os << prefix << type << " " << name << "(" << ParamString() << ") "
                   << AttributeString() << " {\n";

      // If this is a required function, put static assert to ensure it's there before calling.
      if (IsRequired()) {
        os << prefix << "  " << "static_assert( HasFun_" << name
                     << "(), \"\\n\\n  ** Error: concept instance missing required function '"
                     << name << "' **\\n\");\n";
        if (type != "void") os << prefix << "return ";
        os << "WRAPPED_T::" << name << "( " << ArgString() << " );\n";
      }

      // ...otherwise call the correct version, depending on if it's there.
      else {
        os << prefix << "  if constexpr (HasFun_" << name << "()) {\n"
           << prefix << "    ";
        if (type != "void") os << "return ";
        os << "WRAPPED_T::" << name << "( " << ArgString() << " );\n"
           << prefix << "  }\n"
           << prefix << "  else {\n"
           << prefix << "    " << default_code << "\n"
           << prefix << "  }\n";
      }
      os << prefix << "}\n";
    }
  }};
