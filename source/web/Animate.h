//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This class manages animations in a web page.
//
//  To build an animation, you must provide a function to be run repeatedly.  When Start()
//  is triggered, the function will be called 60 time per second (or as close as possible),
//  until Stop() is caled.
//
//  Inputs to the constructor include the function to run each animation step, and
//  zero or more UI elements that should be updated after each frame.
//
//  Parameters to the animation function can be:
//    * double (representing time since the last frame)
//    * a const reference to the animation object itself
//    * nothing
//
//  Control methods:
//    void Start()
//    void Stop()
//    void Step()
//    void ToggleActive()
//
//  Access methods:
//    bool GetActive() const
//    bool GetDoStep() const
//    double GetStartTime() const
//    double GetPrevTime() const
//    double GetCurTime() const
//    double GetStepTime() const
//    double GetRunTime() const
//    int GetFrameCount() const
//
//  Config methods:
//    void SetCallback(const std::function<void(const Animate &)> & fun)
//    void SetCallback(const std::function<void(double)> & fun)
//    void SetCallback(const std::function<void()> & fun)


#ifndef EMP_WEB_ANIMATE_H
#define EMP_WEB_ANIMATE_H

#include <functional>

#include "../base/assert.h"
#include "../base/vector.h"

#include "Button.h"
#include "emfunctions.h"
#include "JSWrap.h"
#include "Widget.h"


namespace emp {
namespace web {

  class Animate {
  protected:
    using anim_fun_t = std::function<void(const Animate &)>;

    anim_fun_t anim_fun;                //< Function to repeatedly run for animation.
    emp::vector<web::Widget> targets;   //< What widgets should be refreshed after each frame?
    bool active;                        //< Is this animation currently running?
    bool do_step;                       //< Should this animation take just a single step?
    size_t callback_id;                 //< Intenral ID for javascript to call back AdvanceFrame()

    double start_time;                  //< At what time did this animation most recently start?
    double prev_time;                   //< What was the time point of the previous frame?
    double cur_time;                    //< What time did the current frame start?
    double run_time;                    //< How much run time has accumulated?

    int frame_count;                    //< How many animation frames have gone by?

    Button toggle_but;                  //< A button to start/stop this animation.

    void LoadTargets() { ; }
    template <typename... T>
    void LoadTargets(const web::Widget & target1, const T&... other_targets) {
      targets.push_back(target1);
      LoadTargets(other_targets...);
    }

    void AdvanceFrame() {
      if (!active && !do_step) return;  // If Stop has been called, halt animating.

      prev_time = cur_time;             // Update timing.
      cur_time = emp::GetTime();
      do_step = false;                  // Make sure we don't keep advancing by a single step.
      if (anim_fun) anim_fun(*this);    // If anim function exist, call it and send this object.
      DoFrame();                        // Call DoFrame in this class.

      // Loop through all widget targets to be redrawn and do so.
      for (auto & w : targets) { w.Redraw(); }

      // Setup the callback for the next frame of the animation.
      EM_ASM_ARGS({
          requestAnimFrame(function() { emp.Callback($0); });
        }, callback_id);

      frame_count++;
    }

    /// DoFrame() is called by default if no animation function is provided.  As such, an animation
    /// can be built by deriving a class from Animate and overriding this function.
    virtual void DoFrame() { ; }

  public:
    Animate() : active(false), do_step(false), run_time(0.0), frame_count(0)
    {
      emp::InitializeAnim();  // Make sure JS is intialized for animations.
      callback_id = JSWrap( std::function<void()>([this](){ this->AdvanceFrame(); }) );
    }

    template <typename... W_TYPES>
    Animate(const anim_fun_t & fun, W_TYPES&... targets) : Animate()
    {
      anim_fun = fun;
      LoadTargets(targets...);
    }

    template <typename... W_TYPES>
    Animate(const std::function<void(double)> & fun, W_TYPES&... targets)
      : Animate([fun,this](const Animate &){fun(GetStepTime());}, targets...) { ; }

    template <typename... W_TYPES>
    Animate(const std::function<void()> & fun, W_TYPES&... targets)
      : Animate([fun](const Animate &){fun();}, targets...) { ; }

    virtual ~Animate() { ; }

    // Do not copy animations directly.
    Animate(const Animate &) = delete;
    Animate & operator=(const Animate &) = delete;

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

    void SetCallback(const anim_fun_t & fun) { anim_fun = fun; }

    void SetCallback(const std::function<void(double)> & fun) {
      anim_fun = [fun, this](const Animate &){fun(GetStepTime());};
    }

    void SetCallback(const std::function<void()> & fun) {
      anim_fun = [fun](const Animate &){fun();};
    }

    Button GetToggleButton(const std::string & but_name, const std::string & start_label="Start", const std::string & stop_label="Stop") {
      toggle_but = Button( [this, but_name, start_label, stop_label]() {
          ToggleActive();
          toggle_but.Label(active ? stop_label : start_label);
        }, start_label, but_name);
      return toggle_but;
    }
  };

}
}

#endif
