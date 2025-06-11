/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2018 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/io/NullStream.hpp
 * @brief A handy no-operation output stream.
 * @note Status: BETA
 */

#pragma once

#ifndef INCLUDE_EMP_IO_NULL_STREAM_HPP_GUARD
#define INCLUDE_EMP_IO_NULL_STREAM_HPP_GUARD

#include <iostream>

namespace emp {

  /// A no-operation buffer class
  class NullBuffer : public std::streambuf {
  public:
    int overflow(int c) { return c; }
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

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_IO_NULL_STREAM_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: nout
