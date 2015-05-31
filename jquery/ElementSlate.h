#ifndef EMP_JQ_ELEMENT_SLATE_H
#define EMP_JQ_ELEMENT_SLATE_H

///////////////////////////////////////////////////////////////////////////////////////////
//
//  Manage a section of the current web page
//

#include <map>
#include <string>

#include "emscripten.h"

#include "../tools/assert.h"
#include "../tools/alert.h"

#include "Element.h"
#include "ElementText.h"

namespace emp {
namespace JQ {

  class ElementSlate : public Element {
  protected:
    std::map<std::string, Element *> element_dict;  // By-name lookup for elements.
    bool initialized;                               // Is element hooked into HTML DOM hierarchy.

    void InitializeChild(Element * child) {
      EM_ASM_ARGS({
          var slate_name = Pointer_stringify($0);
          var elem_name = Pointer_stringify($1);
          $( '#' + slate_name ).append('<span id=\'' + elem_name + '\'></span>');
        }, GetName().c_str(), child->GetName().c_str() );
    }

    // Return a text element for appending, either the current last element or build new one.
    ElementText & GetTextElement() {
      // If the final element is not text, add one.
      if (children.size() == 0 || children.back()->IsText() == false)  {
        std::string new_name = name + std::string("__") + std::to_string(children.size());
        Element * new_child = new ElementText(new_name, this);
        children.push_back(new_child);

        // If this slate is already initialized, we should immediately initialize the child.
        if (initialized) InitializeChild(new_child);
      }
      return *((ElementText *) children.back());
    }
    
    virtual bool Register(Element * new_element) {
      // @CAO Make sure that name is not already used?
      element_dict[new_element->GetName()] = new_element;
      
      // Also register in parent, if available.
      if (parent) parent->Register(new_element);
      
      return true; // Registration successful.
    }
  
public:
    ElementSlate(const std::string & name, Element * in_parent=nullptr)
      : Element(name, in_parent), initialized(false)
    { ; }
    ~ElementSlate() { ; }
    
    // Do not allow Managers to be copied
    ElementSlate(const ElementSlate &) = delete;
    ElementSlate & operator=(const ElementSlate &) = delete;

    bool Contains(const std::string & name) {
      return element_dict.find(name) != element_dict.end();
    }
    Element & FindElement(const std::string & name) {
      emp_assert(Contains(name));
      return *(element_dict[name]);
    }



    // Add additional children on to this element.
    Element & Append(const std::string & in_text) {
      return GetTextElement().Append(in_text);
    }

    Element & Append(const std::function<std::string()> & in_fun) {
      return GetTextElement().Append(in_fun);
    }



    virtual void UpdateNow() {
      if (!initialized) {
        // We can assume that the base div tags have been added already. Expand with contents!
        for (auto * child : children) {
          EM_ASM_ARGS({
              var slate_name = Pointer_stringify($0);
              var elem_name = Pointer_stringify($1);
              $( '#' + slate_name ).append('<span id=\'' + elem_name + '\'></span>');
            }, GetName().c_str(), child->GetName().c_str() );
        }

        initialized = true;

      }

      // Otherwise sub-elements should be in place -- just update them!
      for (auto * child : children) child->UpdateNow();

      modified = false;
    }
    

    virtual void PrintHTML(std::ostream & os) {
      os << "<span id=\"" << name <<"\">\n";
      for (auto * element : children) {
        element->PrintHTML(os);
      }
      os << "</span>\n";
    }

  };

};
};

#endif
