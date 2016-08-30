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

    void Inc() { emp_assert(active, "Incrementing deleted pointer!"); count++; }
    void Dec() {
      // If this pointer is not active, it doesn't matter how many copies we have.
      if (active == false) return;

      emp_assert(count > 0, "Decrementing Ptr count; already zero!");        // Do not decrement more copies than we have!

      // Make sure one of these conditions is true:
      // * We are not getting rid of the last copy, -or-
      // * We've already deleted this pointer, -or-
      // * We're not responsible for deleting this pointer.
      emp_assert(count > 1 || owner == false, "Removing last reference to owned Ptr!");
      count--;
    }

    void MarkDeleted() {
      emp_assert(active == true, "Deleting same emp::Ptr a second time!");
      emp_assert(owner == true, "Deleting emp::Ptr we don't own!");
      active = false;
    }
    void Claim() {
      emp_assert(owner == false, "Claiming an emp::Ptr that we already own!");
      owner = true;
    }
    void Surrender() {
      emp_assert(active == true, "Surrendering emp::Ptr that was deallocated!");
      emp_assert(owner == true, "Surrendering emp::Ptr that we don't own!");
      owner = false;
    }
  };


  class PtrTracker {
  private:
    std::map<void *, PtrInfo> ptr_info;
    bool verbose;

    // Make sure trackers can't be built outside of this class.
    PtrTracker() : verbose(false) { ; }
    PtrTracker(const PtrTracker &) = delete;
    PtrTracker(PtrTracker &&) = delete;
    PtrTracker & operator=(const PtrTracker &) = delete;
    PtrTracker & operator=(PtrTracker &&) = delete;
  public:
    ~PtrTracker() { ; }

    bool GetVerbose() const { return verbose; }
    void SetVerbose(bool v=true) { verbose = v; }

    // Treat this class as a singleton with a single Get() method to retrieve it.
    static PtrTracker & Get() { static PtrTracker tracker; return tracker; }

    // Some simple accessors
    bool HasPtr(void * ptr) const {
      if (verbose) std::cout << "HasPtr: " << ((uint64_t) ptr) << std::endl;
      return ptr_info.find(ptr) != ptr_info.end();
    }
    bool IsActive(void * ptr) const {
      if (verbose) std::cout << "Active: " << ((uint64_t) ptr) << std::endl;
      if (!HasPtr(ptr)) return false;
      return ptr_info.find(ptr)->second.IsActive();
    }
    bool IsOwner(void * ptr) const {
      if (verbose) std::cout << "Owner:  " << ((uint64_t) ptr) << std::endl;
      if (!HasPtr(ptr)) return false;
      return ptr_info.find(ptr)->second.IsOwner();
    }
    int GetCount(void * ptr) const {
      if (verbose) std::cout << "Count:  " << ((uint64_t) ptr) << std::endl;
      if (!HasPtr(ptr)) return 0;
      return ptr_info.find(ptr)->second.GetCount();
    }

    // This pointer was just created as a Ptr!
    void New(void * ptr) {
      if (ptr == nullptr) return;
      if (verbose) std::cout << "New:    " << ((uint64_t) ptr) << std::endl;
      emp_assert(!HasPtr(ptr) || !IsActive(ptr)); // Make sure pointer is not already stored!
      ptr_info[ptr] = PtrInfo(true);
    }

    // This pointer was already created, but given to Ptr.
    void Old(void * ptr) {
      if (ptr == nullptr) return;
      if (verbose) std::cout << "Old:    " << ((uint64_t) ptr) << std::endl;
      // If we already have this pointer, just increment the count.  Otherwise track it now.
      if (HasPtr(ptr) && IsActive(ptr)) Inc(ptr);
      else ptr_info[ptr] = PtrInfo(false);
    }
    void Inc(void * ptr) {
      if (ptr == nullptr) return;
      if (verbose) std::cout << "Inc:    " << ((uint64_t) ptr) << std::endl;
      emp_assert(HasPtr(ptr));  // Make sure pointer IS already stored!
      ptr_info[ptr].Inc();
    }
    void Dec(void * ptr) {
      if (ptr == nullptr) return;
      if (verbose) std::cout << "Dec:    " << ((uint64_t) ptr) << std::endl;
      emp_assert(HasPtr(ptr));  // Make sure pointer IS already stored!
      ptr_info[ptr].Dec();
    }
    void MarkDeleted(void * ptr) {
      if (verbose) std::cout << "Delete: " << ((uint64_t) ptr) << std::endl;
      emp_assert(HasPtr(ptr));  // Make sure pointer IS already stored!
      ptr_info[ptr].MarkDeleted();
    }
  };


#undef EMP_IF_MEMTRACK
#ifdef EMP_TRACK_MEM
#define EMP_IF_MEMTRACK(...) { __VA_ARGS__ }
#else
#define EMP_IF_MEMTRACK(...)
#endif

  template <typename TYPE>
  class Ptr {
  private:
    TYPE * ptr;

#ifdef EMP_TRACK_MEM
    static PtrTracker & Tracker() { return PtrTracker::Get(); }
#endif
  public:
    Ptr() : ptr(nullptr) { ; }
    template <typename T2> Ptr(T2 * in_ptr, bool is_new=true) : ptr(in_ptr) {
      (void) is_new;  // Avoid unused parameter error when EMP_IF_MEMTRACK is off.
      EMP_IF_MEMTRACK( if (is_new) Tracker().New(ptr); else Tracker().Old(ptr); );
    }
    template <typename T2> Ptr(Ptr<T2> _in) : ptr(_in.Raw()) {
      EMP_IF_MEMTRACK( Tracker().Inc(ptr); );
    }
    Ptr(std::nullptr_t) : Ptr() { ; }
    Ptr(TYPE & obj) : Ptr(&obj, false) {;} // Pre-existing objects NOT tracked
    Ptr(const Ptr<TYPE> & _in) : ptr(_in.ptr) { EMP_IF_MEMTRACK( Tracker().Inc(ptr); ); }
    Ptr(Ptr<TYPE> && _in) : ptr(_in.ptr) { _in.ptr=nullptr; }
    ~Ptr() { EMP_IF_MEMTRACK( Tracker().Dec(ptr); ); }

    bool IsNull() const { return ptr == nullptr; }
    TYPE * Raw() { return ptr; }
    const TYPE * const Raw() const { return ptr; }
    template <typename T2> T2 * Cast() { return (T2*) ptr; }
    template <typename T2> const T2 * const Cast() const { return (T2*) ptr; }
    template <typename T2> T2 * DynamicCast() {
      emp_assert(dynamic_cast<T2>(ptr) != nullptr);
      return (T2*) ptr;
    }

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
      ptr = new TYPE(std::forward<T>(args)...);
      EMP_IF_MEMTRACK(Tracker().New(ptr););
    }
    void Delete() {
      EMP_IF_MEMTRACK( Tracker().MarkDeleted(ptr); );
      EMP_IF_MEMTRACK( Tracker().Dec(ptr); );
      delete ptr;
    }

    template <typename T2>
    Ptr<TYPE> & operator=(T2 * _in) {
      EMP_IF_MEMTRACK( if (ptr) Tracker().Dec(ptr); );
      ptr = _in;
      EMP_IF_MEMTRACK(Tracker().New(ptr););
      return *this;
    }
    template <typename T2>
    Ptr<TYPE> & operator=(Ptr<T2> _in) {
      EMP_IF_MEMTRACK( if (ptr) Tracker().Dec(ptr); );
      ptr=_in.Raw();
      EMP_IF_MEMTRACK(Tracker().Inc(ptr););
      return *this;
    }
    Ptr<TYPE> & operator=(Ptr<TYPE> & _in) {
      EMP_IF_MEMTRACK( if (ptr) Tracker().Dec(ptr); );
      ptr=_in.Raw();
      EMP_IF_MEMTRACK(Tracker().Inc(ptr););
      return *this;
    }
    Ptr<TYPE> & operator=(Ptr<TYPE> && _in) {
      ptr = _in.ptr;
      _in.ptr = nullptr;
      return *this;
    }
    TYPE & operator*() { return *ptr; }
    TYPE * operator->() { return ptr; }
    const TYPE * const operator->() const { return ptr; }
    operator TYPE *() { return ptr; }
    operator bool() { return ptr != nullptr; }

    // Allow Ptr to be treated as an array?
    // @CAO commented out for now; would need to track array status to call delete[]
    // TYPE & operator[](int pos) { return ptr[pos]; }
    // const TYPE & operator[](int pos) const { return ptr[pos]; }

    bool operator==(const Ptr<TYPE> & in_ptr) const { return ptr == in_ptr.ptr; }
    bool operator!=(const Ptr<TYPE> & in_ptr) const { return ptr != in_ptr.ptr; }
    bool operator<(const Ptr<TYPE> & in_ptr)  const { return ptr < in_ptr.ptr; }
    bool operator<=(const Ptr<TYPE> & in_ptr) const { return ptr <= in_ptr.ptr; }
    bool operator>(const Ptr<TYPE> & in_ptr)  const { return ptr > in_ptr.ptr; }
    bool operator>=(const Ptr<TYPE> & in_ptr) const { return ptr >= in_ptr.ptr; }

    bool operator==(const TYPE * in_ptr) const { return ptr == in_ptr; }
    bool operator!=(const TYPE * in_ptr) const { return ptr != in_ptr; }
    bool operator<(const TYPE * in_ptr)  const { return ptr < in_ptr; }
    bool operator<=(const TYPE * in_ptr) const { return ptr <= in_ptr; }
    bool operator>(const TYPE * in_ptr)  const { return ptr > in_ptr; }
    bool operator>=(const TYPE * in_ptr) const { return ptr >= in_ptr; }

    // Some debug testing functions
#ifdef EMP_TRACK_MEM
    int DebugGetCount() const { return Tracker().GetCount(ptr); }
#endif

  };


    // Create a helper to replace & operator.
    template <typename T> Ptr<T> to_ptr(T & _in) { return Ptr<T>(_in); }

}

#endif // EMP_PTR_H
