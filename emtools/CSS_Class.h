#ifndef EMP_JQ_CLASS_H
#define EMP_JQ_CLASS_H

///////////////////////////////////////////////////////////////////////////////////////////
//
//  A CSS class tracking font style, etc.
//

#include <string>

namespace emp {

  class JQClass {
  private:
    std::string font;  // font-family: "Times New Roman", Georgia, Serif;
    double size;       // font-size: 100% (or other)   -- this should be 1.0 for normal
    bool italic;       // font-style: italic (vs. normal)
    bool small_caps;   // font-variant: small-caps;
    bool bold;         // font-weight: bold (vs. normal)
  public:
    JQClass() { ; }
    JQClass(const JQClass &) = default;
    JQClass & operator=(const JQClass &) = default;
  };

};


#endif
