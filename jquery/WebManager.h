#ifndef EMP_WEB_MANAGER_H
#define EMP_WEB_MANAGER_H

///////////////////////////////////////////////////////////////////////////////////////////
//
//  Manage the current web page
//

#include <map>
#include <string>

#include "emscripten.h"

#include "../tools/assert.h"

#include "WebElement.h"

namespace emp {

  class WebManager : public WebElement {
  private:
    std::map<std::string, WebElement *> element_map;    
    std::string end_tag;

  public:
    WebManager(const std::string & name)
      : WebElement(name)
      , end_tag(name + std::string("__end"))
    {
      EM_ASM({
          var name = Pointer_stringify($0);
          var e_tag = Pointer_stringify($1);
          $( document ).ready(function() {
              $( '#' + name ).after('<div id=\'' + e_tag '\'></div>');
            });
        }, name.c_str(), end_tag.c_str());;
    }
    WebManager(const WebManager &) = delete;
    ~WebManager() { ; }
    
    WebManager & operator=(const WebManager &) = delete;

    bool Contains(const std::string & name) { return element_map.find(name) != element_map.end(); }
    WebElement & Get(const std::string & name) {
      emp_assert(Contains(name));
      return element_map[name];
    }

    void AddFront(WebElement & in_element) {
      element_map[in_element.GetName()] = &in_element;
      EM_ASM({
          var manager_name = Pointer_stringify($0);
          var e_tag = Pointer_stringify($1);
          $( document ).ready(function() {
              $( '#' + manager_name ).after('<div id=\'' + e_tag '\'></div>');
            });
        }, GetName().c_str(), in_element.GetName().c_str());;
    }

  };

};

#endif
