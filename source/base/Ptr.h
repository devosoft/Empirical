/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file Ptr.h
 *  @brief A wrapper for pointers that does careful memory tracking (but only in debug mode).
 *  @note Status: BETA
 *
 *  Ptr objects behave as normal pointers under most conditions.  However, if a program is
 *  compiled with EMP_TRACK_MEM set, then these pointers perform extra tests to ensure that
 *  they point to valid memory and that memory is freed before pointers are released.
 *
 *  If you trip an assert, you can re-do the run a track a specific pointer by defining
 *  EMP_ABORT_PTR_NEW or EMP_ABORT_PTR_DELETE to the ID of the pointer in question.  This will
 *  allow you to track the pointer more easily in a debugger.
 *
 *  @todo Track information about emp::vector and emp::array objects to make sure we don't
 *    point directly into them? (A resize() could make such pointers invalid!) Or better, warn
 *    it vector memory could have moved.
 *  @todo Get working with threads
 */

#ifndef EMP_PTR_H
#define EMP_PTR_H

#include <unordered_map>

#include "assert.h"
#include "vector.h"

namespace emp {

  namespace internal {
    /// An anonymous log2 calculator for hashing below.
    static constexpr size_t Log2(size_t x) { return x <= 1 ? 0 : (Log2(x/2) + 1); }

    static bool ptr_debug = false;
  }
  void SetPtrDebug(bool _d = true) { internal::ptr_debug = _d; }
  bool GetPtrDebug() { return internal::ptr_debug; }

  enum class PtrStatus { DELETED=0, ACTIVE, ARRAY };

  class PtrInfo {
  private:
    const void * ptr;   ///< Which pointer are we keeping data on?
    int count;          ///< How many of this pointer do we have?
    PtrStatus status;   ///< Has this pointer been deleted? (i.e., we should no longer access it!)
    size_t array_bytes; ///< How big is the array pointed to (in bytes)?

  public:
    PtrInfo(const void * _ptr) : ptr(_ptr), count(1), status(PtrStatus::ACTIVE), array_bytes(0) {
      if (internal::ptr_debug) std::cout << "Created info for pointer: " << ptr << std::endl;
    }
    PtrInfo(const void * _ptr, size_t _array_bytes)
      : ptr(_ptr), count(1), status(PtrStatus::ARRAY), array_bytes(_array_bytes)
    {
      emp_assert(_array_bytes >= 1);
      if (internal::ptr_debug) {
        std::cout << "Created info for array pointer (bytes=" << array_bytes << "): "
                  << ptr << std::endl;
      }
    }
    PtrInfo(const PtrInfo &) = default;
    PtrInfo(PtrInfo &&) = default;
    PtrInfo & operator=(const PtrInfo &) = default;
    PtrInfo & operator=(PtrInfo &&) = default;

    ~PtrInfo() {
      if (internal::ptr_debug) std::cout << "Deleted info for pointer " << ptr << std::endl;
    }

    /// What pointer does this one hold information about?
    const void * GetPtr() const noexcept { return ptr; }

    /// How many Ptr objects point to the associated position?
    int GetCount() const noexcept { return count; }

    /// If this ptr is to an array, how many bytes large is the array (may be different from size!)
    size_t GetArrayBytes() const noexcept { return array_bytes; }

    /// Is this pointer currently valid to access?
    bool IsActive() const noexcept { return (bool) status; }

    /// Is this pointer pointing to an array?
    bool IsArray() const noexcept { return status == PtrStatus::ARRAY; }

    /// Denote that this pointer is an array.
    void SetArray(size_t bytes) noexcept { array_bytes = bytes; status = PtrStatus::ARRAY; }

    /// Add one more pointer.
    void Inc(size_t id) {
      if (internal::ptr_debug) std::cout << "Inc info for pointer " << ptr << std::endl;
      emp_assert(status != PtrStatus::DELETED, "Incrementing deleted pointer!", id);
      count++;
    }

    /// Remove a pointer.
    void Dec(size_t id) {
      if (internal::ptr_debug) std::cout << "Dec info for pointer " << ptr << std::endl;

      // Make sure that we have more than one copy, -or- we've already deleted this pointer
      emp_assert(count > 1 || status == PtrStatus::DELETED, "Removing last reference to owned Ptr!", id);
      count--;
    }

    /// Indicate that the associated position has been deleted.
    void MarkDeleted() {
      if (internal::ptr_debug) std::cout << "Marked deleted for pointer " << ptr << std::endl;
      status = PtrStatus::DELETED;
    }

    /// Debug utility to determine if everything looks okay with this pointer's information.
    bool OK() const noexcept {
      if (ptr == nullptr) return false;     // Should not have info for a null pointer.
      if (status == PtrStatus::ARRAY) {
        if (array_bytes == 0) return false; // Arrays cannot be size 0.
        if (count == 0) return false;       // Active arrays must have pointers to them.
      }
      if (status == PtrStatus::ACTIVE) {
        if (array_bytes > 0) return false;  // non-arrays must be array size 0.
        if (count == 0) return false;       // Active pointers must have references to them.
      }
      return true;
    }
  };


  /// Facilitate tracking of all Ptr objects in this run.
  class PtrTracker {
  private:
    std::unordered_map<const void *, size_t> ptr_id;  ///< Associate raw pointers with unique IDs
    emp::vector<PtrInfo> id_info;                     ///< Associate IDs with pointer information.
    static constexpr size_t UNTRACKED_ID = (size_t) -1;

    // Make PtrTracker a singleton.
    PtrTracker() : ptr_id(), id_info() {
      std::cout << "EMP_TRACK_MEM: Pointer tracking is active!\n";
    }
    PtrTracker(const PtrTracker &) = delete;
    PtrTracker(PtrTracker &&) = delete;
    PtrTracker & operator=(const PtrTracker &) = delete;
    PtrTracker & operator=(PtrTracker &&) = delete;

  public:
    ~PtrTracker() {
      // Track stats about pointer record.
      size_t total = 0;
      size_t remain = 0;
      emp::vector<PtrInfo> undeleted_info;

      // Scan through live pointers and make sure all have been deleted.
      for (const auto & info : id_info) {
        total++;
        if (info.GetCount()) remain++;

        if (info.IsActive()) {
          undeleted_info.push_back(info);
        }
      }

      if (undeleted_info.size()) {
        std::cerr << undeleted_info.size() << " undeleted pointers at end of exectution.\n";
        for (size_t i = 0; i < undeleted_info.size() && i < 10; ++i) {
          const auto & info = undeleted_info[i];
          std::cerr << "  PTR=" << info.GetPtr()
                    << "  count=" << info.GetCount()
                    << "  active=" << info.IsActive()
                    << "  id=" << ptr_id[info.GetPtr()]
                    << std::endl;
        }
        abort();
      }

      std::cout << "EMP_TRACK_MEM: No memory leaks found!\n "
                << total << " pointers found; ";
      if (remain) {
        std::cout << remain << " still exist with a non-null value (but have been properly deleted)";
      } else std::cout << "all have been cleaned up fully.";
      std::cout << std::endl;
    }

    /// Treat this class as a singleton with a single Get() method to retrieve it.
    static PtrTracker & Get() { static PtrTracker tracker; return tracker; }

    /// Get the info associated with an existing pointer.
    PtrInfo & GetInfo(const void * ptr) { return id_info[ptr_id[ptr]]; }
    PtrInfo & GetInfo(size_t id) { return id_info[id]; }

    /// Determine if a pointer is being tracked.
    bool HasPtr(const void * ptr) const {
      if (internal::ptr_debug) std::cout << "HasPtr: " << ptr << std::endl;
      return ptr_id.find(ptr) != ptr_id.end();
    }

    /// Retrive the ID associated with a pointer.
    size_t GetCurID(const void * ptr) { emp_assert(HasPtr(ptr)); return ptr_id[ptr]; }

    /// Lookup how many pointers are being tracked.
    size_t GetNumIDs() const { return id_info.size(); }

    /// How big is an array associated with an ID?
    size_t GetArrayBytes(size_t id) const { return id_info[id].GetArrayBytes(); }

    /// Check if an ID is for a pointer that has been deleted.
    bool IsDeleted(size_t id) const {
      if (id == UNTRACKED_ID) return false;   // Not tracked!
      if (internal::ptr_debug) std::cout << "IsDeleted: " << id << std::endl;
      return !id_info[id].IsActive();
    }

    /// Is a pointer active and ready to be used?
    bool IsActive(const void * ptr) {
      if (internal::ptr_debug) std::cout << "IsActive: " << ptr << std::endl;
      if (ptr_id.find(ptr) == ptr_id.end()) return false; // Not in database.
      return GetInfo(ptr).IsActive();
    }

    /// Is a pointer id associated with a pointer that's active and ready to be used?
    bool IsActiveID(size_t id) {
      if (id == UNTRACKED_ID) return false;
      if (id >= id_info.size()) return false;
      return id_info[id].IsActive();
    }

    /// Is an ID associated with an array?
    bool IsArrayID(size_t id) {
      if (internal::ptr_debug) std::cout << "IsArrayID: " << id << std::endl;
      return id_info[id].IsArray();
    }

    /// How many Ptr objects are associated with an ID?
    int GetIDCount(size_t id) const {
      if (internal::ptr_debug) std::cout << "Count:  " << id << std::endl;
      return id_info[id].GetCount();
    }

    /// This pointer was just created as a Ptr!
    size_t New(const void * ptr) {
      emp_assert(ptr);     // Cannot track a null pointer.
      size_t id = id_info.size();
#ifdef EMP_ABORT_PTR_NEW
      if (id == EMP_ABORT_PTR_NEW) {
        std::cerr << "Aborting at creation of Ptr id " << id << std::endl;
        abort();
      }
#endif
      if (internal::ptr_debug) std::cout << "New:    " << id << " (" << ptr << ")" << std::endl;
      // Make sure pointer is not already stored -OR- hase been deleted (since re-use is possible).
      emp_assert(!HasPtr(ptr) || IsDeleted(GetCurID(ptr)), id);
      id_info.emplace_back(ptr);
      ptr_id[ptr] = id;
      return id;
    }

    /// This pointer was just created as a Ptr ARRAY!
    size_t NewArray(const void * ptr, size_t array_bytes) {
      size_t id = New(ptr);  // Build the new pointer.
      if (internal::ptr_debug) std::cout << "  ...Array of size " << array_bytes << std::endl;
      id_info[id].SetArray(array_bytes);
      return id;
    }

    /// Increment the nuber of Pointers associated with an ID
    void IncID(size_t id) {
      if (id == UNTRACKED_ID) return;   // Not tracked!
      if (internal::ptr_debug) std::cout << "Inc:    " << id << std::endl;
      id_info[id].Inc(id);
    }

    /// Decrement the nuber of Pointers associated with an ID
    void DecID(size_t id) {
      if (id == UNTRACKED_ID) return;   // Not tracked!
      auto & info = id_info[id];
      if (internal::ptr_debug) std::cout << "Dec:    " << id << "(" << info.GetPtr() << ")" << std::endl;
      emp_assert(info.GetCount() > 0, "Decrementing Ptr, but already zero!",
                 id, info.GetPtr(), info.IsActive());
      info.Dec(id);
    }

    /// Mark the pointers associated with this ID as deleted.
    void MarkDeleted(size_t id) {
#ifdef EMP_ABORT_PTR_DELETE
      if (id == EMP_ABORT_PTR_DELETE) {
        std::cerr << "Aborting at deletion of Ptr id " << id << std::endl;
        abort();
      }
#endif
      if (internal::ptr_debug) std::cout << "Delete: " << id << std::endl;
      emp_assert(id_info[id].IsActive(), "Deleting same emp::Ptr a second time!", id);
      id_info[id].MarkDeleted();
    }
  };


//////////////////////////////////
//
//  --- Ptr implementation ---
//
//////////////////////////////////

#ifdef EMP_TRACK_MEM

  namespace {
    // @CAO: Build this for real!
    template <typename FROM, typename TO>
    bool PtrIsConvertable(FROM * ptr) { return true; }
    // emp_assert( (std::is_same<TYPE,T2>() || dynamic_cast<TYPE*>(in_ptr)) );

    // Debug information provided for each pointer type.
    struct PtrDebug {
      size_t current;
      size_t total;
      PtrDebug() : current(0), total(0) { ; }
      void AddPtr() { current++; total++; }
      void RemovePtr() { current--; }
    };
  }

  template <typename TYPE>
  class Ptr {
  public:
    TYPE * ptr;                 ///< The raw pointer associated with this Ptr object.
    size_t id;                  ///< A unique ID for this pointer type.
    using element_type = TYPE;  ///< Type being pointed at.

    static constexpr size_t UNTRACKED_ID = (size_t) -1;

    static PtrDebug & DebugInfo() { static PtrDebug info; return info; } // Debug info for each type
    static PtrTracker & Tracker() { return PtrTracker::Get(); }  // Single tracker for al Ptr types

    /// Construct a null Ptr by default.
    Ptr() : ptr(nullptr), id(UNTRACKED_ID) {
      if (internal::ptr_debug) std::cout << "null construct: " << ptr << std::endl;
    }

    /// Construct using copy constructor
    Ptr(const Ptr<TYPE> & _in) : ptr(_in.ptr), id(_in.id) {
      if (internal::ptr_debug) std::cout << "copy construct: " << ptr << std::endl;
      Tracker().IncID(id);
    }

    /// Construct using move constructor
    Ptr(Ptr<TYPE> && _in) : ptr(_in.ptr), id(_in.id) {
      if (internal::ptr_debug) std::cout << "move construct: " << ptr << std::endl;
      _in.ptr = nullptr;
      _in.id = UNTRACKED_ID;
      // No IncID or DecID in Tracker since we just move the id.
    }

    /// Construct from a raw pointer of campatable type.
    template <typename T2>
    Ptr(T2 * in_ptr, bool track=false) : ptr(in_ptr), id(UNTRACKED_ID)
    {
      if (internal::ptr_debug) std::cout << "raw construct: " << ptr << ". track=" << track << std::endl;
      emp_assert( (PtrIsConvertable<T2, TYPE>(in_ptr)) );

      // If this pointer is already active, link to it.
      if (Tracker().IsActive(ptr)) {
        id = Tracker().GetCurID(ptr);
        Tracker().IncID(id);
      }
      // If we are not already tracking this pointer, but should be, add it.
      else if (track) {
        id = Tracker().New(ptr);
        DebugInfo().AddPtr();
      }
    }

    /// Construct from a raw pointer of campatable ARRAY type.
    template <typename T2>
    Ptr(T2 * _ptr, size_t array_size, bool track) : ptr(_ptr), id(UNTRACKED_ID)
    {
      const size_t array_bytes = array_size * sizeof(T2);
      if (internal::ptr_debug) std::cout << "raw ARRAY construct: " << ptr
                               << ". size=" << array_size << "(" << array_bytes
                               << " bytes); track=" << track << std::endl;
      emp_assert( (PtrIsConvertable<T2, TYPE>(_ptr)) );

      // If this pointer is already active, link to it.
      if (Tracker().IsActive(ptr)) {
        id = Tracker().GetCurID(ptr);
        Tracker().IncID(id);
        emp_assert(Tracker().GetArrayBytes(id) == array_bytes); // Make sure pointer is consistent.
      }
      // If we are not already tracking this pointer, but should be, add it.
      else if (track) {
        id = Tracker().NewArray(ptr, array_bytes);
        DebugInfo().AddPtr();
      }
    }

    /// Construct from another Ptr<> object of compatable type.
    template <typename T2>
    Ptr(Ptr<T2> _in) : ptr(_in.Raw()), id(_in.GetID()) {
      if (internal::ptr_debug) std::cout << "inexact copy construct: " << ptr << std::endl;
      emp_assert( (PtrIsConvertable<T2, TYPE>(_in.Raw())), id );
      Tracker().IncID(id);
    }

    /// Construct from nullptr.
    Ptr(std::nullptr_t) : Ptr() {
      if (internal::ptr_debug) std::cout << "null construct 2." << std::endl;
    }

    /// Destructor.
    ~Ptr() {
      if (internal::ptr_debug) {
        std::cout << "destructing Ptr instance ";
        if (ptr) std::cout << id << " (" << ptr << ")\n";
        else std::cout << "(nullptr)\n";
      }
      Tracker().DecID(id);
    }

    /// Is this Ptr currently nullptr?
    bool IsNull() const { return ptr == nullptr; }

    /// Convert this Ptr to a raw pointer that isn't going to be tracked.
    TYPE * Raw() {
      emp_assert(Tracker().IsDeleted(id) == false, "Do not convert deleted Ptr to raw.", id);
      return ptr;
    }

    /// Convert this Ptr to a const raw pointer that isn't going to be tracked.
    const TYPE * const Raw() const {
      emp_assert(Tracker().IsDeleted(id) == false, "Do not convert deleted Ptr to raw.", id);
      return ptr;
    }

    /// Cast this Ptr to a different type.
    template <typename T2> Ptr<T2> Cast() {
      emp_assert(Tracker().IsDeleted(id) == false, "Do not cast deleted pointers.", id);
      return (T2*) ptr;
    }

    /// Cast this Ptr to a const Ptr of a different type.
    template <typename T2> const Ptr<const T2> Cast() const {
      emp_assert(Tracker().IsDeleted(id) == false, "Do not cast deleted pointers.", id);
      return (T2*) ptr;
    }

    /// Dynamically cast this Ptr to another type; throw an assert of the cast fails.
    template <typename T2> Ptr<T2> DynamicCast() {
      emp_assert(dynamic_cast<T2*>(ptr) != nullptr);
      emp_assert(Tracker().IsDeleted(id) == false, "Do not cast deleted pointers.", id);
      return (T2*) ptr;
    }

    /// Get the unique ID associated with this pointer.
    size_t GetID() const { return id; }

    /// Reallocate this Ptr to a newly allocated value using arguments passed in.
    template <typename... T>
    void New(T &&... args) {
      Tracker().DecID(id);                            // Remove a pointer to any old memory...

      ptr = new TYPE(std::forward<T>(args)...); // Special new that uses allocated space.
      // ptr = (TYPE*) malloc (sizeof(TYPE));            // Build a new raw pointer.
      // emp_emscripten_assert(ptr);                     // No exceptions in emscripten; assert alloc!
      // ptr = new (ptr) TYPE(std::forward<T>(args)...); // Special new that uses allocated space.

      if (internal::ptr_debug) std::cout << "Ptr::New() : " << ptr << std::endl;
      id = Tracker().New(ptr);                        // And track it!
      DebugInfo().AddPtr();
    }

    /// Reallocate this Ptr to a newly allocated array using the size passed in.
    template <typename... T>
    void NewArray(size_t array_size, T &&... args) {
      Tracker().DecID(id);                              // Remove a pointer to any old memory...

      // @CAO: This next portion of code is allocating an array of the appropriate type.
      //       We are currently using "new", but should shift over to malloc since new throws an
      //       exception when there's a problem, which will trigger an abort in Emscripten mode.
      //       We'd rather be able to identify a more specific problem.
      ptr = new TYPE[array_size];                     // Build a new raw pointer to an array.
      // ptr = (TYPE*) malloc (array_size * sizeof(TYPE)); // Build a new raw pointer.
      // emp_emscripten_assert(ptr, array_size);           // No exceptions in emscripten; assert alloc!
      // for (size_t i = 0; i < array_size; i++) {
      //   new (ptr + i*sizeof(TYPE)) TYPE(args...);
      // }

      if (internal::ptr_debug) std::cout << "Ptr::NewArray() : " << ptr << std::endl;
      id = Tracker().NewArray(ptr, array_size * sizeof(TYPE));   // And track it!
      DebugInfo().AddPtr();
    }

    /// Delete this pointer (must NOT be an array).
    void Delete() {
      emp_assert(ptr, "Deleting null Ptr.");
      emp_assert(id < Tracker().GetNumIDs(), id, "Deleting Ptr that we are not resposible for.");
      emp_assert(Tracker().IsArrayID(id) == false, id, "Trying to delete array pointer as non-array.");
      if (internal::ptr_debug) std::cout << "Ptr::Delete() : " << ptr << std::endl;
      delete ptr;
      Tracker().MarkDeleted(id);
      DebugInfo().RemovePtr();
    }

    /// Delete this pointer to an array (must be an array).
    void DeleteArray() {
      emp_assert(id < Tracker().GetNumIDs(), id, "Deleting Ptr that we are not resposible for.");
      emp_assert(ptr, "Deleting null Ptr.");
      emp_assert(Tracker().IsArrayID(id), id, "Trying to delete non-array pointer as array.");
      if (internal::ptr_debug) std::cout << "Ptr::DeleteArray() : " << ptr << std::endl;
      delete [] ptr;
      Tracker().MarkDeleted(id);
      DebugInfo().RemovePtr();
    }

    /// Convert this pointer to a hash value.
    size_t Hash() const {
      // Chop off useless bits of pointer...
      static constexpr size_t shift = internal::Log2(1 + sizeof(TYPE));
      return (size_t)(ptr) >> shift;
    }
    struct hash_t { size_t operator()(const Ptr<TYPE> & t) const { return t.Hash(); } };

    /// Copy assignment
    Ptr<TYPE> & operator=(const Ptr<TYPE> & _in) {
      if (internal::ptr_debug) std::cout << "copy assignment" << std::endl;
      emp_assert(Tracker().IsDeleted(_in.id) == false, _in.id, "Do not copy deleted pointers.");
      if (id != _in.id) {        // Assignments only need to happen if ptrs are different.
        Tracker().DecID(id);
        ptr = _in.ptr;
        id = _in.id;
        Tracker().IncID(id);
      }
      return *this;
    }

    /// Move assignment
    Ptr<TYPE> & operator=(Ptr<TYPE> && _in) {
      if (internal::ptr_debug) std::cout << "move assignment: " << _in.ptr << std::endl;
      emp_assert(Tracker().IsDeleted(_in.id) == false, _in.id, "Do not move deleted pointers.");
      if (ptr != _in.ptr) {
        Tracker().DecID(id);   // Decrement references to former pointer at this position.
        ptr = _in.ptr;
        id = _in.id;
        _in.ptr = nullptr;
        _in.id = UNTRACKED_ID;
      }
      return *this;
    }

    /// Assign to a raw pointer of the correct type; if this is already tracked, hooked in
    /// correctly, otherwise don't track.
    template <typename T2>
    Ptr<TYPE> & operator=(T2 * _in) {
      if (internal::ptr_debug) std::cout << "raw assignment" << std::endl;
      emp_assert( (PtrIsConvertable<T2, TYPE>(_in)) );

      Tracker().DecID(id);    // Decrement references to former pointer at this position.
      ptr = _in;              // Update to new pointer.

      // If this pointer is already active, link to it.
      if (Tracker().IsActive(ptr)) {
        id = Tracker().GetCurID(ptr);
        Tracker().IncID(id);
      }
      // Otherwise, since this ptr was passed in as a raw pointer, we do not manage it.
      else {
        id = UNTRACKED_ID;
      }

      return *this;
    }

    /// Assign to a convertable Ptr
    template <typename T2>
    Ptr<TYPE> & operator=(Ptr<T2> _in) {
      if (internal::ptr_debug) std::cout << "convert-copy assignment" << std::endl;
      emp_assert( (PtrIsConvertable<T2, TYPE>(_in.Raw())), _in.id );
      emp_assert(Tracker().IsDeleted(_in.id) == false, _in.id, "Do not copy deleted pointers.");
      Tracker().DecID(id);
      ptr = _in.Raw();
      id = _in.GetID();
      Tracker().IncID(id);
      return *this;
    }

    /// Dereference a pointer.
    TYPE & operator*() {
      // Make sure a pointer is active and non-null before we dereference it.
      emp_assert(Tracker().IsDeleted(id) == false /*, typeid(TYPE).name() */, id);
      emp_assert(ptr != nullptr, "Do not dereference a null pointer!");
      return *ptr;
    }

    /// Dereference a pointer to a const type.
    const TYPE & operator*() const {
      // Make sure a pointer is active before we dereference it.
      emp_assert(Tracker().IsDeleted(id) == false /*, typeid(TYPE).name() */, id);
      emp_assert(ptr != nullptr, "Do not dereference a null pointer!");
      return *ptr;
    }

    /// Follow a pointer.
    TYPE * operator->() {
      // Make sure a pointer is active before we follow it.
      emp_assert(Tracker().IsDeleted(id) == false /*, typeid(TYPE).name() */, id);
      emp_assert(ptr != nullptr, "Do not follow a null pointer!");
      return ptr;
    }

    /// Follow a pointer to a const target.
    TYPE * const operator->() const {
      // Make sure a pointer is active before we follow it.
      emp_assert(Tracker().IsDeleted(id) == false /*, typeid(TYPE).name() */, id);
      emp_assert(ptr != nullptr, "Do not follow a null pointer!");
      return ptr;
    }

    /// Indexing into array
    TYPE & operator[](size_t pos) {
      emp_assert(Tracker().IsDeleted(id) == false /*, typeid(TYPE).name() */, id);
      emp_assert(Tracker().IsArrayID(id), "Only arrays can be indexed into.", id);
      emp_assert(Tracker().GetArrayBytes(id) > (pos*sizeof(TYPE)),
        "Indexing out of range.", id, ptr, pos, sizeof(TYPE), Tracker().GetArrayBytes(id));
      emp_assert(ptr != nullptr, "Do not follow a null pointer!");
      return ptr[pos];
    }

    /// Indexing into const array
    const TYPE & operator[](size_t pos) const {
      emp_assert(Tracker().IsDeleted(id) == false /*, typeid(TYPE).name() */, id);
      emp_assert(Tracker().IsArrayID(id), "Only arrays can be indexed into.", id);
      emp_assert(Tracker().GetArrayBytes(id) > (pos*sizeof(TYPE)),
        "Indexing out of range.", id, ptr, pos, sizeof(TYPE), Tracker().GetArrayBytes(id));
      emp_assert(ptr != nullptr, "Do not follow a null pointer!");
      return ptr[pos];
    }

    /// Auto-case to raw pointer type.
    operator TYPE *() {
      // Make sure a pointer is active before we convert it.
      emp_assert(Tracker().IsDeleted(id) == false /*, typeid(TYPE).name() */, id);

      // We should not automatically convert managed pointers to raw pointers; use .Raw()
      emp_assert(id == UNTRACKED_ID /*, typeid(TYPE).name() */, id,
                 "Use Raw() to convert to an untracked Ptr");
      return ptr;
    }

    /// Does this pointer exist?
    operator bool() { return ptr != nullptr; }

    /// Does this const pointer exist?
    operator bool() const { return ptr != nullptr; }

    /// Does this Ptr point to the same memory position?
    bool operator==(const Ptr<TYPE> & in_ptr) const { return ptr == in_ptr.ptr; }

    /// Does this Ptr point to different memory positions?
    bool operator!=(const Ptr<TYPE> & in_ptr) const { return ptr != in_ptr.ptr; }

    /// Does this Ptr point to a memory position before another?
    bool operator<(const Ptr<TYPE> & in_ptr)  const { return ptr < in_ptr.ptr; }

    /// Does this Ptr point to a memory position before or equal to another?
    bool operator<=(const Ptr<TYPE> & in_ptr) const { return ptr <= in_ptr.ptr; }

    /// Does this Ptr point to a memory position after another?
    bool operator>(const Ptr<TYPE> & in_ptr)  const { return ptr > in_ptr.ptr; }

    /// Does this Ptr point to a memory position after or equal to another?
    bool operator>=(const Ptr<TYPE> & in_ptr) const { return ptr >= in_ptr.ptr; }


    /// Does this Ptr point to the same memory position as a raw pointer?
    bool operator==(const TYPE * in_ptr) const { return ptr == in_ptr; }

    /// Does this Ptr point to different memory positions as a raw pointer?
    bool operator!=(const TYPE * in_ptr) const { return ptr != in_ptr; }

    /// Does this Ptr point to a memory position before a raw pointer?
    bool operator<(const TYPE * in_ptr)  const { return ptr < in_ptr; }

    /// Does this Ptr point to a memory position before or equal to a raw pointer?
    bool operator<=(const TYPE * in_ptr) const { return ptr <= in_ptr; }

    /// Does this Ptr point to a memory position after a raw pointer?
    bool operator>(const TYPE * in_ptr)  const { return ptr > in_ptr; }

    /// Does this Ptr point to a memory position after or equal to a raw pointer?
    bool operator>=(const TYPE * in_ptr) const { return ptr >= in_ptr; }


    /// Some debug testing functions
    int DebugGetCount() const { return Tracker().GetIDCount(id); }
    bool DebugIsArray() const { return Tracker().IsArrayID(id); }
    size_t DebugGetArrayBytes() const { return Tracker().GetArrayBytes(id); }
    bool DebugIsActive() const { return Tracker().IsActiveID(id); }

    bool OK() const {
      // Untracked ID's should not have pointers in the Tracker.
      if (id == UNTRACKED_ID) return !Tracker().HasPtr(ptr);

      // Make sure this pointer is linked to the correct info.
      if (Tracker().GetInfo(id).GetPtr() != ptr) return false;

      // And make sure that info itself is okay.
      return Tracker().GetInfo(id).OK();
    }

    // Prevent use of new and delete on Ptr
    // static void* operator new(std::size_t) noexcept {
    //   emp_assert(false, "No Ptr::operator new; use emp::NewPtr for clarity.");
    //   return nullptr;
    // }
    // static void* operator new[](std::size_t sz) noexcept {
    //   emp_assert(false, "No Ptr::operator new[]; use emp::NewPtrArray for clarity.");
    //   return nullptr;
    // }
    //
    // static void operator delete(void* ptr, std::size_t sz) {
    //   emp_assert(false, "No Ptr::operator delete; use Delete() member function for clarity.");
    // }
    // static void operator delete[](void* ptr, std::size_t sz) {
    //   emp_assert(false, "No Ptr::operator delete[]; use DeleteArray() member function for clarity.");
    // }

  };

#else


  template <typename TYPE>
  class Ptr {
  private:
    TYPE * ptr;

  public:
    using element_type = TYPE;

    Ptr() : ptr(nullptr) {}                                              ///< Default constructor
    Ptr(const Ptr<TYPE> & _in) : ptr(_in.ptr) {}                         ///< Copy constructor
    Ptr(Ptr<TYPE> && _in) : ptr(_in.ptr) {}                              ///< Move constructor
    template <typename T2> Ptr(T2 * in_ptr, bool=false) : ptr(in_ptr) {} ///< Construct from raw ptr
    template <typename T2> Ptr(T2 * _ptr, size_t, bool) : ptr(_ptr) {}   ///< Construct from array
    template <typename T2> Ptr(Ptr<T2> _in) : ptr(_in.Raw()) {}          ///< From compatible Ptr
    Ptr(std::nullptr_t) : Ptr() {}                                       ///< From nullptr
    ~Ptr() { ; }                                                         ///< Destructor

    bool IsNull() const { return ptr == nullptr; }
    TYPE * Raw() { return ptr; }
    const TYPE * const Raw() const { return ptr; }
    template <typename T2> Ptr<T2> Cast() { return (T2*) ptr; }
    template <typename T2> const Ptr<const T2> Cast() const { return (T2*) ptr; }
    template <typename T2> Ptr<T2> DynamicCast() { return dynamic_cast<T2*>(ptr); }

    template <typename... T>
    void New(T &&... args) { ptr = new TYPE(std::forward<T>(args)...); }  // New raw pointer.
    void NewArray(size_t array_size) { ptr = new TYPE[array_size]; }
    void Delete() { delete ptr; }
    void DeleteArray() { delete [] ptr; }

    size_t Hash() const {
      static constexpr size_t shift = internal::Log2(1 + sizeof(TYPE));  // Chop off useless bits...
      return (size_t)(ptr) >> shift;
    }
    struct hash_t { size_t operator()(const Ptr<TYPE> & t) const { return t.Hash(); } };

    // Copy/Move assignments
    Ptr<TYPE> & operator=(const Ptr<TYPE> & _in) { ptr = _in.ptr; return *this; }
    Ptr<TYPE> & operator=(Ptr<TYPE> && _in) { ptr = _in.ptr; _in.ptr = nullptr; return *this; }

    // Assign to compatible Ptr or raw (non-managed) pointer.
    template <typename T2> Ptr<TYPE> & operator=(T2 * _in) { ptr = _in; return *this; }
    template <typename T2> Ptr<TYPE> & operator=(Ptr<T2> _in) { ptr = _in.Raw(); return *this; }

    // Dereference a pointer.
    TYPE & operator*() { return *ptr; }
    const TYPE & operator*() const { return *ptr; }

    // Follow a pointer.
    TYPE * operator->() { return ptr; }
    TYPE * const operator->() const { return ptr; }

    // Indexing into array
    TYPE & operator[](size_t pos) { return ptr[pos]; }
    const TYPE & operator[](size_t pos) const { return ptr[pos]; }

    // Auto-case to raw pointer type.
    operator TYPE *() { return ptr; }

    operator bool() { return ptr != nullptr; }
    operator bool() const { return ptr != nullptr; }

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

    // Stubs for debug-related functions when outside debug mode.
    int DebugGetCount() const { return -1; }
    bool DebugIsArray() const { emp_assert(false); return false; }
    size_t DebugGetArrayBytes() const { return 0; }
    bool DebugIsActive() const { return true; }
    bool OK() const { return true; }
  };

#endif

  // IO
  template <typename T>
  std::ostream & operator<<(std::ostream & out, const emp::Ptr<T> & ptr) {
    out << ptr.Raw();
    return out;
  }

  // @CAO: Reading a pointer from a stream seems like a terrible idea in most situations, but I
  // can imagine limited circumstances where it would be needed.
  template <typename T, typename... Ts>
  std::istream & operator>>(std::istream & is, emp::Ptr<T> & ptr) {
    T * val;
    is >> val;
    ptr = val;
    return is;
  }

  /// Convert a T* to a Ptr<T>.  By default, don't track.
  template <typename T> Ptr<T> ToPtr(T * _in, bool own=false) { return Ptr<T>(_in, own); }

  /// Convert a T* to a Ptr<T> that we DO track.
  template <typename T> Ptr<T> TrackPtr(T * _in, bool own=true) { return Ptr<T>(_in, own); }

  /// Create a new Ptr of the target type; use the args in the constructor.
  template <typename T, typename... ARGS> Ptr<T> NewPtr(ARGS &&... args) {
    auto ptr = new T(std::forward<ARGS>(args)...);
    // auto ptr = (T*) malloc (sizeof(T));         // Build a new raw pointer.
    // emp_assert(ptr);                            // No exceptions in emscripten; assert alloc!
    // new (ptr) T(std::forward<ARGS>(args)...);   // Special new that uses allocated space.
    return Ptr<T>(ptr, true);
  }

  /// Copy an object pointed to and return a Ptr to the copy.
  template <typename T> Ptr<T> CopyPtr(Ptr<T> in) { return NewPtr<T>(*in); }

  /// Copy a vector of objects pointed to; return a vector of Ptrs to the new copies.
  template <typename T> emp::vector<Ptr<T>> CopyPtrs(const emp::vector<Ptr<T>> & in) {
    emp::vector<Ptr<T>> out_ptrs(in.size());
    for (size_t i = 0; i < in.size(); i++) out_ptrs[i] = CopyPtr(in[i]);
    return out_ptrs;
  }

  /// Copy a vector of objects pointed to by using their Clone() member function; return vector.
  template <typename T> emp::vector<Ptr<T>> ClonePtrs(const emp::vector<Ptr<T>> & in) {
    emp::vector<Ptr<T>> out_ptrs(in.size());
    for (size_t i = 0; i < in.size(); i++) out_ptrs[i] = in[i]->Clone();
    return out_ptrs;
  }

  /// Create a pointer to an array of objects.
  template <typename T, typename... ARGS> Ptr<T> NewArrayPtr(size_t array_size, ARGS &&... args) {
    auto ptr = new T[array_size];                     // Build a new raw pointer.
    // const size_t alloc_size = array_size * sizeof(T);
    // auto ptr = (T*) malloc (alloc_size);
    emp_assert(ptr, array_size);                      // No exceptions in emscripten; assert alloc!
    // for (size_t i = 0; i < array_size; i++) {         // Loop through all array elements.
    //   new (ptr + i*sizeof(T)) T(args...);             //    ...and initialize them.
    // }
    return Ptr<T>(ptr, array_size, true);
  }


}

#endif // EMP_PTR_H
