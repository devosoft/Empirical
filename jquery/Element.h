#ifndef EMP_JQ_ELEMENT_H
#define EMP_JQ_ELEMENT_H

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Base class for a single element on a web page (a paragraph, a table, etc.)
//

#include <emscripten.h>
#include <string>

#include "../tools/assert.h"
#include "../tools/string_utils.h"

#include "events.h"

namespace emp {
namespace JQ {

  class Element {
  protected:
    bool initialized;   // Has this element been initialized into the HTML DOM hierarchy.
    bool modified;      // Has this element been modified since the last update? 
    std::string name;

    // Track hiearchy
    Element * parent;
    std::vector<Element *> children;

  public:
    Element(const std::string & in_name, Element * in_parent)
      : initialized(false), modified(true), name(in_name), parent(in_parent)
    {
      emp_assert(name.size() > 0);  // Make sure a name was included.
      // @CAO ensure the name consists of just alphanumeric chars (plus '_' & '-'?)
      Register(this);
    }
    Element(const Element &) = delete;
    virtual ~Element() {
      // Recursively delete children.
      for (Element * cur_element : children) { delete cur_element; }
    }
    Element & operator=(const Element &) = delete;

    // Functions to access current state
    bool IsInitialized() const { return initialized; }
    bool IsModified() const { return modified; }

    virtual bool IsText() const { return false; }

    const std::string GetName() { return name; }
    Element * GetParent() { return parent; }

    // Functions to access children
    int GetNumChildren() const { return children.size(); }
    Element & GetChild(int id) {
      emp_assert(id >= 0 && id < children.size());
      return *(children[id]);
    }
    const Element & GetChild(int id) const {
      emp_assert(id >= 0 && id < children.size());
      return *(children[id]);
    }

    // Function to make current element as modified and pass information up to parents.
    void SetModified() {
      if (modified) return;   // Stop recursion if already not up-to-date.
      modified = true;
      if (parent) parent->SetModified();
    }

    // Register is used to lookup classes by name.  Should exist in classes that manage
    // multiple element; below is the default version.
    virtual bool Register(Element * new_element) {
      if (!parent) return false;  // Top level should always have an override for Register()

      parent->Register(new_element);
      return true;
    }


    // // Add additional children on to this element.
    // Element & Append(const std::string & in_text) {
    //   SetModified();
    //   return *this;
    // }

    // template <typename IN_TYPE>
    // Element & Append(const IN_TYPE & in_text) {
    //   return Append(emp::to_string(in_text));
    // }

    // template <typename IN_TYPE>
    // Element & operator<<(IN_TYPE && in_val) { return Append(std::forward<IN_TYPE>(in_val)); }

    // UpdateNow() refreshes the document immediately (and should only be called if that's okay!)
    virtual void UpdateNow() = 0;  // Overrides must check initialized and set modified to false!

    // Update() refreshes the document as soon as it's ready.
    void Update() {
      std::function<void(void)> update_fun = [this](){ this->UpdateNow(); };
      OnDocumentReady( update_fun );

      // OnDocumentReady( [this](){ this->UpdateNow(); } );
      // OnDocumentReady( std::function<void()> ( std::bind(&Element::UpdateNow, this) ) );
    }

    virtual void PrintHTML(std::ostream & os) = 0;
  };

};
};

#endif
