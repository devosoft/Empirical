/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ContiguousStream.h
 *  @brief Useful for streaming data to contiguous memory.
 *  @note Status: RELEASE
 */

#include <iostream>

#include "../base/vector.h"

namespace emp {


  // Buffers data into contiguous memory, backs ContiguousStream
  class ContiguousBuffer : public std::streambuf {

  // profiling result compared to stringstream
  // http://quick-bench.com/3isiMMHuy81XFn3p1YAk9oaI1mk
  // http://quick-bench.com/a6GS0hx4ZZj-IsM48BpZF1TzEAw

  // helpful links curated by @nateriz
  // http://www.cplusplus.com/reference/streambuf/streambuf/sputc/
  // http://www.cplusplus.com/reference/streambuf/streambuf/overflow/
  // https://en.cppreference.com/w/cpp/io/basic_streambuf
  // https://gcc.gnu.org/onlinedocs/libstdc++/manual/streambufs.html

  private:

    // idea: we want an uninitialized vector like
    // http://andreoffringa.org/p/uvector/uvector.h
    // but want to do less work to get it,
    // so just make a vector of these instead!
    // profiling result: http://quick-bench.com/GB8SEE5N2I_Q4qcYUl7UjvTg-OY
    struct uninitialized_char {


      char val;

      // necessary to prevent zero-initialization of val
      uninitialized_char() { ; }

      operator char() const { return val; }

    };

    emp::vector<uninitialized_char> buffer;

  public:

    using const_iterator = emp::vector<uninitialized_char>::const_iterator;

    /// @param init_size num bytes to reserve initially
    ContiguousBuffer(const size_t init_size=1024)
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
    inline const char* GetData() const {
      // this is NOT undefined behavior
      // https://en.cppreference.com/w/cpp/language/reinterpret_cast
      return reinterpret_cast<const char*>(buffer.data());
    }

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

    /// Begin iterator on stored data
    inline const_iterator cbegin() const {
      return std::cbegin(buffer);
    }

    /// End iterator on stored data
    inline const_iterator cend() const {
      return std::cend(buffer);
    }

  private:

    // return a mutable pointer to contiguous memory data was streamed to
    inline char* GetData() {
      // https://en.cppreference.com/w/cpp/language/reinterpret_cast
      // this is NOT undefined behavior
      return reinterpret_cast<char*>(buffer.data());
    }

    /// return buffer's capacity, in bytes
    inline size_t GetCapacity() const { return buffer.size(); }

    // sputc calls this when we run out of space in buffer.
    int_type overflow(int_type c) override {

      // increase the buffer's capacity by approx golden ratio
      const size_t prev_capacity = GetCapacity();
      buffer.resize(
        prev_capacity
        + std::max(prev_capacity/2, static_cast<size_t>(1)) // grow at least one
      );

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

  // Streams data to contiguous memory, backed by ContiguousBuffer
  class ContiguousStream : public std::ostream {

  private:

    ContiguousBuffer buffer;

  public:

    using const_iterator = ContiguousBuffer::const_iterator;

    ContiguousStream(const size_t init_size=1024)
    : std::ostream(&buffer)
      , buffer(
        std::max(static_cast<size_t>(1), init_size) // 0 init size breaks!
      )
    { ; }

    /// Reset stream pointers while retaining underlying memory allocation.
    void Reset() { buffer.Reset(); }

    /// Return a pointer to contiguous memory storing streamed data.
    inline const char* GetData() const { return buffer.GetData(); }

    /// Number of bytes currently stored.
    inline size_t GetSize() const { return buffer.GetSize(); }

    /// Copy stored data into an ostream.
    /// Useful for degugging.
    void Print(std::ostream & os=std::cout) const { buffer.Print(os); }

    /// Begin iterator on stored data
    inline const_iterator cbegin() const { return buffer.cbegin(); }

    /// End iterator on stored data
    inline const_iterator cend() const { return buffer.cend(); }

  };

}
