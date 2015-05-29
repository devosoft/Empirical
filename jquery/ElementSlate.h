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
    std::map<std::string, Element *> element_dict;
    std::string end_tag;

    bool text_avail;

    ElementText & GetFinalText() {
      // If the final element is not text, add one.
      if (text_avail == false) {
        std::string new_name = name + std::string("__") + std::to_string(children.size());
        children.push_back(new ElementText(new_name, this));
        text_avail = true;
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
      : Element(name, in_parent)
      , end_tag(name + std::string("__end"))
      , text_avail(false)
    {
      // Setup an end-tag so that we can always track the end of this managed space.
   // EM_ASM_ARGS({
   //     var name = Pointer_stringify($0);
   //     var e_tag = Pointer_stringify($1);
   //     $( document ).ready(function() {
   //         $( '#' + name ).after('<div id=\'' + e_tag + '\'></div>');
   //       });
   //   }, name.c_str(), end_tag.c_str());;
    }
    ~ElementSlate() { ; }

    // Do not allow Managers to be copied
    ElementSlate(const ElementSlate &) = delete;
    ElementSlate & operator=(const ElementSlate &) = delete;

    bool Contains(const std::string & name) {
      return element_dict.find(name) != element_dict.end();
    }
    Element & GetElement(const std::string & name) {
      emp_assert(Contains(name));
      return *(element_dict[name]);
    }

    // Add additional children on to this element.
    ElementSlate & Append(const std::string & in_text) {
      GetFinalText().Append(in_text);
      SetModified();
      return *this;
    }

    template <typename IN_TYPE>
    ElementSlate & Append(const IN_TYPE & in_text) {
      return Append(emp::to_string(in_text));
    }

    template <typename IN_TYPE>
    ElementSlate & operator<<(IN_TYPE && in_val) { return Append(std::forward<IN_TYPE>(in_val)); }

    //   element_map[in_element.GetName()] = &in_element;
    //   EM_ASM_ARGS({
    //       var manager_name = Pointer_stringify($0);
    //       var el_tag = Pointer_stringify($1);
    //       $( document ).ready(function() {
    //           // $( '#' + manager_name ).after('<div id=\'' + el_tag '\'>' + el_text + '</div>');
    //           $( '#' + manager_name ).after('tag=' + el_tag);
    //           $( '#' + manager_name ).after('manager=' + manager_name + '<br>');
    //           // $( '#' + manager_name ).after('<div id=\'' + el_tag + '\'></div>');
    //           $('<div id=\'' + el_tag + '\'>TEST!!!!</div>').appendTo( '#' + manager_name );
    //           $( '#' + el_tag ).html( 'YAT (Yet Another Test)' );
    //         });
    //     }, GetName().c_str(), in_element.GetName().c_str());;
    // }

    // void AddFront(Element & in_element) {
    //   element_map[in_element.GetName()] = &in_element;
    //   EM_ASM_ARGS({
    //       var manager_name = Pointer_stringify($0);
    //       var el_tag = Pointer_stringify($1);
    //       $( document ).ready(function() {
    //           // $( '#' + manager_name ).after('<div id=\'' + el_tag '\'>' + el_text + '</div>');
    //           $( '#' + manager_name ).after('tag=' + el_tag);
    //           $( '#' + manager_name ).after('manager=' + manager_name + '<br>');
    //           // $( '#' + manager_name ).after('<div id=\'' + el_tag + '\'></div>');
    //           $('<div id=\'' + el_tag + '\'>TEST!!!!</div>').appendTo( '#' + manager_name );
    //           $( '#' + el_tag ).html( 'YAT (Yet Another Test)' );
    //         });
    //     }, GetName().c_str(), in_element.GetName().c_str());;
    // }

    virtual void PrintHTML(std::ostream & os) {
      os << "<div id=\"" << name <<"\">\n";
      for (auto * element : children) {
        element->PrintHTML(os);
      }
      os << "</div>\n";
    }

  };

};
};

#endif
