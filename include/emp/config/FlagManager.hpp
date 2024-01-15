/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023-24.
 *
 *  @file FlagManager.hpp
 *  @brief This file contains tools for dealing with command-line flags (from argv and argc).
 *  @note Status: ALPHA
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

#ifndef EMP_CONFIG_FLAGMANAGER_HPP_INCLUDE
#define EMP_CONFIG_FLAGMANAGER_HPP_INCLUDE

#include <functional>
#include <iostream>
#include <map>
#include <string>

#include "../base/vector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../tools/String.hpp"

namespace emp {

  class FlagInfo {
  public:
    using fun_t = std::function<void(const emp::vector<String> &)>;

  private:
    String desc;          ///< Name to type to trigger option.  E.g. "--help"
    size_t min_args = 0;  ///< Minumum number of arguments option needs to operate.
    size_t max_args = 0;  ///< Maximum number of arguments option can handle.
    fun_t fun;            ///< Function to run when this option is selected.
    char shortcut = '\0'; ///< Single-letter shortcut for this option.  E.g., 'h' is for "-h"

    String group="none";  ///< Which option group does this belong to?

  public:
    FlagInfo() { }
    FlagInfo(String desc, size_t min_args, size_t max_args,
             fun_t fun, char shortcut='\0')
      : desc(desc), min_args(min_args), max_args(max_args), fun(fun), shortcut(shortcut)
    { }

    FlagInfo & operator=(const FlagInfo &) = default;

    const String & GetDesc() const { return desc; }
    size_t GetMinArgs() const { return min_args; };
    size_t GetMaxArgs() const { return max_args; };
    fun_t GetFun() const { return fun; };
    char GetShortcut() const { return shortcut; };
    const String & GetGroup() const { return group; }

    FlagInfo SetDesc(const String & in) { desc = in; return *this; }
    FlagInfo SetMinArgs(size_t in) { min_args = in; return *this; };
    FlagInfo SetMaxArgs(size_t in) { max_args = in; return *this; };
    FlagInfo SetFun(fun_t in) { fun = in; return *this; };
    FlagInfo & SetShortcut(char in) { shortcut = in; return *this; }
    FlagInfo & SetGroup(String in) { group = in; return *this; }

    template <typename... Ts>
    void Run(Ts &&... args) { fun(std::forward<Ts>(args)...); }
  };

  class FlagManager {
  private:
    emp::vector<String> args;    ///< Command-line arguments to be processed
    emp::vector<String> extras;  ///< Set of command line arguments not handled by FlagManager

    std::map<String, FlagInfo> flag_options; ///< Set of flags known by this manager.
    std::map<char, String> shortcuts;        ///< Single-character shortcuts to particular flags.

    struct GroupInfo {
      String name;
      String desc;
    };
    emp::vector<GroupInfo> groups;   ///< Info about flag groups to organize "help".
    int cur_group = -1;              ///< Which group are we currently adding to?

    // --- Helper functions ---
    int _FindGroupID(const String & name) {
      for (size_t i = 0; i < groups.size(); ++i) {
        if (groups[i].name == name) return static_cast<int>(i);
      }
      return -1;
    }

    FlagInfo & _AddOption(String name, std::function<void(const emp::vector<String> &)> fun,
                   size_t min_args=0, size_t max_args=npos, String desc="") {
      flag_options[name] = FlagInfo{desc, min_args, max_args, fun};
      if (cur_group >= 0) flag_options[name].SetGroup(groups[cur_group].name);
      return flag_options[name];
    }
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

    /// Add a new option group.
    int AddGroup(String name, String desc="") {
      cur_group = static_cast<int>(groups.size());
      groups.emplace_back(name, desc);
      return cur_group;
    }

    /// Change the option group back to a previously created group.
    int SetGroup(String name) {
      cur_group = _FindGroupID(name);
      return cur_group;
    }

    /// Add a new option that doesn't do anything when triggered.
    FlagInfo & AddOption(String name, String desc="") {
      return _AddOption(name, [](const emp::vector<String> &){}, 0, 0, desc);
    }

    /// Add an option that doesn't take any arguments and runs a function when triggered.
    FlagInfo & AddOption(String name, std::function<void()> fun, String desc="") {
      return _AddOption(name, [fun](const emp::vector<String> &){fun();}, 0, 0, desc);
    }

    /// Add an option that takes one argument that it passes to a function when triggered.
    FlagInfo & AddOption(String name, std::function<void(String)> fun, String desc="") {
      return _AddOption(name, [fun](const emp::vector<String> & in){fun(in[0]);}, 1, 1, desc);
    }

    /// Add an option that takes two arguments that it passes to a function when triggered.
    FlagInfo & AddOption(String name, std::function<void(String,String)> fun, String desc="") {
      return _AddOption(name, [fun](const emp::vector<String> & in){fun(in[0],in[1]);}, 2, 2, desc);
    }

    /// Add an option that takes a vector of arguments that it passes to a function when triggered.
    FlagInfo & AddOption(String name, std::function<void(const emp::vector<String> &)> fun,
                         size_t min_args=0, size_t max_args=npos, String desc="") {
      return _AddOption(name, fun, min_args, max_args, desc);
    }

    // Allow an option to have a single-letter flag (e.g. "-h" is short for "--help")
    template <typename FUN_T>
    FlagInfo & AddOption(char shortcut, String name, FUN_T fun, String desc="") {
      FlagInfo & option = AddOption(name, fun, desc);
      shortcuts[shortcut] = name;
      option.SetShortcut(shortcut);
      return option;
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
      for (size_t i = 1; i <= option.GetMinArgs(); ++i) {
        flag_args.push_back(args[cur_pos+i]);
      }
      option.Run(flag_args);
      return option.GetMinArgs();
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

    size_t GroupSize(emp::String group_name) const {
      size_t size = 0;
      for (const auto & [name, options] : flag_options) {
        if (options.GetGroup() == group_name) ++size;
      }
      return size;
    }

    void PrintGroupOptions(emp::String group, std::ostream & os=std::cout) const {
      for (const auto & [name, options] : flag_options) {
        if (options.GetGroup() != group) continue;
        os << "  " << name;
        if (options.GetShortcut()) {
          os << " (or '-" << options.GetShortcut() << "')";
        }
        if (options.GetDesc().size()) {
          os << " : " << options.GetDesc();
        }
        os << "\n";
      }
    }

    void PrintOptions(std::ostream & os=std::cout) const {
      // Step through groups, printing each of them.
      for (const auto & group : groups) {
        os << "=== " << group.name << " ===\n";
        if (group.desc.size()) os << group.desc << '\n';
        PrintGroupOptions(group.name, os);
        os << '\n';
      }
      os.flush();
      
      // Print any uncategorized options
      if (GroupSize("none") == 0) return; // No uncategorized options.

      // If there are other groups, list this one as specifically uncategorized.
      if (groups.size()) os << "=== Other Options ===\n";
      PrintGroupOptions("none", os);
      os.flush();
    }
  };

}

#endif // #ifndef EMP_CONFIG_FLAGMANAGER_HPP_INCLUDE
