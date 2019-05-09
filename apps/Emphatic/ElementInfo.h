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

  std::string ParamString() const {
    std::string out_str;
    for (size_t i = 0; i < params.size(); i++) {
      if (i) out_str += ", ";
      out_str += emp::to_string(params[i].type, " ", params[i].name);
    }
    return out_str;
  }

  std::string AttributeString() const {
    std::string out_str;
    for (const auto & x : attributes) {
      out_str += " ";
      out_str += x;
    }
    return out_str;
  }

  std::string ArgString() const {
    std::string out_str;
    for (size_t i = 0; i < params.size(); i++) {
      if (i) out_str += ", ";
      out_str += params[i].name;
    }
    return out_str;
  }
};
