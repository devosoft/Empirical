#ifndef EMP_INSTRUCTION_LIB_H
#define EMP_INSTRUCTION_LIB_H

#include <functional>
#include <string>
#include <vector>

namespace emp {
  struct InstInfo {
    std::string name;    // Name of this instruction
    char short_name;     // Single character representation of this instruction
    int cycle_cost;      // CPU Cycle Cost to execute this instruction
    double mut_weight;   // Relative weight of mutating to this instruction.
  };

  template <typename HARDWARE_TYPE> class InstLib {
  private:
    std::vector< std::function<bool(HARDWARE_TYPE&)> inst_lib;
    std::vector<InstInfo> inst_info;

  public:
    InstLib() { ; }
    ~InstLib() { ; }

    
  };
};

#endif
