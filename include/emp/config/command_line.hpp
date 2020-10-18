//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file contains tools for dealing with command-line arguments (argv and argc).
//
//  Functions here include:
//
//    emp::vector<std::string> args_to_strings(int argc, char* argv[]);
//        - Convert the standard command-line args to a more managable vector of strings.
//
//    int find_arg(const emp::vector<std::string> & args, const std::string & pattern);
//        - Return index where a specified argument can be found (or -1 if it doesn't exist).
//
//    bool has_arg(const emp::vector<std::string> & args, const std::string & pattern);
//        - Return true if a particular argument was set on the command line
//
//    bool use_arg(emp::vector<std::string> & args, const std::string & pattern);
//        - Same as has_arg(), but remove the argument from the set of available args.
//
//
//    Development notes:
//    * Add has_flag() and use_flag() functions to more gracefully handle flags.
//      For example, if -a and -b are legal flags, -ab should trigger both of them.
//
//    * Process arguments from left to right, rather than out of order?
//
//    * Identify errors if arguments to a flag begin with a '-' and should be a
//      flag themselves?  Or, more generally, recognize if an argument is clearly
//      the wrong type (e.g., a string where an int was expected)?

#ifndef EMP_COMMAND_LINE_H
#define EMP_COMMAND_LINE_H

#include <string>

#include "../base/vector.hpp"
#include "../tools/string_utils.hpp"

namespace emp {
  namespace cl {

    // Convert input arguments to a vector of strings for easier processing.
    emp::vector<std::string> args_to_strings(int argc, char* argv[]) {
      emp::vector<std::string> args;
      for (size_t i = 0; i < (size_t) argc; i++) {
        args.push_back(argv[i]);
      }
      return args;
    }

    // Search through args to find a specific value.
    int find_arg(const emp::vector<std::string> & args, const std::string & pattern) {
      for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == pattern) return (int) i;
      }
      return -1;
    }

    // Return true/false if a specific argument is present.
    bool has_arg(const emp::vector<std::string> & args, const std::string & pattern) {
      return (find_arg(args, pattern) != -1);
    }

    // Return true/false if a specific argument is present and REMOVE IT.
    bool use_arg(emp::vector<std::string> & args, const std::string & pattern) {
      const int pos = find_arg(args, pattern);
      if (pos >= 0) args.erase(args.begin()+pos);
      return (pos != -1);
    }

    // Load the value from an argument with the provided pattern into the provided variable.
    template <typename T>
    int get_arg_value(emp::vector<std::string> & args, const std::string & pattern, T & var) {
      const int pos = find_arg(args, pattern);        // Find the pattern in the set of arguments!
      if (pos == -1) return 0;                        // Arg not found; abort, return 0 (not found)
      if (pos >= (int) args.size() - 1) return -1;    // No value!  Abort, return -1 (error)
      var = emp::from_string<T>(args[(size_t)pos+1]); // Store the found value.
      return 1;                                       // Indicate success!
    }

    // ...assume arg is a PAIR of strings.
    int get_arg_value(emp::vector<std::string> & args, const std::string & pattern,
                      std::string & var1, std::string & var2) {
      const int pos = find_arg(args, pattern);
      if (pos == -1) return 0;                      // Arg not found.
      if (pos >= (int) args.size() - 2) return -1;  // No room for both values!
      var1 = args[(size_t)pos+1];
      var2 = args[(size_t)pos+2];
      return 1;
    }


    // Same as get arg_value, but ALSO remove the args.
    template <typename... Ts>
    int use_arg_value(emp::vector<std::string> & args, const std::string & pattern, Ts &... vars) {
      const int result = get_arg_value(args, pattern, vars...);
      const int pos = find_arg(args, pattern);
      if (result == 1) args.erase(args.begin()+pos, args.begin()+pos+sizeof...(Ts)+1);
      return result;
    }


  }
}

#endif
