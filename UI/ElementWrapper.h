#ifndef EMP_UI_ELEMENT_WRAPPER_H
#define EMP_UI_ELEMENT_WRAPPER_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Wrapper to build an specialized HTML element.
//

#include "Element.h"

namespace emp {
namespace UI {

  template <typename BASE_TYPE>
  class ElementWrapper : public Element, public BASE_TYPE {
  private:
    void UpdateHTML() override { HTML.str("");  BASE_TYPE::WriteHTML(HTML); }
    void UpdateCSS() override { BASE_TYPE::TriggerCSS(); }
    void UpdateJS() override { BASE_TYPE::TriggerJS(); }

  protected:
    ElementWrapper(const ElementWrapper & src, Element * parent, const std::string & ext)
      : Element(src, parent, ext), BASE_TYPE(src) { ; }

  public:
    ElementWrapper(const BASE_TYPE & in_info, Element * in_parent)
    : Element(in_info.GetDivID(), in_parent), BASE_TYPE(in_info) { ; }

    virtual Element * Clone(Element * parent, const std::string & ext) const override {
      return new ElementWrapper(*this, parent, ext);
    };

    BASE_TYPE & DivID(const std::string & in_name) { 
      emp_assert(false && "Cannot change div ID after div is created.");
      return *this;
    }

    virtual std::string GetType() override {
      return std::string("Element") + BASE_TYPE::TypeName();
    }

    virtual bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") override {
      bool ok = true;

      if (verbose) {
        ss << prefix << "Scanning: emp::UI::ElementWrapper<" << BASE_TYPE::TypeName()
           << "> with name = '" << name << "'" << std::endl;
      }

      Element::OK(ss, verbose, prefix);  // Check base class; same obj, don't change depth
      // @CAO Make sure the base widget class is okay?

      return ok;
    }
  };

};
};

#endif

