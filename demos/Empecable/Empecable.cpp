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

#include "../../include/emp/base/assert.hpp"
#include "../../include/emp/base/vector.hpp"
#include "../../include/emp/config/command_line.hpp"
#include "../../include/emp/io/File.hpp"
#include "../../include/emp/tools/String.hpp"
#include "../../include/emp/config/FlagManager.hpp"

#include "Lexer.hpp"

struct Checks {
  bool copyright = true;         // Check copyright at the front.
  bool no_end_spaces = true;     // Check the no lines end with spaces.
  bool no_tabs = true;           // Check that no tabs are in code.
  bool include_guards = true;    // Check that the file is protected by include guards.
  bool pragma_once = true;       // Check that the file has a "#pragma once"
  bool include_order = true;     // Check that the includes in the file are grouped correctly.

  void SetAll(bool _in=true) {
    copyright = _in;
    no_end_spaces = _in;
    no_tabs = _in;
    include_guards = _in;
    pragma_once = _in;
    include_order = _in;
  }
};

class Empecable {
private:
  // Options used:
  Checks checks;
  bool verbose = false;

  emp::FlagManager flags;

public:
  Empecable(int argc, char * argv[]) : flags(argc, argv) {
    flags.AddGroup("Basic Operation");
    flags.AddOption('a', "all", [this](){ SetAll(true); },
      "Activate ALL fixes (except those explicitly excluded; see below)");
    flags.AddOption('h', "help", [this](){ PrintHelp(); },
      "Get additional information about options.");
    flags.AddOption('v', "verbose", [this](){ verbose = true; },
      "Provide more detailed output");
        
    flags.AddGroup("Activating Specific Fixes");
    flags.AddOption("copyright", [this]{ checks.copyright = true; });
    flags.AddOption("no_end_spaces", [this]{ checks.no_end_spaces = true; });
    flags.AddOption("no_tabs", [this]{ checks.no_tabs = true; });
    flags.AddOption("include_guards", [this]{ checks.include_guards = true; });
    flags.AddOption("pragma_once", [this]{ checks.pragma_once = true; });
    flags.AddOption("include_order", [this]{ checks.include_order = true; });
  
    flags.AddGroup("Deactivate Specific Fixes");
    flags.AddOption("no_copyright", [this]{ checks.copyright = false; });
    flags.AddOption("no_no_end_spaces", [this]{ checks.no_end_spaces = false; });
    flags.AddOption("no_no_tabs", [this]{ checks.no_tabs = false; });
    flags.AddOption("no_include_guards", [this]{ checks.include_guards = false; });
    flags.AddOption("no_pragma_once", [this]{ checks.pragma_once = false; });
    flags.AddOption("no_include_order", [this]{ checks.include_order = false; });
    
    flags.Process();
  
    auto filenames = flags.GetExtras();
    if (filenames.size() == 0) {
      std::cout << "No files listed." << std::endl;
      PrintUsage();
    }
    for (emp::String filename : filenames) {
      ProcessFile(filename);
    }    
  }

  Empecable & SetAll(bool _in=true) {
    checks.copyright = _in;
    checks.no_end_spaces = _in;
    checks.no_tabs = _in;
    checks.include_guards = _in;
    checks.pragma_once = _in;
    checks.include_order = _in;
    return *this;
  }

  Empecable & SetVerbose(bool _in) { verbose = _in; return *this; }

  void PrintUsage() const {
    std::cout << "Usage: " << flags[0] << " {options ...} files ..." << std::endl;
  }

  void PrintVersion() const {
    std::cout << "File formatter version 0.1." << std::endl;
  }

  void PrintHelp() const {
    PrintVersion();
    std::cout << '\n';
    PrintUsage();
    std::cout << '\n';
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

    // DO PROCESSING HERE!

    return modified;
  }

  bool Process_FixHeadings(emp::File & file) {
    std::cout << "STUB: Fixing Front Matter" << std::endl;
    (void) file;
    return false;
  }

  bool Process_Whitespace(emp::File & file) {    
    // Check indentation; remove end spaces and all tabs
    bool diff = false;

    for (emp::String & line : file) {
      // Fix any usage of the tab character; assume two-space indentation.
      diff |= line.ReplaceAll("\t", "  ");

      // Remove any spaces at the end of the line.
      while (line.back() == ' ') {
        diff = true;
        line.pop_back();
      }
    }

    return diff;
  }

  bool Process_IncludeOrder(emp::File & file) {    
    std::cout << "STUB: Fixing Order of Includes" << std::endl;
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
