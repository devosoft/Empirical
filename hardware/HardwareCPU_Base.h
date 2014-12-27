#ifndef EMP_HARDWARE_CPU_BASE_H
#define EMP_HARDWARE_CPU_BASE_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  HardwareCPU_Base is a base class for all CPU-style hardware types, with a number of helper
//  classes and member functions.
//
//  Note: all components are protected so they can only be used internally in derived classes.
//
//  These include:
//    CPUStack<STACK_SIZE> - a templated class that builds a fast, roll-over integer stack.
//    CPUHead - a class that points to a position in a vector of instructions
//


#include <vector>

namespace emp {

  template <typename INST_TYPE> class HardwareCPU_Base {
  protected:
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


    // Heads point to a position in memory; since they can be assoicated with multiple memories,
    // they must also track *which* memory they are currently working in.
    class CPUHead {
    private:
      std::vector<INST_TYPE> * memory;
      int position;

      void Adjust() {
        const int mem_size = memory->size();
        if (position < 0) position = (position % mem_size) + mem_size;
        else if (position > mem_size) position %= mem_size;
        // Note, % only happens if out of range.
      }
    public:
      CPUHead() : memory(NULL), position(0) { ; }
      CPUHead(std::vector<INST_TYPE> & _mem, int _pos=0) : memory(&_mem), position(_pos) { ; }
      CPUHead(const CPUHead & _in) : memory(_in.memory), position(_in.position) { ; }
      ~CPUHead() { ; }

      std::vector<INST_TYPE> & GetMemory() { return *memory; }
      int GetPosition() const { return position; }

      const INST_TYPE & GetInst() const { return (*memory)[position]; }
      INST_TYPE & GetInst() { return (*memory)[position]; }

      CPUHead & operator=(const CPUHead & _in) {
        memory = _in.memory;
        position = _in.position;
        return *this;
      }

      bool operator==(const CPUHead & _in) const {
        return (memory == _in.memory && position == _in.position);
      }
      bool operator!=(const CPUHead & _in) const { return !operator==(_in); }

      CPUHead & Set(int _pos) { position = _pos; return *this; }
      CPUHead & Set(std::vector<INST_TYPE> & _mem, int _pos=0) {
        memory = &_mem; position = _pos; return *this;
      }

      CPUHead & operator++() { if (++position >= memory.size()) position = 0; return *this; }
      CPUHead & operator--() { if (--position < 0) position = memory.size() - 1; return *this; }      

      CPUHead & operator+=(int shift) { position += shift; Adjust(); return *this; }
      CPUHead & operator-=(int shift) { position -= shift; Adjust(); return *this; }
    };


  };

};

#endif

