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

#include "../../include/emp/io/io_utils.hpp"
#include "../../include/emp/tools/String.hpp"

#include "Lexer.hpp"

namespace fs = std::filesystem;

enum class Mode {
  Silent,        // No output, just error code
  Normal,        // Basic summary output
  Verbose,       // Full output
  Interactive,   // Full output with questions
  DEBUG          // Extra output for debugging
};

// ========= Input Helpers =========

emp::String GetInput(const emp::String & prompt) {
  emp::Print(prompt);
  emp::String input{};

  while (input.size() == 0) {
    std::getline(std::cin, input);
    // std::cin >> input;
    if (input == "\\n" || input.size() == 0) emp::Print("No input found.");
  }
  return input;
}

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

emp::String ToFilename(const std::string & filename) {
  return emp::ANSI::MakeGreen(filename);
}

template <typename... Ts>
emp::String ToBoldRed(Ts &&... args) {
  return emp::MakeString(args...).AsANSIRed().AsANSIBold();
}

template <typename... Ts>
emp::String ToBoldYellow(Ts &&... args) {
  return emp::MakeString(args...).AsANSIYellow().AsANSIBold();
}

template <typename... Ts>
void PrintError(Ts &&... args) {
  emp::PrintLn(ToBoldRed("ERROR: ", args...));
}

template <typename... Ts>
void PrintWarning(Ts &&... args) {
  emp::PrintLn(ToBoldYellow("Warning: ", args...));
}

// ========= Screen output Helpers ==========

template <typename... Ts>
inline void InternalError(emplex::Token token, Ts &&... args) {
  std::cerr << ToBoldRed("Internal Empecable Error (Line ", token.line_id, "): ")
            << emp::MakeString(args...) << std::endl;
  exit(1);
}
