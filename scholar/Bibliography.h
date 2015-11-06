// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_BIBLIOGRAPHY_H
#define EMP_BIBLIOGRAPHY_H

//////////////////////////////////////////////////////////////////////////////////////////////
//
//  A collection of references.
//

#include "Citation.h"

namespace emp {

  class Bibliography {
  private:
    std::vector<Citation> refs;
    std::set<std::string> keywords;
  public:
    Bibliography() { ; }
    ~Bibliography() { ; }

    void AddRef(const Citation & in_ref) {
      refs.push_back(in_ref);
    }
  };

};

#endif
