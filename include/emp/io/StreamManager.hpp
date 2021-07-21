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
      bool owned;
    }

    // Struct to hold all streams of a given type.
    template <typename T, bool CONSTRUCT_WITH_NAME=false>
    struct StreamCollection {
      stream_t type;
      access_t access;
      using info_t = StreamInfo<T>;
      std::unordered_map<std::string, info_t> streams;

      StreamCollection(stream_t in_type, access_t in_access) : type(in_type), access(in_access) { }
      ~StreamCollection() {
        for (auto & [name, info] : streams) if (info.owned) info.ptr.Delete();
      }

      bool Has(const std::string & name) const { return emp::Has(streams, name); }
 
      T & Add(const std::string & name, emp::Ptr<T> ptr, bool owned=false) {
        emp_assert(!Has(name));
        streams[name] = info_t{type, ptr, owned};
        return *(streams[name].ptr);
      }

      T & Add(const std::string & name) {
        if constexpr (CONTRUCT_WITH_NAME) ptr = emp::Ptr<T>(name);
        else ptr = emp::Ptr<T>();
        return Add(name, ptr, true);
      }

      T & Get(const std::string name) {
        emp_assert(Has(name));
        return *(streams[name].ptr);        
      }
    };

    // File streams
    StreamCollection<std::ifstream,true> ifstream_collect;
    StreamCollection<std::ofstream,true> ofstream_collect;
    StreamCollection<std::fstream,true> fstream_collect;

    // String streams
    StreamCollection<std::stringstream> sstream_collect;

    // Other streams
    StreamCollection<std::istream> istream_collect;
    StreamCollection<std::ostream> ostream_collect;
    StreamCollection<std::iostream> iostream_collect;

    stream_t default_input_type = STD_STREAM;
    stream_t default_output_type = STD_STREAM;    
    stream_t default_io_type = STD_STREAM;    

  public:
    StreamManager()
      : ifstream_collect(FILE_STREAM, INPUT)
      , ofstream_collect(FILE_STREAM, OUTPUT)
      , fstream_collect(FILE_STREAM, IO)
      , sstream_collect(STRING_STREAM, IO)
      , istream_collect(STD_STREAM, INPUT)
      , ostream_collect(STD_STREAM, OUTPUT)
      , iostream_collect(STD_STREAM, IO)
      { }
    StreamManager(const StreamManager &) = delete;
    StreamManager(StreamManager &&) = default;
    ~StreamManager() { }

    // Check to see if certain streams are being managed.
    bool HasInputFileStream(const std::string & name) { return ifstream_collect.Has(name); }
    bool HasOutputFileStream(const std::string & name) { return ofstream_collect.Has(name); }
    bool HasIOFileStream(const std::string & name) { return fstream_collect.Has(name); }
    bool HasStringStream(const std::string & name) { return sstream_collect.Has(name); }
    bool HasStdInputStream(const std::string & name) { return istream_collect.Has(name); }
    bool HasStdOutputStream(const std::string & name) { return ostream_collect.Has(name); }
    bool HasStdIOStream(const std::string & name) { return iostream_collect.Has(name); }

    bool HasInputOnlyStream(const std::string & name) {
      return HasInputFileStream(name) || HasStdInputStream(name);
    }
    bool HasOutputOnlyStream(const std::string & name) {
      return HasOutputFileStream(name) || HasStdOutputStream(name);
    }
    bool HasIOStream(const std::string & name) {
      return HasIOFileStream(name) || HasStringStream(name) || HasStdIOStream(name);
    }
    bool Has(const std::string & name) {
      return HasInputOnlyStream(name) || HasOutputOnlyStream(name) || HasIOStream(name);
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
