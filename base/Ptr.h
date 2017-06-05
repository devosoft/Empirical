//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A wrapper for pointers that does careful memory tracking (but only in debug mode).
//  Status: BETA
//
//  This version of pointers act as normal pointers under most conditions.  However,
//  if a program is compiled with EMP_TRACK_MEM set, then these pointers perform extra
//  tests to ensure that they point to valid memory and that memory is freed before
//  pointers are released.
//
//
//  Developer Notes:
//  * Switch pointer tracking to unordered_map?  (shouild be faster and smaller)
//  * Should we add on tracking for arrays (to allow indexing and know when to call delete[])?
//    Or just assume that arrays will be handled with emp::array?
//  * Make operator delete (and operator new) work with Ptr.

#ifndef EMP_PTR_H
#define EMP_PTR_H

#include <map>

#include "assert.h"

namespace emp {

  class PtrInfo {
  private:
    int count;        // How many of this pointer do we have?
    bool active;      // Has this pointer been deleted? (i.e., we should no longer access it!)
    bool owner;       // Are we responsible for deleting this pointer?
  public:
    PtrInfo(bool is_owner=false) : count(1), active(true), owner(is_owner) { ; }
    ~PtrInfo() { ; }

    int GetCount() const { return count; }
    bool IsActive() const { return active; }
    bool IsOwner() const { return owner; }

    void Inc() { emp_assert(active, "Incrementing deleted pointer!"); count++; }
    void Dec() {
      // Make sure one of these conditions is true:
      // * We are not getting rid of the last copy, -or-
      // * We've already deleted this pointer, -or-
      // * We're not responsible for deleting this pointer.
      emp_assert(count > 1 || active == false || owner == false,
                 "Removing last reference to owned Ptr!");
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
    void Release() {
      emp_assert(active == true, "Can only release allocated emp::Ptr!");
      emp_assert(owner == true, "Can only release emp::Ptr that we own!");
      owner = false;
    }
  };


  class PtrTracker {
  private:
    std::map<void *, PtrInfo> ptr_info;
    bool verbose;

    // Make PtrTracker a singleton.
    PtrTracker() : ptr_info(), verbose(false) { ; }
    PtrTracker(const PtrTracker &) = delete;
    PtrTracker(PtrTracker &&) = delete;
    PtrTracker & operator=(const PtrTracker &) = delete;
    PtrTracker & operator=(PtrTracker &&) = delete;
  public:
    ~PtrTracker() {
      // Track stats about pointer record.
      size_t total = 0;
      size_t owned = 0;
      size_t remain = 0;

      // Scan through remaining pointers and make sure all have been deleted.
      for (auto & x : ptr_info) {
        const PtrInfo & info = x.second;

        total++;
        if (info.IsOwner()) owned++;
        if (info.GetCount()) remain++;

        if (info.IsOwner() == false) continue;
        //emp_assert(info.GetCount() == 0, ptr, info.GetCount(), info.IsActive(), info.IsOwner());
        emp_assert(info.IsActive() == false,
                   x.first, info.GetCount(), info.IsActive(), info.IsOwner());
      }
      std::cout << "EMP_TRACK_MEM: No memory leaks found!\n "
                << total << " pointers found; "
                << owned << " owned and "
                << remain << " still have pointers to them (after deletion.)"
                << std::endl;
    }

    bool GetVerbose() const { return verbose; }
    void SetVerbose(bool v=true) { verbose = v; }

    // Treat this class as a singleton with a single Get() method to retrieve it.
    static PtrTracker & Get() { static PtrTracker tracker; return tracker; }

    // Some simple accessors
    bool HasPtr(void * ptr) const {
      if (verbose) std::cout << "HasPtr: " << ptr << std::endl;
      return ptr_info.find(ptr) != ptr_info.end();
    }

    bool IsActive(void * ptr) const {
      if (verbose) std::cout << "Active: " << ptr << std::endl;
      if (!HasPtr(ptr)) return false;
      return ptr_info.find(ptr)->second.IsActive();
    }

    bool IsOwner(void * ptr) const {
      if (verbose) std::cout << "Owner:  " << ptr << std::endl;
      if (!HasPtr(ptr)) return false;
      return ptr_info.find(ptr)->second.IsOwner();
    }

    int GetCount(void * ptr) const {
      if (verbose) std::cout << "Count:  " << ptr << std::endl;
      if (!HasPtr(ptr)) return 0;
      return ptr_info.find(ptr)->second.GetCount();
    }

    // This pointer was just created as a Ptr!
    void New(void * ptr) {
      if (ptr == nullptr) return;
      if (verbose) std::cout << "New:    " << ptr << std::endl;
      // Make sure pointer is not already stored -OR- hase been deleted (since re-use is possible).
      emp_assert(!HasPtr(ptr) || !IsActive(ptr));
      ptr_info[ptr] = PtrInfo(true);
    }

    // This pointer was already created, but given to Ptr.
    void Old(void * ptr) {
      if (ptr == nullptr) return;
      if (verbose) std::cout << "Old:    " << ptr << std::endl;
      // If we already have this pointer, just increment the count.  Otherwise track it now.
      if (HasPtr(ptr) && IsActive(ptr)) Inc(ptr);
      else ptr_info[ptr] = PtrInfo(false);
    }

    // Pointer is not currently owned, but should be.
    void Claim(void *ptr) {
      if (ptr == nullptr) return;
      if (verbose) std::cout << "Claim:  " << ptr << std::endl;
      // Make sure pointer IS already stored BUT is NOT active.
      emp_assert(HasPtr(ptr) && !IsActive(ptr));
      ptr_info[ptr].Claim();
    }

    void Inc(void * ptr) {
      if (ptr == nullptr) return;
      if (verbose) std::cout << "Inc:    " << ptr << std::endl;
      emp_assert(HasPtr(ptr));  // Make sure pointer IS already stored!
      ptr_info[ptr].Inc();
    }

    void Dec(void * ptr) {
      if (ptr == nullptr) return;
      if (verbose) std::cout << "Dec:    " << ptr << std::endl;
      emp_assert(HasPtr(ptr));  // Make sure pointer is already stored!
      auto & info = ptr_info[ptr];
      emp_assert(info.GetCount() > 0, "Decrementing Ptr count; already zero!", ptr,
                 info.IsActive(), info.IsOwner());
      info.Dec();
    }

    void MarkDeleted(void * ptr) {
      if (verbose) std::cout << "Delete: " << ptr << std::endl;
      emp_assert(HasPtr(ptr));  // Make sure pointer is already stored!
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
    using element_type = TYPE;

    Ptr() : ptr(nullptr) { ; }
    template <typename T2>
    Ptr(T2 * in_ptr, bool is_new=false) : ptr(in_ptr) {
      (void) is_new;  // Avoid unused parameter error when EMP_IF_MEMTRACK is off.
      emp_assert( dynamic_cast<TYPE*>(in_ptr) );
      EMP_IF_MEMTRACK( if (is_new) Tracker().New(ptr); else Tracker().Old(ptr); );
    }
    template <typename T2>
    Ptr(Ptr<T2> _in) : ptr(_in.Raw()) {
      emp_assert( dynamic_cast<TYPE*>(_in.Raw()) );
      EMP_IF_MEMTRACK( Tracker().Inc(ptr); );
    }
    Ptr(std::nullptr_t) : Ptr() { ; }
    template <typename T2>
    Ptr(Ptr<T2> && _in) : ptr(_in.ptr) {
      emp_assert( dynamic_cast<TYPE*>(_in.ptr) );
      _in.ptr=nullptr;
    }
    ~Ptr() { EMP_IF_MEMTRACK( Tracker().Dec(ptr); ); }

    static void SetVerbose(bool v=true) { EMP_IF_MEMTRACK( Tracker().SetVerbose(v); ); }

    bool IsNull() const { return ptr == nullptr; }
    TYPE * Raw() { return ptr; }
    const TYPE * const Raw() const { return ptr; }
    template <typename T2> T2 * Cast() { return (T2*) ptr; }
    template <typename T2> const T2 * const Cast() const { return (T2*) ptr; }
    template <typename T2> T2 * DynamicCast() {
      emp_assert(dynamic_cast<T2*>(ptr) != nullptr);
      return (T2*) ptr;
    }

    template <typename... T>
    void New(T &&... args) {
      EMP_IF_MEMTRACK( if (ptr) Tracker().Dec(ptr); );  // Remove a pointer to any old memory...
      ptr = new TYPE(std::forward<T>(args)...);         // Build a new raw pointer.
      EMP_IF_MEMTRACK(Tracker().New(ptr););             // And track it!
    }
    void Delete() {
      EMP_IF_MEMTRACK( Tracker().MarkDeleted(ptr); );
      // EMP_IF_MEMTRACK( Tracker().Dec(ptr); );
      delete ptr;
    }

    void Claim() {
      EMP_IF_MEMTRACK(Tracker().Claim(ptr););
    }

    // Assign to a pointer of the correct type.
    template <typename T2>
    Ptr<TYPE> & operator=(T2 * _in) {
      emp_assert( dynamic_cast<TYPE*>(_in) );
      EMP_IF_MEMTRACK( if (ptr) Tracker().Dec(ptr); );
      ptr = _in;
      EMP_IF_MEMTRACK( Tracker().Old(ptr); );  // We do not own a raw pointer unless we claim it.
      return *this;
    }
    template <typename T2>
    Ptr<TYPE> & operator=(Ptr<T2> _in) {
      emp_assert( dynamic_cast<TYPE*>(_in.Raw()) );
      EMP_IF_MEMTRACK( if (ptr) Tracker().Dec(ptr); );
      ptr=_in.Raw();
      EMP_IF_MEMTRACK(Tracker().Inc(ptr););
      return *this;
    }
    template <typename T2>
    Ptr<TYPE> & operator=(Ptr<T2> && _in) {
      emp_assert( dynamic_cast<TYPE*>(_in.Raw()) );
      ptr = _in.ptr;
      _in.ptr = nullptr;
      return *this;
    }

    // Dereference a pointer.
    TYPE & operator*() {
      // Make sure a pointer is active before we dereference it.
      EMP_IF_MEMTRACK( emp_assert(Tracker().IsActive(ptr) == true, typeid(TYPE).name()); );
      return *ptr;
    }
    const TYPE & operator*() const {
      // Make sure a pointer is active before we dereference it.
      EMP_IF_MEMTRACK( emp_assert(Tracker().IsActive(ptr) == true, typeid(TYPE).name()); );
      return *ptr;
    }

    // Follow a pointer.
    TYPE * operator->() {
      // Make sure a pointer is active before we follow it.
      EMP_IF_MEMTRACK( emp_assert(Tracker().IsActive(ptr) == true, typeid(TYPE).name()); );
      return ptr;
    }
    const TYPE * const operator->() const {
      // Make sure a pointer is active before we follow it.
      EMP_IF_MEMTRACK( emp_assert(Tracker().IsActive(ptr) == true, typeid(TYPE).name()); );
      return ptr;
    }

    // Auto-case to raw pointer type.
    operator TYPE *() {
      // Make sure a pointer is active before we convert it.
      EMP_IF_MEMTRACK( emp_assert(Tracker().IsActive(ptr) == true, typeid(TYPE).name()); );

      // We should not automatically convert managed pointers to raw pointers
      EMP_IF_MEMTRACK( emp_assert(Tracker().IsOwner(ptr) == false, typeid(TYPE).name()); );
      return ptr;
    }

    operator bool() { return ptr != nullptr; }

    // Comparisons to other Ptr objects
    bool operator==(const Ptr<TYPE> & in_ptr) const { return ptr == in_ptr.ptr; }
    bool operator!=(const Ptr<TYPE> & in_ptr) const { return ptr != in_ptr.ptr; }
    bool operator<(const Ptr<TYPE> & in_ptr)  const { return ptr < in_ptr.ptr; }
    bool operator<=(const Ptr<TYPE> & in_ptr) const { return ptr <= in_ptr.ptr; }
    bool operator>(const Ptr<TYPE> & in_ptr)  const { return ptr > in_ptr.ptr; }
    bool operator>=(const Ptr<TYPE> & in_ptr) const { return ptr >= in_ptr.ptr; }

    // Comparisons to raw pointers.
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
  template <typename T> Ptr<T> ToPtr(T & _in) { return Ptr<T>(_in); }
  template <typename T> Ptr<T> ToPtr(T * _in, bool own=false) { return Ptr<T>(_in, own); }
  template <typename T> Ptr<T> TrackPtr(T * _in, bool own=true) { return Ptr<T>(_in, own); }

  template <typename T, typename... ARGS> Ptr<T> NewPtr(ARGS &&... args) {
    return Ptr<T>(new T(std::forward<ARGS>(args)...), true);
  }
}

#endif // EMP_PTR_H
