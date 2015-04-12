#ifndef EMP_WEB_CLASS_H
#define EMP_WEB_CLASS_H

///////////////////////////////////////////////////////////////////////////////////////////
//
//  A CSS class tracking font style, etc.
//

#include <string>

namespace emp {

  class WebClass {
  private:
    std::string font;  // font-family: "Times New Roman", Georgia, Serif;
    double size;       // font-size: 100% (or other)   -- this should be 1.0 for normal
    bool italic;       // font-style: italic (vs. normal)
    bool small_caps;   // font-variant: small-caps;
    bool bold;         // font-weight: bold (vs. normal)
  public:
    WebClass() { ; }
    WebClass(const WebClass &) = default;
    WebClass & operator=(const WebClass &) = default;
  };

};


#endif
