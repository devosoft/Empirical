#ifndef EMP_JQ_ELEMENT_H
#define EMP_JQ_ELEMENT_H

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Base class for a single element on a web page (a paragraph, a table, etc.)
//

#include <emscripten.h>
#include <string>

#include "../tools/assert.h"

#include "events.h"

namespace emp {
namespace JQ {

  class Element {
  protected:
    bool initialized;
    bool up_to_date;
    std::string name;

    // Track hiearchy
    Element * parent;
    std::vector<Element *> children;

  public:
    Element(const std::string & in_name)
      : initialized(false), up_to_date(false), name(in_name), parent(nullptr)
    {
      emp_assert(name.size() > 0);  // Make sure the name exists!
      // @CAO ensure the name consists of just alphanumeric chars (plus '_' & '-'?)
    }
    Element(const Element &) = delete;
    virtual ~Element() {
      // Recursively delete children.
      for (Element * cur_element : children) { delete cur_element; }
    }
    Element & operator=(const Element &) = delete;

    // Functions to access current state
    bool IsInitialized() const { return initialized; }
    bool IsUpToDate() const { return up_to_date; }
    const std::string GetName() { return name; }
    Element * GetParent() { return parent; }

    // Functions to access children
    int GetNumChildren() const { return children.size(); }
    Element & GetChild(int id) { return *(children[id]); }
    const Element & GetChild(int id) const { return *(children[id]); }

    // Function to make current element as modified and pass information up to parents.
    void SetModified() {
      up_to_date = false;
      if (parent) parent->SetModified();
    }

    // Add additional children on to this element.
    Element & Append(const std::string & in_text) {
      SetModified();
    }

    template <typename IN_TYPE>
    Element & operator<<(IN_TYPE && in_val) { return Append(in_val); }


    // Update() refreshes the document as soon as it's ready.
    void Update() { OnDocumentReady( [this](){ this->UpdateNow(); } ); }

    // UpdateNow() refreshes the document immediately (and should only be called if that's okay!)
    virtual void UpdateNow() { up_to_date = true; }
  };

};
};

#endif
