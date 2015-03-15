#ifndef EMP_PTR_H
#define EMP_PTR_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This version of pointers act as normal pointers under most conditions.  However, if
//  a program is compiled with EMP_TRACK_MEM set, then these pointers will performs
//  substantial extra tests to make sure that they point to valid memory and that all
//  memory is freed before pointers are released.
//

namespace emp {
  
  template <typename TYPE>
  class Ptr {
  private:
    TYPE * ptr;
  public:
    Ptr() : ptr(NULL) { ; }
    Ptr(TYPE * in_ptr) : ptr(in_ptr) { ; }
    Ptr(TYPE & obj) : ptr(&obj) { ; }
    Ptr(const Ptr<TYPE> & _in) : ptr(_in.ptr) { ; }
    ~Ptr() { ; }

    void New() { ptr = new type; }
    void Delete() { delete ptr; }
  };

};

#endif
