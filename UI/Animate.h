#ifndef EMP_UI_ANIMATE_H
#define EMP_UI_ANIMATE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This class manages animations in a web page.
//
//  Inputs to the constructor include the function to run each animation step, and
//  zero or more elements that should be updated post-animation.
//
//  The function may take an optional double (representing the time since start)
//

#include <functional>
#include <vector>

#include "Element.h"
#include "../emtools/JSWrap.h"

namespace emp {
namespace UI {

  class Animate {
  private:
    std::function<void(double)> anim_fun;
    std::vector<UI::Element *> targets;
    bool active;                          // Is this animation running?
    int callback_id;

    void LoadTargets() { ; }
    template <typename... T>
    void LoadTargets(UI::Element & target1, T&... other_targets) {
      targets.push_back(&target1);
      LoadTargets(other_targets...);
    }

    void Step() {
      if (!active) return;  // If Stop has been called, don't propagate further.

      anim_fun(0.0);        // @CAO send actual timing info!

      // Setup the callback for the next frame of the animation.
      EM_ASM_ARGS({
          requestAnimFrame(function() { emp.Callback($0); });
          // requestAnimFrame(function() { alert("Inside!"); });
        }, callback_id);
    }

  public:
    template <typename... E_TYPES>
    Animate(const std::function<void(double)> & fun, E_TYPES&... targets) 
      : anim_fun(fun), active(false)
    {
      LoadTargets(targets...);
      callback_id = JSWrap( std::function<void()>([this](){ this->Step(); }) );
    }
    Animate() = delete;
    Animate(const Animate &) = delete;
    ~Animate() { ; }

    void Start() { if (active) return; active=true; Step(); }
    void Stop() { active = false; }
  };

}
}

#endif
