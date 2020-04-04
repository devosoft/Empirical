/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  FileManager.h
 *  @brief The FileManager object links filenames to open files.
 *  @note  Status: BETA
 *
 *  @todo  Ideally should also work with emp::File
 */


#ifndef EMP_FILE_MANAGER_H
#define EMP_FILE_MANAGER_H

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

#include "../base/Ptr.h"
#include "map_utils.h"

namespace emp {

  /// A class to maintin files for loading, writing, storing, and easy access to components.
  class FileManager {
  protected:
    emp::unordered_map<std::string, emp::Ptr<std::ostream>> streams;

  public:
    FileManager() : streams() { ; }
    FileManager(const FileManager &) = default;
    FileManager(FileManager &&) = default;
    ~FileManager() { ; }

    std::ostream & get_ostream(const std::string & filename, const std::string & stdout_name="cout") {
      if (filename == stdout_name) return std::cout;
      if (!emp::Has(streams, filename)) streams[filename] = emp::NewPtr<ofstream>(filename);
      return *streams[filename];
    }

  };
  
}

#endif
