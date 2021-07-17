/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020-2021.
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
    std::unordered_map<std::string, emp::Ptr<std::ostream>> out_streams;

  public:
    StreamManager() : out_streams() { ; }
    StreamManager(const StreamManager &) = default;
    StreamManager(StreamManager &&) = default;
    ~StreamManager() { ; }

    std::ostream & GetOutputStream(const std::string & filename="cout", const std::string & stdout_name="cout") {
      if (filename == "" || filename == stdout_name) return std::cout;
      if (!emp::Has(out_streams, filename)) {
        out_streams[filename] = emp::NewPtr<std::ofstream>(filename);
      }
      return *out_streams[filename];
    }

    bool HasStream(const std::string & filename) { return emp::Has(out_streams, filename); }

    std::ostream & get_ostream(const std::string & filename="cout", const std::string & stdout_name="cout") {
      return GetOutputStream(filename, stdout_name);
    }

  };

}

#endif
