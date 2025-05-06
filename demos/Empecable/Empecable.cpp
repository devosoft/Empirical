/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2024-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file
 * @brief Load a series of filenames and clean up each file.
 *
 * Features:
 * - Check spelling for the code files.
 * - Remove illegal characters including \r, \t, and end-of-line spaces
 * - Ensure the file ends in a newline
 * - Ensure that 2 spaces are used for indent levels (esp. after `{` at eol?})
 * - Group and track include files
 * - Ensure include guards or #pragma once (or both) exist.
 * - Ensure no merge conflict markers are in the file (and help with merge?)
 * - Track any special features about files or projects including custom spelling.
 * - Fully configure actions with a Empecable.cfg file.
 * - Find the .Empirical/ directory for configurations (or create one if it doesn't exist)
 * 
 * Possible add-on features to develop?
 * - Produce a levelization map
 * - Dynamic control over boilerplate (for easy scaling to other projects)
 * - Guided shifting of boilerplate to a new format
 * - Spacing must always shift by 0 or 2 OR somehow align with previous line?
 * - Make sure include files have corresponding test files.
 * - Make sure test files are not empty (or effectively empty)
 */

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <unordered_set>

#include "../../include/emp/base/assert.hpp"
#include "../../include/emp/base/vector.hpp"
#include "../../include/emp/config/command_line.hpp"
#include "../../include/emp/config/FlagManager.hpp"
#include "../../include/emp/io/File.hpp"
#include "../../include/emp/io/io_utils.hpp"
#include "../../include/emp/tools/String.hpp"
#include "../../include/emp/tools/string_utils.hpp"

#include "Lexer.hpp"

namespace fs = std::filesystem;

struct Checks {
  // Important fixes; on by default.
  bool require_copyright = true;      // Check Copyright at the front.
  bool prevent_end_spaces = true;     // Check the no lines end with spaces.
  bool disallow_tabs = true;          // Check that no tabs are in code.
  bool require_include_guards = true; // Check that the file is protected by include guards.
  bool require_pragma_once = true;    // Check that the file has a "#pragma once"
  bool sort_include = true;           // Check that the includes in the file are grouped correctly.

  // Warnings that you can turn on.
  size_t warn_line_width = 0;     // Warn if a line is too long (zero = off)

  void SetAll(bool _in=true) {
    require_copyright = _in;
    prevent_end_spaces = _in;
    disallow_tabs = _in;
    require_include_guards = _in;
    require_pragma_once = _in;
    sort_include = _in;
  }
};

class Empecable {
private:
  // Options used:
  Checks checks;
  bool verbose = false;
  bool interactive = false;

  emp::FlagManager flags;             // Tracker for command-line flags that were set.
  emp::vector<emp::String> filenames; // Set of files to process.
  emp::vector<fs::path> filepaths;    // Set of full paths for each file.
  size_t active_file = static_cast<size_t>(-1);  // Which file are we working with?

  // Lexer information.
  emplex::Lexer lexer;
  emp::vector<emplex::Token> tokens;

  fs::path word_file = "word_list.txt";
  std::optional<fs::path> emp_dir = std::nullopt; // Put emp_dir here if one is found.
  using word_set_t = std::unordered_set<emp::String>;
  using word_map_t = std::unordered_map<emp::String, emp::String>;
  word_set_t project_words;   // Dictionary of legal words for this project.
  word_set_t file_words;      // Addition legal words in only this file.
  word_set_t skip_words;      // Words to skip over for now.
  word_map_t replacement_map; // Track replacement words to use.
  size_t issue_count = 0;     // Number of issues detected in current file.

  // Track changes that need to be saved.
  bool cpp_file_changed = false;
  bool project_changed = false;


  // === HELPER FUNCTIONS ===

  template <typename... Ts>
  emp::String ToBoldRed(Ts &&... args) {
    return emp::MakeString(args...).AsANSIRed().AsANSIBold();
  }

  void SetupOptionFlags() {
    flags.AddGroup("Basic Operation");
    flags.AddOption('a', "all", [this](){ SetAll(true); },
      "Activate ALL fixes (except those explicitly excluded; see below)");
    flags.AddOption('h', "help", [this](){ PrintHelp(); },
      "Get additional information about options.");
    flags.AddOption('i', "interactive", [this](){ interactive = true; },
      "Interactively fix file problems");
    flags.AddOption('v', "verbose", [this](){ verbose = true; },
      "Provide more detailed output");
    flags.AddOption('w', "word_file", [this](emp::String filename){ word_file = filename.str(); },
      "Specify the word file to use for spell checks.");

    flags.AddGroup("Activating Specific Fixes");
    flags.AddOption("require_copyright", [this](){ checks.require_copyright = true; });
    flags.AddOption("prevent_end_spaces", [this](){ checks.prevent_end_spaces = true; });
    flags.AddOption("disallow_tabs", [this](){ checks.disallow_tabs = true; });
    flags.AddOption("require_include_guards", [this](){ checks.require_include_guards = true; });
    flags.AddOption("require_pragma_once", [this](){ checks.require_pragma_once = true; });
    flags.AddOption("sort_include", [this](){ checks.sort_include = true; });

    flags.AddGroup("Deactivate Specific Fixes");
    flags.AddOption("no_require_copyright", [this](){ checks.require_copyright = false; });
    flags.AddOption("no_prevent_end_spaces", [this](){ checks.prevent_end_spaces = false; });
    flags.AddOption("no_disallow_tabs", [this](){ checks.disallow_tabs = false; });
    flags.AddOption("no_sort_include_guards", [this](){ checks.require_include_guards = false; });
    flags.AddOption("no_require_pragma_once", [this](){ checks.require_pragma_once = false; });
    flags.AddOption("no_sort_include", [this](){ checks.sort_include = false; });

    flags.AddGroup("Extra Warnings to Enable (off by default)");
    flags.AddOption('W', "max_line_width",
      [this](emp::String max_w){ checks.warn_line_width = max_w.AsULL(); } );
  }

  // Check the default key press options that should work from any menu.
  // Return whether the key was used.
  bool TestDefaultKeyOptions(char key) {
    switch (key) {
    case 'h':
      std::cout << "Help info should go here..." << std::endl;
      return true;
    case 'q':
      std::cout << "Quitting!" << std::endl;
      exit(0);
    case 's':
      if (active_file < filenames.size()) SaveFile();
      else std::cout << "No active file to save." << std::endl;
      SaveProjectConfig();
      return true;
    }
    return false; // Key not used.
  }

  fs::path MakeEmpiricalDir(fs::path common_path) {
    fs::path option_path = common_path;
    if (!fs::is_directory(option_path)) option_path = option_path.parent_path();
    std::cout <<
      "No .Empirical/ folder could be found in any parent directory.\n"
      "Where should it be created?\n";
    size_t opt_id = 0;
    while (!option_path.empty() && opt_id < 10 && emp::CanWriteToDirectory(option_path)) {
      std::cout << ToOption(std::to_string(opt_id)) << " - " << option_path << "\n";
      ++opt_id;
      if (option_path == option_path.parent_path()) break;
      option_path = option_path.parent_path();
    }
    Print(ToOption("q"), " - Quit.\n");
    bool done = false, used = false;
    fs::path out_path = common_path;
    while (!done) {
      used = false;
      char key = emp::GetIOChar();
      switch (key) {
      case '9': if (opt_id <= 9) break; out_path = out_path.parent_path(); [[fallthrough]];
      case '8': if (opt_id <= 8) break; out_path = out_path.parent_path(); [[fallthrough]];
      case '7': if (opt_id <= 7) break; out_path = out_path.parent_path(); [[fallthrough]];
      case '6': if (opt_id <= 6) break; out_path = out_path.parent_path(); [[fallthrough]];
      case '5': if (opt_id <= 5) break; out_path = out_path.parent_path(); [[fallthrough]];
      case '4': if (opt_id <= 4) break; out_path = out_path.parent_path(); [[fallthrough]];
      case '3': if (opt_id <= 3) break; out_path = out_path.parent_path(); [[fallthrough]];
      case '2': if (opt_id <= 2) break; out_path = out_path.parent_path(); [[fallthrough]];
      case '1': if (opt_id <= 1) break; out_path = out_path.parent_path(); [[fallthrough]];
      case '0':
        out_path /= ".Empirical";
        std::cout << "Creating directory: " << out_path << std::endl;
        std::filesystem::create_directory(out_path);
        done = true;
        break;
      default:
        used = TestDefaultKeyOptions(key);
      }
      if (!done && !used) {
        std::cout << "Unknown option " << ToBoldRed("'", key, "'") << std::endl;
      }

    }
    return out_path;
  }

  void LoadWords() {
    // Try loading the default word_file.
    std::ifstream file(word_file);

    // If word_file fails to load, look for (or make) a .Empirical/ directory.
    if (!file) {     
      // Find the .Empirical/ folder that should have config files in it. 
      fs::path common_path = emp::FindCommonPath(filepaths);
      emp_dir = emp::FindFolderInPath(".Empirical", common_path);

      // If the emp_dir does not exist, create one.
      if (!emp_dir) emp_dir = MakeEmpiricalDir(common_path);

      // Try again to load in the base set of words to compare, this time from .Empirical
      word_file = *emp_dir / word_file;
      file.open(word_file);

      // If we have still failed, report the error and abort.
      if (!file) {
        std::cerr << ToBoldRed("Error:") << " Unable to open file '" << word_file << "'.\n";
        exit(1);
      }
    }

    std::cout << "Loaded word file: '" << emp::ANSI::MakeGreen(word_file.string()) << "'.\n";

    // Now that we have the file set up, actually load the words!
    emp::String word;
    while (std::getline(file, word)) {
      project_words.insert(word.Filter(emp::IDCharSet()));
    }
  }

  void SaveFile() {
    emp::String filename = filenames[active_file];
    if (!cpp_file_changed) {
      std::cout << "No changes need to be saved in '" << emp::ANSI::MakeGreen(filename)
                << "'." << std::endl;
      return;
    }
    std::cout << "Saving '" << emp::ANSI::MakeGreen(filename) << "'." << std::endl;
    std::ofstream file(filename);
    for (auto & token : tokens) {
      file << token.lexeme;
    }
    file.close();
  }

  void SaveProjectConfig() {
    if (!project_changed) {
      std::cout << "No changes need to be saved in '" << emp::ANSI::MakeGreen(word_file.string())
                << "'." << std::endl;
      return;
    }
    std::cout << "Saving '" << emp::ANSI::MakeGreen(word_file.string()) << "'." << std::endl;

    // Move the project_words to a vector and sort them.
    emp::vector<emp::String> out_words;
    out_words.reserve(project_words.size());
    for (emp::String word : project_words) out_words.emplace_back(word);
    std::sort(out_words.begin(), out_words.end());

    // Print the new file
    std::ofstream file(word_file);
    for (emp::String word : out_words) file << word << '\n';
  }

  // Print tokens to the screen with a particular token highlighted in read.
  void ReportError(emp::String error, size_t token_pos) {
    namespace ANSI = emp::ANSI;

    // Report the actual error.
    std::cout << ANSI::MakeBold(ANSI::MakeYellow("Found: ")) << error << '\n';

    // Determine the set of tokens to print.
    size_t start = token_pos;
    size_t end = token_pos+1;
    // Rewind to beginning of line.
    while (start > 0 && tokens[start-1].lexeme != "\n") --start;
    // Fast forward to end of line.
    while ((end < tokens.size() && tokens[end].lexeme != "\n")) ++end;

    // Format and print the line number.
    emp::String line_num = emp::MakeString(tokens[token_pos].line_id).PadFront(' ', 5)+':';
    // std::cout << line_num.AsANSIYellow();
    std::cout << line_num.AsANSIBrightWhite().AsANSIBold();

    // Print the series of tokens on this line, highlighting the problem.
    for (size_t i = start; i < end; ++i) {
      if (i == token_pos) std::cout << ToBoldRed(tokens[i].lexeme).AsANSIUnderline();
      else std::cout << tokens[i].lexeme;
    }
    std::cout << '\n';
  }

  void AddProjectWord(emp::String word) {
    emp_assert(!emp::Has(project_words, word), word);
    std::cout << "Added '" << word.AsANSICyan() << "' to project dictionary." << std::endl;
    project_words.insert(word);
    project_changed = true;
  }

  void AddFileWord(emp::String word) {
    emp_assert(!emp::Has(file_words, word), word);
    std::cout << "Added '" << word.AsANSICyan() << "' to file dictionary." << std::endl;
    file_words.insert(word);
    cpp_file_changed = true;
  }

  void DoReplace(size_t token_pos, bool change_all) {
    emp::String word = tokens[token_pos].lexeme;

    std::cout << "Enter replacement word: ";
    emp::String new_word;
    std::cin >> new_word;

    // Make change to THIS token.
    tokens[token_pos].lexeme = new_word;

    // Record change for future tokens.
    if (change_all) replacement_map[word] = new_word;

    std::cout << "Replacing ";
    if (change_all) std::cout << " all instances of ";
    std::cout << "'" << word.AsANSICyan() << "' with '" << new_word.AsANSICyan() << "'.\n";

    cpp_file_changed = true;
  }

  // Interface options for consistency.
  // 'd' - Delete (or 'D' to Delete All)
  // 'r' - Replace with (or 'R' to Replace All)
  // 'i' - Ignore (or 'I' to Ignore All)
  // 'p' - Add lowercase word to project dictionary (or 'P' to preserve current case)
  // 'f' - Add lowercase word to file dictionary (or 'F' to preserve current case)
  // 'h' - Help
  // 'q' - Quit without saving (or 'Q' to quit and save)
  // 's' - Save current updated file (or 'S' to "Save As")
  // 'z' - Undo
  // 'y' or 'n' - Answer a specific question.

  template <typename... Ts>
  void Print(Ts &&... args) { std::cout << emp::MakeString(args...); }

  emp::String ToOption(emp::String key) {
    return emp::MakeString('\'', key.AsANSIMagenta(), '\'');
  }

  bool TestWordToken(size_t token_pos) {
    emp::String word(tokens[token_pos].lexeme);
    emp::String lower_word = word.AsLower();
    
    // If a word is short (2 or fewer letters) or is in the dictionary (either directly or or as
    // a lowercase word), mark it as valid.
    if (word.size() <= 2 ||
        project_words.contains(word) ||
        project_words.contains(lower_word) ||
        file_words.contains(word) ||
        file_words.contains(lower_word) ||
        skip_words.contains(word) ||
        skip_words.contains(lower_word)) return true;

    // We have an unknown word.
    ReportError(emp::MakeString("Unknown word '", word.AsANSICyan(), "'"), token_pos);

    if (interactive) {
      Print(ToOption("p"), " - Add '", lower_word.AsANSICyan(), "' to main PROJECT dictionary");
      if (word.HasUpper()) {
        Print(" or ", ToOption("P"), " for case-sensitive '", word.AsANSICyan(), "'");
      }
      std::cout << "\n";

      Print(ToOption("f"), " - Add '", lower_word.AsANSICyan(), "' to this FILE's dictionary");
      if (word.HasUpper()) {
        Print("  or ", ToOption("F"), " for case-sensitive '", word.AsANSICyan(), "'");
      }
      std::cout << "\n";

      Print(ToOption("r"), " - Replace this instance of '", word.AsANSICyan(), "' or ",
            ToOption("R"), " to replace ALL instances\n");
      Print(ToOption("i"), " - Ignore this instance of '", word.AsANSICyan(), "'  or ",
            ToOption("I"), " to ignore ALL instances\n");

      bool done = false;
      while (!done) {
        char key = emp::GetIOChar();
        switch (key) {
          case 'p': AddProjectWord(lower_word);   done = true; break;
          case 'P': AddProjectWord(word);         done = true; break;
          case 'f': AddFileWord(lower_word);      done = true; break;
          case 'F': AddFileWord(word);            done = true; break;
          case 'r': DoReplace(token_pos, false);  done = true; break;
          case 'R': DoReplace(token_pos, true);   done = true; break;
          case 'i':
            std::cout << "Skipping '" << word.AsANSICyan() << "'!" << std::endl;
            done = true;
            break;
          case 'I':
            std::cout << "Ignoring all instances of '" << word.AsANSICyan() << "'!" << std::endl;
            skip_words.insert(word);
            done = true;
            break;
          default:
            bool used = TestDefaultKeyOptions(key);
            if (!used) {
              std::cout << "Unknown key " << ToBoldRed("'", key, "'") << std::endl;
            }
        }
      }
    }

    return false;
  }

public:
  Empecable(int argc, char * argv[]) : flags(argc, argv) {
    SetupOptionFlags();
    flags.Process();

    // Collect the filenames
    filenames = flags.GetExtras();
    if (filenames.size() == 0) {
      std::cout << "No files listed." << std::endl;
      PrintUsage();
      exit(0);
    }

    // Validate the files and convert to full paths.
    size_t err_count = 0;
    for (const std::string & name : filenames) {
      fs::path p = fs::absolute(name);
      if (fs::exists(p) && fs::is_regular_file(p)) {
        p = fs::canonical(name);
        filepaths.push_back(p);
      } else {
        std::cerr << ToBoldRed("Error:") << " file does not exist or is not a regular file: " << name << "\n";
        ++err_count;
      }
    }
    if (err_count) {
      std::cerr << err_count << " errors occurred. Exiting.\n";
      exit(1);
    }

    LoadWords();

    for (active_file = 0; active_file < filenames.size(); ++active_file) {
      ProcessFile();
      std::cout << "Finished '" << emp::ANSI::MakeGreen(filenames[active_file])
                << "'. Saving." << std::endl;
      SaveFile();
      SaveProjectConfig();
    }

  }

  ~Empecable() {
    SaveProjectConfig();
  }

  Empecable & SetAll(bool _in=true) { checks.SetAll(_in); return *this; }

  Empecable & SetVerbose(bool _in) { verbose = _in; return *this; }

  void PrintUsage() const {
    std::cout << "Usage: " << flags[0] << " {options ...} files ..." << std::endl;
    std::cout << "Type `" << flags[0] << " -h` for more detailed help." << std::endl;
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

  bool LoadFile(emp::String filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cout << ToBoldRed("ERROR: '", filename, "' failed to open.\n");
      return false;
    }
    tokens = lexer.Tokenize(file);
    issue_count = 0;    // Reset issues for new file.

    return true;
  }

  bool ProcessFile() {
    emp::String filename = filenames[active_file];
    std::cout << "=== File: " << filename.AsANSIBrightCyan() << " ===\n";

    if (!LoadFile(filename)) return false;

    cpp_file_changed = false; // New file, so no changes!
    
    for (size_t token_pos = 0; token_pos < tokens.size(); ++token_pos) {
      const auto & token = tokens[token_pos];
      bool found_issue = false;
      switch (token.id) {
        using namespace emplex;
      case Lexer::ID_WORD:
        found_issue = !TestWordToken(token_pos);
        break;
      case Lexer::ID_ERR_END_LINE_WS:        
        ReportError("Extra whitespace at end of line:", token_pos);
        found_issue = true;
        break;
      case Lexer::ID_ERR_WS:
        ReportError("Illegal whitespace:", token_pos);
        found_issue = true;
        break;
      default:
        break;
      }
      if (found_issue) {
        std::cout << "-------------------------------------------------------------------------------\n";
        ++issue_count;
      }
    }

    std::cout << '\n' << ToBoldRed("=== ", issue_count, " issues found ===") << std::endl;

    return issue_count;
  }
};

int main(int argc, char * argv[])
{
  class Empecable formatter(argc, argv);
}
