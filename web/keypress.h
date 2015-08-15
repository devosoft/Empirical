#ifndef EMP_WEB_KEYPRESS_H
#define EMP_WEB_KEYPRESS_H

/////////////////////////////////////////////////////////////////////////////////////////////
//
//  An tracker for keypresses in HTML5 pages.
//

#include <functional>
#include <map>

#include "../emtools/html5_events.h"
#include "../emtools/JSWrap.h"

namespace emp {
namespace web {

  using namespace std::placeholders;

  class KeypressManager {
  private:
    std::map<int, std::function<bool(const html5::KeyboardEvent &)> > fun_map;
    int next_order;  // Ordering to use if not specified (always last)
    uint32_t callback_id;

    bool DoCallback(const html5::KeyboardEvent & evt_info) {
      bool handled = false;
      for (auto fun_entry : fun_map) {
        if (fun_entry.second(evt_info) == true) {
          handled = true;
          break;
        }
      }

      return handled;
    };


  public:
    KeypressManager() : next_order(0) {
      std::function<bool(const html5::KeyboardEvent &)> callback_fun =
        std::bind( &KeypressManager::DoCallback, this, _1 );
      callback_id = JSWrap( callback_fun );

      EM_ASM_ARGS({
          document.addEventListener('keydown', function(evt) {
              var is_used = emp.Callback($0, evt);
              if (is_used == 1) evt.preventDefault();
            }, false);

        }, callback_id);
    }
    ~KeypressManager() {
      // @CAO Technically we should make sure to remove the event listener at this point.
      // This would require us to keep track of the function that it is calling so that we can
      // pass it back in to trigger the removal.
    }

    int GetFunCount() const { return (int) fun_map.size(); }
    int GetNextOrder() const { return next_order; }

    void AddKeydownCallback(std::function<bool(const html5::KeyboardEvent &)> cb_fun, int order=-1)
    {
      if (order == -1) order = next_order;
      if (order >= next_order) next_order = order+1;

      fun_map[order] = cb_fun;
    }

    void AddKeydownCallback(char key, std::function<void()> cb_fun, int order=-1)
    {
      if (order == -1) order = next_order;
      if (order >= next_order) next_order = order+1;

      fun_map[order] =
        [key, cb_fun](const html5::KeyboardEvent & evt)
        { if (evt.keyCode == key) { cb_fun(); return true; } return false; };
    }

    void AddKeydownCallback(const std::string & key_set, std::function<void()> cb_fun, int order=-1)
    {
      if (order == -1) order = next_order;
      if (order >= next_order) next_order = order+1;

      fun_map[order] =
        [key_set, cb_fun](const html5::KeyboardEvent & evt)
        { if (key_set.find(evt.keyCode) == std::string::npos) return false; cb_fun(); return true;};
    }
  };

};
};

#endif
