#ifndef EMP_KEYPRESS_H
#define EMP_KEYPRESS_H

/////////////////////////////////////////////////////////////////////////////////////////////
//
//  An tracker for keypresses in HTML5 pages.
//

#include <functional>
#include <map>
#include "../tools/callbacks.h"

namespace emp {

  using namespace std::placeholders;

  class KeypressManager {
  private:
    std::map<int, std::function<bool(const EventInfo &)> > fun_map;
    int next_order;  // Ordering to use if not specified (always last)

    void DoCallback(const EventInfo & evt_info) {
      bool handled = false;
      for (auto fun_entry : fun_map) {
        if (fun_entry.second(evt_info) == true) {
          handled = true;
          break;
        }
      }

      // @CAO If an event has been handled, don't pass it on!
      if (handled) { ; }
    };


  public:
    KeypressManager() : next_order(0) {
      auto keypress_callback =
        new MethodCallback_Event<KeypressManager>(this, &KeypressManager::DoCallback);

      EM_ASM_ARGS({
          document.addEventListener('keydown', function(evt) {
              var ptr = Module._malloc(32); // 8 ints @ 4 bytes each...
              setValue(ptr,    evt.layerX,   'i32');
              setValue(ptr+4,  evt.layerY,   'i32');
              setValue(ptr+8,  evt.button,   'i32');
              setValue(ptr+12, evt.keyCode,  'i32');
              setValue(ptr+16, evt.altKey,   'i32');
              setValue(ptr+20, evt.ctrlKey,  'i32');
              setValue(ptr+24, evt.metaKey,  'i32');
              setValue(ptr+28, evt.shiftKey, 'i32');

              empJSDoCallback($0, ptr);
              Module._free(ptr);
              evt.preventDefault();
            }, false);

        }, (int) keypress_callback);
    }
    ~KeypressManager() {
      // @CAO Technically we should make sure to remove the event listener at this point.
      // This would require us to keep track of the function that it is calling so that we can
      // pass it back in to trigger the removal.
    }

    void AddKeydownCallback(std::function<bool(const EventInfo &)> cb_fun, int order=-1) {
      if (order == -1) order = next_order;
      if (order >= next_order) next_order = order+1;

      fun_map[order] = cb_fun;
    }
  };

};

#endif
