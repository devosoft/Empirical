#ifndef EMP_WEB_MANAGER_H
#define EMP_WEB_MANAGER_H

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

#include "WebElement.h"

namespace emp {

  class WebManager : public WebElement {
  private:
    std::map<std::string, WebElement *> element_map;    
    std::vector<WebElement *> element_vector;
    std::string end_tag;

  public:
    WebManager(const std::string & name)
      : WebElement(name)
      , end_tag(name + std::string("__end"))
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
    ~WebManager() {
      // Delete all of the elements created as part of this managed space.
      for (auto i : element_vector) {
        delete i;
      }
    }

    // Do not allow WebManagers to be copied
    WebManager(const WebManager &) = delete;
    WebManager & operator=(const WebManager &) = delete;

    bool Contains(const std::string & name) { return element_map.find(name) != element_map.end(); }
    WebElement & Get(const std::string & name) {
      emp_assert(Contains(name));
      return *(element_map[name]);
    }

    // void AddFront(WebElement & in_element) {
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
