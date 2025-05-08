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

#include "Lexer.hpp"

namespace fs = std::filesystem;

enum class Mode {
  Silent,        // No output, just error code.
  Normal,        // Basic summary output.
  Verbose,       // Full output
  Interactive    // Full output with questions.
};

// ========= ANSI Output Format Helpers ==========

emp::String ToOption(emp::String key) {
  return emp::MakeString('\'', key.AsANSIMagenta(), '\'');
}

// Each character in the string provided will be converted into a distinct option.
emp::String ToOptionSet(emp::String keys) {
  emp::String out;
  for (size_t i = 0; i < keys.size(); ++i) {
    if (i) out += '/';
    out += emp::ANSI::MakeMagenta(std::string(1,keys[i]));
  }
  return out;
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
inline void InternalError(emplex::Token token, Ts &&... args) {
  std::cerr << ToBoldRed("Internal Empecable Error (Line ", token.line_id, "): ")
            << emp::MakeString(args...) << std::endl;
  exit(1);
}
