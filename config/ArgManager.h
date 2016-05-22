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

    class ArgManager {
    private:
      std::vector<std::string> args;
      std::vector<std::string> arg_names;
      std::vector<std::string> arg_descs;

    public:
      ArgManager() { ; }
      ArgManager(int argc, char* argv[]) : args(args_to_strings(argc, argv)) { ; }
      ~ArgManager() { ; }

      int size() { return (int) args.size(); }
      auto begin() -> decltype(args.begin()) { return args.begin(); }
      auto end() -> decltype(args.end()) { return args.end(); }
      std::string & operator[](int i) { return args[i]; }
      const std::string & operator[](int i) const { return args[i]; }

      // UseArg takes a name, a variable and an optional description.  If the name exists,
      // it uses the next argument to change the value of the variable.
      // Return 1 if found, 0 if not found, and -1 if error (no value provided)
      template <typename T>
      int UseArg(const std::string & name, T & var, const std::string & desc="") {
        arg_names.push_back(name);
        arg_descs.push_back(desc);
        return use_arg_value(args, name, var);
      }

      // UseArg can also take a config object and a name, and use the argument to set the
      // config object.
      int UseArg(const std::string & name, Config & config, const std::string & cfg_name,
                 const std::string & desc="") {
        arg_names.push_back(name);
        arg_descs.push_back(desc);
        std::string var;
        bool rv = use_arg_value(args, name, var);
        if (rv==1) config.Set(cfg_name, var);
        return rv;
      }

      // UseFlag take a name and an optional description.  If the name exists, return true,
      // otherwise return false.
      bool UseFlag(const std::string & name, const std::string & desc="") {
        arg_names.push_back(name);
        arg_descs.push_back(desc);
        return use_flag(args, name);
      }

      void PrintHelp(std::ostream & os) {
        int max_name_size = 0;
        for (const auto & name : arg_names) {
          if (max_name_size < (int) name.size()) max_name_size = (int) name.size();
        }
        for (int i = 0; i < arg_names.size(); i++) {
          os << arg_names[i]
             << std::string(max_name_size + 1 - (int) arg_names[i].size(), ' ')
             << arg_descs[i]
             << std::endl;
        }
      }
    };


  }
}

#endif
