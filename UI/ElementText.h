#ifndef EMP_UI_ELEMENT_TEXT_H
#define EMP_UI_ELEMENT_TEXT_H

///////////////////////////////////////////////////////////////////////////////////////////
//
//  Manage a section of the current web page
//

#include <functional>
#include <string>
#include <vector>

#include <emscripten.h>

#include "../tools/DynamicStringSet.h"

#include "Element.h"

namespace emp {
namespace UI {

  class ElementText : public Element, public Text {
  private:
    DynamicStringSet strings;

    void UpdateHTML() {
      HTML.str("");       // Clear the current text.
      HTML << strings;    // Save the current value of the strings.
    }
    void UpdateCSS() {
      TriggerCSS();
    }

  public:
    ElementText(const Text & in_text, Element * in_parent)
      : Element(in_text.GetDivID(), in_parent), Text(in_text) { ; }
    ~ElementText() { ; }

    // Do not allow Managers to be copied
    ElementText(const ElementText &) = delete;
    ElementText & operator=(const ElementText &) = delete;

    virtual bool IsText() const override { return true; }

    void ClearText() { strings.Clear(); }

    Element & Append(const std::string & in_text) override {
      strings.Append(in_text);
      return *this;
    }

    Element & Append(const std::function<std::string()> & in_fun) override {
      strings.Append(in_fun);
      return *this;
    }

    virtual std::string GetType() {
      return "ElementText";
    }

    virtual bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
      bool ok = true;
      if (verbose) {
        ss << prefix << "Scanning: emp::ElementText with name = '" << name << "'" << std::endl;
      }

      Element::OK(ss, verbose, prefix);  // Check base class; same obj, don't change prefix
      // @CAO Make sure the DynamicStringSet is okay?

      return ok;
    }

  };

};
};

#endif
