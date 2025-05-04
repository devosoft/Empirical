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

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
 
#include "../base/assert.hpp"
#include "../base/vector.hpp"

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

  char GetIOChar() {
    #ifdef _WIN32
      return _getch();
    #else
      termios old_term;
      char ch;
      tcgetattr(STDIN_FILENO, &old_term);
      termios new_term = old_term;
      new_term.c_lflag &= ~(ICANON | ECHO);
      tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
      read(STDIN_FILENO, &ch, 1);
      tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
      return ch;
    #endif
  }


  // ====================== File System Functions ======================

  namespace std_fs = std::filesystem;

  using split_path_t = emp::vector<std_fs::path>;

  // Take a provided path and split it up into each directory (possibly ending in a filename.)
  split_path_t SplitPath(const std_fs::path & in_path) {
    split_path_t components;
    for (const auto& part : in_path) components.push_back(part);
    return components;
  };

  // Take a split path and turn it back into a combined std_fs::path.
  std_fs::path JoinPath(const split_path_t & path) {
    std_fs::path result;
    for (const auto & part : path) result /= part;
    return result;
  }

  // Find the target folder found along a given file path (for example, .git/ or .vscode/)
  std::optional<std_fs::path> FindFolderInPath(std::string folder_name, std_fs::path test_path) {
    emp_assert(std_fs::exists(test_path), test_path);
    if (!std_fs::is_directory(test_path)) test_path = test_path.parent_path();
    while (!test_path.empty()) {
      std_fs::path candidate = test_path / folder_name;
      if (std_fs::exists(candidate) && std_fs::is_directory(candidate)) {
        return candidate;
      }
      test_path = test_path.parent_path();
    }
    return std::nullopt;  // Not found
  }
  
  // Given two split-up paths, reduce the first to the common path.
  void ReduceToCommonPath(split_path_t & path1, const split_path_t & path2) {
    size_t match_size = 0;
    while (match_size < path1.size() && match_size < path2.size() &&
           path1[match_size] == path2[match_size]) ++match_size;
    path1.resize(match_size);
  }

  // Given a collection of files, find the portion of their path that they all share.
  template <typename CONTAINER_T>
  [[nodiscard]] std_fs::path FindCommonPath(const CONTAINER_T & filenames) {
    if (filenames.empty()) return {};

    // Start with the first path split into components.
    auto it = filenames.begin();
    split_path_t common = SplitPath(*it);
  
    // Step through each of the other paths, narrowing down the common components.
    while (++it != filenames.end()) {
      ReduceToCommonPath(common, SplitPath(paths[i]));
    }
  
    // Reassemble the common path.
    return JoinPath(common);
  } 
}

#endif // #ifndef EMP_IO_IO_UTILS_HPP_INCLUDE
 