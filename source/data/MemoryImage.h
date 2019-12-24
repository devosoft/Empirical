/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018-2019.
 *
 *  @file  MemoryImage.h
 *  @brief A series of bytes that can store a collection of arbitrary objects.
 * 
 *  There MemoryImage forms a base class that maintains a series of bytes.  The derived class can
 *  either have those bytes as a dynamic size (MemoryVector) or static size (MemoryArray).
 */

#ifndef EMP_MEMORY_IMAGE_H
#define EMP_MEMORY_IMAGE_H

// For std::memcpy
#include <cstring>

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../base/array.h"
#include "../base/vector.h"

namespace emp {

  /// A MemoryImage is a full set of variable values, linked together.  They can represent
  /// all of the variables in a scope or in a class.
  template <typename MEM_T>
  class MemoryImage {
  protected:
    using byte_t = unsigned char;
    MEM_T memory;          ///< The specific memory values.
    size_t free_pos = 0;   ///< Next position to add data to.

  public:
    MemoryImage() : memory() { ; }
    MemoryImage(const MemoryImage & _in) : memory(_in.memory), free_pos(_in.free_pos) { }
    MemoryImage(MemoryImage && _in) : memory(std::move(_in.memory)) { ; }

    ~MemoryImage() { emp_assert(free_pos == 0, "Must manually delete memory before destructing."); }

    size_t size() const { return free_pos; }
    size_t GetSize() const { return free_pos; }

    /// Get a typed pointer to a specific position in this image.
    template <typename T> emp::Ptr<T> GetPtr(size_t pos) {
      emp_assert(pos + sizeof(T) <= GetSize(), pos, sizeof(T), GetSize());
      return reinterpret_cast<T*>(&memory[pos]);
    }

    /// Get a proper reference to an object represented in this image.
    template <typename T> T & GetRef(size_t pos) {
      emp_assert(pos + sizeof(T) <= GetSize(), pos, sizeof(T), GetSize());
      return *(reinterpret_cast<T*>(&memory[pos]));
    }

    /// Get a const reference to an object represented in this image.
    template <typename T> const T & GetRef(size_t pos) const {
      emp_assert(pos + sizeof(T) <= GetSize(), pos, sizeof(T), GetSize());
      return *(reinterpret_cast<const T*>(&memory[pos]));
    }

    byte_t & operator[](size_t pos) { return memory[pos]; }
    const byte_t & operator[](size_t pos) const { return memory[pos]; }

    /// Build a new object of the provided type at the memory position indicated.
    template <typename T, typename... ARGS>
    void Construct(size_t pos, ARGS &&... args) {
      emp_assert(pos + sizeof(T) <= GetSize(), pos, sizeof(T), GetSize());
      new (GetPtr<T>(pos).Raw()) T( std::forward<ARGS>(args)... );
    }

    /// Destruct an object of the provided type at the memory position indicated; don't release memory!
    template <typename T>
    void Destruct(size_t pos) {
      emp_assert(pos + sizeof(T) <= GetSize(), pos, sizeof(T), GetSize());
    	GetPtr<T>(pos)->~T();
    }

    /// Copy an object from another MemoryImage with an identical layout.
    template<typename T>
    void CopyObj(size_t pos, const MemoryImage & image2) {
      emp_assert(pos + sizeof(T) <= GetSize(), pos, sizeof(T), GetSize());
      Construct<T, const T &>(pos, image2.GetRef<T>(pos));
    }

    /// Move an object from another MemoryImage with an identical layout.
    template<typename T>
    void MoveObj(size_t pos, MemoryImage & image2) {
      emp_assert(pos + sizeof(T) <= GetSize(), pos, sizeof(T), GetSize());
      Construct<T, const T &>(pos, std::move(image2.GetRef<T>(pos)));  // Move the object.
      image2.Destruct(pos);                                            // Destruct old version.
    }
  };

  class MemoryVector;

  template <size_t SIZE>
  class MemoryArray : public MemoryImage< emp::array<std::byte, SIZE> > {
  protected:
    using base_t = MemoryImage< emp::array<std::byte, SIZE> >;
    using base_t::memory;
    using base_t::free_pos;
  public:
    ~MemoryArray() { }

    void resize(size_t new_size) {
      emp_assert(free_pos <= SIZE, free_pos, SIZE);
      free_pos = new_size;
    }

    /// Add a new object to this memory, just after allocated space.
    template <typename T, typename... ARGS>
    size_t AddObject(ARGS &&... args) {
      const size_t obj_pos = free_pos;
      free_pos += sizeof(T);
      base_t::template Construct<T>(obj_pos, std::forward<ARGS>(args)...);
      return obj_pos;
    }

    /// Copy provided memory from another MemoryArray
    template <size_t IN_SIZE>
    void RawCopy(MemoryArray<IN_SIZE> & in_image) {
      emp_assert(free_pos == 0, free_pos, "Must clean up memory image before a RawCopy into it.");
      emp_assert(in_image.free_pos <= SIZE, SIZE, in_image.free_pos, IN_SIZE);
      std::memcpy(&memory, &in_image.memory, in_image.free_pos);
      free_pos = in_image.free_pos;
    }

    /// Copy provided memory from another type of MemoryImage (it may be slower...)
    void RawCopy(MemoryVector & in_image);
  };

  class MemoryVector : public MemoryImage< emp::vector<std::byte> > {
  protected:
    using base_t = MemoryImage<emp::vector<std::byte>>;

  public:
    MemoryVector() = default;
    MemoryVector(size_t num_bytes) {
      memory.resize(num_bytes);
      free_pos = num_bytes;
    }
    MemoryVector(const MemoryVector &) = default;
    ~MemoryVector() { }

    void resize(size_t new_size) { memory.resize(new_size); free_pos = new_size; }

    /// Increase the size of this memory to add a new object inside it.
    template <typename T, typename... ARGS>
    size_t AddObject(ARGS &&... args) {
      const size_t obj_pos = memory.size();
      memory.resize(obj_pos + sizeof(T));
      free_pos = memory.size();
      Construct<T>(obj_pos, std::forward<ARGS>(args)...);
      return obj_pos;
    }

    /// Copy provided memory from another MemoryVector
    void RawCopy(MemoryVector & in_image) {
      emp_assert(memory.size() == 0, memory.size(), "Must clean up memory image before a RawCopy into it.");
      memory = in_image.memory;
    }

    /// Copy provided memory from a MemoryArray (it may be slower...)
    template <size_t SIZE> void RawCopy(MemoryArray<SIZE> & in_image);
  };


  //////////////////////////////////////
  //
  //   -- Function definitions --
  //

  /// Copy provided memory from another type of MemoryImage (it may be slower...)
  template <size_t SIZE>
  void MemoryArray<SIZE>::RawCopy(MemoryVector & in_image) {
    emp_assert(free_pos == 0, free_pos, "Must clean up memory image before a RawCopy into it.");
    emp_assert(in_image.size() <= SIZE, SIZE, in_image.size());
    for (size_t i = 0; i < in_image.size(); i++) memory[i] = in_image.memory[i];
    free_pos = in_image.size();
  }

  /// Copy provided memory from another MemoryImage (it may be slower...)
  template <size_t SIZE>
  void MemoryVector::RawCopy(MemoryArray<SIZE> & in_image) {
    emp_assert(memory.size() == 0, memory.size(), "Must clean up memory image before a RawCopy into it.");
    memory.resize(in_image.size());
    free_pos = memory.size();
    for (size_t i = 0; i < size(); i++) memory[i] = in_image.memory[i];
  }

}

#endif
