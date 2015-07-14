#ifndef EMP_UI_ANIMATE_H
#define EMP_UI_ANIMATE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This class manages animations in a web page.
//
//  Inputs to the constructor include the function to run each animation step, and
//  zero or more elements that should be updated post-animation.
//
//  The function may take an optional double (representing time since the last frame),
//  an optional reference to the animation object, or both.
//

#include <functional>

#include "../emtools/emfunctions.h"
#include "../emtools/JSWrap.h"
#include "../tools/assert.h"
#include "../tools/vector.h"

#include "Element.h"


namespace emp {
namespace UI {

  class Animate {
  private:
    std::function<void(double)> anim_fun;
    emp::vector<UI::Element *> targets;
    bool active;                          // Is this animation running?
    int callback_id;

    double start_time;
    double prev_time;
    double cur_time;

    void LoadTargets() { ; }
    template <typename... T>
    void LoadTargets(UI::Element & target1, T&... other_targets) {
      targets.push_back(&target1);
      LoadTargets(other_targets...);
    }

    void Step() {
      emp_assert(anim_fun);

      if (!active) return;              // If Stop has been called, halt animating.

      prev_time = cur_time;             // Update timing.
      cur_time = emp::GetTime();
      anim_fun(cur_time - prev_time);   // Call anim function, sending time since last frame.

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

    template <typename... E_TYPES>
    Animate(const std::function<void()> & fun, E_TYPES&... targets) 
      : Animate([fun](double){fun();}, targets...) { ; }

    Animate() { callback_id = JSWrap( std::function<void()>([this](){ this->Step(); }) ); }

    Animate(const Animate &) = delete;
    ~Animate() { ; }

    void Start() {
      if (active) return;          // If animation is already active, ignore start.
      active=true;                 // Mark active.
      start_time = emp::GetTime(); // Record the time that we started the animation.
      cur_time = start_time;       // Initialize cur_time to now.
      Step();                      // Take the first animation step to get going.
    }
    void Stop() { active = false; }
    void ToggleActive() { if (active) Stop(); else Start(); }

    bool GetActive() const { return active; }
    double GetStartTime() const { return start_time; }
    double GetPrevTime() const { return prev_time; }
    double GetCurTime() const { return cur_time; }

    double GetStepTime() const { return cur_time - prev_time; }
    double GetRunTime() const { return cur_time - start_time; }

    void SetCallback(const std::function<void(double)> & fun) { anim_fun = fun; }
    void SetCallback(const std::function<void()> & fun) { anim_fun = [fun](double){fun();}; }
  };

}
}

#endif
