
#ifndef EMP_META_GP_HARDWARE_H
#define EMP_META_GP_HARDWARE_H

#include <unordered_map>

namespace emp {
  namespace MetaGP {

    class MetaHardware {
    public:

      struct Event {
        std::unordered_map<int, double> msg;
      };

      // A core manages a single 'thread' of execution.
      // Multiple cores may be running simultaneously.
      class Core {
      public:
        // using call_stack_t = emp::vector<BaseHardware_State>;

      protected:
        // Stack
        // call_stack_t call_stack;

      public:
        Core() { ; }
      };

    protected:
      double min_bind_threshold;
      double max_call_depth;
      double max_core_count;

      std::unordered_map<int, double> shared_memory;  // Memory shared among all cores.
      //std::program

    public:

    };
  }
}


#endif
