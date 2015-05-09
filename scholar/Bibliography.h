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
