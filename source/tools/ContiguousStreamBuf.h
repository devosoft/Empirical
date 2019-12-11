/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ContiguousStreamBuf.h
 *  @brief Useful for backing an ostream where data is stored in contiguous
 *         memory.
 *  @note Status: RELEASE
 */

#include <iostream>

#include "../base/vector.h"

namespace emp {

  class ContiguousStreamBuf : public std::streambuf {

  // profiling result compared to stringstream
  // http://quick-bench.com/LJR58W5q8Eld2nLg9kuQn_Mmwms
  // http://quick-bench.com/1Lt52EaVZp5t-27_Xy4etLI953w
  // http://quick-bench.com/fykiP0O-AZljZ6ldtpRc4H3y8U0

  // helpful links curated by @nateriz
  // http://www.cplusplus.com/reference/streambuf/streambuf/sputc/
  // http://www.cplusplus.com/reference/streambuf/streambuf/overflow/
  // https://en.cppreference.com/w/cpp/io/basic_streambuf
  // https://gcc.gnu.org/onlinedocs/libstdc++/manual/streambufs.html

  private:

    emp::vector<char> buffer;

  public:

    using const_iterator = emp::vector<char>::const_iterator;

    /// @param init_size num bytes to reserve initially
    ContiguousStreamBuf(const size_t init_size=1024)
    : buffer(
      std::max(static_cast<size_t>(1), init_size) // 0 init size breaks!
    )
    {
      Reset();
    }

    /// Reset stream pointers while retaining underlying memory allocation.
    void Reset() {
      this->setp(
        GetData(), // pbase & pptr
        GetData() + GetCapacity() // epptr
      );
    }

    /// Return a pointer to contiguous memory storing streamed data.
    inline const char* GetData() const { return buffer.data(); }

    /// Number of bytes currently stored.
    inline size_t GetSize() const {
      return std::distance<const char*>(
        GetData(),
        this->pptr()
      );
    }

    /// Copy stored data into an ostream.
    /// Useful for degugging.
    void Print(std::ostream & os=std::cout) const {
      for (const auto & c : buffer) {
        os << c;
      }
    }

    inline const_iterator cbegin() const {
      return std::cbegin(buffer);
    }
    inline const_iterator cend() const {
      return std::cend(buffer);
    }

  private:

    // return a pointer to contiguous memory data was streamed to
    // (necessary to play nice with annoying safeguards
    // against casting char * -> const char *)
    inline char* GetData() { return buffer.data(); }

    /// return buffer's capacity, in bytes
    inline size_t GetCapacity() const { return buffer.capacity(); }

    // sputc calls this when we run out of space in buffer.
    int_type overflow(int_type c) override {

      // double the buffer's capacity
      const size_t prev_capacity = GetCapacity();
      buffer.resize(prev_capacity * 2);

      // update the 3 streambuf pointers to point to the fresh write area
      this->setp(
        GetData() + prev_capacity, // pbase & pptr
        GetData() + GetCapacity() // epptr
      );

      // add the character that originally overflowed.
      *(this->pptr()) = c;
      pbump(1);

      return c;

    }

  };

}
