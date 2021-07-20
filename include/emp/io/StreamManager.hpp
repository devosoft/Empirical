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
#include <sstream>
#include <unordered_map>

#include "../base/Ptr.hpp"
#include "../datastructs/map_utils.hpp"

namespace emp {

  /// A class to maintain files and other streams.
  class StreamManager {
  public:
    enum stream_t {
      FILE_STREAM, STRING_STREAM, STD_STREAM
    };
  protected:
    std::unordered_map<std::string, emp::Ptr<std::ostream>> of_streams;
    std::unordered_map<std::string, emp::Ptr<std::stringstream>> string_streams;

    stream_t default_input_type = STD_STREAM;
    stream_t default_output_type = STD_STREAM;    

  public:
    StreamManager() : of_streams() { ; }
    StreamManager(const StreamManager &) = default;
    StreamManager(StreamManager &&) = default;
    ~StreamManager() {
      for (auto [name, ptr] : of_streams) ptr.Delete();
      for (auto [name, ptr] : string_streams) ptr.Delete();
    }

    std::ostream & GetOutputStream(const std::string & filename="cout", const std::string & stdout_name="cout") {
      if (filename == "" || filename == stdout_name) return std::cout;
      if (!emp::Has(of_streams, filename)) {
        of_streams[filename] = emp::NewPtr<std::ofstream>(filename);
      }
      return *of_streams[filename];
    }

    std::stringstream & GetStringStream(const std::string & name) {
      if (!emp::Has(string_streams, name)) {
        string_streams[name] = emp::NewPtr<std::stringstream>();
      }
      return *string_streams[name];
    }

    bool HasOutputStream(const std::string & filename) { return emp::Has(of_streams, filename); }
    bool HasStringStream(const std::string & name) { return emp::Has(string_streams, name); }
    bool HasStream(const std::string & name) {
      return HasOutputStream(name) || HasStringStream(name);
    }

    std::ostream & get_ostream(const std::string & filename="cout", const std::string & stdout_name="cout") {
      return GetOutputStream(filename, stdout_name);
    }

  };

}

#endif
