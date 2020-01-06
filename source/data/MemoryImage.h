/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  MemoryImage.h
 *  @brief A managed set of Bytes to store any kind of data.
 *  @note Status: ALPHA
 *
 */

#ifndef EMP_MEMORY_IMAGE_H
#define EMP_MEMORY_IMAGE_H

#include <unordered_map>
#include <cstring>        // For std::memcpy

#include "../base/assert.h"
#include "../base/Ptr.h"

namespace emp {

  /// A helper class for DataMap; tracks a memory image (sequence of bytes).

  class MemoryImage {
  private:
    emp::Ptr<std::byte> image = nullptr;
    size_t size = 0;

  public:
    MemoryImage() = default;
    MemoryImage(size_t in_size) : image( emp::NewArrayPtr<std::byte>(in_size) ), size(in_size) { ; }
    ~MemoryImage() { if (image) image.DeleteArray(); }

    size_t GetSize() const { return size; }

    /// Get a typed pointer to a specific position in this image.
    template <typename T> emp::Ptr<T> GetPtr(size_t pos) {
      emp_assert(pos + sizeof(T) <= GetSize(), pos, sizeof(T), GetSize());
      return reinterpret_cast<T*>(&image[pos]);
    }

    template <typename T> emp::Ptr<const T> GetPtr(size_t pos) const {
      emp_assert(pos + sizeof(T) <= GetSize(), pos, sizeof(T), GetSize());
      return reinterpret_cast<T const *>(&image[pos]);
    }

    /// Get proper references to an object represented in this image.
    template <typename T> T & Get(size_t pos) { return *GetPtr<T>(pos); }
    template <typename T> const T & Get(size_t pos) const { return *GetPtr<T>(pos); }

    /// Change the size of this memory.  Assume all cleanup and setup is done elsewhere.
    void RawResize(size_t new_size) {
      // If the size is already good, stop here.
      if (GetSize() == new_size) return;

      if (image) image.DeleteArray();   // If there was memory here, free it.
      size = new_size;                  // Determine the new size.
      if (size) image.NewArray(size);   // Allocate the new space.
      else image = nullptr;             // Or set to null if now empty.
    }

    /// Copy all of the bytes directly from another memory image.  Size manipulation must be
    /// done beforehand to ensure sufficient space is availabe.
    void RawCopy(const MemoryImage & in_memory) {
      emp_assert(GetSize() >= in_memory.GetSize());
      if (in_memory.GetSize() == 0) return; // Nothing to copy!

      // Copy byte-by-byte into this memory.
      std::memcpy(image.Raw(), in_memory.image.Raw(), in_memory.GetSize());
    }

    /// Steal the memory from the image passed in.  Current memory should have been cleaned up
    /// and set to null already.
    void RawMove(MemoryImage & in_memory) {
      emp_assert(image.IsNull());
      image = in_memory.image;
      size = in_memory.size;
      in_memory.image = nullptr;
      in_memory.size = 0;
    }

    /// Build a new object of the provided type at the memory position indicated.
    template <typename T, typename... ARGS>
    void Construct(size_t id, ARGS &&... args) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      new (GetPtr<T>(id).Raw()) T( std::forward<ARGS>(args)... );
    }

    /// Destruct an object of provided type at memory position indicated; don't release memory!
    template <typename T>
    void Destruct(size_t id) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      GetPtr<T>(id)->~T();
    }

    /// Copy an object from another MemoryImage with an identical DataLayout.
    template<typename T>
    void CopyObj(size_t id, const MemoryImage & from_image) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      Construct<T, const T &>(id, from_image.Get<T>(id));
    }

    /// Move an object from another MemoryImage with an identical DataLayout.
    template<typename T>
    void MoveObj(size_t id, MemoryImage & from_image) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      Construct<T, const T &>(id, std::move(from_image.Get<T>(id)));     // Move the object.
      from_image.Destruct<T>(id);                                        // Destruct old version.
    }
  };

}

#endif
