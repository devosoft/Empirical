

// This file maintains information about events available in virtual hardware.

#ifndef EMP_EVENT_LIB_H
#define EMP_EVENT_LIB_H

#include <functional>
#include <unordered_set>

#include "../tools/FunctionSet.h"

namespace emp {

  template<typename HARDWARE_T>
  class EventLib {
  public:
    using hardware_t = HARDWARE_T;
    using event_t = typename hardware_t::event_t;
    using fun_t = std::function<void(hardware_t &, const event_t &)>;
    using properties_t = std::unordered_set<std::string>;
    using fun_set_t = FunctionSet<void(hardware_t &, const event_t &)>;

    struct EventDef {
      std::string name;     // Name of this event.
      fun_t handler;        // Function to call to handle this event.
      std::string desc;     // Description of event.
      properties_t properties; // Any properties that should be associated with this type of event.
      fun_set_t on_trigger;  // Functions to call when this type of event is triggered.

      EventDef(const std::string & _n, fun_t _handler, const std::string & _d,
        const properties_t & _properties, const fun_set_t & _on_trigger_fun_set)
        : name(_n), handler(_handler), desc(_d), properties(_properties),
          on_trigger(_on_trigger_fun_set) { ; }
      EventDef(const EventDef &) = default;
    };

  protected:
    

  public:

  //void TriggerEvent(hardware_t &, const event_t &) { ; }

  };

}

#endif
