// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is 
// Copyright (C) Michigan State University, 2015. It is licensed 
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_COMMAND_LINE_H
#define EMP_COMMAND_LINE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This file contains tools for dealing with command-line arguments (argv and argc).
//
//  Functions here include:
//
//    std::vector<std::string> args_to_strings(int argc, char* argv[]);
//        - Convert the standard command-line args to a more managable vector of strings.
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
//    * Add get_arg_value() to find an argument, and return the next arg as its value.
//      Should default to string, but can have variants (or templated?) to allow other values
//


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
    
    
    // Return true/false if a specific argument is present.
    bool has_arg(const std::vector<std::string> & args, const std::string & pattern) {
      for (int i = 0; i < (int) args.size(); i++) {
        if (args[i] == pattern) return true;
      }
      return false;
    }
    
    // Return true/false if a specific argument is present and REMOVE IT.
    bool use_arg(std::vector<std::string> & args, const std::string & pattern) {
      for (int i = 0; i < (int) args.size(); i++) {
        if (args[i] == pattern) {
          args.erase(args.begin()+i);
          return true;
        }
      }
      return false;
    }

  }
}

#endif
