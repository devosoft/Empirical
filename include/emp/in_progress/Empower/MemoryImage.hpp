/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2018 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/in_progress/Empower/MemoryImage.hpp
 * @brief A collection of arbitrary objects stored in a chunk of memory.
 */

#pragma once

#ifndef INCLUDE_EMP_IN_PROGRESS_EMPOWER_MEMORY_IMAGE_HPP_GUARD
#define INCLUDE_EMP_IN_PROGRESS_EMPOWER_MEMORY_IMAGE_HPP_GUARD

#include <stddef.h>

#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"

namespace emp {

  /// A MemoryImage is a full set of variable values, linked together.  They can represent
  /// all of the variables in a scope or in a class.
  class MemoryImage {
  private:
    using byte_t = unsigned char;
    emp::vector<byte_t> memory;  ///< The specific memory values.

  public:
    MemoryImage() : memory() { ; }

    MemoryImage(MemoryImage && _in) : memory(std::move(_in.memory)) { _in.memory.resize(0); }

    // Note: No copy constructor since each object needs to be copied independently.

    ~MemoryImage() {
      emp_assert(memory.size() == 0, "Must manually delete memory before destructing.");
    }

    template <typename T>
    emp::Ptr<T> GetPtr(size_t pos) {
      return reinterpret_cast<T *>(&memory[pos]);
    }

    template <typename T>
    T & GetRef(size_t pos) {
      return *(reinterpret_cast<T *>(&memory[pos]));
    }

    template <typename T>
    const T & GetRef(size_t pos) const {
      return *(reinterpret_cast<const T *>(&memory[pos]));
    }

    byte_t & operator[](size_t pos) { return memory[pos]; }

    const byte_t & operator[](size_t pos) const { return memory[pos]; }

    size_t size() const { return memory.size(); }

    void resize(size_t new_size) { memory.resize(new_size); }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_IN_PROGRESS_EMPOWER_MEMORY_IMAGE_HPP_GUARD
