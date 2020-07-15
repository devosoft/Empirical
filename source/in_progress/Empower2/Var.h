/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Var.h
 *  @brief A collection of information about a single, instantiated variable in Empower
 * 
 * 
 *  DEVELOPER NOTES
 *  Pros & Cons vs. Other version of Var being developed
 *    + MUCH simpler code; most details handled inside the Var class
 *    + More extensible memory management is possible where all variables are clustered
 *    - Slightly more overhead since each variable needs to know its type, not just each set of variables.
 *    - Potentially slower at going through different variables in an org since not all clustered.
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
    virtual Ptr<VarBase> Clone() const = 0;      ///< Make a copy of the derived version of this Var.

    virtual std::string GetName() const = 0;     ///< Get the name of this variable
    virtual std::string GetDesc() const = 0;     ///< Get a description of this variable

    virtual size_t GetTypeID() const = 0;        ///< Get the unique type ID for this variable
    virtual std::string GetTypeName() const = 0; ///< Get the (C++ mangled) name for this type

    virtual void SetDefault() = 0;               ///< Restore this variable to its default value.

  };

  /// A version of Var that knows its type.
  template <typename TYPE>
  class VarType : public VarBase {
  protected:
    TYPE value;
  public:
    VarType() { ; }
    VarType(TYPE & in_val) : value(in_val) { ; }

    using var_t = TYPE;

    size_t GetTypeID() const override { return GetTypeValue<var_t>(); }    
    std::string GetTypeName() const override { return typeid(var_t).name(); }
    TYPE & GetValue() { return value; }
    const TYPE & GetValue() const { return value; }
  };

  /// A version of var that knows its name and description.
  template <typename TYPE, typename NAME, typename DESC>
  class VarInfo : public VarType<TYPE> {
  private:
    using this_t = VarInfo<TYPE,NAME,DESC>;
    using parent_t = VarType<TYPE>;
    using parent_t::value;

  public:
    VarInfo() { ; }
    VarInfo(TYPE in_val) : VarType<TYPE>(in_val) { ; }
    VarInfo(const VarInfo &) = default;
    VarInfo(VarInfo &&) = default;

    VarInfo & operator=(const VarInfo &) = default;
    VarInfo & operator=(VarInfo &&) = default;

    Ptr<VarBase> Clone() const override { return NewPtr<this_t>(value); }

    std::string GetName() const override { return NAME.AsString(); }
    std::string GetDesc() const override { return DESC.AsString(); }

    void SetDefault() override { value = TYPE(); }
  };


  class Var {
  private:
    Ptr<VarBase> var_info;

    // MakeVar must be a friend of Var to access its internal constructor.
    template <typename TYPE, auto NAME, auto DESC>
    friend Var MakeVar();

    /// Private constructor for Var where pointer to info is directly provided.
    Var(Ptr<VarBase> _vinfo) : var_info(_vinfo) { ; }

  public:
    Var() : var_info(nullptr) { ; }
    Var(const Var & _in) : var_info(_in.var_info->Clone()) { ; }
    Var(Var && _in) : var_info(_in.var_info) { _in.var_info = nullptr; }

    ~Var() { if (var_info) var_info.Delete(); }

    Var & operator=(const Var & _in) {
      var_info = _in.var_info->Clone();
      return *this;
    }

    Var & operator=(Var && _in) {
      var_info = _in.var_info;
      _in.var_info = nullptr;
      return *this;
    }

    /// Restore a variable to its (non-const) original value.
    template <typename T>
    T & Restore() {
      emp_assert(var_info.IsNull() == false);
      return var_info.Cast<VarType<T>>()->GetValue();
    }

    /// Restore a variable to its (const) original value.
    template <typename T>
    const T & Restore() const {
      emp_assert(var_info.IsNull() == false);
      return var_info.Cast<VarType<T>>()->GetValue();
    }
  };


  template <typename TYPE, typename NAME, typename DESC>
  Var MakeVar() {
    return Var( NewPtr< VarInfo<TYPE,NAME,DEFAULT,DESC> >() );
  }

  // template <typename T>
  // Var MakeVar(std::string name, std::string desc="") {
  //   return Var( NewPtr< VarInfo<T>(name, desc) );
  // }
  // template <typename T, typename DEFAULT_T>
  // Var MakeVar(std::string name, std::string desc, DEFAULT_T default_val) {
  //   return Var( NewPtr< VarInfo<T>(name, desc, default_val) );
  // }

}

#endif