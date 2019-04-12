//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A simple ArgManager tool for sythesizing command-line arguments and config files.


#ifndef EMP_CL_ARG_MANAGER_H
#define EMP_CL_ARG_MANAGER_H

#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <iterator>

#include "base/Ptr.h"
#include "base/vector.h"
#include "command_line.h"
#include "config.h"

namespace emp {

  /// A simple class to manage command-line arguments that were passed in.

  class ArgManager {

  private:
    std::multimap<std::string, emp::vector<std::string>> packs;
    std::multimap<std::string, size_t> counts;
    std::multimap<std::string, std::string> descs;

  public:
    // Convert input arguments to a vector of strings for easier processing.
    static emp::vector<std::string> args_to_strings(int argc, char* argv[]) {
      emp::vector<std::string> args;
      for (size_t i = 0; i < (size_t) argc; i++) {
        args.push_back(argv[i]);
      }
      return args;
    }

    // Make counts specification for builtin commands
    static std::multimap<std::string, size_t> MakeBuiltinCounts() {
      return {
        {"help", 0},
        {"gen", 1},
        {"make-const", 1}
      };
    }

    // Make desc specification for builtin commands
    static std::multimap<std::string, std::string> MakeBuiltinDescs() {
      return {
        {"help", "Print help information."},
        {"gen", "Generate configuration file."},
        {"make-const", "Generate const version of macros file."}
      };
    }

    ArgManager(
      int argc,
      char* argv[],
      std::multimap<std::string, size_t> counts = std::multimap<std::string, size_t>(),
      std::multimap<std::string, std::string> descs = std::multimap<std::string, std::string>()
    ) : ArgManager(ArgManager::args_to_strings(argc, argv), counts, descs) { ; }

    ArgManager(
      const emp::vector<std::string> args,
      const std::multimap<std::string, size_t> counts_ = std::multimap<std::string, size_t>(),
      const std::multimap<std::string, std::string> descs_  = std::multimap<std::string, std::string>()
    ) : counts(counts_), descs(descs_) {

      emp::vector<std::string> deflagged = args;
      for(auto & val : deflagged) {
        val.erase(0, val.find_first_not_of('-'));
      }

      for(size_t i = 0; i < args.size(); ++i) {

        if( !counts.count(deflagged[i]) ) {

          size_t j = i;
          while( j < args.size() && !counts.count(deflagged[j]) ) ++j;

          packs.insert(
            {
              "_positional",
              emp::vector<std::string>(
                std::next(std::begin(args), i),
                std::next(std::begin(args), j)
              )
            }
          );

          i = j-1;

        } else {

          const auto r_proc = packs.equal_range(deflagged[i]);
          const size_t n_proc = std::distance(r_proc.first, r_proc.second);

          const auto r_spec = counts.equal_range(deflagged[i]);
          const size_t n_spec = std::distance(r_spec.first, r_spec.second);

          size_t n = n_spec ?
            std::next(r_spec.first, n_proc % n_spec)->second : 0;

          if (i+n+1 <= args.size()) packs.insert(
              {
                deflagged[i],
                emp::vector<std::string>(
                  std::next(std::begin(args), i+1),
                  std::next(std::begin(args), i+n+1)
                )
              }
            );

          i += n;

        }

      }
    }

    ArgManager(
      std::multimap<std::string, emp::vector<std::string>> packs_,
      const std::multimap<std::string, size_t> counts_ = std::multimap<std::string, size_t>(),
      const std::multimap<std::string, std::string> descs_  = std::multimap<std::string, std::string>()
    ) : packs(packs_), counts(counts_) { ; }

    ~ArgManager() { ; }

    /// UseArg consumes an argument pack accessed by a certain name.
    std::optional<emp::vector<std::string>> UseArg(
      const std::string & name,
      std::optional<size_t> req_size=std::nullopt
    ) {
      auto res = packs.count(name) && (
          !req_size || *req_size == packs.lower_bound(name)->second.size()
        ) ? std::make_optional(packs.lower_bound(name)->second)
        : std::nullopt;
      if (res) packs.erase(packs.lower_bound(name));

      return res;
    }

    /// ViewArg returns all argument packs under a certain name.
    emp::vector<emp::vector<std::string>> ViewArg(const std::string & name) {

      emp::vector<emp::vector<std::string>> res;

      const auto range = packs.equal_range(name);
      for (auto it = range.first; it != range.second; ++it) {
        res.push_back(it->second);
      }

      return res;

    }

    // Process builtin commands.
    /// Return bool for "should program proceed" (i.e., true=continue, false=exit).
    bool ProcessBuiltin(
      const emp::Ptr<const Config> config=nullptr,
      std::ostream & os=std::cout
    ) {

      if (UseArg("help")) {
        PrintHelp(os);
        return false;
      }

      bool proceed = true;

      if (const auto res = UseArg("gen", 1); res && config) {
        const std::string cfg_file = res->front();
        os << "Generating new config file: " << cfg_file << std::endl;
        config->Write(cfg_file);
        proceed = false;
      }

      if (const auto res = UseArg("make-const", 1); res && config)  {
        const std::string macro_file = res->front();
        os << "Generating new macros file: " << macro_file << std::endl;
        config->WriteMacros(macro_file, true);
        proceed = false;
      }

      return proceed;

    }

    /// Print the current state of the ArgManager.
    void Print(std::ostream & os=std::cout) const {

      for(const auto it : packs ) {
        os << it.first << ":";
        for(const auto v : it.second ) {
          os << " " << v;
        }
        os << std::endl;
      }

    }

    /// Print information about all known argument types and what they're for; make pretty.
    void PrintHelp(std::ostream & os=std::cout) const {

      for( auto name_it = descs.begin();
           name_it !=  descs.end();
           name_it = descs.upper_bound(name_it->first)
      ) {
        auto d_range = descs.equal_range(name_it->first);
        auto c_range = counts.equal_range(name_it->first);

        auto d_it = d_range.first;
        auto c_it = c_range.first;

        while (d_it != d_range.second && c_it != c_range.second) {

          if(d_it == d_range.second) d_it = d_range.first;
          if(c_it == c_range.second) c_it = c_range.first;

          os << "-"
             << name_it->first
             << ( c_range.first != c_range.second
                  ? emp::to_string(" [", c_it->second, "]") : ""
                )
             << std::endl
             << "   | "
             << (d_range.first != d_range.second ? d_it->second : "")
             << std::endl;

          if (d_range.first != d_range.second) ++d_it;
          if (c_range.first != c_range.second) ++c_it;

        }
      }
    }

    /// Test if there are any unused arguments, and if so, output an error.
    bool HasUnused(std::ostream & os=std::cerr) const {
      if (packs.size() > 1) {
        os << "Unused arg packs:" << std::endl;
        for(const auto & p : packs) {
          os << " " << p.first;
          for(const auto & v : p.second) {
            os << " " << v;
          }
          os << std::endl;
        }
        PrintHelp(os);
        return true;
      }
      return false;
    }

    /// Convert settings from a configure object to command-line arguments.
    /// Return bool for "should program proceed" (i.e., true=continue, false=exit).
    void ApplyConfigOptions(Config & config) {

      // Scan through the config object to generate command line flags for each setting.
      for (auto e : config) {

        const auto entry = e.second;

        const auto res = UseArg(entry->GetName(), std::make_optional(1));

        if (res) config.Set(entry->GetName(), (*res)[0]);

      }

    }

  };

  namespace cl {

    /// A simple class to manage command-line arguments that were passed in.
    /// Derived from emp::vector<std::string>, but with added functionality for argument handling.
    class ArgManager : public emp::vector<std::string> {
    private:
      using parent_t = emp::vector<std::string>;
      emp::vector<std::string> arg_names;
      emp::vector<std::string> arg_descs;

    public:
      ArgManager() : parent_t(), arg_names(), arg_descs() { ; }
      ArgManager(int argc, char* argv[])
       : parent_t(args_to_strings(argc, argv)), arg_names(), arg_descs() { ; }
      ~ArgManager() { ; }

      /// UseArg takes a name, a variable and an optional description.  If the name exists,
      /// it uses the next argument to change the value of the variable.
      /// Return 1 if found, 0 if not found, and -1 if error (no value provided)
      template <typename T>
      int UseArg(const std::string & name, T & var, const std::string & desc="") {
        arg_names.push_back(name);
        arg_descs.push_back(desc);
        return use_arg_value(*this, name, var);
      }

      /// UseArg can also take a config object and a name, and use the argument to set the
      /// config object.
      int UseArg(const std::string & name, Config & config, const std::string & cfg_name,
                 const std::string & desc="") {
        arg_names.push_back(name);
        arg_descs.push_back(desc);
        std::string var;
        bool rv = use_arg_value(*this, name, var);
        if (rv==1) config.Set(cfg_name, var);
        return rv;
      }

      /// UseFlag takes a name and an optional description.  If the name exists, return true,
      /// otherwise return false.
      bool UseFlag(const std::string & name, const std::string & desc="") {
        arg_names.push_back(name);
        arg_descs.push_back(desc);
        return use_arg(*this, name);
      }

      /// Print information about all known argument types and what they're for; make pretty.
      void PrintHelp(std::ostream & os) const {
        size_t max_name_size = 0;
        for (const auto & name : arg_names) {
          if (max_name_size < name.size()) max_name_size = name.size();
        }
        for (size_t i = 0; i < arg_names.size(); i++) {
          os << arg_names[i]
             << std::string(max_name_size + 1 - arg_names[i].size(), ' ')
             << arg_descs[i]
             << std::endl;
        }
      }

      /// Test if there are any unprocessed arguments, and if so, output an error.
      bool HasUnknown(std::ostream & os=std::cerr) const {
        if (size() > 1) {
          os << "Unknown args:";
          for (size_t i = 1; i < size(); i++) os << " " << (*this)[i];
          os << std::endl;
          PrintHelp(os);
          return true;
        }
        return false;
      }

      /// Leaving TestUnknown for backward compatability; returns opposite of HasUnknown().
      bool TestUnknown(std::ostream & os=std::cerr) const { return !HasUnknown(os); }

      /// Convert settings from a configure object to command-line arguments.
      /// Return bool for "should program proceed" (i.e., true=continue, false=exit).
      bool ProcessConfigOptions(Config & config, std::ostream & os,
                                const std::string & cfg_file="",
                                const std::string & macro_file="")
      {
        // Scan through the config object to generate command line flags for each setting.
        for (auto e : config) {
          auto entry = e.second;
          std::string desc = emp::to_string( entry->GetDescription(),
                                             " (type=", entry->GetType(),
                                             "; default=", entry->GetDefault(), ')' );
          UseArg(to_string('-', entry->GetName()), config, entry->GetName(), desc);
        }

        // Determine if we're using any special options for comman line flags.
        bool print_help    = UseFlag("--help", "Print help information.");
        bool create_config = cfg_file.size() && UseFlag("--gen", "Generate configuration file.");
        bool const_macros  = macro_file.size() && UseFlag("--make-const", "Generate const version of macros file.");

        if (print_help)    { PrintHelp(os); return false; }
        if (create_config) {
          os << "Generating new config file: " << cfg_file << std::endl;
          config.Write(cfg_file);
          return false;
        }
        if (const_macros)  {
          os << "Generating new macros file: " << macro_file << std::endl;
          config.WriteMacros(macro_file, true);
          return false;
        }

        return true;
      }

    };


  }
}

#endif
