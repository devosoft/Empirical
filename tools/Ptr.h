#ifndef EMP_PTR_H
#define EMP_PTR_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This version of pointers act as normal pointers under most conditions.  However,
//  if a program is compiled with EMP_TRACK_MEM set, then these pointers perform extra
//  tests to ensure that they point to valid memory and that memory is freed before
//  pointers are released.
//

#include <map>

#undef EMP_IF_TRACK
#ifdef EMP_TRACK_MEM
#define EMP_IF_TRACK(STATEMENT) STATEMENT;
#else
#define EMP_IF_TRACK(STATEMENT)
#endif

namespace emp {

  class PtrInfo {
  private:
    int count;
    bool active;
  public:
    PtrInfo() : count(0), active(true) { ; }

    int GetCount() const { return count; }
    bool IsActive() const { return active; }

    void Inc() { count++; }
    void Dec() { count--; }
    void MarkDeleted() { active = false; }
  };
    

  
  template <typename TYPE>
  class PtrTracker {
  private:
    std::map<TYPE *, PtrInfo> ptr_count;

    PtrTracker() { ; } // Make sure trackers can't be built outside of this class.
  public:
    ~PtrTracker() { ; }

    static PtrTracker<TYPE> & Get() { static PtrTracker<TYPE> tracker; return tracker; }

    void Inc(TYPE * ptr) { ptr_count[ptr].Inc(); }
    void Dec(TYPE * ptr) {
      // @CAO Make sure this pointer is not dropping below zero! (Internal error?)
      // @CAO Make sure this pointer is deleted before it drops to zero.
      ptr_count[ptr].Dec();
    }
    void MarkDeleted() {
      // @CAO Make sure this pointer exists before deleting it!
      ptr_count[ptr].MarkDeleted();
    }
  };

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

    Ptr<TYPE> & operator=(const Ptr<TYPE> & _in) { ptr=_in.ptr; return *this; }
    TYPE & operator*() { return *ptr; }
  };

};

#endif
