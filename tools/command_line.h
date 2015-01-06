#ifndef EMP_COMMAND_LINE_H
#define EMP_COMMAND_LINE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This file contains tools for dealing with command-line arguments (argv and argc).
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

  };
};

#endif
