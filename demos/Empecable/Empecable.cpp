/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2024-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 * 
 * @file
 * @brief Load a series of filenames and clean up each file.
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
#include "../../../include/emp/config/FlagManager.hpp"

class Empecable {
private:
  // Which formatting fixed should we do?
  bool fix_front_matter = false;   // Should we update the headers?
  bool fix_indentation = false;    // Should we check indentation and change tabs to spaces?
  bool fix_include_order = false;  // Should we group includes and alphabetize them?
  bool fix_whitespace = false;     // Should whitespace at end be just a single newline?
  bool use_include_guards = false; // Should we ensure proper include guards?

  // Other flags
  bool verbose = false;

  emp::FlagManager flags;

public:
  Empecable(int argc, char * argv[]) : flags(argc, argv) {
    flags.AddGroup("Basic Operation");
    flags.AddOption('h', "help", [this](){ PrintHelp(); },
      "Get additional information about options.");
    flags.AddOption('v', "verbose", [this](){ SetVerbose(true); },
      "Provide more detailed output");
        
    flags.AddGroup("Activating Fixes");
    flags.AddOption('a', "all", [this](){ SetAll(true); },
      "Activate ALL fixes (except those explicitly excluded; see below)");
    flags.AddOption('g', "fix-guards", [this](){ SetUseGuards(true); },
      "Fix include guards to have correct names at beginning and end of file");
    flags.AddOption('f', "fix-front", [this](){ SetFixFrontMatter(true); },
      "Fix front-matter comments (copyright, description, etc.)");
    flags.AddOption('i', "fix-indents", [this](){ SetFixIndents(true); },
      "Fix indenting to be two chars, no tabs");
    flags.AddOption('o', "fix-order", [this](){ SetFixIncludes(true); },
      "Fix order of #includes to be grouped and alphabetical");
    flags.AddOption('w', "fix-whitespace", [this](){ SetFixSpaces(true); },
      "Fix whitespace (remove spaces at end of lines)");
  
    flags.AddGroup("Disabling Fixes");
    flags.AddOption('G', "no-fix-guards", [this](){ SetUseGuards(false); },
      "Disable fixing of include guards");
    flags.AddOption('F', "no-fix-front", [this](){ SetFixFrontMatter(false); },
      "Disable fixing of front matter (copyright, description, etc.)");
    flags.AddOption('I', "no-fix-indents", [this](){ SetFixIndents(false); },
      "Disable fixing of indentations (2 char, no tabs)");
    flags.AddOption('O', "no-fix-order", [this](){ SetFixIncludes(false); },
      "Disable fixing of include order (grouped and alphabetical)");
    flags.AddOption('W', "no-fix-whitespace", [this](){ SetFixSpaces(false); },
      "Disable fixing of whitespace (no spaces at end of lines)");
    
    flags.Process();
  
    auto filenames = flags.GetExtras();
    for (emp::String filename : filenames) {
      ProcessFile(filename);
    }    
  }

  Empecable & SetAll(bool _in=true) {
    fix_front_matter = _in;
    fix_indentation = _in;
    fix_include_order = _in;
    fix_whitespace = _in;
    use_include_guards = _in;
    return *this;
  }
  Empecable & SetFixFrontMatter(bool _in) { fix_front_matter = _in;       return *this; }
  Empecable & SetFixIndents(bool _in)     { fix_indentation = _in;    return *this; }
  Empecable & SetFixIncludes(bool _in)    { fix_include_order = _in;  return *this; }
  Empecable & SetFixSpaces(bool _in)      { fix_whitespace = _in;     return *this; }
  Empecable & SetUseGuards(bool _in)      { use_include_guards = _in; return *this; }

  Empecable & SetVerbose(bool _in) { verbose = _in; return *this; }

  void PrintVersion() const {
    std::cout << "File formatter version 0.1." << std::endl;
  }

  void PrintHelp() const {
    PrintVersion();
    std::cout << "Usage: " << flags[0] << " [flags] [files]\n";
    flags.PrintOptions();
    exit(0);
  }

  bool ProcessFile(emp::String filename) {
    emp::File file(filename);

    std::cout << "File: " << filename << std::endl;
    if (file.HasError()) {
      std::cout << "ERROR: " << file.GetError();
      return false;
    }

    bool modified = false;
    if (fix_front_matter) modified |= Process_FixHeadings(file);
    if (fix_indentation) modified |= Process_Indentation(file);
    if (fix_include_order) modified |= Process_IncludeOrder(file);
    if (fix_whitespace) modified |= Process_EndWhitespace(file);
    if (use_include_guards) modified |= Process_IncludeGuards(file);

    return modified;
  }

  bool Process_FixHeadings(emp::File & file) {
    std::cout << "STUB: Fixing Front Matter" << std::endl;
    (void) file;
    return false;
  }

  bool Process_Indentation(emp::File & file) {    
    std::cout << "STUB: Fixing Indentation" << std::endl;
    (void) file;
    return false;
  }

  bool Process_IncludeOrder(emp::File & file) {    
    std::cout << "STUB: Fixing Order of Includes" << std::endl;
    (void) file;
    return false;
  }

  bool Process_EndWhitespace(emp::File & file) {    
    std::cout << "STUB: Fixing Whitespace" << std::endl;
    (void) file;
    return false;
  }

  bool Process_IncludeGuards(emp::File & file) {    
    std::cout << "STUB: Fixing Guards" << std::endl;
    (void) file;
    return false;
  }
};

int main(int argc, char * argv[])
{
  class Empecable formatter(argc, argv);
}
