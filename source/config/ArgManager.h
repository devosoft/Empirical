//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A simple ArgManager tool for sythesizing command-line arguments and config files.


#ifndef EMP_CL_ARG_MANAGER_H
#define EMP_CL_ARG_MANAGER_H

#include <string>
#include <vector>

#include "command_line.h"
#include "config.h"

namespace emp {
  namespace cl {

    /// A simple class to manage command-line arguments that were passed in.
    class ArgManager {
    private:
      emp::vector<std::string> args;
      emp::vector<std::string> arg_names;
      emp::vector<std::string> arg_descs;

    public:
      ArgManager() : args(), arg_names(), arg_descs() { ; }
      ArgManager(int argc, char* argv[])
       : args(args_to_strings(argc, argv)), arg_names(), arg_descs() { ; }
      ~ArgManager() { ; }

      size_t size() const { return args.size(); }
      auto begin() -> decltype(args.begin()) { return args.begin(); }
      auto end() -> decltype(args.end()) { return args.end(); }
      std::string & operator[](size_t i) { return args[i]; }
      const std::string & operator[](size_t i) const { return args[i]; }

      /// UseArg takes a name, a variable and an optional description.  If the name exists,
      /// it uses the next argument to change the value of the variable.
      /// Return 1 if found, 0 if not found, and -1 if error (no value provided)
      template <typename T>
      int UseArg(const std::string & name, T & var, const std::string & desc="") {
        arg_names.push_back(name);
        arg_descs.push_back(desc);
        return use_arg_value(args, name, var);
      }

      /// UseArg can also take a config object and a name, and use the argument to set the
      /// config object.
      int UseArg(const std::string & name, Config & config, const std::string & cfg_name,
                 const std::string & desc="") {
        arg_names.push_back(name);
        arg_descs.push_back(desc);
        std::string var;
        bool rv = use_arg_value(args, name, var);
        if (rv==1) config.Set(cfg_name, var);
        return rv;
      }

      /// UseFlag takes a name and an optional description.  If the name exists, return true,
      /// otherwise return false.
      bool UseFlag(const std::string & name, const std::string & desc="") {
        arg_names.push_back(name);
        arg_descs.push_back(desc);
        return use_arg(args, name);
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
        if (args.size() > 1) {
          os << "Unknown args:";
          for (size_t i = 1; i < args.size(); i++) os << " " << args[i];
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
				const std::string & macro_file="") {
        for (auto e : config) {
          auto entry = e.second;
          std::string desc = emp::to_string( entry->GetDescription(),
                " (type=", entry->GetType(), "; default=", entry->GetDefault(), ')' );
          UseArg(to_string('-', entry->GetName()), config, entry->GetName(), desc);
        }

        bool print_help    = UseFlag("--help", "Print help information.");
        bool create_config = cfg_file.size() && UseFlag("--gen", "Generate configuration file.");
        bool const_macros  = macro_file.size() && UseFlag("--const", "Generate const version of macros file.");

        if (print_help)    { PrintHelp(os); return false; }
        if (create_config) { config.Write(cfg_file); return false; }
        if (const_macros)  { config.WriteMacros(macro_file, true); return false; }

        return true;
      }

    };


  }
}

#endif
