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
      std::function<void(double)> setting;   // Function to set at each step
      std::function<double(double)> timing;  // Time dilation to use (e.g., to ease in and out)
      double start_val;
      double end_val;
    };

    struct Event {
      double time;
      std::function<void()> trigger;
    };

    double duration;
    Widget default_target;
    emp::vector<Path*> paths;
    emp::vector<Event*> trigger;

  public:
    Tween(double d, const Widget & t=nullptr) : duration(d), default_target(t) { ; }
    ~Tween();

    void AddPath(double duration, Widget w, std::string setting, double start_val, double end_val) {
    }
  };

}
}


#endif
