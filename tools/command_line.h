//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file contains tools for dealing with command-line arguments (argv and argc).
//
//  Functions here include:
//
//    std::vector<std::string> args_to_strings(int argc, char* argv[]);
//        - Convert the standard command-line args to a more managable vector of strings.
//
//    int find_arg(const std::vector<std::string> & args, const std::string & pattern);
//        - Return index where a specified argument can be found (or -1 if it doesn't exist).
//
//    bool has_arg(const std::vector<std::string> & args, const std::string & pattern);
//        - Return true if a particular argument was set on the command line
//
//    bool use_arg(std::vector<std::string> & args, const std::string & pattern);
//        - Same as has_arg(), but remove the argument for the set of available args.
//
//
//    Development notes:
//    * Add has_flag() and use_flag() functions to more gracefully handle flags.
//      For example, if -a and -b are legal flags, -ab should trigger both of them.
//
//    * Add find_arg() to return the position of an argument (and -1 if not found?)
//
//    * std::string get_arg_value(std::vector<std::string> & args,
//                                const std::string & pattern,
//                                std::string default_val="")
//       - Same as has_arg(), but returns the NEXT string as the value of the argument.


#ifndef EMP_COMMAND_LINE_H
#define EMP_COMMAND_LINE_H

#include <string>
#include <vector>

namespace emp {
  namespace cl {

    // Convert input arguments to a vector of strings for easier processing.
    std::vector<std::string> args_to_strings(int argc, char* argv[]) {
      std::vector<std::string> args;
      for (int i = 0; i < argc; i++) {
        args.push_back(argv[i]);
      }
      return args;
    }

    // Search through args to find a specific value.
    int find_arg(const std::vector<std::string> & args, const std::string & pattern) {
      for (int i = 0; i < (int) args.size(); i++) {
        if (args[i] == pattern) return i;
      }
      return -1;
    }

    // Return true/false if a specific argument is present.
    bool has_arg(const std::vector<std::string> & args, const std::string & pattern) {
      return (find_arg(args, pattern) != -1);
    }

    // Return true/false if a specific argument is present and REMOVE IT.
    bool use_arg(std::vector<std::string> & args, const std::string & pattern) {
      const int pos = find_arg(args, pattern);
      if (pos >= 0) args.erase(args.begin()+pos);
      return (pos != -1);
    }


  }
}

#endif
