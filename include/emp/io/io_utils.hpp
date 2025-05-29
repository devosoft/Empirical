/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025
*/
/**
 *  @file
 *  @brief Tools to simplify IO, such as with a command line
 *  @note Status: BETA
 *
 */

#ifndef EMP_IO_IO_UTILS_HPP_INCLUDE
#define EMP_IO_IO_UTILS_HPP_INCLUDE

#include <compare>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

#include "../base/assert.hpp"
#include "../base/vector.hpp"
#include "../tools/String.hpp"

#ifdef _WIN32
  #include <conio.h>
  namespace emp {
    static constexpr bool WINDOWS_IO = true;
  }
#else
  #include <termios.h>
  #include <unistd.h>
  namespace emp {
    static constexpr bool WINDOWS_IO = false;
  }
#endif

namespace emp {

  template <typename... ARG_Ts>
  inline void PrintTo(std::ostream & os, ARG_Ts &&... args) {
    // If nothing to print, skip.
    if constexpr (sizeof...(args) == 0) { return; }
    else (os << ... << std::forward<ARG_Ts>(args));
  }

  template <typename... ARG_Ts>
  void Print(ARG_Ts &&... args) { PrintTo(std::cout, std::forward<ARG_Ts>(args)...); }

  template <typename... ARG_Ts>
  void PrintLn(ARG_Ts &&... args) { Print(std::forward<ARG_Ts>(args)..., '\n'); }

  void PrintRepeat(char ch, size_t count) { Print(emp::String(count, ch)); }

  template <typename T>
  void PrintRepeat(T arg, size_t count) { for (size_t i=0; i < count; ++i) Print(arg); }

  template <typename... ARG_Ts>
  void PrintRepeatLn(ARG_Ts &&... args) { PrintRepeat(std::forward<ARG_Ts>(args)...), PrintLn(); }


  // A shortcut to represent specific IO characters.
  struct IOChar {
    int char_id=0;

    static constexpr int UNKNOWN = 1000;
    static constexpr int ESCAPE  = 1001;
    static constexpr int UP      = 1002;
    static constexpr int RIGHT   = 1003;
    static constexpr int DOWN    = 1004;
    static constexpr int LEFT    = 1005;

    IOChar(int value=0) : char_id(value) { }
    IOChar(const IOChar &) = default;

    IOChar & operator=(const IOChar &) = default;
    auto operator<=>(const IOChar &) const = default;

    std::string GetName() const {
      switch(char_id) {
      case 0: return "\\0"; // "Null", Or "\\u" ?
      case 1: return "Start of Heading";
      case 2: return "Start of Text";
      case 3: return "End of Text";
      case 4: return "End of Transmission";
      case 5: return "Enquiry";
      case 6: return "Acknowledge";
      case 7: return "\\a";  // "Audible Bell"
      case 8: return "\\b";  // "Backspace"
      case 9: return "\\t";  // "Tab"
      case 10: return "\\n"; // "Line Feed"
      case 11: return "\\v"; // "Vertical Tab"
      case 12: return "\\f"; // "Form Feed";
      case 13: return "\\r"; // "Carriage Return";
      case 14: return "Shift Out";
      case 15: return "Shift In";
      case 16: return "Data Link Escape";
      case 17: return "Device Control One (XON)";
      case 18: return "Device Control Two";
      case 19: return "Device Control Three (XOFF)";
      case 20: return "Device Control Four";
      case 21: return "Negative Acknowledge";
      case 22: return "Synchronous Idle";
      case 23: return "End of Transmission Block";
      case 24: return "Cancel";
      case 25: return "End of medium";
      case 26: return "Substitute";
      case 27: return "\\e"; // "Escape";
      case 28: return "File Separator";
      case 29: return "Group Separator";
      case 30: return "Record Separator";
      case 31: return "Unit Separator";

      case IOChar::ESCAPE: return "ESC";
      case IOChar::UP: return "UP";
      case IOChar::DOWN: return "DOWN";
      case IOChar::RIGHT: return "RIGHT";
      case IOChar::LEFT: return "LEFT";
      default:
      if (char_id < 128) return emp::String(1, static_cast<char>(char_id));
      }

      return "Unknown";
    }

    operator int() const { return char_id; }
  };

  IOChar GetIOChar() {
    #ifdef _WIN32
      int key = _getch();
      if (key == 0 || key == 224) {
        switch (_getch()) {
          case 72: return IOChar::UP;
          case 80: return IOChar::DOWN;
          case 75: return IOChar::LEFT;
          case 77: return IOChar::RIGHT;
          default: return IOChar::UNKNOWN;
        }
      }
      return key;
    #else
      termios old_term;
      tcgetattr(STDIN_FILENO, &old_term);
      termios new_term = old_term;
      new_term.c_lflag &= ~(ICANON | ECHO);
      tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

      char key;
      read(STDIN_FILENO, &key, 1);

      if (key == '\033') {  // Escape character
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) == 0 ||
            read(STDIN_FILENO, &seq[1], 1) == 0) {
          tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
          return IOChar::ESCAPE;
        }

        tcsetattr(STDIN_FILENO, TCSANOW, &old_term);

        if (seq[0] == '[') {
          switch (seq[1]) {
            case 'A': return IOChar::UP;
            case 'B': return IOChar::DOWN;
            case 'C': return IOChar::RIGHT;
            case 'D': return IOChar::LEFT;
          }
        }

        return IOChar::UNKNOWN;
      }

      tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
      return key;
    #endif
  }


  // ====================== File System Functions ======================

  namespace std_fs = std::filesystem;

  using split_path_t = emp::vector<std_fs::path>;

  // Take a provided path and split it up into each directory (possibly ending in a filename.)
  split_path_t SplitPath(const std_fs::path & in_path) {
    split_path_t components;
    for (const auto & part : in_path) components.push_back(part);
    return components;
  };

  // Take a split path and turn it back into a combined std_fs::path.
  std_fs::path JoinPath(const split_path_t & path) {
    std_fs::path result;
    for (const auto & part : path) result /= part;
    return result;
  }

  // Find the target folder found along a given file path (for example, .git/ or .vscode/)
  [[nodiscard]] std::optional<std_fs::path> FindFolderInPath(std::string folder_name, std_fs::path test_path) {
    emp_assert(std_fs::exists(test_path), test_path);
    if (!std_fs::is_directory(test_path)) test_path = test_path.parent_path();
    while (!test_path.empty() && test_path != "/") {
      std_fs::path candidate = test_path / folder_name;
      if (std_fs::exists(candidate) && std_fs::is_directory(candidate)) {
        return candidate;
      }
      std_fs::path parent_path = test_path.parent_path();
      if (parent_path == test_path) break; // If we are no longer descending, stop!
      test_path = parent_path;
    }
    return std::nullopt;  // Not found
  }

  // Given two split-up paths, reduce the first to the common path.
  void ReduceToCommonPath(split_path_t & path1, const split_path_t & path2) {
    size_t match_size = 0;
    while (match_size < path1.size() &&
           match_size < path2.size() &&
           path1[match_size] == path2[match_size]) ++match_size;
    path1.resize(match_size);
  }

  // Take a provided path and split it up into each directory (possibly ending in a filename.)
  std_fs::path FindCommonPath(std_fs::path path1, std_fs::path path2) {
    if (path1 == path2) return path1;

    auto p1_it = path1.begin();
    auto p2_it = path2.begin();
    std_fs::path result;

    while (p1_it != path1.end() && p2_it != path2.end() && *p1_it == *p2_it) {
      result /= *p1_it;
      ++p1_it;
      ++p2_it;
    }

    return result;
  };

  // Given a collection of file paths, find the portion that they all share.
  template <typename CONTAINER_T>
  [[nodiscard]] std_fs::path FindCommonPath(const CONTAINER_T & filenames) {
    if (filenames.empty()) return {};

    // Start with the first path split into components.
    auto it = filenames.begin();
    split_path_t common = SplitPath(*it);

    // Step through each of the other paths, narrowing down the common components.
    while (++it != filenames.end()) {
      ReduceToCommonPath(common, SplitPath(*it));
    }

    // Reassemble the common path.
    return JoinPath(common);
  }

  bool CanWriteToDirectory(const std_fs::path & dir,
                           std::string test_name=".Empirical_test_delete_me") {
    std::filesystem::path test_file = dir / test_name;
    std::ofstream ofs(test_file);
    if (!ofs) return false; // Failed to create file.
    ofs.close();
    std::filesystem::remove(test_file);
    return true;
  }
}

#endif // #ifndef EMP_IO_IO_UTILS_HPP_INCLUDE

// Local settings for Empecable file checker.
// empecable_words: termios tcgetattr lflag tcsetattr getch ofs
