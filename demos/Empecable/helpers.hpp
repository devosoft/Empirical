/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file demos/Empecable/helpers.hpp
 * @brief Extra helper functions for Empecable
 */

#pragma once

#ifndef DEMOS_EMPECABLE_HELPERS_HPP_GUARD
#define DEMOS_EMPECABLE_HELPERS_HPP_GUARD

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

template <typename... Ts>
emp::String GetInput(Ts &... args) {
  emp::Print(emp::MakeString(args...));
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
emp::String ToBoldGreen(Ts &&... args) {
  return emp::MakeString(args...).AsANSIGreen().AsANSIBold();
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

// Take in a set of mid-comment lines.
// Strip off stars at the beginning and replace with simple " * "
emp::String AdjustCommentStars(emp::String comment, const emp::String prefix = " *") {
  auto lines = comment.Slice('\n', emp::StringSyntax::None());

  // Scan all lines to determine how many chars to clip from the beginning.
  size_t clip = comment.size();
  for (auto line : lines) {
    bool found_star = false;
    for (size_t pos = 0; pos < line.size() && pos < clip; ++pos) {
      if (line[pos] == ' ') continue;          // Allow beginning spaces.
      if (!found_star && line[pos] == '*') {   // Allow ONE star
        found_star = true;
        continue;
      }
      // If we made it here, we've hit a char on this line that indicates prefix is done!
      clip = pos;
    }
  }

  // Do the clipping!
  emp::String result;
  for (auto line : lines) {
    if (line.size() <= clip) result += prefix + '\n';
    else { result += prefix + " " + line.erase(0, clip) + '\n'; }
  }

  return result;
}


#endif // #ifndefDEMOS_EMPECABLE_HELPERS_HPP_GUARD

