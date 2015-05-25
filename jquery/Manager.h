#ifndef EMP_JQ_MANAGER_H
#define EMP_JQ_MANAGER_H

///////////////////////////////////////////////////////////////////////////////////////////
//
//  Manage a section of the current web page
//

#include <map>
#include <string>
#include <vector>

#include "emscripten.h"

#include "../tools/assert.h"
#include "../tools/alert.h"

#include "JQElement.h"

namespace emp {

  class JQManager : public JQElement {
  private:
    std::map<std::string, JQElement *> element_map;    
    std::vector<JQElement *> element_vector;
    std::string end_tag;
    bool ready;

  public:
    JQManager(const std::string & name)
      : JQElement(name)
      , end_tag(name + std::string("__end"))
      , ready(false)
    {
      // Setup an end-tag so that we can always track the end of this managed space.
      EM_ASM_ARGS({
          var name = Pointer_stringify($0);
          var e_tag = Pointer_stringify($1);
          $( document ).ready(function() {
              $( '#' + name ).after('<div id=\'' + e_tag + '\'></div>');
            });
        }, name.c_str(), end_tag.c_str());;
    }
    ~JQManager() {
      // Delete all of the elements created as part of this managed space.
      for (auto i : element_vector) {
        delete i;
      }
    }

    // Do not allow JQManagers to be copied
    JQManager(const JQManager &) = delete;
    JQManager & operator=(const JQManager &) = delete;

    bool Contains(const std::string & name) { return element_map.find(name) != element_map.end(); }
    JQElement & Get(const std::string & name) {
      emp_assert(Contains(name));
      return *(element_map[name]);
    }

    JQElement & AddFront(const std::string & html_string) {
      // Create the new element (and its unique tag name).
      const std::string new_tag = emp::to_string(GetName(), "__", element_vector.size());
      JQElement * new_element = new JQElement(new_tag, html_string);

      // Store the new element.
      element_vector.push_back(new_element);
      element_map[new_tag] = new_element;
    }

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

    // void AddFront(JQElement & in_element) {
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

  };

};

#endif
