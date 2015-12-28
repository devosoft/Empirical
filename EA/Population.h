//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2015.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines a Population template for use in evolutionary algorithms.
//
//  MEMBER can be any object type with the following properties:
//  * The copy constructor must work.
//
//  Dervied version of this class (with more complex population structuress)
//  may have additional requirements.


#ifndef EMP_WEB_H
#define EMP_WEB_H

#include "../tools/vector.h"

namespace emp {

  template <typename MEMBER>
  class Population {
  protected:
    emp::vector<MEMBER *> pop;

  public:
    Population() { ; }
    Population(const Population &) = default;
    ~Population() { ; }
    Population & operator=(const Population &) = default;

    int GetSize() const { return (int) pop.size(); }
    MEMBER * operator[](int i) { return pop[i]; }
    
    void Insert(const MEMBER & mem) { pop.push_back(new MEMBER(mem)); }
  };

};

#endif
