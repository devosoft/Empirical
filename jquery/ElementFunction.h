#ifndef EMP_JQ_ELEMENT_FUNCTION_H
#define EMP_JQ_ELEMENT_FUNCTION_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Track a function pointer to call for printing.
//

#include <functional>

#include "Element.h"

namespace emp {
namespace JQ {

  class ElementFunction : public Element {
  private:
    std::function<std::string()> fun;

  public:
    ElementFunction(const std::string & in_name, Element * in_parent,
                    const std::function<std::string()> & in_fun)
      : Element(in_name, in_parent), fun(in_fun) { ; }
    ~ElementFunction() { ; }

    void UpdateNow() {      
      EM_ASM_ARGS({
          var elem_name = Pointer_stringify($0);
          var text = Pointer_stringify($1);
          $( '#' + elem_name ).html(text);
        }, GetName().c_str(), fun().c_str() );
    }

    virtual void PrintHTML(std::ostream & os) {
      os << " " << fun() << " ";
    }

  };

};
};

#endif
