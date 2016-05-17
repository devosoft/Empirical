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
    bool use_flag(std::vector<std::string> & args, const std::string & pattern) {
      const int pos = find_arg(args, pattern);
      if (pos >= 0) args.erase(args.begin()+pos);
      return (pos != -1);
    }

    // Return 1 for arg set to value, 0 for arg not found, and -1 for invalid use.
    // ...assume arg is a single string.
    int get_arg_value(std::vector<std::string> & args, const std::string & pattern, std::string & var) {
      const int pos = find_arg(args, pattern);
      if (pos == -1) return 0;                      // Arg not found.
      if (pos >= (int) args.size() - 1) return -1;  // No room for a value!
      var = args[pos+1];
      return 1;
    }

    // ...assume arg is a PAIR of strings.
    int get_arg_value(std::vector<std::string> & args, const std::string & pattern,
                      std::string & var1, std::string & var2) {
      const int pos = find_arg(args, pattern);
      if (pos == -1) return 0;                      // Arg not found.
      if (pos >= (int) args.size() - 2) return -1;  // No room for both values!
      var1 = args[pos+1];
      var2 = args[pos+2];
      return 1;
    }

    // ...assume arg is a single int.
    int get_arg_value(std::vector<std::string> & args, const std::string & pattern, int & var) {
      const int pos = find_arg(args, pattern);
      if (pos == -1) return 0;                      // Arg not found.
      if (pos >= (int) args.size() - 1) return -1;  // No room for a value!
      var = stoi(args[pos+1]);
      return 1;
    }

    // ...assume arg is a single double.
    int get_arg_value(std::vector<std::string> & args, const std::string & pattern, double & var) {
      const int pos = find_arg(args, pattern);
      if (pos == -1) return 0;                      // Arg not found.
      if (pos >= (int) args.size() - 1) return -1;  // No room for a value!
      var = stod(args[pos+1]);
      return 1;
    }


    // Same as get arg_value, but ALSO remove the args.
    template <typename... Ts>
    int use_arg_value(std::vector<std::string> & args, const std::string & pattern, Ts &... vars) {
      const int result = get_arg_value(args, pattern, vars...);
      const int pos = find_arg(args, pattern);
      if (result == 1) args.erase(args.begin()+pos, args.begin()+pos+sizeof...(Ts)+1);
      return result;
    }

    class ArgManager {
    private:
      std::vector<std::string> args;
      std::vector<std::string> arg_names;
      std::vector<std::string> arg_descs;

    public:
      ArgManager() { ; }
      ArgManager(int argc, char* argv[]) : args(args_to_strings(argc, argv)) { ; }
      ~ArgManager() { ; }

      int GetArgCount() { return (int) args.size(); }

      // UseArg takes a name, a variable and an optional description.  If the name exists,
      // it uses the next argument to change the value of the variable.
      // Return 1 if found, 0 if not found, and -1 if error (no value provided)
      template <typename T>
      int UseArg(const std::string & name, T & var, const std::string & desc="") {
        arg_names.push_back(name);
        arg_descs.push_back(desc);
        return use_arg_value(args, name, var);
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
