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
//  * Should we add on tracking for arrays (to allow indexing and know when to call delete[])?
//    Or just assume that arrays will be handled with emp::array?
//  * Should we track information about vector and array object to make sure we don't point
//    directly into them? (A resize() could make those pointers invalid!)

#ifndef EMP_PTR_H
#define EMP_PTR_H

#include <unordered_map>

#include "assert.h"
#include "vector.h"

namespace {
  /// An anonymous log2 calculator for hashing below.
  static constexpr size_t Log2(size_t x) { return x <= 1 ? 0 : (Log2(x/2) + 1); }
}

namespace emp {

  namespace {
    static bool ptr_debug = false;
  }
  void SetPtrDebug(bool _d = true) { ptr_debug = _d; }
  bool GetPtrDebug() { return ptr_debug; }

  enum class PtrStatus { DELETED=0, ACTIVE, ARRAY };

  class PtrInfo {
  private:
    const void * ptr;   // Which pointer are we keeping data on?
    int count;          // How many of this pointer do we have?
    PtrStatus status;   // Has this pointer been deleted? (i.e., we should no longer access it!)
    size_t array_size;  // How big is the array pointed to?

  public:
    PtrInfo(const void * _ptr) : ptr(_ptr), count(1), status(PtrStatus::ACTIVE), array_size(0) {
      if (ptr_debug) std::cout << "Created info for pointer: " << ptr << std::endl;
    }
    PtrInfo(const void * _ptr, size_t _array_size)
      : ptr(_ptr), count(1), status(PtrStatus::ARRAY), array_size(_array_size)
    {
      emp_assert(_array_size >= 1);
      if (ptr_debug) {
        std::cout << "Created info for array pointer (size=" << array_size << "): "
                  << ptr << std::endl;
      }
    }
    PtrInfo(const PtrInfo &) = default;
    PtrInfo(PtrInfo &&) = default;
    PtrInfo & operator=(const PtrInfo &) = default;
    PtrInfo & operator=(PtrInfo &&) = default;

    ~PtrInfo() {
      if (ptr_debug) std::cout << "Deleted info for pointer " << ptr << std::endl;
    }

    const void * GetPtr() const { return ptr; }
    int GetCount() const { return count; }
    size_t GetArraySize() const { return array_size; }
    bool IsActive() const { return (bool) status; }
    bool IsArray()  const { return status == PtrStatus::ARRAY; }

    void Inc() {
      if (ptr_debug) std::cout << "Inc info for pointer " << ptr << std::endl;
      emp_assert(status != PtrStatus::DELETED, "Incrementing deleted pointer!"); count++;
    }
    void Dec() {
      if (ptr_debug) std::cout << "Dec info for pointer " << ptr << std::endl;

      // Make sure that we have more than one copy, -or- we've already deleted this pointer
      emp_assert(count > 1 || status == PtrStatus::DELETED, "Removing last reference to owned Ptr!");
      count--;
    }

    void MarkDeleted() {
      if (ptr_debug) std::cout << "Marked deleted for pointer " << ptr << std::endl;
      emp_assert(status != PtrStatus::DELETED, "Deleting same emp::Ptr a second time!");
      status = PtrStatus::DELETED;
    }

  };


  class PtrTracker {
  private:
    std::unordered_map<const void *, size_t> ptr_id;
    emp::vector<PtrInfo> id_info;

    // Make PtrTracker a singleton.
    PtrTracker() : ptr_id(), id_info() { ; }
    PtrTracker(const PtrTracker &) = delete;
    PtrTracker(PtrTracker &&) = delete;
    PtrTracker & operator=(const PtrTracker &) = delete;
    PtrTracker & operator=(PtrTracker &&) = delete;

    PtrInfo & GetInfo(const void * ptr) { return id_info[ptr_id[ptr]]; }
  public:
    ~PtrTracker() {
      // Track stats about pointer record.
      size_t total = 0;
      size_t remain = 0;

      // Scan through live pointers and make sure all have been deleted.
      for (const auto & info : id_info) {
        total++;
        if (info.GetCount()) remain++;

        emp_assert(info.IsActive() == false, info.GetPtr(), info.GetCount(), info.IsActive());
      }

      std::cout << "EMP_TRACK_MEM: No memory leaks found!\n "
                << total << " pointers found; "
                << remain << " still have pointers to them (after deletion.)"
                << std::endl;
    }

    // Treat this class as a singleton with a single Get() method to retrieve it.
    static PtrTracker & Get() { static PtrTracker tracker; return tracker; }

    // Some simple accessors
    bool HasPtr(const void * ptr) const {
      if (ptr_debug) std::cout << "HasPtr: " << ptr << std::endl;
      return ptr_id.find(ptr) != ptr_id.end();
    }

    size_t GetCurID(const void * ptr) { emp_assert(HasPtr(ptr)); return ptr_id[ptr]; }
    size_t GetNumIDs() const { return id_info.size(); }
    size_t GetArraySize(size_t id) const { return id_info[id].GetArraySize(); }

    bool IsDeleted(size_t id) const {
      if (id == (size_t) -1) return false;   // Not tracked!
      if (ptr_debug) std::cout << "IsDeleted: " << id << std::endl;
      return !id_info[id].IsActive();
    }

    bool IsActive(const void * ptr) {
      if (ptr_debug) std::cout << "IsActive: " << ptr << std::endl;
      if (ptr_id.find(ptr) == ptr_id.end()) return false; // Not in database.
      return GetInfo(ptr).IsActive();
    }

    bool IsArrayID(size_t id) {
      if (ptr_debug) std::cout << "IsArrayID: " << id << std::endl;
      return id_info[id].IsArray();
    }

    int GetIDCount(size_t id) const {
      if (ptr_debug) std::cout << "Count:  " << id << std::endl;
      return id_info[id].GetCount();
    }

    // This pointer was just created as a Ptr!
    size_t New(const void * ptr) {
      emp_assert(ptr);     // Cannot track a null pointer.
      size_t id = id_info.size();
      if (ptr_debug) std::cout << "New:    " << id << " (" << ptr << ")" << std::endl;
      // Make sure pointer is not already stored -OR- hase been deleted (since re-use is possible).
      emp_assert(!HasPtr(ptr) || IsDeleted(GetCurID(ptr)));
      id_info.emplace_back(ptr);
      ptr_id[ptr] = id;
      return id;
    }

    // This pointer was just created as a Ptr ARRAY!
    size_t NewArray(const void * ptr, size_t array_size) {
      emp_assert(ptr);     // Cannot track a null pointer.
      // Make sure pointer is not already stored -OR- has been deleted (since re-use is possible).
      emp_assert(!HasPtr(ptr) || IsDeleted(GetCurID(ptr)));

      size_t id = id_info.size();
      if (ptr_debug) std::cout << "New Array:    " << id << " (" << ptr << ")" << std::endl;
      id_info.emplace_back(ptr, array_size);
      ptr_id[ptr] = id;
      return id;
    }

    void IncID(size_t id) {
      if (id == (size_t) -1) return;   // Not tracked!
      if (ptr_debug) std::cout << "Inc:    " << id << std::endl;
      id_info[id].Inc();
    }

    void DecID(size_t id) {
      if (id == (size_t) -1) return;   // Not tracked!
      auto & info = id_info[id];
      if (ptr_debug) std::cout << "Dec:    " << id << "(" << info.GetPtr() << ")" << std::endl;
      emp_assert(info.GetCount() > 0, "Decrementing Ptr, but already zero!",
                 id, info.GetPtr(), info.IsActive());
      info.Dec();
    }

    void MarkDeleted(size_t id) {
      if (ptr_debug) std::cout << "Delete: " << id << std::endl;
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
  }

  template <typename TYPE>
  class Ptr {
  private:
    TYPE * ptr;
    size_t id;

    static PtrTracker & Tracker() { return PtrTracker::Get(); }

  public:
    using element_type = TYPE;

    // Construct a null Ptr by default.
    Ptr() : ptr(nullptr), id((size_t) -1) {
      if (ptr_debug) std::cout << "null construct: " << ptr << std::endl;
    }

    // Construct using copy constructor
    Ptr(const Ptr<TYPE> & _in) : ptr(_in.ptr), id(_in.id) {
      if (ptr_debug) std::cout << "copy construct: " << ptr << std::endl;
      Tracker().IncID(id);
    }

    // Construct using move constructor
    Ptr(Ptr<TYPE> && _in) : ptr(_in.ptr), id(_in.id) {
      if (ptr_debug) std::cout << "move construct: " << ptr << std::endl;
      _in.id = (size_t) -1;
    }

    // Construct from a raw pointer of campatable type.
    template <typename T2>
    Ptr(T2 * in_ptr, bool track=false) : ptr(in_ptr), id((size_t) -1)
    {
      if (ptr_debug) std::cout << "raw construct: " << ptr << ". track=" << track << std::endl;
      emp_assert( (PtrIsConvertable<T2, TYPE>(in_ptr)) );

      // If this pointer is already active, link to it.
      if (Tracker().IsActive(ptr)) {
        id = Tracker().GetCurID(ptr);
        Tracker().IncID(id);
      }
      // If we are not already tracking this pointer, but should be, add it.
      else if (track) id = Tracker().New(ptr);
    }

    // Construct from a raw pointer of campatable ARRAY type.
    template <typename T2>
    Ptr(T2 * _ptr, size_t array_size, bool track) : ptr(_ptr), id((size_t) -1)
    {
      if (ptr_debug) std::cout << "raw ARRAY construct: " << ptr
                               << ". size=" << array_size
                               << "; track=" << track << std::endl;
      emp_assert( (PtrIsConvertable<T2, TYPE>(_ptr)) );

      // If this pointer is already active, link to it.
      if (Tracker().IsActive(ptr)) {
        id = Tracker().GetCurID(ptr);
        Tracker().IncID(id);
        emp_assert(Tracker().GetArraySize(id) == array_size); // Make sure pointer is consistent.
      }
      // If we are not already tracking this pointer, but should be, add it.
      else if (track) id = Tracker().NewArray(ptr, array_size);
    }

    // Construct from another Ptr<> object of compatable type.
    template <typename T2>
    Ptr(Ptr<T2> _in) : ptr(_in.Raw()), id(_in.GetID()) {
      if (ptr_debug) std::cout << "inexact copy construct: " << ptr << std::endl;
      emp_assert( (PtrIsConvertable<T2, TYPE>(_in.Raw())) );
      Tracker().IncID(id);
    }

    // Construct from nullptr.
    Ptr(std::nullptr_t) : Ptr() {
      if (ptr_debug) std::cout << "null construct 2." << std::endl;
    }

    // Destructor.
    ~Ptr() {
      if (ptr_debug) std::cout << "destructing " << id << " (" << ptr << ")" << std::endl;
      Tracker().DecID(id);
    }

    bool IsNull() const { return ptr == nullptr; }
    TYPE * Raw() {
      emp_assert(Tracker().IsDeleted(id) == false, "Do not convert deleted Ptr to raw.");
      return ptr;
    }
    const TYPE * const Raw() const {
      emp_assert(Tracker().IsDeleted(id) == false, "Do not convert deleted Ptr to raw.");
      return ptr;
    }
    template <typename T2> T2 * Cast() {
      emp_assert(Tracker().IsDeleted(id) == false, "Do not cast deleted pointers.");
      return (T2*) ptr;
    }
    template <typename T2> const T2 * const Cast() const {
      emp_assert(Tracker().IsDeleted(id) == false, "Do not cast deleted pointers.");
      return (T2*) ptr;
    }
    template <typename T2> T2 * DynamicCast() {
      emp_assert(dynamic_cast<T2*>(ptr) != nullptr);
      emp_assert(Tracker().IsDeleted(id) == false, "Do not cast deleted pointers.");
      return (T2*) ptr;
    }
    size_t GetID() const { return id; }

    template <typename... T>
    void New(T &&... args) {
      Tracker().DecID(id);                        // Remove a pointer to any old memory...
      ptr = new TYPE(std::forward<T>(args)...);   // Build a new raw pointer.
      if (ptr_debug) std::cout << "Ptr::New() : " << ptr << std::endl;
      id = Tracker().New(ptr);                    // And track it!
    }
    template <typename... T>
    void NewArray(size_t array_size) {
      Tracker().DecID(id);                        // Remove a pointer to any old memory...
      ptr = new TYPE[array_size];                 // Build a new raw pointer to an array.
      if (ptr_debug) std::cout << "Ptr::NewArray() : " << ptr << std::endl;
      id = Tracker().New(ptr, array_size);        // And track it!
    }
    void Delete() {
      emp_assert(id < Tracker().GetNumIDs(), id, "Deleting Ptr that we are not resposible for.");
      emp_assert(ptr, "Deleting null Ptr.");
      emp_assert(Tracker().IsArrayID(id) == false, "Trying to delete array pointer as non-array.");
      Tracker().MarkDeleted(id);
      if (ptr_debug) std::cout << "Ptr::Delete() : " << ptr << std::endl;
      delete ptr;
    }
    void DeleteArray() {
      emp_assert(id < Tracker().GetNumIDs(), id, "Deleting Ptr that we are not resposible for.");
      emp_assert(ptr, "Deleting null Ptr.");
      emp_assert(Tracker().IsArrayID(id), "Trying to delete non-array pointer as array.");
      Tracker().MarkDeleted(id);
      if (ptr_debug) std::cout << "Ptr::DeleteArray() : " << ptr << std::endl;
      delete [] ptr;
    }

    size_t Hash() const {
      // Chop off useless bits of pointer...
      static constexpr size_t shift = Log2(1 + sizeof(TYPE));
      return (size_t)(ptr) >> shift;
    }
    struct hash_t { size_t operator()(const Ptr<TYPE> & t) const { return t.Hash(); } };

    // Copy assignment
    Ptr<TYPE> & operator=(const Ptr<TYPE> & _in) {
      if (ptr_debug) std::cout << "copy assignment" << std::endl;
      emp_assert(Tracker().IsDeleted(_in.id) == false, "Do not copy deleted pointers.");
      if (id != _in.id) {        // Assignments only need to happen if ptrs are different.
        Tracker().DecID(id);
        ptr = _in.ptr;
        id = _in.id;
        Tracker().IncID(id);
      }
      return *this;
    }

    // Move assignment
    Ptr<TYPE> & operator=(Ptr<TYPE> && _in) {
      if (ptr_debug) std::cout << "move assignment" << std::endl;
      emp_assert(Tracker().IsDeleted(_in.id) == false, "Do not copy deleted pointers.");
      ptr = _in.ptr;
      id = _in.id;
      _in.ptr = nullptr;
      _in.id = (size_t) -1;
      return *this;
    }

    // Assign to a pointer of the correct type.
    template <typename T2>
    Ptr<TYPE> & operator=(T2 * _in) {
      if (ptr_debug) std::cout << "raw assignment" << std::endl;
      emp_assert( (PtrIsConvertable<T2, TYPE>(_in)) );

      if (ptr) Tracker().DecID(id);   // Decrement references to former pointer at this position.
      ptr = _in;                      // Update to new pointer.

      // If this pointer is already active, link to it.
      if (Tracker().IsActive(ptr)) {
        id = Tracker().GetCurID(ptr);
        Tracker().IncID(id);
      }
      // Otherwise, since this ptr was passed in as a raw pointer, we do not manage it.

      return *this;
    }

    // Assign to a convertable Ptr
    template <typename T2>
    Ptr<TYPE> & operator=(Ptr<T2> _in) {
      if (ptr_debug) std::cout << "convert-copy assignment" << std::endl;
      emp_assert( (PtrIsConvertable<T2, TYPE>(_in.Raw())) );
      emp_assert(Tracker().IsDeleted(_in.id) == false, "Do not copy deleted pointers.");
      Tracker().DecID(id);
      ptr = _in.Raw();
      id = _in.GetID();
      Tracker().IncID(id);
      return *this;
    }

    // Dereference a pointer.
    TYPE & operator*() {
      // Make sure a pointer is active and non-null before we dereference it.
      emp_assert(Tracker().IsDeleted(id) == false, typeid(TYPE).name());
      emp_assert(ptr != nullptr, "Do not dereference a null pointer!");
      return *ptr;
    }
    const TYPE & operator*() const {
      // Make sure a pointer is active before we dereference it.
      emp_assert(Tracker().IsDeleted(id) == false, typeid(TYPE).name());
      emp_assert(ptr != nullptr, "Do not dereference a null pointer!");
      return *ptr;
    }

    // Follow a pointer.
    TYPE * operator->() {
      // Make sure a pointer is active before we follow it.
      emp_assert(Tracker().IsDeleted(id) == false, typeid(TYPE).name());
      emp_assert(ptr != nullptr, "Do not follow a null pointer!");
      return ptr;
    }
    const TYPE * const operator->() const {
      // Make sure a pointer is active before we follow it.
      emp_assert(Tracker().IsDeleted(id) == false, typeid(TYPE).name());
      emp_assert(ptr != nullptr, "Do not follow a null pointer!");
      return ptr;
    }

    // Auto-case to raw pointer type.
    operator TYPE *() {
      // Make sure a pointer is active before we convert it.
      emp_assert(Tracker().IsDeleted(id) == false, typeid(TYPE).name());

      // We should not automatically convert managed pointers to raw pointers
      emp_assert(id == (size_t) -1, typeid(TYPE).name());
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
    int DebugGetCount() const { return Tracker().GetIDCount(id); }

    // Prevent use of new and delete on Ptr
    static void* operator new(std::size_t) noexcept {
      emp_assert(false, "No Ptr::operator new; use emp::NewPtr for clarity.");
      return nullptr;
    }
    static void* operator new[](std::size_t sz) noexcept {
      emp_assert(false, "No Ptr::operator new[]; use emp::NewPtrArray for clarity.");
      return nullptr;
    }

    static void operator delete(void* ptr, std::size_t sz) {
      emp_assert(false, "No Ptr::operator delete; use Delete() member function for clarity.");
    }
    static void operator delete[](void* ptr, std::size_t sz) {
      emp_assert(false, "No Ptr::operator new[]; use DeleteArray() member function for clarity.");
    }

  };

#else


  template <typename TYPE>
  class Ptr {
  private:
    TYPE * ptr;

  public:
    using element_type = TYPE;

    Ptr() : ptr(nullptr) {}                                               // Default constructor
    Ptr(const Ptr<TYPE> & _in) : ptr(_in.ptr) {}                          // Copy constructor
    Ptr(Ptr<TYPE> && _in) : ptr(_in.ptr) {}                               // Move constructor
    template <typename T2> Ptr(T2 * in_ptr, bool=false) : ptr(in_ptr) {}  // Construct from raw ptr
    template <typename T2> Ptr(Ptr<T2> _in) : ptr(_in.Raw()) {}           // From compatible Ptr
    Ptr(std::nullptr_t) : Ptr() {}                                        // From nullptr
    ~Ptr() { ; }                                                          // Destructor

    bool IsNull() const { return ptr == nullptr; }
    TYPE * Raw() { return ptr; }
    const TYPE * const Raw() const { return ptr; }
    template <typename T2> T2 * Cast() { return (T2*) ptr; }
    template <typename T2> const T2 * const Cast() const { return (T2*) ptr; }
    template <typename T2> T2 * DynamicCast() { return dynamic_cast<T2*>(ptr); }

    template <typename... T>
    void New(T &&... args) { ptr = new TYPE(std::forward<T>(args)...); }  // New raw pointer.
    void NewArray(size_t array_size) { ptr = new TYPE[array_size]; }
    void Delete() { delete ptr; }
    void DeleteArray() { delete [] ptr; }

    size_t Hash() const {
      static constexpr size_t shift = Log2(1 + sizeof(TYPE));  // Chop off useless bits...
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
    const TYPE * const operator->() const { return ptr; }

    // Auto-case to raw pointer type.
    operator TYPE *() { return ptr; }

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


  // Create a helper to replace & operator.
  template <typename T> Ptr<T> ToPtr(T * _in, bool own=false) { return Ptr<T>(_in, own); }
  template <typename T> Ptr<T> TrackPtr(T * _in, bool own=true) { return Ptr<T>(_in, own); }

  template <typename T, typename... ARGS> Ptr<T> NewPtr(ARGS &&... args) {
    return Ptr<T>(new T(std::forward<ARGS>(args)...), true);
  }
}

#endif // EMP_PTR_H
