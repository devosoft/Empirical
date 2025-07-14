/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2015-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/Animate.hpp
 * @brief Manage animations on a web site.
 *
 * To build an animation, you must provide a function to be run repeatedly.  When Start()
 * is triggered, the function will be called 60 time per second (or as close as possible),
 * until Stop() is called.
 *
 * Parameters to the animation function can be:
 *   * double (representing time since the last frame)
 *   * a const reference to the animation object itself
 *   * nothing
 *
 * Control methods:
 *   void Start()
 *   void Stop()
 *   void Step()
 *   void ToggleActive()
 *
 * Access methods:
 *   bool GetActive() const
 *   bool GetDoStep() const
 *   double GetStartTime() const
 *   double GetPrevTime() const
 *   double GetCurTime() const
 *   double GetStepTime() const
 *   double GetRunTime() const
 *   int GetFrameCount() const
 *
 * Config methods:
 *   void SetCallback(const std::function<void(const Animate &)> & fun)
 *   void SetCallback(const std::function<void(double)> & fun)
 *   void SetCallback(const std::function<void()> & fun)
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_ANIMATE_HPP_GUARD
#define INCLUDE_EMP_WEB_ANIMATE_HPP_GUARD

#include <functional>
#include <stddef.h>

#include "../base/assert.hpp"
#include "../base/vector.hpp"

#include "Button.hpp"
#include "JSWrap.hpp"
#include "Widget.hpp"
#include "emfunctions.hpp"

namespace emp {
namespace web {

  class Animate {
  protected:
    using anim_fun_t = std::function<void(const Animate &)>;

    anim_fun_t anim_fun;               ///< Function to repeatedly run for animation.
    emp::vector<web::Widget> targets;  ///< Widgets to refresh after each frame.
    bool active = false;               ///< Is this animation running?
    bool do_step = false;              ///< Are we taking a single frame step?
    size_t callback_id = 0;            ///< ID used by JS to call AdvanceFrame()

    double start_time = 0.0;  ///< When the animation was last started.
    double prev_time  = 0.0;  ///< When the previous frame occurred.
    double cur_time   = 0.0;  ///< When the current frame began.
    double run_time   = 0.0;  ///< Total time the animation has run.

    int frame_count = 0;      ///< How many frames have elapsed.

    Button toggle_but;
    Button step_but;

    void LoadTargets() { ; }

    template <typename... T>
    void LoadTargets(const web::Widget & target1, const T &... other_targets) {
      targets.push_back(target1);
      LoadTargets(other_targets...);
    }

    void AdvanceFrame() {
      if (!active && !do_step) return;

      prev_time = cur_time;
      cur_time  = emp::GetTime();
      do_step   = false;

      if (anim_fun) anim_fun(*this);
      DoFrame();

      for (auto & w : targets) w.Redraw();

      // Use modern requestAnimationFrame directly.
      MAIN_THREAD_ASYNC_EM_ASM({
        requestAnimationFrame(function() { emp.Callback($0); });
      }, callback_id);

      frame_count++;
    }

    /// Optional override for user-defined animation frames.
    virtual void DoFrame() { ; }

  public:
    Animate() {
      callback_id = JSWrap(std::function<void()>([this]() { this->AdvanceFrame(); }));
    }

    template <typename... W_TYPES>
    Animate(const anim_fun_t & fun, W_TYPES &... targets) : Animate() {
      anim_fun = fun;
      LoadTargets(targets...);
    }

    template <typename... W_TYPES>
    Animate(const std::function<void(double)> & fun, W_TYPES &... targets)
      : Animate([fun, this](const Animate &) { fun(GetStepTime()); }, targets...) { }

    template <typename... W_TYPES>
    Animate(const std::function<void()> & fun, W_TYPES &... targets)
      : Animate([fun](const Animate &) { fun(); }, targets...) { }

    virtual ~Animate() {
      JSDelete(callback_id);  // Clean up JS callback
    }

    Animate(const Animate &) = delete;
    Animate & operator=(const Animate &) = delete;

    void Start() {
      if (active) return;
      active     = true;
      do_step    = false;
      start_time = emp::GetTime();
      cur_time   = start_time;
      AdvanceFrame();
    }

    void Stop() {
      run_time += emp::GetTime() - start_time;
      active = false;
    }

    void Step() {
      do_step = true;
      AdvanceFrame();
    }

    void ToggleActive() { active ? Stop() : Start(); }

    bool GetActive() const { return active; }
    bool GetDoStep() const { return do_step; }
    double GetStartTime() const { return start_time; }
    double GetPrevTime() const { return prev_time; }
    double GetCurTime() const { return cur_time; }
    double GetStepTime() const { return cur_time - prev_time; }
    double GetRunTime() const { return run_time + (active ? (cur_time - start_time) : 0.0); }
    int GetFrameCount() const { return frame_count; }

    void SetCallback(const anim_fun_t & fun) { anim_fun = fun; }

    void SetCallback(const std::function<void(double)> & fun) {
      anim_fun = [fun, this](const Animate &) { fun(GetStepTime()); };
    }

    void SetCallback(const std::function<void()> & fun) {
      anim_fun = [fun](const Animate &) { fun(); };
    }

    Button GetToggleButton(const std::string & but_name,
                           const std::string & start_label = "Start",
                           const std::string & stop_label  = "Stop") {
      toggle_but = Button(
        [this, but_name, start_label, stop_label]() {
          ToggleActive();
          toggle_but.SetLabel(active ? stop_label : start_label);
        },
        start_label,
        but_name);
      return toggle_but;
    }

    Button GetStepButton(const std::string & but_name, const std::string & label = "Step") {
      step_but = Button(
        [this]() {
          this->do_step = true;
          this->AdvanceFrame();
        },
        label,
        but_name);
      return step_but;
    }
  };

}}  // namespace emp::web

#endif  // #ifndef INCLUDE_EMP_WEB_ANIMATE_HPP_GUARD
