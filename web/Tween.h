#ifndef EMP_WEB_TWEEN_H
#define EMP_WEB_TWEEN_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This class manages the gradual movement of a widget from one state to another.
//

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
    };

    struct Event {
      double time;
      std::function<void()> trigger;
    };

    double duration;
    Widget default_target;
    emp::vector<Path*> paths;
    emp::vector<Event*> events;

    static double LINEAR(double in) { return in; }

  public:
    Tween(double d, const Widget & t=nullptr) : duration(d), default_target(t) { ; }
    ~Tween() {
      for (auto * p : paths) delete p;
      for (auto * e : events) delete e;
    }
    
    double GetDuration() const { return duration; }
    Widget GetDefaultTarget() const { return default_target; }
    
    void AddPath(double duration, std::function<void(double)> set_fun,
                 double start_val, double end_val, std::function<double(double)> timing=LINEAR) {
      Path * new_path = new Path({set_fun, start_val, end_val, timing});
      paths.push_back(new_path);
    }

    void AddPath(double duration, Widget w, std::string setting, double start_val, double end_val) {
    }
  };

}
}


#endif
