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
 *
 * Master list of interface options for consistency.
 *  'd' - Delete (or 'D' to Delete All)
 *  'r' - Replace with (or 'R' to Replace All)
 *  'i' - Ignore (or 'I' to Ignore All)
 *  'p' - Add lowercase word to project dictionary (or 'P' to preserve current case)
 *  'f' - Add lowercase word to file dictionary (or 'F' to preserve current case)
 *  'h' - Help
 *  'q' - Quit without saving (or 'Q' to quit and save)
 *  's' - Save current updated file (or 'S' to "Save As")
 *  'z' - Undo
 *  'y' or 'n' - Answer a specific question.
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

#include "helpers.hpp"
#include "Lexer.hpp"
#include "ReviewFile.hpp"

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

  emp::FlagManager flags;         // Tracker for command-line flags that were set.
  emp::vector<ReviewFile> files;  // Files to be reviewed.
  size_t active_file = 0;         // Which file are we working with?

  // Lexer information.
  emplex::Lexer lexer;

  fs::path word_file = "word_list.txt";
  std::optional<fs::path> emp_dir = std::nullopt; // Put emp_dir here if one is found.

  using word_set_t = std::unordered_set<emp::String>;
  using word_map_t = std::unordered_map<emp::String, emp::String>;
  word_set_t project_words;   // Dictionary of legal words for this project.
  word_set_t skip_words;      // Words to skip over for now.
  word_map_t replacement_map; // Track replacement words to use.

  bool project_changed = false; // Have there been any project-level changes requiring save?


  // === HELPER FUNCTIONS ===

  ReviewFile & File() { return files[active_file]; }
  const ReviewFile & File() const { return files[active_file]; }

  emplex::Token GetToken(size_t pos) const { return File().GetToken(pos); }
  emp::String GetLexeme(size_t pos) const { return File().GetLexeme(pos); } 

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
      PrintLn("Help info should go here...");
      return true;
    case 'q':
      PrintLn("Quitting!");
      exit(0);
    case 's':
      if (active_file < files.size()) File().Save();
      else PrintLn("No active file to save.");
      SaveProjectConfig();
      return true;
    }
    return false; // Key not used.
  }

  bool AskYesNo(emp::String question) {
    PrintLn(question, " (", ToOptionSet("yn"), ")");
    std::cout.flush();
    char key = emp::GetIOChar();
    while (true) {
      switch (key) {
      case 'y': case 'Y': return true;
      case 'n': case 'N': return false;
      default:
        if (!TestDefaultKeyOptions(key)) {
          PrintLn("Unknown option ", ToBoldRed("'", key, "'"));
        }
      }
    }
  }

  fs::path MakeEmpiricalDir(fs::path common_path) {
    fs::path option_path = common_path;
    if (!fs::is_directory(option_path)) option_path = option_path.parent_path();
    PrintLn("No .Empirical/ folder could be found in any parent directory.");
    PrintLn("Where should it be created?");
    size_t opt_id = 0;
    while (!option_path.empty() && opt_id < 10 && emp::CanWriteToDirectory(option_path)) {
      PrintLn(ToOption(std::to_string(opt_id)), " - ", option_path);
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
        PrintLn("Creating directory: ", out_path);
        std::filesystem::create_directory(out_path);
        done = true;
        break;
      default:
        used = TestDefaultKeyOptions(key);
      }
      if (!done && !used) {
        PrintLn("Unknown option ", ToBoldRed("'", key, "'"));
      }

    }
    return out_path;
  }

  // Find the sub-path that all files have in common.
  fs::path FindCommonPath() {
    if (files.size() == 0) return {};

    fs::path common_path = files[0].GetPath();
    for (size_t i=1; i < files.size(); ++i) {
      common_path = emp::FindCommonPath(common_path, files[i].GetPath());
    }

    return common_path;
  }

  // Find the .Empirical directory for config files.
  void SetEMPDir() {
      fs::path common_path = FindCommonPath();
      emp_dir = emp::FindFolderInPath(".Empirical", common_path);

      // If the emp_dir does not exist, create one.
      if (!emp_dir) emp_dir = MakeEmpiricalDir(common_path);
  }

  void LoadWords() {
    // Try loading the default word_file.
    std::ifstream file(word_file);

    // If word_file fails to load, look for (or make) a .Empirical/ directory.
    if (!file) {     
      SetEMPDir();
      word_file = *emp_dir / word_file;
      file.open(word_file);

      // If we have still failed, report the error and abort.
      if (!file) {
        std::cerr << ToBoldRed("Error:") << " Unable to open file '" << word_file << "'.\n";
        exit(1);
      }
    }

    PrintLn("Loaded word file: '", ToFilename(word_file), "'.");

    // Now that we have the file set up, actually load the words!
    emp::String word;
    while (std::getline(file, word)) {
      project_words.insert(word);
    }
  }

  void SaveProjectConfig() {
    if (!project_changed) {
      PrintLn("No changes need to be saved in '", ToFilename(word_file), "'.");
      return;
    }
    PrintLn("Saving '", ToFilename(word_file), "'.");

    // Move the project_words to a vector and sort them.
    emp::vector<emp::String> out_words;
    out_words.reserve(project_words.size());
    for (emp::String word : project_words) out_words.emplace_back(word);
    std::sort(out_words.begin(), out_words.end());

    // Print the new file
    std::ofstream file(word_file);
    for (emp::String word : out_words) file << word << '\n';
  }

  void AddProjectWord(emp::String word) {
    emp_assert(!emp::Has(project_words, word), word);
    PrintLn("Added '", word.AsANSICyan(), "' to project dictionary.");
    project_words.insert(word);
    project_changed = true;
  }

  void DoReplace(size_t token_pos) {
    const emp::String old_word = GetLexeme(token_pos);
    const emp::String new_word = replacement_map[old_word];
    File().SetLexeme(token_pos, new_word);
    if (verbose || interactive) {
      PrintLn("Replacing '", old_word.AsANSICyan(), "' with '", new_word.AsANSICyan(), "'.");
    }
  }  

  void QueryReplace(size_t token_pos, bool change_all) {
    emp::String word = GetLexeme(token_pos);

    Print("Enter replacement word: ");
    emp::String new_word;
    std::cin >> new_word;

    // Make change to THIS token.
    File().SetLexeme(token_pos, new_word);

    // Record change for future tokens.
    if (change_all) replacement_map[word] = new_word;

    Print("Replacing ");
    if (change_all) Print(" all instances of ");
    PrintLn("'", word.AsANSICyan(), "' with '", new_word.AsANSICyan(), "'.");
  }

  bool TestWordToken(size_t token_pos) {
    emp::String word(GetLexeme(token_pos));
    
    // If we have a valid word, return true.
    if (word.size() <= 2 ||                       // Allow all short (1 or 2 char) words
        project_words.contains(word) ||           // Check the white list for project
        File().HasWord(word) ||                   // Check the file-specific white list
        skip_words.contains(word)) return true;   // Check words to skip.

    // If this word has uppercase, also check the lowercase version.
    if (word.HasUpper()) {
      emp::String lower_word = word.AsLower();
      if (project_words.contains(lower_word) ||
          File().HasWord(lower_word) ||
          skip_words.contains(lower_word)) return true;
    }

    // See if we already have a replacement set up for this word.
    if (replacement_map.contains(word)) DoReplace(token_pos);

    // We have an unknown word.
    File().ReportIssue(emp::MakeString("Unknown word '", word.AsANSICyan(), "'"), token_pos);

    if (interactive) {
      emp::String lower_word = word.AsLower();
      Print(ToOption("p"), " - Add '", lower_word.AsANSICyan(), "' to main PROJECT dictionary");
      if (word.HasUpper()) {
        Print(" or ", ToOption("P"), " for case-sensitive '", word.AsANSICyan(), "'");
      }
      PrintLn();

      Print(ToOption("f"), " - Add '", lower_word.AsANSICyan(), "' to this FILE's dictionary");
      if (word.HasUpper()) {
        Print("  or ", ToOption("F"), " for case-sensitive '", word.AsANSICyan(), "'");
      }
      PrintLn();

      Print(ToOption("r"), " - Replace this instance of '", word.AsANSICyan(), "' or ",
            ToOption("R"), " to replace ALL instances\n");
      Print(ToOption("i"), " - Ignore this instance of '", word.AsANSICyan(), "'  or ",
            ToOption("I"), " to ignore ALL instances\n");

      bool done = false;
      while (!done) {
        char key = emp::GetIOChar();
        switch (key) {
          case 'p': AddProjectWord(lower_word);     done = true; break;
          case 'P': AddProjectWord(word);           done = true; break;
          case 'f': File().AddWord(lower_word);     done = true; break;
          case 'F': File().AddWord(word);           done = true; break;
          case 'r': QueryReplace(token_pos, false); done = true; break;
          case 'R': QueryReplace(token_pos, true);  done = true; break;
          case 'i':
            PrintLn("Skipping '", word.AsANSICyan(), "'!");
            done = true;
            break;
          case 'I':
            PrintLn("Ignoring all instances of '", word.AsANSICyan(), "'!");
            skip_words.insert(word);
            done = true;
            break;
          default:
            bool used = TestDefaultKeyOptions(key);
            if (!used) PrintLn("Unknown key ", ToBoldRed("'", key, "'"));
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
    emp::vector<emp::String> filenames = flags.GetExtras();
    if (filenames.size() == 0) {
      PrintLn("No files listed.");
      PrintUsage();
      exit(0);
    }

    // Validate the filenames, set up full paths, and save as ReviewFile objects.
    size_t err_count = 0;
    for (const std::string & name : filenames) {
      files.emplace_back(name);
      if (!files.back().IsValid()) ++err_count;
    }
    if (err_count) {
      std::cerr << err_count << " errors opening files. Exiting.\n";
      exit(1);
    }

    LoadWords();

    // Step through all of the files.
    for (active_file = 0; active_file < filenames.size(); ++active_file) {
      ProcessFile();
      PrintLn("Finished '", ToFilename(File().GetName()), "'. Saving.\n");
      File().Save();
    }
    SaveProjectConfig();

  }

  ~Empecable() { }

  Empecable & SetAll(bool _in=true) { checks.SetAll(_in); return *this; }

  Empecable & SetVerbose(bool _in) { verbose = _in; return *this; }

  void PrintUsage() const {
    PrintLn("Usage: ", flags[0], " {options ...} files ...");
    PrintLn("Type `", flags[0], " -h` for more detailed help.");
  }

  void PrintVersion() const {
    PrintLn("File formatter version 0.1.");
  }

  void PrintHelp() const {
    PrintVersion();
    PrintLn();
    PrintUsage();
    PrintLn();
    flags.PrintOptions();
    exit(0);
  }

  void ProcessFile() {
    PrintLn("=== File: ", File().GetName().AsANSIBrightCyan(), " ==n");

    if (!File().Load(lexer)) return; // File failed to load.

    for (size_t token_pos = 0; token_pos < File().NumTokens(); ++token_pos) {
      bool found_issue = false;
      switch (File().GetTokenID(token_pos)) {
        using namespace emplex;
      case Lexer::ID_WORD:
        found_issue = !TestWordToken(token_pos);
        break;
      case Lexer::ID_ERR_END_LINE_WS:        
        File().ReportIssue("Extra whitespace at end of line:", token_pos);
        if (AskYesNo("Remove? ")) File().ClearLexeme(token_pos);
        found_issue = true;
        break;
      case Lexer::ID_ERR_WS:
        File().ReportIssue("Illegal whitespace:", token_pos);
        if (AskYesNo("Remove? ")) File().ClearLexeme(token_pos);
        found_issue = true;
        break;
      default:
        break;
      }

      // Skip a line between issues.
      if (found_issue) {
        PrintLn("-------------------------------------------------------------------------------");
      }
    }

    PrintLn('\n', ToBoldRed("=== ", File().GetNumIssues(), " issues found ==="));
  }
};

int main(int argc, char * argv[])
{
  class Empecable formatter(argc, argv);
}
