/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  MemoryImage.h
 *  @brief A collection of arbitrary objects stored in a chunk of memory.
 */

#ifndef EMP_MEMORY_IMAGE_H
#define EMP_MEMORY_IMAGE_H

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../base/vector.h"

namespace emp {

  /// A MemoryImage is a full set of variable values, linked together.  They can represent
  /// all of the variables in a scope or in a class.
  class MemoryImage {
  private:
    using byte_t = unsigned char;
    emp::vector<byte_t> memory;      ///< The specific memory values.

  public:
    MemoryImage() : memory() { ; }
    MemoryImage(MemoryImage && _in) : memory(std::move(_in.memory)) { _in.memory.resize(0); }
    // Note: No copy constructor since each object needs to be copied independently.

    ~MemoryImage() { emp_assert(memory.size() == 0, "Must manually delete memory before destructing."); }

    template <typename T> emp::Ptr<T> GetPtr(size_t pos) {
      return reinterpret_cast<T*>(&memory[pos]);
    }
    template <typename T> T & GetRef(size_t pos) {
      return *(reinterpret_cast<T*>(&memory[pos]));
    }
    template <typename T> const T & GetRef(size_t pos) const {
      return *(reinterpret_cast<const T*>(&memory[pos]));
    }

    byte_t & operator[](size_t pos) { return memory[pos]; }
    const byte_t & operator[](size_t pos) const { return memory[pos]; }
    size_t size() const { return memory.size(); }
    void resize(size_t new_size) { memory.resize(new_size); }
  };

}

#endif