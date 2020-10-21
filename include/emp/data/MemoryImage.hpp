/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  MemoryImage.hpp
 *  @brief A managed set of Bytes to store any kind of data.
 *  @note Status: ALPHA
 *
 */

#ifndef EMP_MEMORY_IMAGE_H
#define EMP_MEMORY_IMAGE_H

#include <cstring>        // For std::memcpy
#include <new>            // For placement new

#include "../base/assert.hpp"
#include "../base/Ptr.hpp"

namespace emp {

  /// A helper class for DataMap; tracks a memory image (sequence of bytes).

  class MemoryImage {
    friend class DataLayout;
  private:
    emp::Ptr<std::byte> image = nullptr;   ///< Current memory image.
    size_t size = 0;                       ///< Size of current image.
    size_t init_to = 0;                    ///< How far if the current image initialized?

    // Setup all of the uninitialized memory to be non-zero.
    void Fuzz() {
      for (size_t i = init_to; i < size; i++) {
        image[i] = std::byte{85};
      }
    }
  public:
    /// Build a default MemoryImage with no initial memory.
    MemoryImage() = default;

    /// Build a memory image of a specific size (but uninitialized.)
    MemoryImage(size_t in_size) : image( emp::NewArrayPtr<std::byte>(in_size) ), size(in_size) { ; }

    /// Destruct a MemoryImage.
    ~MemoryImage() {
      emp_assert(init_to == 0, "A memory image must be deinitialized before deconstruction.");
      if (image) image.DeleteArray();
    }

    size_t GetSize() const { return size; }
    size_t GetInitSize() const { return init_to; }

    /// Get a typed pointer to a specific position in this image.
    template <typename T=void> emp::Ptr<T> GetPtr(size_t pos) {
      if constexpr (!std::is_same_v<T,void>) {
        emp_assert(pos + sizeof(T) <= GetSize(), pos, sizeof(T), GetSize());
      }
      return reinterpret_cast<T*>(&image[pos]);
    }

    template <typename T=void> emp::Ptr<const T> GetPtr(size_t pos) const {
      if constexpr (!std::is_same_v<T,void>) {
        emp_assert(pos + sizeof(T) <= GetSize(), pos, sizeof(T), GetSize());
      }
      return emp::Ptr<const std::byte>(&image[pos]).ReinterpretCast<const T>();
    }

    /// Get proper references to an object represented in this image.
    template <typename T> T & Get(size_t pos) {
      emp_assert(pos < GetInitSize(), "Only get a reference for initialized memory.");
      return *GetPtr<T>(pos);
    }
    template <typename T> const T & Get(size_t pos) const {
      emp_assert(pos < GetInitSize(), "Only get a reference for initialized memory.");
      return *GetPtr<T>(pos);
    }

    /// Change the size of this memory.  Assume all cleanup and setup is done elsewhere.
    void RawResize(size_t new_size) {
      // If the size is already good, stop here.
      if (GetSize() == new_size) return;

      if (image) {
        emp_assert(GetInitSize() == 0, "Only uninitialized memory should be resized.");
        image.DeleteArray();   // If there was memory here, free it.
      }
      size = new_size;                  // Determine the new size.
      if (size) image.NewArray(size);   // Allocate the new space.
      else image = nullptr;             // Or set to null if now empty.
    }

    /// Copy all of the bytes directly from another memory image.  Size manipulation must be
    /// done beforehand to ensure sufficient space is availabe.
    void RawCopy(const MemoryImage & from_memory) {
      emp_assert(GetSize() >= from_memory.GetSize());
      if (from_memory.GetSize() == 0) return; // Nothing to copy!

      emp_assert(GetInitSize() == 0, "Only uninitialized memory should be copied over.");
      emp_assert(from_memory.GetInitSize() == from_memory.GetSize(),
                 "Only fully initialized memory should be copied from.");

      // Copy byte-by-byte into this memory.
      std::memcpy(image.Raw(), from_memory.image.Raw(), from_memory.GetSize());
    }

    /// Steal the memory from the image passed in.  Current memory should have been cleaned up
    /// and set to null already.
    void RawMove(MemoryImage & from_memory) {
      emp_assert(init_to == 0, "Must move to a clear image.");

      if (from_memory.GetSize() == 0) return;  // Nothing to do!

      if (image) image.DeleteArray();

      image = from_memory.image;
      size = from_memory.size;
      init_to = from_memory.init_to;
      from_memory.image = nullptr;
      from_memory.size = 0;
      from_memory.init_to = 0;
    }

    /// Build a new object of the provided type at the memory position indicated.
    template <typename T, typename... ARGS>
    void Construct(size_t id, ARGS &&... args) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      emp_assert(GetInitSize() <= id, "Should not construct into initialized memory.");
      new (GetPtr<T>(id).Raw()) T( std::forward<ARGS>(args)... );
    }

    /// Destruct an object of provided type at memory position indicated; don't release memory!
    template <typename T>
    void Destruct(size_t id) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      emp_assert(id < GetInitSize(), "Should only destruct initialized memory.");
      GetPtr<T>(id)->~T();
    }

    /// Copy an object from another MemoryImage with an identical DataLayout.
    template<typename T>
    void CopyObj(size_t id, const MemoryImage & from_image) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      emp_assert(id < from_image.GetInitSize(), "Should only copy from initialized memory.");
      emp_assert(GetInitSize() <= id, "Should not copy into initialized memory.");
      Construct<T, const T &>(id, from_image.Get<T>(id));
    }

    /// Move an object from another MemoryImage with an identical DataLayout.
    template<typename T>
    void MoveObj(size_t id, MemoryImage & from_image) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      emp_assert(id < from_image.GetInitSize(), "Should only move from initialized memory.");
      emp_assert(GetInitSize() <= id, "Should not move into initialized memory.");
      Construct<T, const T &>(id, std::move(from_image.Get<T>(id)));     // Move the object.
      from_image.Destruct<T>(id);                                        // Destruct old version.
    }
  };

}

#endif
