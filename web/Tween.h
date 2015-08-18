#ifndef EMP_WEB_TWEEN_H
#define EMP_WEB_TWEEN_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This class manages the gradual movement of a widget from one state to another.
//

#include "../emtools/emfunctions.h"

#include "Widget.h"


namespace emp {
namespace web {

  class Tween {
  private:
    struct Path {
      std::function<void(double)> set_fun;   // Function to set at each step
      double start_val;                      // What value should the path begin with?
      double end_val;                        // What value should the path end with?
      std::function<double(double)> timing;  // Time dilation to use (e.g., to ease in and out)

      void Set(const double frac) {
        const double cur_val = (end_val - start_val) * timing(frac) + start_val;
        set_fun(cur_val);
      }
    };

    struct Event {
      double time;
      std::function<void()> trigger;
    };

    double duration;                                // How long should this Tween take?
    Widget default_target;                          // Default widget to use for specifying paths
    emp::vector<Path*> paths;                       // Paths to be updated as part of this tween.
    emp::vector<Event*> events;                     // Events to be triggered as specific steps.
    emp::vector<std::function<void()>> update_funs; // Call after paths are triggered to UD screen.
    emp::vector<Widget> dependants;                 // Widgets to be refreshed at each frame.

    bool running;
    int callback_id;

    double start_time;  // When did the most recent run start?
    double cur_time;    // What time is it now?
    double run_time;    // How long did this previously run?

    static double LINEAR(double in) { return in; }

    void AdvanceFrame() {
      if (!running) return;             // If Stop has been called, halt animating.

      cur_time = emp::GetTime();
      double frac = (cur_time - start_time + run_time) / duration;

      // If we're done, make sure we go to the last frame and stop!
      if (frac > 1.0) { frac = 1.0; running = false; }

      //@CAO test if any events should be triggered...

      // Loop through each path and adjust accordingly...
      for (auto * p : paths) {
        p->Set(frac);
      }

      // Loop through each update function to make sure it gets triggered.
      for (auto f : update_funs) {
        f();
      }

      // Loop through all dependants be redrawn and do so.
      for (auto & w : dependants) { w.Redraw(); }

      // Setup the callback for the next frame of the Tween.
      EM_ASM_ARGS({
          requestAnimFrame(function() { emp.Callback($0); });
        }, callback_id);
    }

  public:
    Tween(double d=1.0, const Widget & t=nullptr)
      : duration(d*1000), default_target(t), running(false)
      , start_time(0.0), cur_time(0.0), run_time(0.0)
    {
      callback_id = JSWrap( std::function<void()>([this](){ this->AdvanceFrame(); }) );
    }
    ~Tween()
    {
      for (auto * p : paths) delete p;
      for (auto * e : events) delete e;
    }
    
    double GetDuration() const { return duration; }
    Widget GetDefaultTarget() const { return default_target; }
    
    Tween & SetDuration(double d) { duration = d*1000; return *this; }
    Tween & SetDefaultTarget(const Widget & w) { default_target = w; return *this; }

    Tween & AddPath(std::function<void(double)> set_fun,
                    double start_val, double end_val, std::function<double(double)> timing=LINEAR) {
      Path * new_path = new Path({set_fun, start_val, end_val, timing});
      paths.push_back(new_path);
      return *this;
    }

    Tween & AddPath(double & set_var,
                    double start_val, double end_val, std::function<double(double)> timing=LINEAR) {
      AddPath( [&set_var](double v) { set_var = v; }, start_val, end_val, timing);
      return *this;
    }

    Tween & AddPath(Widget w, std::string setting, double start_val, double end_val) {
      emp_assert(false && "need to fill in!");
      return *this;
    }

    Tween & AddUpdate(std::function<void(void)> ud_fun) {
      update_funs.push_back(ud_fun);
      return *this;
    }

    Tween & AddDependant(Widget w) { dependants.push_back(w); return *this; }

    void Start() {
      if (running) return;          // Already running!
      running = true;
      start_time = emp::GetTime();  // Record the time that we started the animation.
      cur_time = start_time;        // Initialize cur_time to now.
      AdvanceFrame();               // Take the first animation step to get going.
    }
    void Stop() {
      run_time += emp::GetTime() - start_time;
      running = false;
    }
  };

}
}


#endif
