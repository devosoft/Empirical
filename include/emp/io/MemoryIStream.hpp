/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  MemoryIStream.hpp
 *  @brief Useful for streaming data from contiguous memory.
 *  @note Status: RELEASE
 */

 #include <streambuf>
 #include <istream>

namespace emp {

  // adapted from https://stackoverflow.com/a/13059195
  struct MemoryBuf: std::streambuf {
    /// @param base address to beginning of memory region to stream from
    /// @param size number of bytes available to stream
    MemoryBuf(char const* base, size_t size) {
      char* p(const_cast<char*>(base));
      this->setg(p, p, p + size);
    }
  };

  struct MemoryIStream: virtual MemoryBuf, std::istream {
    /// @param base address to beginning of memory region to stream from
    /// @param size number of bytes available to stream
    MemoryIStream(char const* base, size_t size)
      : MemoryBuf(base, size)
      , std::istream(static_cast<std::streambuf*>(this)) {
    }
  };

}
