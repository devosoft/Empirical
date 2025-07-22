/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2015-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/debug/debug.hpp
 * @brief Basic tools for use in developing high-assurance code.
 * @note Status: BETA
 */

#pragma once

#ifndef INCLUDE_EMP_DEBUG_DEBUG_HPP_GUARD
#define INCLUDE_EMP_DEBUG_DEBUG_HPP_GUARD

#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "../base/notify.hpp"

#define EMP_DEBUG_PRINT(...) std::cout << "[" << #__VA_ARGS__ << "] = " << __VA_ARGS__ << std::endl

namespace emp {

/// BlockRelease(true) will halt compilation if NDEBUG is on and EMP_NO_BLOCK is off.
/// It is useful to include alongside debug code that you want to remember to remove when you
/// are done debugging; it is automatically included with the emp_debug() function below.
/// If you want to intentionally compile in release mode, make sure to define EMP_NO_BLOCK.
#ifdef NDEBUG
#ifdef EMP_NO_BLOCK
#define BlockRelease(BLOCK)
#else  // #ifdef EMP_NO_BLOCK
#define BlockRelease(BLOCK) \\
        std::cerr << "Release block at " << __FILE___ << ", line " << __LINE__ << std::endl;\\
        static_assert(!BLOCK, "Release blocked due to debug material.")
#endif  // #ifdef EMP_NO_BLOCK : #else
#else   // #ifdef NDEBUG
#define BlockRelease(BLOCK)
#endif  // #ifdef NDEBUG : #else

  /// The EMP_DEBUG macro executes its contents in debug mode, but otherwise ignores them.
  /// test_debug() can be used inside of an if-constexpr for code you want only in debug mode.
#ifdef NDEBUG
#define EMP_DEBUG(...)

  constexpr bool test_debug() { return false; }
#else  // #ifdef NDEBUG
#define EMP_DEBUG(...) __VA_ARGS__

  constexpr bool test_debug() { return true; }
#endif  // #ifdef NDEBUG : #else

  template <typename... Ts>
  void emp_debug_print(Ts &&... args) {
    (std::cerr << ... << std::forward<Ts>(args));
    std::cerr << std::endl;
  }

/// emp_debug_only() will print its contents as a message in debug mode and BLOCK release mode until
/// it is removed.  It's a useful too for printing "Ping1", "Ping2", etc, but not forgetting to
/// remove them.
#define emp_debug_only(...) { BlockRelease(true); emp::emp_debug_print(__VA_ARGS__); }


  /// emp_debug(...) will print its contents in debug mode, but ignore them otherwise.
#ifdef NDEBUG
#define emp_debug(...)
#else  // #ifdef NDEBUG
#define emp_debug(...) emp::emp_debug_print(__VA_ARGS__)
#endif  // #ifdef NDEBUG : #else

  /// Track particular lines of code to report errors about them from elsewhere.
  static auto & GetDebugLineMap() {
    static std::unordered_map<std::string, std::string> line_map;
    return line_map;
  }

  static bool HasDebugLine(std::string name) { return GetDebugLineMap().contains(name); }

  static auto & GetDebugLine(std::string name) { return GetDebugLineMap()[name]; }

  [[maybe_unused]] static void AddDebugLine(std::string name, std::string file, size_t line) {
    std::stringstream ss;
    ss << file << ':' << line;
    notify::TestError(HasDebugLine(name), "REPLACING debug line named '", name, "'.");
    GetDebugLine(name) = ss.str();
  }

#ifdef NDEBUG
#define EMP_TRACK_LINE(NAME)
#else  // #ifdef NDEBUG
#define EMP_TRACK_LINE(NAME) emp::AddDebugLine(NAME, __FILE__, __LINE__)
#endif  // #ifdef NDEBUG : #else

// DebugStack will use RAII to track which functions you are in.
struct DebugStackInfo {
  std::string filename{};
  int line_num = -1;
  std::string extras{};
  std::string FilePos() const {
    return filename + ':' + std::to_string(line_num);
  }
};

static std::vector<DebugStackInfo> & GetDebugStack() {
  static std::vector<DebugStackInfo> debug_stack;
  return debug_stack;
}

struct DebugStackEntry {
  DebugStackInfo info;
  DebugStackEntry(std::string filename, int line_num, std::string extras)
    : info({filename, line_num, extras}) { GetDebugStack().push_back(info); }
  ~DebugStackEntry() {
    if (GetDebugStack().back().filename != info.filename ||
        GetDebugStack().back().line_num != info.line_num) {
      notify::Error("Debug Stack Corruption!  Found '", GetDebugStack().back().FilePos(),
                    "', but expected '", info.FilePos(), "'.");
    }
    GetDebugStack().pop_back();
  }
};

static std::string DebugStackToString() {
  const auto & stack = GetDebugStack();
  std::string out{"\n"};
  for (size_t i = stack.size()-1; i < stack.size(); --i) {
    out += std::to_string(stack.size()-i) + ": " + stack[i].FilePos()
        + "(" + std::to_string(stack.size()-i) + ": " + stack[i].extras + ")\n";
  }
  return out;
}

#ifdef NDEBUG
#define DEBUG_STACK(...)
#else  // #ifdef NDEBUG
#define DEBUG_STACK() emp::DebugStackEntry emp_debug_stack_entry(__FILE__, __LINE__, __func__);
#endif  // #ifdef NDEBUG : #else


}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_DEBUG_DEBUG_HPP_GUARD
