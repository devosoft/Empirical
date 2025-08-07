/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2020 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/io/MemoryIStream.hpp
 * @brief Useful for streaming data from contiguous memory.
 * Status: RELEASE
 */

#pragma once

#ifndef INCLUDE_EMP_IO_MEMORY_ISTREAM_HPP_GUARD
#define INCLUDE_EMP_IO_MEMORY_ISTREAM_HPP_GUARD

#include <istream>
#include <stddef.h>
#include <streambuf>

namespace emp {

  // adapted from https://stackoverflow.com/a/13059195
  struct MemoryBuf : std::streambuf {
    /// @param base address to beginning of memory region to stream from
    /// @param size number of bytes available to stream
    MemoryBuf(const char * base, size_t size) {
      char * p(const_cast<char *>(base));
      this->setg(p, p, p + size);
    }
  };

  struct MemoryIStream
    : virtual MemoryBuf
    , std::istream {
    /// @param base address to beginning of memory region to stream from
    /// @param size number of bytes available to stream
    MemoryIStream(const char * base, size_t size)
      : MemoryBuf(base, size), std::istream(static_cast<std::streambuf *>(this)) {}
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_IO_MEMORY_ISTREAM_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: setg buf
