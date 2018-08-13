/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  OrganismTypeBase.h
 *  @brief Base class for all Organsim Types in MABE; describes how organism functions.
 *
 *  This file details all of the basic functionality that all organisms MUST have, providing
 *  reasonable defaults when such are possible.
 * 
 *  Every organism type must define an internal type 'Organism' that determines the type of each
 *  individual org.
 */

#ifndef MABE_ORGANISM_TYPE_BASE_H
#define MABE_ORGANISM_TYPE_BASE_H

#include <map>

#include "base/Ptr.h"
#include "base/vector.h"

#include "ModuleBase.h"

namespace mabe {

  class OrganismTypeBase : public ModuleBase {
  protected:
    struct FunctionBase { template <typename T> auto Convert(); };
    template <typename T> struct Function : public FunctionBase {
      std::function<T> fun;
      Function(std::function<T> _fun) : fun(_fun) { ; }
    };

    using fun_ptr_t = emp::Ptr<FunctionBase>;
    using fun_vec_t = emp::vector<fun_ptr_t>;
    std::map< std::string, fun_vec_t > fun_map;

  public:
    OrganismTypeBase(const std::string & in_name) : ModuleBase(in_name) { ; }    

    static constexpr mabe::ModuleType GetModuleType() { return ModuleType::ORGANISM_TYPE; }
  };

  template <typename T> auto OrganismTypeBase::FunctionBase::Convert() {
    return (emp::Ptr<OrganismTypeBase::Function<T>>) dynamic_cast< OrganismTypeBase::Function<T> >(this);
  }
}

#endif

