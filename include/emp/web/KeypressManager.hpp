/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file  KeypressManager.hpp
 *  @brief KeypressManager is a tracker for keypresses in HTML5 pages.
 *
 *  When a KeypressManager is created, it can be given functions to run in response
 *  to different types of key presses via overloaded version of the AddKeydownCallback
 *  method.  Each of these accepts an order parameter that is optional and, if provided,
 *  will indicate the order in which tests should be performed to resolve a keypress.
 *  If order is not provided, tests will occur in the order that they were given to the
 *  manager.
 *
 *  The specific versions of AddKeydownCallback are:
 *
 *    void AddKeydownCallback(std::function<bool(const emp::web::KeyboardEvent &)> cb_fun,
 *                            int order=-1)
 *
 *      Link a function to the KeypressManager that is called for any unresolved keypress.
 *      The function must take in an emp::web::KeyboardEvent (which includes information about
 *      the specific key pressed as well as any modifiers such as SHIFT or CTRL) and it
 *      must return a boolean value indicating whether it has resolved the keypress.
 *
 *    void AddKeydownCallback(char key, std::function<void()> cb_fun, int order=-1)
 *
 *      Link a specific key to a target function to be called when that key is pressed.
 *      The function my return a void and take no arguments.
 *
 *    void AddKeydownCallback(const std::string & key_set, std::function<void()> cb_fun,
 *                            int order=-1)
 *
 *      Same as the previous method, but will respond to any of the keys in the provided
 *      string.
 *
 *
 *  @todo Technically we should make sure to remove the event listener in the destructor.
 *        This would require us to keep track of the function that it is calling so that we can
 *        pass it back in to trigger the removal.
 */

#ifndef EMP_WEB_KEYPRESS_MANAGER_H
#define EMP_WEB_KEYPRESS_MANAGER_H

#include <functional>
#include <map>
#include <locale>

#include "../base/errors.hpp"

#include "events.hpp"
#include "JSWrap.hpp"

namespace emp {
namespace web {

  using namespace std::placeholders;

  class KeypressManager {
  private:
    std::map<int, std::function<bool(const KeyboardEvent &)> > fun_map;
    int next_order;  // Ordering to use if not specified (always last)
    uint32_t callback_id;

    bool DoCallback(const KeyboardEvent & evt_info) {
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
      std::function<bool(const KeyboardEvent &)> callback_fun =
        std::bind( &KeypressManager::DoCallback, this, _1 );
      callback_id = JSWrap( callback_fun );

      MAIN_THREAD_EM_ASM({
          document.addEventListener('keydown', function(evt) {
              var is_used = emp.Callback($0, evt);
              if (is_used == 1) evt.preventDefault();
            }, false);

        }, callback_id);
    }
    ~KeypressManager() {
    }

    int GetFunCount() const { return (int) fun_map.size(); }
    int GetNextOrder() const { return next_order; }

    ///  Link a function to the KeypressManager that is called for any unresolved keypress.
    ///  The function must take in an emp::web::KeyboardEvent (which includes information about
    ///  the specific key pressed as well as any modifiers such as SHIFT or CTRL) and it
    ///  must return a boolean value indicating whether it has resolved the keypress.
    void AddKeydownCallback(
      std::function<bool(const KeyboardEvent &)> cb_fun,
      int order=-1
    ) {
      if (order == -1) order = next_order;
      if (order >= next_order) next_order = order+1;

      fun_map[order] = cb_fun;
    }

    ///  Link a specific key to a target function to be called when that key is pressed.
    ///  The function my return a void and take no arguments.
    /// Specify keys as lowercase characters. To sepcify uppercase, you'll
    /// need to monitor fo rthe shift modifier associated with a KeypressEvent.
    void AddKeydownCallback(
      char key,
      std::function<void()> cb_fun,
      int order=-1
    ) {
      if (order == -1) order = next_order;
      if (order >= next_order) next_order = order+1;

      if (std::isupper(key)) emp::LibraryWarning(
        "Uppercase character was passed for the key argument. ",
        "To specify uppercase, you'll need to monitor for the shift modifier associated with a KeypressEvent."
      );

      key = std::toupper(key);

      fun_map[order] =
        [key, cb_fun](const KeyboardEvent & evt)
        { if (evt.keyCode == key) { cb_fun(); return true; } return false; };
    }

    /// Provide a whole set of keys that should all trigger the same function, including an
    /// ordering for priority.
    /// Specify keys as lowercase characters. To sepcify uppercase, you'll
    /// need to monitor fo rthe shift modifier associated with a KeypressEvent.
    void AddKeydownCallback(
      const std::string & key_set,
      const std::function<void()> & cb_fun,
      int order
    ) {
      if (order >= next_order) next_order = order+1;

      if (std::any_of(
        std::begin(key_set),
        std::end(key_set),
        ::isupper
      )) emp::NotifyWarning(
        "Uppercase character was passed for the key argument. ",
        "To specify uppercase, you'll need to monitor for the shift modifier associated with a KeypressEvent."
      );

      std::string uppercase_key_set{key_set};
      std::transform(
        std::begin(uppercase_key_set),
        std::end(uppercase_key_set),
        std::begin(uppercase_key_set),
        ::toupper
      );

      fun_map[order] = [key_set, cb_fun](const KeyboardEvent & evt) {
        if (key_set.find((char)evt.keyCode) == std::string::npos) {
          return false;
        }
        cb_fun();
        return true;
      };
    }

    /// Provide a whole set of keys that should all trigger the same function; use default ordering.
    void AddKeydownCallback(
      const std::string & key_set,
      const std::function<void()> & cb_fun
    ) {
      AddKeydownCallback(key_set, cb_fun, next_order);
    }
  };

};
};

#endif
