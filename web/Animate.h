#ifndef EMP_WEB_ANIMATE_H
#define EMP_WEB_ANIMATE_H

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

#include "../emtools/emfunctions.h"
#include "../emtools/JSWrap.h"
#include "../tools/assert.h"
#include "../tools/vector.h"

#include "Widget.h"


namespace emp {
namespace web {

  class Animate {
  private:
    std::function<void(const Animate &)> anim_fun;
    emp::vector<web::Widget> targets;   // What widgets should be refreshed on each frame?
    bool active;                        // Is this animation running?
    bool do_step;                       // Should this animation take a single step?
    int callback_id;

    double start_time;                  // At what time did this animation most recently start?
    double prev_time;                   // What was the time point of the previous frame?
    double cur_time;                    // What time did the current frame start?
    double run_time;                    // How much run time has accumulated?

    int frame_count;

    void LoadTargets() { ; }
    template <typename... T>
    void LoadTargets(const web::Widget & target1, const T&... other_targets) {
      targets.push_back(target1);
      LoadTargets(other_targets...);
    }

    void AdvanceFrame() {
      emp_assert(anim_fun);

      if (!active && !do_step) return;  // If Stop has been called, halt animating.

      prev_time = cur_time;             // Update timing.
      cur_time = emp::GetTime();
      do_step = false;                  // Make sure we don't keep advancing by a single step.
      anim_fun(*this);                  // Call anim function, sending this object.

      // Loop through all widget targets to be redrawn and do so.
      for (auto & w : targets) { w.Redraw(); }

      // Setup the callback for the next frame of the animation.
      EM_ASM_ARGS({
          requestAnimFrame(function() { emp.Callback($0); });
        }, callback_id);

      frame_count++;
    }

  public:
    template <typename... W_TYPES>
    Animate(const std::function<void(const Animate &)> & fun, W_TYPES&... targets) 
      : anim_fun(fun), active(false), do_step(false), run_time(0.0), frame_count(0)
    {
      LoadTargets(targets...);
      callback_id = JSWrap( std::function<void()>([this](){ this->AdvanceFrame(); }) );
    }

    template <typename... W_TYPES>
    Animate(const std::function<void(double)> & fun, W_TYPES&... targets) 
      : Animate([fun,this](const Animate &){fun(GetStepTime());}, targets...) { ; }

    template <typename... W_TYPES>
    Animate(const std::function<void()> & fun, W_TYPES&... targets) 
      : Animate([fun](const Animate &){fun();}, targets...) { ; }

    Animate()
      : active(false), do_step(false), run_time(0.0), frame_count(0)
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
    void Stop() {
      run_time += emp::GetTime() - start_time;
      active = false;
    }
    void Step() {
      do_step = true;
      AdvanceFrame();
      // @CAO modify run time?
    }
    void ToggleActive() { if (active) Stop(); else Start(); }

    bool GetActive() const { return active; }
    bool GetDoStep() const { return do_step; }
    double GetStartTime() const { return start_time; }
    double GetPrevTime() const { return prev_time; }
    double GetCurTime() const { return cur_time; }

    double GetStepTime() const { return cur_time - prev_time; }
    double GetRunTime() const { return run_time + cur_time - start_time; }

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
