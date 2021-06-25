/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  StreamManager.hpp
 *  @brief The StreamManager object links names to files or other streams.
 *  @note  Status: BETA
 *
 *  @todo  Ideally should also work with emp::File
 */


#ifndef EMP_STREAM_MANAGER_H
#define EMP_STREAM_MANAGER_H

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

#include "../base/Ptr.hpp"
#include "../datastructs/map_utils.hpp"

namespace emp {

  /// A class to maintain files and other streams.
  class StreamManager {
  protected:
    std::unordered_map<std::string, emp::Ptr<std::ostream>> streams;

  public:
    StreamManager() : streams() { ; }
    StreamManager(const StreamManager &) = default;
    StreamManager(StreamManager &&) = default;
    ~StreamManager() { ; }

    std::ostream & get_ostream(const std::string & filename, const std::string & stdout_name="cout") {
      if (filename == "" || filename == stdout_name) return std::cout;
      if (!emp::Has(streams, filename)) {
        streams[filename] = emp::NewPtr<std::ofstream>(filename);
      }
      return *streams[filename];
    }

  };

}

#endif
