//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2020.
//  Released under the MIT Software license; see doc/LICENSE

#ifndef __SELECTION_H__
#define __SELECTION_H__

#include "d3_init.h"

#include <iostream>
#include <string>
#include <typeinfo>
#include <map>
#include <array>

#include "../../base/assert.h"
#include "../js_utils.h"
#include "../JSWrap.h"

namespace D3 {

namespace internal {

  /// You probably never want to instantiate this class. Its sole purpose is to hold code for
  /// methods that are common to selections and transitions.
  ///
  /// Developer note: It's also handy if you want to allow a function to accept either a selection
  /// or transition. This is a good idea any time you are only using methods that are applicable to
  /// either, and the person calling the function may want to animate its results.
  template <typename DERIVED>
  class SelectionOrTransition : public D3_Base {
  public:

      SelectionOrTransition() { ; }
      SelectionOrTransition(int id) : D3_Base(id) { ; }
      SelectionOrTransition(const SelectionOrTransition<DERIVED> & s) : D3_Base(s) { ; }

      /// Create a new selection/transition containing the first element matching the
      /// [selector] string that are within this current selection/transition
      DERIVED Select(const std::string & selector) const {
        const int new_id = NextD3ID();

        EM_ASM({
          const id = $0;
          const selector = UTF8ToString($1);
          const new_id = $2;
          var new_selection = emp_d3.objects[id].select(selector);
          emp_d3.objects[new_id] = new_selection;
        }, this->id, selector.c_str(), new_id);

        return DERIVED(new_id);
      }

      /// Create a new selection/transition containing all elements matching the [selector]
      /// string that are within this current selection/transition
      DERIVED SelectAll(const std::string & selector) const {
        const int new_id = NextD3ID();

        EM_ASM({
          const id = $0;
          const selector = UTF8ToString($1);
          const new_id = $2;
  	      var new_selection = emp_d3.objects[id].selectAll(selector);
          emp_d3.objects[new_id] = new_selection;
        }, this->id, selector.c_str(), new_id);

        return DERIVED(new_id);
      }

      // TODO - Consider re-implementing the 'Call' wrapper but allow for arbitrary arguments to the
      //        function being called.

      /// Returns a new selection/transition, representing the current selection/transition
      /// filtered by [selector]. [selector] can be a C++ function that returns a bool, a
      /// string representing a function in either the d3, emp, or window namespaces that
      /// returns a bool, or a string containing a selector to filter by.
      ///
      /// For more information see the
      /// [D3 documentation](https://github.com/d3/d3-3.x-api-reference/blob/master/Selections.md#filter)
      DERIVED Filter(const std::string & selector) const {
        const int new_id = NextD3ID();

        EM_ASM({
          const id = $0;
          const new_id = $1;
          const selector_str = UTF8ToString($2);
          var sel = emp_d3.find_function(selector_str);
          emp_d3.objects[new_id] = emp_d3.objects[id].filter(sel);
        }, this->id, new_id, selector.c_str());

        return DERIVED(new_id);
      }



  };

}

}

#endif
