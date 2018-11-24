/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Var.h
 *  @brief A collection of information about a single, instantiated variable in Empower
 */

#ifndef EMP_VAR_H
#define EMP_VAR_H

#include <string>

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../meta/TypeID.h"

namespace emp {

  class VarBase {
  protected:
    std::string name;
    std::string desc;

  public:
    const std::string GetName() const { return name; }
    const std::string GetDesc() const { return desc; }

    void SetDefault() = 0;
  };

  template <typename TYPE>
  class VarType : public VarBase {
  public:
    using var_t = TYPE;
    
  };

  template <typename TYPE, const char * NAME, auto DEFAULT, const char * DESC>
  class VarInfo : public VarType<TYPE> {
  private:
    TYPE value;
  public:

    void SetDefault() override { value = DEFAULT; }
  };


}

#endif