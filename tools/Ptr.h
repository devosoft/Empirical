#ifndef EMP_PTR_H
#define EMP_PTR_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This version of pointers act as normal pointers under most conditions.  However,
//  if a program is compiled with EMP_TRACK_MEM set, then these pointers perform extra
//  tests to ensure that they point to valid memory and that memory is freed before
//  pointers are released.
//

// #define EMP_TRACK_MEM

#include <map>

#include "assert.h"

namespace emp {

  class PtrInfo {
  private:
    int count;     // How many of this pointer do we have?
    bool active;   // Has this pointer been deleted? (i.e., we should no longer access it!)
    bool owner;    // Are we responsible for deleting this pointer?
  public:
    PtrInfo(bool is_owner=false) : count(0), active(true), owner(is_owner) { ; }
    ~PtrInfo() { ; }

    int GetCount() const { return count; }
    bool IsActive() const { return active; }
    bool IsOwner() const { return owner; }

    void Inc() { count++; }
    void Dec() {
      emp_assert(count > 0);        // Do not decrement more copies than we have!

      // Make sure one of these conditions is true:
      // * We are not getting rid of the last copy, -or-
      // * We've already deleted this pointer, -or-
      // * We're not responsible for deleting this pointer.
      emp_assert(count > 1 || active == false || owner == false);
      count--;
    }

    void MarkDeleted() {
      emp_assert(active == true);   // Do not delete a pointer more than once!
      emp_assert(owner == true);    // We should only be deleting pointers we own!
      active = false;
    }
    void Claim() {
      emp_assert(owner == false);    // We can only claim pointer we don't already own!
      owner = true;
    }
    void Surrender() {
      emp_assert(active == true);   // Do not surrender an inactive pointer!
      emp_assert(owner == true);    // We can only surrender a pointer we own!
      owner = false;
    }
  };
  
  
  template <typename TYPE>
  class PtrTracker {
  private:
    std::map<TYPE *, PtrInfo> ptr_count;

    // Make sure trackers can't be built outside of this class.
    PtrTracker() { ; } 
    PtrTracker(const PtrTracker<int> &) { ; } 
    PtrTracker<int> & operator=(const PtrTracker<TYPE> &) { return this; }
  public:
    ~PtrTracker() { ; }

    // Treat this class as a singleton with a single Get() method to retrieve it.
    static PtrTracker<TYPE> & Get() { static PtrTracker<TYPE> tracker; return tracker; }

    // This pointer was just created as a Ptr!
    void New(TYPE * ptr) {
      emp_assert(ptr_count.find(ptr) == ptr_count.end()); // Make sure pointer does not exist!
      ptr_count[ptr] = PtrInfo(true);
    }
 
    // This pointer was already created, but given to Ptr.
    void Old(TYPE * ptr) {
      emp_assert(ptr_count.find(ptr) == ptr_count.end()); // Make sure pointer does not exist!
      ptr_count[ptr] = PtrInfo(false);
    }
    void Inc(TYPE * ptr) {
      emp_assert(ptr_count.find(ptr) != ptr_count.end()); // Make sure pointer already exists!
      ptr_count[ptr].Inc();
    }
    void Dec(TYPE * ptr) {
      ptr_count[ptr].Dec();
    }
    void MarkDeleted(TYPE * ptr) {
      emp_assert(ptr_count.find(ptr) != ptr_count.end()); // Make sure pointer actually exists!
      ptr_count[ptr].MarkDeleted();
    }
  };


#undef EMP_IF_MEMTRACK
#ifdef EMP_TRACK_MEM
#define EMP_IF_MEMTRACK(STATEMENTS) { STATEMENTS }
#else
#define EMP_IF_MEMTRACK(STATEMENTS)
#endif

  template <typename TYPE>
  class Ptr {
  private:
    TYPE * ptr;

#ifdef EMP_TRACK_MEM
    static PtrTracker<TYPE> & Tracker() { return PtrTracker<TYPE>::Get(); }
#endif
  public:
    Ptr() : ptr(NULL) { ; }
    Ptr(TYPE * in_ptr) : ptr(in_ptr) { EMP_IF_MEMTRACK( Tracker().New(ptr); ); }
    Ptr(TYPE & obj) : ptr(&obj) { EMP_IF_MEMTRACK( Tracker().Old(ptr); ); }
    Ptr(const Ptr<TYPE> & _in) : ptr(_in.ptr) { EMP_IF_MEMTRACK( Tracker().Inc(ptr); ); }
    ~Ptr() { EMP_IF_MEMTRACK( Tracker().Dec(ptr); ); }

    bool IsNull() { return ptr == NULL; }

    void New() {
      ptr = new TYPE;
      EMP_IF_MEMTRACK(Tracker().New(ptr););
    }
    void New(const TYPE & init_val) {
      ptr = new TYPE(init_val);
      EMP_IF_MEMTRACK(Tracker().New(ptr););
    }
    void Delete() {
      delete ptr;
      EMP_IF_MEMTRACK( Tracker().MarkDeleted(ptr); );
    }

    Ptr<TYPE> & operator=(const Ptr<TYPE> & _in) {
      ptr=_in.ptr;
      EMP_IF_MEMTRACK(Tracker().Inc(ptr););
      return *this;
    }

    TYPE & operator*() { return *ptr; }
  };

};

#endif
