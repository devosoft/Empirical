/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file  events.h
 *  @brief Event handlers that use JQuery.
 *
 *  @todo Events can be further sub-divided and built up (similar to DataNode objects) so that we
 *    save only the information that we're planning to use.  This may be slightly faster (given
 *    how frequently some of these like mouse move might be used), but likely to be more compelx.
 *  @todo An alternative speed-up might be to save the current event somewhere in emp_i on the
 *    Javascript side, and then just request the information that we might need.  This approach
 *    should be easier to implement, but since everything would be copied anyway in such as case,
 *    it might not actually be a speedup.
 */

#ifndef EMP_WEB_EVENTS_H
#define EMP_WEB_EVENTS_H

#include <utility>

#include "JSWrap.hpp"

namespace emp {
namespace web {

  /// Runs the specified function when the document is finished loading and being set up.
  template <typename FUN_TYPE> void OnDocumentReady(FUN_TYPE && fun) {
    // const size_t fun_id = JSWrapOnce(fun);
    const size_t fun_id = JSWrap(std::forward<FUN_TYPE>(fun), "", true);
    (void) fun_id;

    EM_ASM_ARGS({  $( document ).ready(function() { emp.Callback($0); });  }, fun_id);
  }

  /// Data common to all web events.
  struct Event {
    bool bubbles;           ///< Is this a bubbling event?
    bool cancelable;        ///< Can the default action be prevented?
    // bool defaultPrevented;  // Has the default action already been prevented?
    // int currentTarget;   // Element whose event listeners triggered this event
    // int eventPhase;      // 0=none, 1=capturing, 2=at target, 3=bubbling
    // bool isTrusted;
    // int target;          // Which element triggered this event?
    // int timeStamp;       // When was event created?
    // std::string type;    // E.g., "mousedown"
    // int view;            // Which window did event occur in?

    // void preventDefault() { ; }
    // void stopImmediatePropagation() { ; }   // Prevents other listeners from being called.
    // void stopPropagation() { ; }

    template <int ARG_ID>
    void LoadFromArg() {
      bubbles = EM_ASM_INT({ return emp_i.cb_args[$0].bubbles; }, ARG_ID);
      cancelable = EM_ASM_INT({ return emp_i.cb_args[$0].cancelable; }, ARG_ID);
    }
  };

  /// Mouse-specific information about web events.
  struct MouseEvent : public Event {
    // All values reflect the state of devices when the event was triggered.
    bool altKey;     ///< Was "ALT" key was pressed?
    bool ctrlKey;    ///< Was "CTRL" key pressed?
    bool metaKey;    ///< Was "META" key pressed?
    bool shiftKey;   ///< Was "SHIFT" key pressed?

    int button;      ///< Which mouse button was pressed?  -1=none  (0/1/2)
    int detail;      ///< How many clicks happened in short succession?

    int clientX;     ///< X-mouse postion, relative to current window
    int clientY;     ///< Y-mouse postion, relative to current window
    int screenX;     ///< X-mouse position, relative to the screen
    int screenY;     ///< Y-mouse position, relative to the screen
    // int buttons;     ///< Which mouse buttons were pressed? Sum: (1/4/2) (Special: 8,16)
    // int relatedTarget    ///< Element related to the element that triggered the mouse event
    // int which     ///< Which mouse button was pressed?  0=none  (1/2/3)

    template <int ARG_ID>
    void LoadFromArg() {
      Event::LoadFromArg<ARG_ID>();

      altKey = EM_ASM_INT({ return emp_i.cb_args[$0].altKey; }, ARG_ID);
      ctrlKey = EM_ASM_INT({ return emp_i.cb_args[$0].ctrlKey; }, ARG_ID);
      metaKey = EM_ASM_INT({ return emp_i.cb_args[$0].metaKey; }, ARG_ID);
      shiftKey = EM_ASM_INT({ return emp_i.cb_args[$0].shiftKey; }, ARG_ID);
      button = EM_ASM_INT({ return emp_i.cb_args[$0].button; }, ARG_ID);
      detail = EM_ASM_INT({ return emp_i.cb_args[$0].detail; }, ARG_ID);
      clientX = EM_ASM_INT({ return emp_i.cb_args[$0].clientX; }, ARG_ID);
      clientY = EM_ASM_INT({ return emp_i.cb_args[$0].clientY; }, ARG_ID);
      screenX = EM_ASM_INT({ return emp_i.cb_args[$0].screenX; }, ARG_ID);
      screenY = EM_ASM_INT({ return emp_i.cb_args[$0].screenY; }, ARG_ID);
    }
  };


  /// Keyboard-specific information about web events.
  struct KeyboardEvent : public Event {
    // All values reflect the state of devices when the event was triggered.
    bool altKey;     ///< Was "ALT" key was pressed?
    bool ctrlKey;    ///< Was "CTRL" key pressed?
    bool metaKey;    ///< Was "META" key pressed?
    bool shiftKey;   ///< Was "SHIFT" key pressed?

    int charCode;    ///< Unicode character pressed
    int keyCode;     ///< Which key was pressed on the keyboard (e.g., 'a' and 'A' are the same)

    template <int ARG_ID>
    void LoadFromArg() {
      Event::LoadFromArg<ARG_ID>();

      altKey = EM_ASM_INT({ return emp_i.cb_args[$0].altKey; }, ARG_ID);
      ctrlKey = EM_ASM_INT({ return emp_i.cb_args[$0].ctrlKey; }, ARG_ID);
      metaKey = EM_ASM_INT({ return emp_i.cb_args[$0].metaKey; }, ARG_ID);
      shiftKey = EM_ASM_INT({ return emp_i.cb_args[$0].shiftKey; }, ARG_ID);
      charCode = EM_ASM_INT({ return emp_i.cb_args[$0].charCode; }, ARG_ID);
      keyCode = EM_ASM_INT({ return emp_i.cb_args[$0].keyCode; }, ARG_ID);
    }
  };


  /// Mouse-wheel-specific information about web events.
  struct WheelEvent : public Event {
    // All values reflect the state of devices when the event was triggered.
    int deltaX;      ///< Horizontal scroll amount.
    int deltaY;      ///< Vertical scroll amount.
    int deltaZ;      ///< Scroll amount of a mouse wheel for the z-axis
    int deltaMode;   ///< The unit of measurements for delta values (pixels, lines or pages)

    template <int ARG_ID>
    void LoadFromArg() {
      Event::LoadFromArg<ARG_ID>();

      deltaX = EM_ASM_INT({ return emp_i.cb_args[$0].deltaX; }, ARG_ID);
      deltaY = EM_ASM_INT({ return emp_i.cb_args[$0].deltaY; }, ARG_ID);
      deltaZ = EM_ASM_INT({ return emp_i.cb_args[$0].deltaZ; }, ARG_ID);
      deltaMode = EM_ASM_INT({ return emp_i.cb_args[$0].deltaMode; }, ARG_ID);
    }
  };
}
}

#endif
