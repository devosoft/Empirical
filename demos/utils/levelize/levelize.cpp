/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file levelize.cpp
 */

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>

#include "../../../include/emp/base/vector.hpp"
#include "../../../include/emp/io/File.hpp"
#include "../../../include/emp/tools/string_utils.hpp"

using level_t = uint32_t;

struct FileInfo {
  std::string filename;
  std::string path;
  std::set<std::string> depends; // Which OTHER files does this one depend on?

  static constexpr level_t NO_LEVEL = (level_t) -1;
  level_t level = NO_LEVEL;
};

int main(int argc, char * argv[])
{
  if (argc == 1) {
    std::cerr << "No files listed.\nPlease run `" << argv[0] << " --help` for more info.\n";
    exit(0);
  }

  // Load in all of the files that we are working with.
  const size_t num_files = argc - 1;
  emp::vector<std::string> files(num_files);
  for (size_t i = 0; i < num_files; i++) files[i] = argv[i+1];

  // Check if we're just supposed to print the help info.
  if (files[0] == "--help") {
    std::cerr << "Format: " << argv[0] << " [args] {filename} [filenames...]\n"
        << "Available args:\n"
        << " -v : verbose output\n";
    exit(0);
  }

  std::cerr << num_files << " files found.  Processing!" << std::endl;

  bool verbose = false;

  // Simplify to just the filenames (remove paths)
  std::map<std::string, FileInfo> file_map;
  emp::vector<std::string> filenames;
  for (std::string & file : files) {
    if (file == "-v") {
      verbose = true;
      continue;
    }
    emp::vector<std::string_view> dir_struct = emp::view_slices(file, '/');
    std::string filename(dir_struct.back());
    file_map[filename].filename = filename;
    file_map[filename].path = file;
    filenames.push_back(filename);
  }

  // For each file, scan for its dependencies.
  for (auto & [filename, info] : file_map) {
    if (verbose) {
      std::cerr << "Scanning '" << filename << "' found at: "
        << info.path << std::endl;
    }

    emp::File file(info.path);
    file.KeepIfContains("#include");      // Only scan through include lines.
    file.RemoveIfContains("third-party"); // Ignore includes from third-party directory (may duplicate names)

    // Now test which OTHER filenames it is including.  Search for the filename with
    // a " or / in front of it (to make sure it's not part of another name)
    int include_count = 0;
    for (const std::string & filename : filenames) {
      if (file.Contains(emp::to_string("\"", filename)) ||
          file.Contains(emp::to_string("/", filename)) ) {
        info.depends.insert(filename);
        include_count++;
      }
    }
    if (verbose) {
      std::cerr << "...has " << include_count << " includes." << std::endl;
    }
  }

  // Now that we know dependences, figure out levels!
  level_t max_level = 0;
  bool progress = true;
  while (progress) {
    progress = false;

    if (verbose) {
      std::cerr << "Processing!" << std::endl;
    }

    // Loop through each file to see if we can determine its level.
    for (auto & [filename, info] : file_map) {
      if (info.level != FileInfo::NO_LEVEL) continue;  // Already has a level!

      // See if we can determine a level for this file.
      level_t new_level = 0;
      for (const std::string & depend_name : info.depends) {
        level_t test_level = file_map[depend_name].level;

        // If a dependency doesn't have a level yet, stop working on this one.
        if (test_level == FileInfo::NO_LEVEL) {
          new_level = FileInfo::NO_LEVEL;
          break;
        }

        // Otherwise see if we need to update our new_level for this file.
        if (test_level >= new_level) new_level = test_level + 1;
      }

      // If we have a level for this file now, use it an indicate progress!
      if (new_level != FileInfo::NO_LEVEL) {
        if (verbose) {
          std::cerr << "..." << info.filename << " assigned to level " << new_level << std::endl;
        }

        info.level = new_level;

        if (new_level > max_level) max_level = new_level;
        progress = true;
      }
    }
  }

  // List out the files and their levels.
  for (level_t level = 0; level <= max_level; level++) {
    std::cout << "============ LEVEL " << level << " ============\n";
    for (auto [filename, info] : file_map) {
      if (info.level != level) continue;
      std::cout << filename << " " << " (" << info.path << ")\n";
      if (level == 0) continue;
      std::cout << " :";
      for (const std::string & name : info.depends) {
        std::cout << " " << name << "(" << file_map[name].level << ")";
      }
      std::cout << std::endl;
    }
  }

  // Identify any files that we were NOT able to handle, if any.
  int unknown_count = 0;
  for (auto & [filename, info] : file_map) {
    if (info.level != FileInfo::NO_LEVEL) continue;  // Has a level!

    // Only print a header for this section if it has entries.
    if (unknown_count++ == 0) {
      std::cout << "\n============ UNKNOWN LEVEL! ============\n";
    }

    std::cout << filename << " " << " (" << info.path << ")\n";
    std::cout << " :";
    for (const std::string & name : info.depends) {
      std::string level = "Unknown";
      if (file_map[name].level != FileInfo::NO_LEVEL) level = emp::to_string(file_map[name].level);
      std::cout << " " << name << "(" << level << ")";
    }
    std::cout << std::endl;
  }
  if (verbose) {
    std::cerr << "Number of files with unknown levels: " << unknown_count << std::endl;
  }

}
