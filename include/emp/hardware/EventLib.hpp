/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file EventLib.hpp
 *  @brief This file maintains information about events available in virtual hardware.
 *
 *  This file is largely based on InstLib.h.
 */

#ifndef EMP_HARDWARE_EVENTLIB_HPP_INCLUDE
#define EMP_HARDWARE_EVENTLIB_HPP_INCLUDE

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <unordered_set>

#include "../base/vector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../functional/FunctionSet.hpp"

namespace emp {

  template<typename HARDWARE_T>
  class EventLib {
  public:
    using hardware_t = HARDWARE_T;
    using event_t = typename hardware_t::event_t;
    using fun_t = std::function<void(hardware_t &, const event_t &)>;    //< Function type alias for event handler functions.
    using properties_t = std::unordered_set<std::string>;                //< Type for event definition properties.
    using fun_set_t = FunctionSet<void(hardware_t &, const event_t &)>;  //< Type for event dispatch function sets.

    /// Event definition structure. Maintains information about a type of event.
    struct EventDef {
      std::string name;         //< Name of this event.
      fun_t handler;            //< Function to call to handle this event.
      std::string desc;         //< Description of event.
      properties_t properties;  //< Any properties that should be associated with this type of event.
      fun_set_t dispatch_funs;  //< Functions to call when this type of event is triggered.

      EventDef(const std::string & _n, fun_t _handler, const std::string & _d,
        const properties_t & _properties)
        : name(_n), handler(_handler), desc(_d), properties(_properties),
          dispatch_funs() { ; }

      EventDef(const EventDef &) = default;
    };

  protected:
    emp::vector<EventDef> event_lib;                //< Full definitions of each event.
    std::map<std::string, size_t> name_map;         //< Event name -> ID map.


  public:
    EventLib() : event_lib(), name_map() { ; }
    EventLib(const EventLib &) = default;
    ~EventLib() { ; }

    EventLib & operator=(const EventLib &) = default;
    EventLib & operator=(EventLib &&) = default;

    /// Get the string name of the specified event definition.
    const std::string & GetName(size_t id) const { return event_lib[id].name; }

    /// Get the handler function of the specified event definition.
    const fun_t & GetHandler(size_t id) const { return event_lib[id].handler; }

    /// Get the dispatch function set of the specified event definition.
    const fun_set_t & GetDispatchFuns(size_t id) const { return event_lib[id].dispatch_funs; }

    /// Get the string description of the specified event definition.
    const std::string & GetDesc(size_t id) const { return event_lib[id].desc; }

    /// Get a const reference to an event definition's properties.
    const properties_t & GetProperties(size_t id) const { return event_lib[id].properties; }

    /// Does the event definition specified by id have the property specified.
    bool HasProperty(size_t id, std::string property) const { return event_lib[id].properties.count(property); }

    /// Get the number of events registered to this event library.
    size_t GetSize() const { return event_lib.size(); }

    /// Get the event ID of the event given by string name.
    size_t GetID(const std::string & name) const {
      emp_assert(Has(name_map, name), name);
      return Find(name_map, name, (size_t)-1);
    }

    /// Add a new event to the event library.
    void AddEvent(const std::string & name,
                  const fun_t & handler_fun,
                  const std::string & desc="",
                  const properties_t & event_properties=properties_t())
    {
      const size_t id = event_lib.size();
      event_lib.emplace_back(name, handler_fun, desc, event_properties);
      name_map[name] = id;
    }

    /// Register a dispatch function for the event specified by id.
    void RegisterDispatchFun(size_t id, fun_t dispatch_fun) {
      event_lib[id].dispatch_funs.Add(dispatch_fun);
    }

    /// Register a dispatch function for the event specified by name.
    void RegisterDispatchFun(const std::string & name, fun_t dispatch_fun) {
      event_lib[GetID(name)].dispatch_funs.Add(dispatch_fun);
    }

    /// Trigger event.
    void TriggerEvent(hardware_t & hw, const event_t & event) const {
      event_lib[event.id].dispatch_funs.Run(hw, event);
    }

    /// Handle event.
    void HandleEvent(hardware_t & hw, const event_t & event) const {
      event_lib[event.id].handler(hw, event);
    }

  };

}

#endif // #ifndef EMP_HARDWARE_EVENTLIB_HPP_INCLUDE
