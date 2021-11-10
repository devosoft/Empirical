/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020-2021.
 *
 *  @file StreamManager.hpp
 *  @brief The StreamManager object links names to files or other streams.
 *  @note  Status: BETA
 *
 *  @todo  Ideally should also work with emp::File
 */

#ifndef EMP_IO_STREAMMANAGER_HPP_INCLUDE
#define EMP_IO_STREAMMANAGER_HPP_INCLUDE


#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "../base/assert.hpp"
#include "../base/error.hpp"
#include "../base/Ptr.hpp"
#include "../datastructs/map_utils.hpp"

namespace emp {

  /// A class to maintain files and other streams.
  class StreamManager {
  public:
    enum class Type { NONE=0, FILE, STRING, OTHER };
    enum class Access { NONE=0, INPUT=1, OUTPUT=2, IO=3 };

  protected:

    // Helper under error conditions.
    static std::iostream & GetDefaultStream() {
      static std::stringstream default_stream;
      return default_stream;
    }

    class StreamInfo {
    protected:
      std::string name;
      bool owned;

    public:
      StreamInfo(const std::string & in_name, bool in_owned) : name(in_name), owned(in_owned) { }
      StreamInfo(const StreamInfo &) = delete;
      virtual ~StreamInfo() {}

      virtual Type GetType() const = 0;
      virtual Access GetAccess() const = 0;

      bool IsFile() const { return GetType() == Type::FILE; }
      bool IsString() const { return GetType() == Type::STRING; }
      bool IsOther() const { return GetType() == Type::OTHER; }

      bool IsInput() const { return GetAccess() == Access::INPUT; }
      bool IsOutput() const { return GetAccess() == Access::OUTPUT; }
      bool IsIO() const { return GetAccess() == Access::IO; }

      bool IsInputFile() const { return IsFile() && IsInput(); }
      bool IsOutputFile() const { return IsFile() && IsOutput(); }
      bool IsIOFile() const { return IsFile() && IsIO(); }

      bool IsOtherInput() const { return IsOther() && IsInput(); }
      bool IsOtherOutput() const { return IsOther() && IsOutput(); }
      bool IsOtherIO() const { return IsOther() && IsIO(); }

      bool InputOK() const { return IsInput() || IsIO(); }
      bool OutputOK() const { return IsOutput() || IsIO(); }

      bool IsOwned() const { return owned; }

      virtual std::istream & GetInputStream() = 0;
      virtual std::ostream & GetOutputStream() = 0;
      virtual std::iostream & GetIOStream() = 0;
    };

    template <typename T, Type TYPE, Access ACCESS>
    class TypedStreamInfo : public StreamInfo {
    protected:
      emp::Ptr<T> ptr = nullptr;

    public:
      // Constructor to build a NEW stream based on the details passed in.
      TypedStreamInfo(const std::string & name) : StreamInfo(name, true) {
        ResetStream();
      }

      // Constructor to use an EXISTING stream that should be passed in.
      TypedStreamInfo(const std::string & name, emp::Ptr<T> in_ptr)
        : StreamInfo(name, false), ptr(in_ptr) { }

      ~TypedStreamInfo() {
        if (owned) ptr.Delete();
      }

      Type GetType() const override { return TYPE; }
      Access GetAccess() const override { return ACCESS; }
      static constexpr bool INPUT_OK = (ACCESS == Access::INPUT || ACCESS == Access::IO);
      static constexpr bool OUTPUT_OK = (ACCESS == Access::OUTPUT || ACCESS == Access::IO);
      T & GetStream() { return *ptr; }

      std::istream & GetInputStream() override {
        if constexpr (!INPUT_OK) {
          emp_error("No input stream!");
          return GetDefaultStream();
        }
        else return *ptr;
      }
      std::ostream & GetOutputStream() override {
        if constexpr (!OUTPUT_OK) {
          emp_error("No output stream!");
          return GetDefaultStream();
        }
        else return *ptr;
      }
      std::iostream & GetIOStream() override {
        if constexpr (ACCESS != Access::IO) {
          emp_error("No IO stream!");
          return GetDefaultStream();
        }
        else return *ptr;
      }

      void ResetStream() {
        if (ptr) ptr.Delete();

        // Build file streams.
        if constexpr (TYPE == Type::FILE) {
          if constexpr (ACCESS == Access::INPUT)       ptr = NewPtr<std::ifstream>(name);
          else if constexpr (ACCESS == Access::OUTPUT) ptr = NewPtr<std::ofstream>(name);
          else if constexpr (ACCESS == Access::IO)     ptr = NewPtr<std::fstream>(name);
        }

        // Build string streams.
        else if constexpr (TYPE == Type::STRING)       ptr = NewPtr<std::stringstream>();

        // Otherwise use std::cin or std::cout
        else {
          owned = false;  // Use a pre-existing stream.
          if constexpr (ACCESS == Access::INPUT)       ptr = &std::cin;
          else if constexpr (ACCESS == Access::OUTPUT) ptr = &std::cout;
          else if constexpr (ACCESS == Access::IO) {
            emp_error("Disallowed stream type...");
            ptr = nullptr;
          }
        }
      }
    };

    // A default class for when we do not have a live stream.
    struct StreamInfo_None : public StreamInfo {
      StreamInfo_None() : StreamInfo("", false) { }

      Type GetType() const override { return Type::NONE; }
      Access GetAccess() const override { return Access::NONE; }
      std::istream & GetInputStream() override { emp_error("No input stream!"); return GetDefaultStream(); }
      std::ostream & GetOutputStream() override { emp_error("No output stream!"); return GetDefaultStream(); }
      std::iostream & GetIOStream() override { emp_error("No IO stream!"); return GetDefaultStream(); }
    };

    // Track a map of all possible streams.
    std::unordered_map<std::string, emp::Ptr<StreamInfo>> streams;

    Type default_input_type = Type::OTHER;  // Use std::cin for default input.
    Type default_output_type = Type::OTHER; // Use std::cout for default output.
    Type default_io_type = Type::STRING;    // Use std::stringstream for default IO.

    // === Helper functions ===

    // Return the correct StreamInfo, or none if it doesn't exist.
    StreamInfo_None info_none;
    StreamInfo & GetInfo(const std::string & name) {
      if (Has(name)) return *(streams[name]);
      return info_none;
    }
    const StreamInfo & GetInfo(const std::string & name) const {
      auto it = streams.find(name);
      if (it == streams.end()) return info_none;
      return *(it->second);
    }

  public:
    StreamManager() { }
    StreamManager(const StreamManager &) = delete;
    // StreamManager(StreamManager &&) = default;
    ~StreamManager() {
      for (auto & [name, info_ptr] : streams) info_ptr.Delete();
    }

    bool Has(const std::string & name) const { return emp::Has(streams, name); }

    // Check to see if certain types of streams are being managed.
    bool HasInputFileStream(const std::string & name) const { return GetInfo(name).IsInputFile(); }
    bool HasOutputFileStream(const std::string & name) const { return GetInfo(name).IsOutputFile(); }
    bool HasIOFileStream(const std::string & name) const { return GetInfo(name).IsIOFile(); }
    bool HasStringStream(const std::string & name) const { return GetInfo(name).IsString(); }
    bool HasStdInputStream(const std::string & name) const { return GetInfo(name).IsOtherInput(); }
    bool HasStdOutputStream(const std::string & name) const { return GetInfo(name).IsOtherOutput(); }
    bool HasStdIOStream(const std::string & name) const { return GetInfo(name).IsOtherIO(); }

    bool HasInputOnlyStream(const std::string & name) const { return GetInfo(name).IsInput(); }
    bool HasOutputOnlyStream(const std::string & name) const { return GetInfo(name).IsOutput(); }
    bool HasIOStream(const std::string & name) const { return GetInfo(name).IsIO(); }

    bool HasInputStream(const std::string & name) const { return GetInfo(name).InputOK(); }
    bool HasOutputStream(const std::string & name) const { return GetInfo(name).OutputOK(); }


    // Build a new stream of specific types.
    template <typename T, Type TYPE, Access ACCESS>
    T & AddStream(const std::string & name) {
      emp_assert(!Has(name), name);                          // Must be a new name.
      using info_t = TypedStreamInfo<T, TYPE, ACCESS>;       // Setup type for info about stream
      emp::Ptr<info_t> info_ptr = emp::NewPtr<info_t>(name); // Build the info (& stream iteslf)
      streams[name] = info_ptr;                              // Store the info for the future
      return info_ptr->GetStream();                          // Return just the stream.
    }

    std::ifstream & AddInputFile(std::string name) {
      return AddStream<std::ifstream, Type::FILE, Access::INPUT>(name);
    }
    std::ofstream & AddOutputFile(std::string name) {
      return AddStream<std::ofstream, Type::FILE, Access::OUTPUT>(name);
    }
    std::fstream & AddFile(std::string name) {
      return AddStream<std::fstream, Type::FILE, Access::IO>(name);
    }

    std::stringstream & AddStringStream(std::string name) {
      return AddStream<std::stringstream, Type::STRING, Access::IO>(name);
    }

    /// Add a stream maintained outside of manager (do not delete without removing first!)
    template <typename T>
    T & AddStream(const std::string & name, T & in_stream) {
      constexpr bool input_ok = std::is_convertible<T,std::istream>();
      constexpr bool output_ok = std::is_convertible<T,std::ostream>();
      constexpr int ACCESS_IN = (int) (input_ok ? Access::INPUT : Access::NONE);
      constexpr int ACCESS_OUT = (int) (output_ok ? Access::OUTPUT : Access::NONE);
      constexpr Access ACCESS = (Access) (ACCESS_IN + ACCESS_OUT);

      emp_assert(!Has(name), name);                            // Must be a new name.
      using info_t = TypedStreamInfo<T, Type::OTHER, ACCESS>;  // Setup type for info about stream
      auto info_ptr = emp::NewPtr<info_t>(name, &in_stream);   // Build the info (& stream iteslf)
      streams[name] = info_ptr;                                // Store the info for the future
      return info_ptr->GetStream();                            // Return just the stream.
    }

    /// Build a default input stream.
    std::istream & AddInputStream(const std::string & name) {
      emp_assert(!Has(name));
      switch (default_input_type) {
      case Type::FILE: return AddInputFile(name);
      case Type::STRING: return AddStringStream(name);
      case Type::OTHER: return AddStream(name, std::cin);
      default:
        emp_error("Default input streams not allowed!", name);
        return GetDefaultStream();
      };
      return GetDefaultStream();
    }

    /// Build a default output stream.
    std::ostream & AddOutputStream(const std::string & name) {
      emp_assert(!Has(name));
      switch (default_output_type) {
      case Type::FILE: return AddOutputFile(name);
      case Type::STRING: return AddStringStream(name);
      case Type::OTHER: return AddStream(name, std::cout);
      default:
        emp_error("Default output streams not allowed!", name);
        return GetDefaultStream();
      };
    }

    /// Build a default IO stream.
    std::iostream & AddIOStream(const std::string & name) {
      emp_assert(!Has(name));
      switch (default_io_type) {
      case Type::FILE: return AddFile(name);
      case Type::STRING: return AddStringStream(name);
      default:
        emp_error("Default output streams not allowed!", name);
        return GetDefaultStream();
      };
    }

    void SetInputDefaultFile() { default_input_type = Type::FILE; }
    void SetOutputDefaultFile() { default_output_type = Type::FILE; }
    void SetIODefaultFile() { default_io_type = Type::FILE; }

    void SetInputDefaultString() { default_input_type = Type::STRING; }
    void SetOutputDefaultString() { default_output_type = Type::STRING; }
    void SetIODefaultString() { default_io_type = Type::STRING; }

    void SetInputDefaultStandard() { default_input_type = Type::OTHER; }
    void SetOutputDefaultStandard() { default_output_type = Type::OTHER; }


    std::istream & GetInputStream(const std::string & name) {
      if (!HasInputStream(name)) return AddInputStream(name);
      return streams[name]->GetInputStream();
    }

    std::ostream & GetOutputStream(const std::string & name) {
      if (!HasOutputStream(name)) return AddOutputStream(name);
      return streams[name]->GetOutputStream();
    }

    std::iostream & GetIOStream(const std::string & name) {
      if (!HasIOStream(name)) return AddIOStream(name);
      return streams[name]->GetIOStream();
    }


    std::ostream & get_ostream(const std::string & name="cout", const std::string & stdout_name="cout") {
      if (name == stdout_name && !Has(name)) AddStream(name, std::cout);
      return GetOutputStream(name);
    }

  };

}

#endif // #ifndef EMP_IO_STREAMMANAGER_HPP_INCLUDE
