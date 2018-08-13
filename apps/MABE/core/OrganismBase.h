/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  OrganismBase.h
 *  @brief Base class for all organisms in MABE
 *
 *  This file details all of the basic functionality that all organisms MUST have, providing
 *  reasonable defaults when such are possible.
 */

#ifndef MABE_ORGANISM_BASE_H
#define MABE_ORGANISM_BASE_H

#include <map>

#include "base/Ptr.h"
#include "base/vector.h"

namespace mabe {

  class OrganismBase {
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
    OrganismBase() { ; }
    virtual ~OrganismBase() { ; }
  };

  template <typename T> auto OrganismBase::FunctionBase::Convert() {
    return (emp::Ptr<OrganismBase::Function<T>>) dynamic_cast< OrganismBase::Function<T> >(this);
  }

}

#endif
