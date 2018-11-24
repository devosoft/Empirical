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
  public:
    virtual Ptr<VarBase> Clone() const = 0;

    virtual std::string GetName() const = 0;
    virtual std::string GetDesc() const = 0;

    virtual size_t GetTypeID() const = 0;        ///< Get the unique type ID for this variable
    virtual std::string GetTypeName() const = 0; ///< Get the (C++ mangled) name for this type

    virtual void SetDefault() = 0;    ///< Restore this variable to its default value.

  };

  template <typename TYPE>
  class VarType : public VarBase {
  public:
    using var_t = TYPE;

    size_t GetTypeID() const override { return GetTypeValue<var_t>(); }    
    std::string GetTypeName() const override { return typeid(base_t).name(); }
  };

  template <typename TYPE, const char * NAME, auto DEFAULT, const char * DESC>
  class VarInfo : public VarType<TYPE> {
  private:
    TYPE value;
  public:
    Ptr<VarBase> Clone() const override { return NewPtr<this_t>(value); }

    std::string GetName() const override { return NAME; }
    std::string GetDesc() const override { return DESC; }

    void SetDefault() override { value = DEFAULT; }
  };


}

#endif