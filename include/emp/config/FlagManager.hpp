/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023.
 *
 *  @file FlagManager.hpp
 *  @brief This file contains tools for dealing with command-line flags (from argv and argc).
 * 
 *  The FlagManager class will take command line arguments (either in its constructor or with
 *  the AddFlags() function) and process them appropriately.
 * 
 *  For setup, the user must run AddOption with the function to call.  Functions to call can take
 *  zero, one, or two Strings as arguments OR they can take a vector of Strings and the range of
 *  allowed arguments should be specified.  When Process() is run, the appropriate function will
 *  be called on each and any invalid arguments will trigger an error.
 * 
 *  Flags are expected to begin with a '-' and non-flags are expected to NOT begin with a '-'.
 * 
 *  If a single dash is followed by multiple characters, each will be processed independently.
 *  So, "-abc" will be the same as "-a -b -c".
 * 
 *  Extra command line arguments will be saves as a vector of strings called "extras" and must
 *  be processed manually.  They can be retrieved with GetExtras().
 * 
 * 
 *  @todo: Make variable numbers of flag arguments work.
 */

#ifndef EMP_CONFIG_FLAG_MANAGER_HPP_INCLUDE
#define EMP_CONFIG_FLAG_MANAGER_HPP_INCLUDE

#include <functional>
#include <iostream>
#include <string>
#include <map>

#include "../base/vector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../tools/String.hpp"

namespace emp {

  class FlagManager {
  private:
    emp::vector<String> args;
    emp::vector<String> extras;

    struct FlagInfo {
      String desc;
      size_t min_args = 0;
      size_t max_args = 0;
      std::function<void(const emp::vector<String> &)> fun;
      char shortcut = '\0';
    };

    std::map<String, FlagInfo> flag_options;
    std::map<char, String> shortcuts;

  public:
    FlagManager() { }
    FlagManager(int argc, char* argv[]) { AddFlags(argc, argv); }

    constexpr static size_t npos = static_cast<size_t>(-1);

    [[nodiscard]] String & operator[](size_t pos) { return args[pos]; }
    [[nodiscard]] const String & operator[](size_t pos) const { return args[pos]; }

    emp::vector<String> GetExtras() const { return extras; }

    [[nodiscard]] size_t Find(String pattern) const {
      for (size_t i = 0; i < args.size(); ++i) if (args[i] == pattern) return i;
      return npos;
    }

    [[nodiscard]] bool Has(String pattern) const { return Find(pattern) != npos; }

    // Return true/false if a specific argument is present and REMOVE IT.
    bool Use(String pattern) {
      size_t pos = Find(pattern);
      if (pos == npos) return false;
      args.erase(args.begin() + pos);
      return true;
    }

    void AddOption(String name, String desc="") {
      flag_options[name] = FlagInfo{desc, 0, 0, [](const emp::vector<String> &){} };
    }
    void AddOption(String name, std::function<void()> fun, String desc="") {
      flag_options[name] = FlagInfo{desc, 0,0, [fun](const emp::vector<String> &){fun();}};
    }
    void AddOption(String name, std::function<void(String)> fun, String desc="") {
      flag_options[name] = FlagInfo{desc, 1,1, [fun](const emp::vector<String> & in){fun(in[0]);}};
    }
    void AddOption(String name, std::function<void(String,String)> fun, String desc="") {
      flag_options[name] = FlagInfo{desc, 2,2, [fun](const emp::vector<String> & in){fun(in[0],in[1]);}};
    }
    void AddOption(String name, std::function<void(const emp::vector<String> &)> fun,
                   size_t min_args=0, size_t max_args=npos, String desc="") {
      flag_options[name] = FlagInfo{desc, min_args,max_args, fun};
    }

    // Allow an option to have a single-letter flag (e.g. "-h" is short for "--help")
    template <typename FUN_T>
    void AddOption(char shortcut, String name, FUN_T fun, String desc="") {
      AddOption(name, fun, desc);
      shortcuts[shortcut] = name;
      flag_options[name].shortcut = shortcut;
    }

    void AddFlags(int argc, char* argv[]) {
      for (size_t i = 0; i < (size_t) argc; i++) {
        args.push_back(argv[i]);
      }
    }

    // Process an argument associated with a particular name; return num additional args used.
    size_t ProcessArg(String name, size_t cur_pos=0) {
      if (!emp::Has(flag_options, name)) { emp::notify::Error("Unknown flag '", name , "'."); }
      auto option = flag_options[name];
      emp::vector<String> flag_args;
      for (size_t i = 1; i <= option.min_args; ++i) {
        flag_args.push_back(args[cur_pos+i]);
      }
      option.fun(flag_args);
      return option.min_args;
    }

    // Process an argument associated with a particular character; return num additional args used.
    size_t ProcessArg(char c, size_t cur_pos=0) {
      if (!emp::Has(shortcuts, c)) { emp::notify::Error("Unknown flag '-", c , "'."); }
      return ProcessArg(shortcuts[c], cur_pos);
    }

    // Process the argument at a given position.  Return number of additional args consumed.
    size_t ProcessFlagSet(String name, size_t cur_pos=0) {
      size_t offset = 0;
      for (size_t i = 1; i < name.size(); ++i) {
        offset += ProcessArg(name[i], cur_pos+offset);
      }
      return offset;
    }

    // Process all of the flag data that we have.
    void Process() {
      for (size_t i = 1; i < args.size(); ++i) {
        String & arg = args[i];
        if (arg[0] == '-') {  // We have a flag!
          if (arg.size() > 1 && arg[1] == '-') i += ProcessArg(arg, i);
          else i += ProcessFlagSet(arg, i);
        }
        else extras.push_back(arg);
      }
    }

    void PrintOptions(std::ostream & os=std::cout) const {
      for (const auto & [name, options] : flag_options) {
        os << "  " << name;
        if (options.shortcut) {
          os << " (or '-" << options.shortcut << "')";
        }
        if (options.desc.size()) {
          os << " : " << options.desc;
        }
        os << "\n";
      }
    }
  };

}

#endif // #ifndef EMP_CONFIG_FLAG_MANAGER_HPP_INCLUDE
