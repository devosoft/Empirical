#ifndef EMP_JQ_ELEMENT_VAR_H
#define EMP_JQ_ELEMENT_VAR_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Track a variable in a jquery element tree.
//

#include "Element.h"
#include "Var.h"

namespace emp {
namespace JQ {

  class ElementVar : public Element {
  private:
    Var_Base * var;

  public:
    ElementVar(const std::string & in_name, Element * in_parent, Var_Base & in_var)
      : Element(in_name, in_parent), var(&in_var) { ; }
    ElementVar(const std::string & in_name, Element * in_parent, Var_Base && in_var)
      : Element(in_name, in_parent), var(in_var) { ; }
    ~ElementVar() { ; }

    void UpdateNow() {      
      EM_ASM_ARGS({
          var elem_name = Pointer_stringify($0);
          var text = Pointer_stringify($1);
          $( '#' + elem_name ).html(text);
        }, GetName().c_str(), var.AsString().c_str() );
    }

    virtual void PrintHTML(std::ostream & os) {
      os << " " << var.AsString() << " ";
    }

  };

};
};

#endif
