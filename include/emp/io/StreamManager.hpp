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
    enum stream_t { FILE_STREAM, STRING_STREAM, STD_STREAM };
    enum access_t { INPUT, OUTPUT, IO }

  protected:
    template <typename T>
    struct StreamInfo {
      stream_t type;
      emp::Ptr<T> ptr;
      bool owned = true;
    }

    std::unordered_map<std::string, StreamInfo<std::istream>> input_streams;
    std::unordered_map<std::string, StreamInfo<std::ostream>> output_streams;
    std::unordered_map<std::string, StreamInfo<std::iostream>> io_streams;

    stream_t default_input_type = STD_STREAM;
    stream_t default_output_type = STD_STREAM;    
    stream_t default_io_type = STD_STREAM;    

    // Helper functions.
    template <access_t ACCESS, typename T>
    T & AddInfo(const std::string & name, emp::Ptr<T> ptr) {
      emp_assert(!Has(name));
      StreamInfo<T> info{FILE_STREAM, ptr, true};
      if constexpr (access == INPUT) input_streams[name] = info;
      if constexpr (access == OUTPUT) output_streams[name] = info;
      if constexpr (access == IO) io_streams[name] = info;
      return *(output_streams[name].ptr);
    }
  public:
    StreamManager() { ; }
    StreamManager(const StreamManager &) = delete;
    StreamManager(StreamManager &&) = default;
    ~StreamManager() {
      for (auto [name, info] : input_streams) if (info.owned) info.ptr.Delete();
      for (auto [name, info] : output_streams) if (info.owned) info.ptr.Delete();
      for (auto [name, info] : io_streams) if (info.owned) info.ptr.Delete();
    }

    // Check to see if certain streams are being managed.
    bool HasInputStream(const std::string & name) { return emp::Has(input_streams, name); }
    bool HasOutputStream(const std::string & name) { return emp::Has(output_streams, name); }
    bool HasIOStream(const std::string & name) { return emp::Has(io_streams, name); }
    bool Has(const std::string & name) {
      return HasInputStream(name) || HasOutputStream(name) || HasIOStream(name);
    }

    // Add streams of specific types.

    std::istream & AddInputFile(std::string name) { return AddInfo<INPUT>(name, NewPtr<std::ifstream>(name)); }

    std::istream & AddInputFile(std::string name) { return AddFileInfo<std::ifstream,INPUT>(name); }
    std::ostream & AddOutputFile(std::string name) { return AddFileInfo<std::ofstream,OUTPUT>(name); }
    std::iostream & AddFile(std::string name) { return AddFileInfo<std::fstream,IO>(name); }
    std::iostream & AddStringStream(std::string name)

    std::ostream & GetOutputStream(const std::string & filename="cout", const std::string & stdout_name="cout") {
      if (filename == "" || filename == stdout_name) return std::cout;
      if (!emp::Has(of_streams, filename)) return AddOutputFile(filename);
      return *of_streams[filename];
    }

    std::stringstream & GetStringStream(const std::string & name) {
      if (!emp::Has(string_streams, name)) {
        string_streams[name] = emp::NewPtr<std::stringstream>();
      }
      return *string_streams[name];
    }

    std::ostream & get_ostream(const std::string & filename="cout", const std::string & stdout_name="cout") {
      return GetOutputStream(filename, stdout_name);
    }

  };

}

#endif
