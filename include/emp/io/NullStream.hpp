/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  NullStream.hpp
 *  @brief A handy no-operation output stream.
 *  @note Status: BETA
 */

#ifndef EMP_NULLSTREAM_H
#define EMP_NULLSTREAM_H

#include <iostream>

namespace emp {

  /// A no-operation buffer class
  class NullBuffer : public std::streambuf {
    public:
      int overflow(int c) {
        return c;
      }
  };

  /// A no-operation output stream class
  class NullStream : public std::ostream {
    public:
      NullStream() : std::ostream(&m_sb) { ; }
    private:
      NullBuffer m_sb{};
  };

  /// No-operation drop-in replacement for std::cout
  static NullStream nout;

}

#endif
