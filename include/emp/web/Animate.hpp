/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file Animate.hpp
 *  @brief Manage animations on a web site.
 *
 *  To build an animation, you must provide a function to be run repeatedly.  When Start()
 *  is triggered, the function will be called 60 time per second (or as close as possible),
 *  until Stop() is caled.
 *
 */

#ifndef EMP_WEB_ANIMATE_HPP_INCLUDE
#define EMP_WEB_ANIMATE_HPP_INCLUDE


#include <functional>

#include "../base/assert.hpp"
#include "../base/vector.hpp"

#include "Button.hpp"
#include "emfunctions.hpp"
#include "JSWrap.hpp"
#include "Widget.hpp"


namespace emp {
namespace web {

  ///  @brief An object that, when active, repeatedly calls a function as fast as possible,
  ///  to a maximum of 60 frames per second.
  ///
  ///  Parameters to the animation function can be:
  ///    * double (representing time since the last frame)
  ///    * a const reference to the animation object itself
  ///    * nothing
  ///
  ///  Control methods:
  ///    void Start()
  ///    void Stop()
  ///    void Step()
  ///    void ToggleActive()
  ///
  ///  Access methods:
  ///    bool GetActive() const
  ///    bool GetDoStep() const
  ///    double GetStartTime() const
  ///    double GetPrevTime() const
  ///    double GetCurTime() const
  ///    double GetStepTime() const
  ///    double GetRunTime() const
  ///    int GetFrameCount() const
  ///
  ///  Config methods:
  ///    void SetCallback(const std::function<void(const Animate &)> & fun)
  ///    void SetCallback(const std::function<void(double)> & fun)
  ///    void SetCallback(const std::function<void()> & fun)

  class Animate {
  protected:

    /// The full version of the animate function takes a const reference to the animate object.
    using anim_fun_t = std::function<void(const Animate &)>;

    anim_fun_t anim_fun;                ///< Function to repeatedly run for animation.
    emp::vector<web::Widget> targets;   ///< What widgets should be refreshed after each frame?
    bool active;                        ///< Is this animation currently running?
    bool do_step;                       ///< Should this animation take just a single step?
    size_t callback_id;                 ///< Intenral ID for javascript to call back AdvanceFrame()

    double start_time;                  ///< At what time did this animation most recently start?
    double prev_time;                   ///< What was the time point of the previous frame?
    double cur_time;                    ///< What time did the current frame start?
    double run_time;                    ///< How much run time has accumulated?

    int frame_count;                    ///< How many animation frames have gone by?

    Button toggle_but;                  ///< A button to start/stop this animation.

    Button step_but;                    ///< A button to advance this animation one step.

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
      MAIN_THREAD_ASYNC_EM_ASM({
          requestAnimFrame(function() { emp.Callback($0); });
        }, callback_id);

      frame_count++;
    }

    /// DoFrame() is called by default if no animation function is provided.  As such, an animation
    /// can be built by deriving a class from Animate and overriding this function.
    virtual void DoFrame() { ; }

  public:
    /// Setup an Animate object to call an anim_fun as fast as possible, at most 60 times a second.
    /// Call virtual function DoFrame() if no other functon is provided (which can be overridden
    /// if you derive a new class from Animate)
    Animate() : active(false), do_step(false), run_time(0.0), frame_count(0)
    {
      emp::InitializeAnim();  // Make sure JS is intialized for animations.
      callback_id = JSWrap( std::function<void()>([this](){ this->AdvanceFrame(); }) );
    }

    /// Construct an Animate object with the function to run each animation step and
    /// zero or more UI elements that should be updated after each frame.
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

    /// Do not copy animations directly.
    Animate(const Animate &) = delete;
    Animate & operator=(const Animate &) = delete;

    /// Start this animation running.
    void Start() {
      if (active) return;          // If animation is already active, ignore start.
      active=true;                 // Mark active.
      do_step=false;               // Shouild be continuously active.
      start_time = emp::GetTime(); // Record the time that we started the animation.
      cur_time = start_time;       // Initialize cur_time to now.
      AdvanceFrame();              // Take the first animation step to get going.
    }

    /// Halt this animation for now.
    void Stop() {
      run_time += emp::GetTime() - start_time;
      active = false;
    }

    /// Take a single step in this animation.
    void Step() {
      do_step = true;
      AdvanceFrame();
      // @CAO modify run time?
    }

    /// Toggle whether this animation is running or paused.
    void ToggleActive() { if (active) Stop(); else Start(); }

    /// Determine if this animation is currently running.
    bool GetActive() const { return active; }

    /// Determine if this animation is currently in the process of running a single step.
    bool GetDoStep() const { return do_step; }

    /// Return the time point that this animation started MOST RECENTLY.
    double GetStartTime() const { return start_time; }

    /// Determine the time point when this animation last updated a frame.
    double GetPrevTime() const { return prev_time; }

    /// Get the current time of the animation.
    double GetCurTime() const { return cur_time; }

    /// Determine how long the last step between frames took.
    double GetStepTime() const { return cur_time - prev_time; }

    /// Determine the total amount of time that this animation has run.
    double GetRunTime() const { return run_time + cur_time - start_time; }

    /// Determine how many total frames have existed thus far in this animation.
    int GetFrameCount() const { return frame_count; }

    /// Set a new function for this animation to call when running that takes a const reference to
    /// the Animation object as an argument.
    void SetCallback(const anim_fun_t & fun) { anim_fun = fun; }

    /// Set a new function for this animation to call when running that takes the amount of time
    /// since the last frame (a double) as an argument.
    void SetCallback(const std::function<void(double)> & fun) {
      anim_fun = [fun, this](const Animate &){fun(GetStepTime());};
    }

    /// Set a new function for this animation to call when running that takes no arguments.
    void SetCallback(const std::function<void()> & fun) {
      anim_fun = [fun](const Animate &){fun();};
    }

    /// Get a toggle button that will start/stop this animation.
    /// @param but_name The HTML identifier used for this button.
    /// @param start_label The name on the button when it will start the animation (default="Start")
    /// @param stop_label The name on the button when it will halt the animation (default="Stop")
    Button GetToggleButton(const std::string & but_name, const std::string & start_label="Start", const std::string & stop_label="Stop") {
      toggle_but = Button( [this, but_name, start_label, stop_label]() {
          ToggleActive();
          toggle_but.SetLabel(active ? stop_label : start_label);
        }, start_label, but_name);
      return toggle_but;
    }

    /// Get a step button that will advance this animation one frame.
    /// @param but_name The HTML identifier used for this button.
    /// @param label The name shown on the button.
    Button GetStepButton(const std::string & but_name, const std::string & label="Step") {
      step_but = Button( [this, but_name, label]() {
          this->do_step = true;
          this->AdvanceFrame();
        }, label, but_name);
      return step_but;
    }
  };

}
}

#endif // #ifndef EMP_WEB_ANIMATE_HPP_INCLUDE
