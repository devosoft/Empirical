//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines the Action class, which provides a simple mechanism to abstract
//  functions from their underlying type and provide run-time names.
//
//  Actions can be a bit heavyweight, but can easily be converted to more lightweight
//  std:function objects.

#include <string>

namespace emp {

  class ActionBase {
  private:
    std::string name;
  public:
    const std::string & GetName() { return name; }
    virtual int GetArgCount() const = 0;
  };

  template <int ARG_COUNT>
  class ActionSize : public ActionBase {
  public:
    int GetArgCount() const { return ARG_COUNT; }
  };

  template <typename... ARGS>
  class Action : public ActionSize<sizeof...(ARGS)> {
  };
  
}
