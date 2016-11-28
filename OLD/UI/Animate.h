#ifndef EMP_UI_ANIMATE_H
#define EMP_UI_ANIMATE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This class manages animations in a web page.
//
//  Inputs to the constructor include the function to run each animation step, and
//  zero or more elements that should be updated post-animation.
//
//  The function may take an optional double (representing time since the last frame)
//  -or- an optional reference to the animation object
//

#include <functional>

#include "../web/emfunctions.h"
#include "../web/JSWrap.h"
#include "../tools/assert.h"
#include "../tools/vector.h"

#include "Element.h"


namespace emp {
namespace UI {

  class Animate {
  private:
    std::function<void(const Animate &)> anim_fun;
    emp::vector<UI::Element *> targets;
    bool active;                          // Is this animation running?
    bool do_step;                         // Should this animation take a single step?
    int callback_id;

    double start_time;
    double prev_time;
    double cur_time;

    int frame_count;

    void LoadTargets() { ; }
    template <typename... T>
    void LoadTargets(UI::Element & target1, T&... other_targets) {
      targets.push_back(&target1);
      LoadTargets(other_targets...);
    }

    void AdvanceFrame() {
      emp_assert(anim_fun);

      if (!active && !do_step) return;    // If Stop has been called, halt animating.

      prev_time = cur_time;               // Update timing.
      cur_time = emp::GetTime();
      do_step = false;                    // Make sure we don't keep stepping.
      anim_fun(*this);                    // Call anim function, sending this object.

      // Setup the callback for the next frame of the animation.
      EM_ASM_ARGS({
          requestAnimFrame(function() { emp.Callback($0); });
          // requestAnimFrame(function() { alert("Inside!"); });
        }, callback_id);

      frame_count++;
    }

  public:
    template <typename... E_TYPES>
    Animate(const std::function<void(const Animate &)> & fun, E_TYPES&... targets) 
      : anim_fun(fun), active(false), do_step(false), frame_count(0)
    {
      LoadTargets(targets...);
      callback_id = JSWrap( std::function<void()>([this](){ this->AdvanceFrame(); }) );
    }

    template <typename... E_TYPES>
    Animate(const std::function<void(double)> & fun, E_TYPES&... targets) 
      : Animate([fun,this](const Animate &){fun(GetStepTime());}, targets...) { ; }

    template <typename... E_TYPES>
    Animate(const std::function<void()> & fun, E_TYPES&... targets) 
      : Animate([fun](const Animate &){fun();}, targets...) { ; }

    Animate()
      : active(false), do_step(false), frame_count(0)
    {
      callback_id = JSWrap( std::function<void()>([this](){ this->AdvanceFrame(); }) );
    }

    Animate(const Animate &) = delete;
    ~Animate() { ; }

    void Start() {
      if (active) return;          // If animation is already active, ignore start.
      active=true;                 // Mark active.
      do_step=false;               // Shouild be continuously active.
      start_time = emp::GetTime(); // Record the time that we started the animation.
      cur_time = start_time;       // Initialize cur_time to now.
      AdvanceFrame();              // Take the first animation step to get going.
    }
    void Stop() { active = false; }
    void Step() { do_step = true; AdvanceFrame(); }
    void ToggleActive() { if (active) Stop(); else Start(); }

    bool GetActive() const { return active; }
    bool GetDoStep() const { return do_step; }
    double GetStartTime() const { return start_time; }
    double GetPrevTime() const { return prev_time; }
    double GetCurTime() const { return cur_time; }

    double GetStepTime() const { return cur_time - prev_time; }
    double GetRunTime() const { return cur_time - start_time; }

    int GetFrameCount() const { return frame_count; }

    void SetCallback(const std::function<void(const Animate &)> & fun) { anim_fun = fun; }

    void SetCallback(const std::function<void(double)> & fun) {
      anim_fun = [fun, this](const Animate &){fun(GetStepTime());};
    }

    void SetCallback(const std::function<void()> & fun) {
      anim_fun = [fun](const Animate &){fun();};
    }
  };

}
}

#endif
