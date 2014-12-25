#ifndef EMP_HARDWARE_CPU_BASE_H
#define EMP_HARDWARE_CPU_BASE_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  HardwareCPU_Base is a base class for all CPU-style hardware types, with a number of helper
//  classes and member functions.
//
//  Note: all components are protected so they can only be used internally in derived classes.
//

#include <vector>

namespace emp {

  template <typename INST_TYPE> class HardwareCPU_Base {
  protected:
    std::vector<INST_TYPE> code;

    HardwareCPU_Base() { ; }
    ~HardwareCPU_Base() { ; }

    // Stacks are often a major component of CPU hardware.  This class is a lossy stack that
    // overflows any data beyond STACK_SIZE.
    template <int STACK_SIZE> class CPUStack {
    private:
      int stack_info[STACK_SIZE];  // All info on stack (loops around)
      int top;                     // Top, active position on stack.
    public:
      CPUStack() : top(0) { Clear(); }
      ~CPUStack() { ; }

      CPUStack & Push(int value) {
        (++top) %= STACK_SIZE;
        stack_info[top] = value;
        return *this;
      }

      int Pop() {
        int out_value = stack_info[top];
        stack_info[top] = 0;
        if (top == 0) top = STACK_SIZE;
        top--;
        return out_value;
      }

      int Top() const { return stack_info[top]; }
      int Capacity() const { return STACK_SIZE; }

      void Clear() { for (int i = 0; i < STACK_SIZE; i++) stack_info[i] = 0; }

      int & operator[](int index) {
        index = (top - index) % STACK_SIZE;  // Flip for direction of stack.
        if (index < 0) index+= STACK_SIZE;
        return stack_info[index];
      }

      int operator[](int index) const {
        index = (top - index) % STACK_SIZE;  // Flip for direction of stack.
        if (index < 0) index+= STACK_SIZE;
        return stack_info[index];
      }


    };

  };

};

#endif

