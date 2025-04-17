/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2024
*/
/**
 *  @file
 *  @brief Load a series of filenames and clean up each file.
 */

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>

#include "../../../include/emp/base/assert.hpp"
#include "../../../include/emp/base/vector.hpp"
#include "../../../include/emp/config/command_line.hpp"
#include "../../../include/emp/io/File.hpp"
#include "../../../include/emp/tools/String.hpp"

class FileFormatter {
private:
  emp::vector<emp::String> args;  ///< Command-line arguments for formatter.

  // Which formatting fixed should we do?
  bool fix_headings = true;       // Should we update the headers?
  bool fix_indentation = true;    // Should we check indentation and change tabs to spaces?
  bool fix_include_order = true;  // Should we group includes and alphabetize them?
  bool fix_end_whitespace = true; // Should whitespace at end be just a single newline?
  bool fix_include_guards = true; // Should we ensure proper include guards?

  // Other flags
  bool verbose = false;

public:
  FileFormatter(const emp::vector<emp::String> & in_args) : args(in_args) {
    // All options on by default; use flags to disable some.
    fix_headings = !emp::cl::UseArg(args, "-H");
    fix_indentation = !emp::cl::UseArg(args, "-I");
    fix_include_order = !emp::cl::UseArg(args, "-O");
    fix_end_whitespace = !emp::cl::UseArg(args, "-W");
    fix_include_guards = !emp::cl::UseArg(args, "-G");
    
    verbose = emp::cl::UseArg(args, "-v");         // Should we print verbose output?

    if (args.size() < 2) {
      std::cerr << "Must provide at least one filename.\n"
        << "Format: " << args[0] << " [options] filename [filnames...]\n"
        << "Options:\n"
        << "  -H disable fixing file HEADINGS\n"
        << "  -T disable replacing TABS\n"
        << "  -v Use verbose output.\n"
        << std::endl;
      exit(1);
    }
  }

  bool ProcessFiles() {
    for (size_t arg_id=1; arg_id < args.size(); ++arg_id) {
      emp::File file(args[arg_id]);

      bool modified = false;
      if (fix_headings) modified |= Process_FixHeadings(file);
      if (fix_indentation) modified |= Process_Indentation(file);
      if (fix_include_order) modified |= Process_IncludeOrder(file);
      if (fix_end_whitespace) modified |= Process_EndWhitespace(file);
      if (fix_include_guards) modified |= Process_IncludeGuards(file);

      return modified;
    }
  }

  bool Process_FixHeadings(emp::File & file) {
  }

  bool Process_Indentation(emp::File & file) {    
  }

  bool Process_IncludeOrder(emp::File & file) {    
  }

  bool Process_EndWhitespace(emp::File & file) {    
  }

  bool Process_IncludeGuards(emp::File & file) {    
  }
};

int main(int argc, char * argv[])
{
  emp::vector<emp::String> args = emp::cl::ArgsToStrings(argc, argv);
  FileFormatter formatter(args);
  formatter.ProcessFiles();
}
