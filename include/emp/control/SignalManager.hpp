/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file SignalManager.hpp
 *  @brief This file defines the SignalManager class, which collects sets of Signals to be looked up
 * or manipulated later.
 */

#ifndef EMP_CONTROL_SIGNALMANAGER_HPP_INCLUDE
#define EMP_CONTROL_SIGNALMANAGER_HPP_INCLUDE

#include <string>
#include <unordered_map>

#include "../tools/string_utils.hpp"

#include "Signal.hpp"

namespace emp {

  class SignalManager : public internal::SignalManager_Base {
  private:
    std::unordered_map<std::string, SignalBase *> signal_map;
    uint32_t next_id=1;
    std::string prefix = "emp_signal_";

    // Generate a unique signal name to prevent duplicates.
    inline std::string GenerateSignalName(const std::string & pre="") {
      if (pre=="") return emp::to_string(prefix, next_id++);  // Use default version.
      return emp::to_string(pre, next_id++);
    }
  public:
    SignalManager() : signal_map() { ; }
    SignalManager(SignalManager &&) = default;     // Normal juggle is okay for move constructor
    SignalManager(const SignalManager & in) : signal_map(), next_id(in.next_id), prefix(in.prefix) {
      // Copy all signals from input manager.
      for (const auto & x : in.signal_map) {
        signal_map[x.first] = x.second->Clone();
      }
    }
    ~SignalManager() { for (auto & x : signal_map) delete x.second; }

    uint32_t GetNextID() const { return next_id; }
    size_t GetSize() const { return signal_map.size(); }

    SignalBase & operator[](const std::string & name) {
      emp_assert(signal_map.find(name) != signal_map.end());
      return *(signal_map[name]);
    }
    const SignalBase & operator[](const std::string & name) const {
      auto it = signal_map.find(name);
      emp_assert(it != signal_map.end());
      return *(it->second);
    }

//    template <typename... ARGS>
    template <typename FUN_T>
    auto & Add(const std::string & name) {
      auto * new_signal = new Signal<FUN_T>(name);
      signal_map[name] = new_signal;
      new_signal->signal_id = next_id++;
      new_signal->managers.push_back(this);
      new_signal->prime_manager = this;
      return *new_signal;
    }

    template <typename FUN_T>
    auto & Add(Signal<FUN_T> & signal) {
      signal_map[signal.name] = &signal;
      signal.signal_id = next_id++;       // @CAO: Who should control the signal id?
      signal.managers.push_back(this);
      return signal;
    }

    void NotifyConstruct(SignalBase * signal) override {
      // This signal is currently being constructed and should be added to this manager.
      signal_map[signal->name] = signal;
      signal->signal_id = next_id++;      // @CAO: Who should control the signal id?
      signal->managers.push_back(this);
    }

    void NotifyDestruct(SignalBase * signal) override {
      // This signal is no longer valid and needs to be removed from this manager.
      signal_map.erase(signal->name);
    }

    void PrintNames(std::ostream & os=std::cout) {
      os << signal_map.size() << " signals found:\n";
      for (auto & x : signal_map) os << "  " << x.first << std::endl;
    }
  };

}

#endif // #ifndef EMP_CONTROL_SIGNALMANAGER_HPP_INCLUDE
