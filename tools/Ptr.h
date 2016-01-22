//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This version of pointers act as normal pointers under most conditions.  However,
//  if a program is compiled with EMP_TRACK_MEM set, then these pointers perform extra
//  tests to ensure that they point to valid memory and that memory is freed before
//  pointers are released.


#ifndef EMP_PTR_H
#define EMP_PTR_H

#include <map>

#include "assert.h"

namespace emp {

  class PtrInfo {
  private:
    int count;     // How many of this pointer do we have?
    bool active;   // Has this pointer been deleted? (i.e., we should no longer access it!)
    bool owner;    // Are we responsible for deleting this pointer?
  public:
    PtrInfo(bool is_owner=false) : count(1), active(true), owner(is_owner) { ; }
    ~PtrInfo() { ; }

    int GetCount() const { return count; }
    bool IsActive() const { return active; }
    bool IsOwner() const { return owner; }

    void Inc() { emp_assert(active); count++; }
    void Dec() {
      // If this pointer is not active, it doesn't matter how many copies we have.
      if (active == false) return; 

      emp_assert(count > 0);        // Do not decrement more copies than we have!

      // Make sure one of these conditions is true:
      // * We are not getting rid of the last copy, -or-
      // * We've already deleted this pointer, -or-
      // * We're not responsible for deleting this pointer.
      emp_assert(count > 1 || owner == false);
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
    bool verbose;

    // Make sure trackers can't be built outside of this class.
    PtrTracker() : verbose (false) { ; } 
    PtrTracker(const PtrTracker<int> &) = delete;
    PtrTracker<int> & operator=(const PtrTracker<TYPE> &) = delete;
  public:
    ~PtrTracker() { ; }

    bool GetVerbose() const { return verbose; }
    void SetVerbose(bool v=true) { verbose = v; }

    // Treat this class as a singleton with a single Get() method to retrieve it.
    static PtrTracker<TYPE> & Get() { static PtrTracker<TYPE> tracker; return tracker; }

    // Some simple accessors
    bool HasPtr(TYPE * ptr) const {
      if (verbose) std::cout << "HasPtr: " << ((uint64_t) ptr) << std::endl;
      return ptr_count.find(ptr) != ptr_count.end();
    }
    bool IsActive(TYPE * ptr) const {
      if (verbose) std::cout << "Active: " << ((uint64_t) ptr) << std::endl;
      if (!HasPtr(ptr)) return false;
      return ptr_count.find(ptr)->second.IsActive();
    }
    bool IsOwner(TYPE * ptr) const {
      if (verbose) std::cout << "Owner:  " << ((uint64_t) ptr) << std::endl;
      if (!HasPtr(ptr)) return false;
      return ptr_count.find(ptr)->second.IsOwner();
    }
    int GetCount(TYPE * ptr) const {
      if (verbose) std::cout << "Count:  " << ((uint64_t) ptr) << std::endl;
      if (!HasPtr(ptr)) return 0;
      return ptr_count.find(ptr)->second.GetCount();
    }

    // This pointer was just created as a Ptr!
    void New(TYPE * ptr) {
      if (verbose) std::cout << "New:    " << ((uint64_t) ptr) << std::endl;
      emp_assert(!HasPtr(ptr) || !IsActive(ptr)); // Make sure pointer is not already stored!
      ptr_count[ptr] = PtrInfo(true);
    }
 
    // This pointer was already created, but given to Ptr.
    void Old(TYPE * ptr) {
      if (verbose) std::cout << "Old:    " << ((uint64_t) ptr) << std::endl;
      emp_assert(!HasPtr(ptr) || !IsActive(ptr)); // Make sure pointer is not already stored!
      ptr_count[ptr] = PtrInfo(false);
    }
    void Inc(TYPE * ptr) {
      if (verbose) std::cout << "Inc:    " << ((uint64_t) ptr) << std::endl;
      emp_assert(HasPtr(ptr));  // Make sure pointer IS already stored!
      ptr_count[ptr].Inc();
    }
    void Dec(TYPE * ptr) {
      if (verbose) std::cout << "Dec:    " << ((uint64_t) ptr) << std::endl;
      emp_assert(HasPtr(ptr));  // Make sure pointer IS already stored!
      ptr_count[ptr].Dec();
    }
    void MarkDeleted(TYPE * ptr) {
      if (verbose) std::cout << "Delete: " << ((uint64_t) ptr) << std::endl;
      emp_assert(HasPtr(ptr));  // Make sure pointer IS already stored!
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
      EMP_IF_MEMTRACK( if (ptr) Tracker().Dec(ptr); );
      ptr = new TYPE;
      EMP_IF_MEMTRACK(Tracker().New(ptr););
    }
    void New(const TYPE & init_val) {
      EMP_IF_MEMTRACK( if (ptr) Tracker().Dec(ptr); );
      ptr = new TYPE(init_val);
      EMP_IF_MEMTRACK(Tracker().New(ptr););
    }
    template <typename... T>
    void New(T... args) {
      EMP_IF_MEMTRACK( if (ptr) Tracker().Dec(ptr); );
      ptr = new TYPE(args...);
      EMP_IF_MEMTRACK(Tracker().New(ptr););
    }
    void Delete() {
      EMP_IF_MEMTRACK( Tracker().MarkDeleted(ptr); );
      EMP_IF_MEMTRACK( Tracker().Dec(ptr); );
      delete ptr;
    }

    Ptr<TYPE> & operator=(const Ptr<TYPE> & _in) {
      EMP_IF_MEMTRACK( if (ptr) Tracker().Dec(ptr); );
      ptr=_in.ptr;
      EMP_IF_MEMTRACK(Tracker().Inc(ptr););
      return *this;
    }

    TYPE & operator*() { return *ptr; }
    TYPE * operator->() { return ptr; }

    bool operator==(const Ptr<TYPE> & in_ptr) { return ptr == in_ptr.ptr; }
    bool operator!=(const Ptr<TYPE> & in_ptr) { return ptr != in_ptr.ptr; }
    bool operator<(const Ptr<TYPE> & in_ptr)  { return ptr < in_ptr.ptr; }
    bool operator<=(const Ptr<TYPE> & in_ptr) { return ptr <= in_ptr.ptr; }
    bool operator>(const Ptr<TYPE> & in_ptr)  { return ptr > in_ptr.ptr; }
    bool operator>=(const Ptr<TYPE> & in_ptr) { return ptr >= in_ptr.ptr; }

    // Some debug testing functions
#ifdef EMP_TRACK_MEM
    int DebugGetCount() const { return Tracker().GetCount(ptr); }
#endif

  };

}

#endif // EMP_PTR_H
