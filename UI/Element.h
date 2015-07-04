#ifndef EMP_UI_ELEMENT_H
#define EMP_UI_ELEMENT_H

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Base class for a single element on a web page (a paragraph, a button, a table, etc.)
//

#include <emscripten.h>
#include <sstream>
#include <string>
#include <typeinfo>

#include "../tools/assert.h"
#include "../tools/string_utils.h"

#include "events.h"
#include "UI_base.h"

#include "Button.h"
#include "Canvas.h"
#include "Image.h"
#include "Table.h"
#include "Text.h"
#include "Slate.h"

namespace emp {
namespace UI {

  class Element {
  protected:
    std::string name;        // Unique DOM id for this element.
    std::stringstream HTML;  // Full HTML contents for this element.

    // Track hiearchy
    Element * parent;
    std::vector<Element *> children;

    // UpdateHTML() makes sure that the HTML stream is up-to-date, CSS is triggered, and
    // any Javascript actions are taken, as per the needs of an element.
    virtual void UpdateHTML() { ; }
    virtual void UpdateCSS() { ; }
    virtual void UpdateJS() { ; }

    bool HasChild(Element * test_child) {
      for (Element * c : children) if (c == test_child) return true;
      return false;
    }

    // If an Append doesn't work with the currnet class, forward it to the parent!
    template <typename FORWARD_TYPE>
    Element & AppendParent(FORWARD_TYPE && arg) {
      emp_assert(parent != nullptr && "Trying to forward append to parent, but no parent!");
      return parent->Append(std::forward<FORWARD_TYPE>(arg));
    }

    // If the current class can't build the requested element, see if its parent can.
    template <typename FORWARD_TYPE>
    Element * ForwardBuild(FORWARD_TYPE && arg, Element * fwd_parent) {
      emp_assert(parent != nullptr && "Trying to forward build to parent, but no parent!");
      return parent->BuildElement(std::forward<FORWARD_TYPE>(arg), fwd_parent);
    }

  public:
    Element(const std::string & in_name, Element * in_parent)
      : name(in_name), parent(in_parent)
    {
      emp_assert(name.size() > 0);  // Make sure a name was included.
      // Ensure the name consists of just alphanumeric chars (plus '_' & '-'?)
      emp_assert( emp::is_valid(name,
                                { emp::is_alphanumeric,
                                  [](char x){return x=='_' || x=='-';}}) );
      Register(this);
    }
    virtual ~Element() {
      // Recursively delete children.
      for (Element * cur_element : children) { delete cur_element; }
    }

    // Do not allow elements to be copied.
    Element(const Element &) = delete;
    Element & operator=(const Element &) = delete;

    // Functions to access current state
    virtual bool IsSlate() const { return false; }
    virtual bool IsTable() const { return false; }
    virtual bool IsText() const { return false; }
    virtual std::string GetWrapperTag() const { return "span"; }

    const std::string & GetName() const { return name; }
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


    // Register is used so we can lookup classes by name.
    // Overridden in classes that manage multiple element; below is the default version.
    virtual void Register(Element * new_element) {
      if (parent) parent->Register(new_element);
    }


    // UpdateNow() refreshes the document immediately (and should only be called if that's okay!)
    // By default: call UpdateHTML (which should be overridden) print HTML_string, and UpdateCSS
    virtual void UpdateNow() {
      UpdateHTML();
      EM_ASM_ARGS({
          var elem_name = Pointer_stringify($0);
          var html_str = Pointer_stringify($1);
          $( '#' + elem_name ).html(html_str);
        }, GetName().c_str(), HTML.str().c_str() );
      UpdateCSS();
      UpdateJS();

      // Now that the parent is up-to-day, update all children.
      for (auto * child : children) child->UpdateNow();
    }


    // Update() refreshes the document once it's ready.
    void Update() {
      // OnDocumentReady( [this](){ this->UpdateNow(); } );
      OnDocumentReady( std::function<void(void)>([this](){ this->UpdateNow(); }) );
    }


    // By default, elements should forward unknown inputs to their parents.
    virtual Element & Append(const std::string & text) { return AppendParent(text); }
    virtual Element & Append(const std::function<std::string()> & fun) { return AppendParent(fun); }
    virtual Element & Append(emp::UI::Button info) { return AppendParent(info); }
    virtual Element & Append(emp::UI::Canvas info) { return AppendParent(info); }
    virtual Element & Append(emp::UI::Image info) { return AppendParent(info); }
    virtual Element & Append(emp::UI::Table info) { return AppendParent(info); }
    virtual Element & Append(emp::UI::Text info) { return AppendParent(info); }
    virtual Element & Append(emp::UI::Slate info) { return AppendParent(info); }

    // Convert arbitrary inputs to a string and try again!
    virtual Element & Append(char in_char) { return Append(emp::to_string(in_char)); }
    virtual Element & Append(double in_num) { return Append(emp::to_string(in_num)); }
    virtual Element & Append(int in_num) { return Append(emp::to_string(in_num)); }

    // Handle special commands
    virtual Element & Append(const emp::UI::Close & close) {
      // See if this is the element we need to close.
      if (GetName() == close.GetID()) {
        emp_assert(parent != nullptr && "Trying to close the outer-most level!");
        return *parent;
      }

      // Otherwise pass the close to the parent!
      return AppendParent(close);
    }

    virtual Element & Append(const emp::UI::GetCell & cell) {
      // GetCell can only be appended to a table class... see if parent is a table.
      return AppendParent(cell);
    }

    virtual Element & Append(const emp::UI::GetRow & row) {
      // GetCell can only be appended to a table class... see if parent is a table.
      return AppendParent(row);
    }

    // Setup << operator to redirect to Append.
    template <typename IN_TYPE>
    Element & operator<<(IN_TYPE && in_val) { return Append(std::forward<IN_TYPE>(in_val)); }


    // BuildElement allows any element to build another as long as one of its ancestors knows how.
    virtual Element * BuildElement(emp::UI::Button info, Element * fwd_parent) {
      return ForwardBuild(info, fwd_parent);
    }
    virtual Element * BuildElement(emp::UI::Canvas info, Element * fwd_parent) {
      return ForwardBuild(info, fwd_parent);
    }
    virtual Element * BuildElement(emp::UI::Image info, Element * fwd_parent) {
      return ForwardBuild(info, fwd_parent);
    }
    virtual Element * BuildElement(emp::UI::Table info, Element * fwd_parent) {
      return ForwardBuild(info, fwd_parent);
    }
    virtual Element * BuildElement(emp::UI::Text info, Element * fwd_parent) {
      return ForwardBuild(info, fwd_parent);
    }
    virtual Element * BuildElement(emp::UI::Slate info, Element * fwd_parent) {
      return ForwardBuild(info, fwd_parent);
    }


    // Print out the contents of this element as HTML.
    virtual void PrintHTML(std::ostream & os) {
      UpdateHTML();
      os << HTML.str();
    }

    void AlertHTML() {
      std::stringstream ss;
      PrintHTML(ss);
      emp::Alert(ss.str());
    }

    
    virtual std::string GetType() = 0;

    // Make sure everything about this element is consistent and logical.
    // ss will include any warnings (conserns, but potentially ok) or errors (serious problems)
    // found.  If verbose is set to true, will also include many additional comments.

    virtual bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
      bool ok = true;
      if (verbose) {
        ss << prefix << "Scanning: emp::UI::Element with name = '" << name << "'" << std::endl;
      }

      if (parent != nullptr && parent->HasChild(this) == false) {
        ss << prefix << "ERROR: this element not listed as child of it's parent!" << std::endl;
        ok = false;
      }

      // Make sure all children are ok (they will report their own errors!
      for (int i = 0; i < (int) children.size(); i++) {
        auto & child = children[i];
        if (child == nullptr) {
          ss << prefix << "ERROR: Child element " << i << " has value 'nullptr'" << std::endl;
          ok = false;
        }
        else if (!child->OK(ss,verbose,prefix+"  ")) ok = false;
      }

      return ok;
    }
  };

};
};

#endif
