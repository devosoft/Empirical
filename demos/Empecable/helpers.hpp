/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file
 * @brief Extra helper functions for Empecable
 */

#pragma once

#include <filesystem>

#include "../../include/emp/tools/String.hpp"

namespace fs = std::filesystem;


// ========= ANSI Output Format Helpers ==========

emp::String ToOption(emp::String key) {
  return emp::MakeString('\'', key.AsANSIMagenta(), '\'');
}

emp::String ToFilename(fs::path filename) {
  return emp::ANSI::MakeGreen(filename.string());
}

template <typename... Ts>
emp::String ToBoldRed(Ts &&... args) {
  return emp::MakeString(args...).AsANSIRed().AsANSIBold();
}

// ========= Screen output Helpers ==========

template <typename... Ts>
inline void Print(Ts &&... args) { std::cout << emp::MakeString(args...); }

template <typename... Ts>
inline void PrintLn(Ts &&... args) { std::cout << emp::MakeString(args...) << std::endl; }

template <typename... Ts>
inline void InternalError(Ts &&... args) {
  std::cerr << ToBoldRed("Internal Empecable Error: ") << emp::MakeString(args...) << std::endl;
  exit(1);
}

