/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2024-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file
 * @brief Load a series of filenames and clean up each file.
 *
 * Features:
 * - Check spelling for the code files (With suggestions, change all, and remembering
 *   changes to use as suggestions in future.)
 * - Remove illegal characters including \r, \t, and end-of-line spaces
 * - Ensure the file ends in a newline
 * - Track any special features about files or projects including custom spelling.
 * - Find the .Empirical/ directory for configurations
 *
 * Some features will also be covered by clang-format
 * - Ensure 2 spaces are used for indent levels.
 * - Group and track include files
 *
 * Additional TODO:
 * + Ensure include guards or #pragma once (or both) exist for header files.
 * + Ensure no merge conflict markers are in the file (and help with merge?)
 * + Create config files that don't already exist.
 * + Dynamic control over boilerplate (for easy scaling to other projects)
 *
 * + Fully configure actions with a Empecable.cfg file in .Empirical/
 * + Allow configure overrides in individual files.
 * + Produce a levelization map
 * + Guided shifting of boilerplate to a new format
 * + Make sure emp/include/ header (.hpp) files have corresponding test files.
 * + Make sure test files are not empty (or effectively empty)
 * + Better suggestions where 'y' always gives you what Empecable thinks is correct action.
 * + remove local words if in project dictionary; remove 'replace' if in either local of project
 *
 * Master list of interface options for consistency.
 *  'a' - Add lowercase word to project dictionary (or 'A' to preserve current case)
 *  'd' - Delete (or 'D' to Delete All)
 *  'r' - Replace with (or 'R' to Replace All)
 *  'i' - Ignore (or 'I' to Ignore All)
 *  'f' - Add lowercase word to file dictionary (or 'F' to preserve current case)
 *  'h' - Help
 *  'q'/'x' - Quit without saving (or 'Q' to quit and save)
 *  's' - Save current updated file (or 'S' to "Save As")
 *  'v' - View section of code.
 *  'z' - Undo
 *  'y' or 'n' - Answer a specific question.
 *  '0' through '9' - Multiple choice options.
 *
 * Color meanings:
 *  red     - errors
 *  cyan    - words
 *  green   - filenames and directories
 *  magenta - keypress options
 *  yellow  - line numbers
 *  blue    - ?
 *
 * Config options:
 * - HeaderExtensions: .hpp|.h|.H|.hh
 * - ProtectHeaders: None|Pragma|Guards|Both
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
#include "../../include/emp/config/SettingsManager.hpp"
#include "../../include/emp/io/ascii_utils.hpp"
#include "../../include/emp/io/File.hpp"
#include "../../include/emp/io/io_utils.hpp"
#include "../../include/emp/math/sequence_utils.hpp"
#include "../../include/emp/tools/String.hpp"
#include "../../include/emp/tools/string_utils.hpp"

#include "helpers.hpp"
#include "Lexer.hpp"
#include "ReviewFile.hpp"

class Empecable {
private:
  emp::SettingsManager settings;
  Mode mode = Mode::Normal;

  emp::FlagManager flags;         // Tracker for command-line flags that were set.
  emp::vector<ReviewFile> files;  // Files to be reviewed.
  size_t active_file = 0;         // Which file are we working with?
  size_t total_issues = 0;

  // Lexer information.
  emplex::Lexer lexer;

  fs::path config_file = "Empecable.cfg";
  fs::path word_file = "word_list.txt";
  fs::path replace_file = "replace_list.txt";
  std::optional<fs::path> emp_dir = std::nullopt; // Put emp_dir here if one is found.

  using word_set_t = std::unordered_set<emp::String>;
  using word_map_t = std::unordered_map<emp::String, emp::String>;
  word_set_t project_words;   // Dictionary of legal words for this project.
  word_set_t skip_words;      // Words to skip over for now.
  word_map_t replacement_map; // Track replacement words to always use.
  word_map_t suggest_map;     // Track replacement words to suggest.

  // Set up the different menus.
  emp::ANSIOptionMenu menu_spelling;
  emp::ANSIOptionMenu menu_tab;
  emp::ANSIOptionMenu menu_eol_space;

  bool project_changed = false; // Have there been any project-level changes requiring save?


  // === HELPER FUNCTIONS ===

  ReviewFile & File() { return files[active_file]; }
  const ReviewFile & File() const { return files[active_file]; }

  emplex::Token GetToken() const { return File().GetToken(); }
  emp::String GetLexeme() const { return File().GetLexeme(); }
  size_t GetLineID() const { return File().GetLineID(); }
  emplex::Token GetToken(size_t pos) const { return File().GetToken(pos); }
  emp::String GetLexeme(size_t pos) const { return File().GetLexeme(pos); }
  size_t GetLineID(size_t pos) const { return File().GetLineID(pos); }

  void ConfigureSettings() {
    settings.AddSetting("Copyright",
      "This file is part of Empirical, https://github.com/devosoft/Empirical\n"
      "Copyright (C) {year} Michigan State University\n"
      "MIT Software license; see doc/LICENSE.md\n\n"
      "@file\n"
      "@brief [File Description]",
      "Full text of the copyright heading that should be at the top of each file."
    );

    settings.AddSetting("HeaderExtensions", ".hpp|.h|.H|.hh",
      "File extensions to assume are C++ headers.");

    settings.AddSetting("CodeExtensions", ".cpp|.C|.cc",
      "File extensions to assume are C++ code files.");

    settings.AddSetting("HeaderProtections", "None|Pragma|Guards|Both",
      "Type of protections against multiple includes to use in headers (None|Pragma|Guards|Both)\n"
      "E.g., if you want either type of protection, but not both, use \"Pragma|Guards\"");

    settings.AddSetting("SpellCheck", true,
      "Count spelling mistakes as errors in a file? (On/Off)");

    settings.AddSetting("ProjectDictionary", true,
      "Track a master list of legal words in .Empirical/word_list.txt (On/Off)");

    settings.AddSetting("FileDictionary", true,
      "Track a local list of legal words in comment at bottom of each file? (On/Off)");

    settings.AddSetting("TrackReplacements", true,
      "Track word replacements in .Empirical/replace_list.txt for future suggestions? (On/Off)");

    settings.AddSetting("Interactive", false,
      "Go into interactive mode by default? (On/Off; Use -i flag to activate from command line)");

    settings.AddSetting("RemoveIllegalChars", true,
      "Remove tabs ('\t') and carriage returns ('\r')? (On/Off)");

    settings.AddSetting("MapLevels", true,
      "Create a levelization map of header files include each other? (On/Off)");
  }

  void SetupOptionFlags() {
    flags.AddGroup("Basic Operation");
    flags.AddOption('c', "config-file", [this](emp::String filename){ config_file = filename.str(); },
      "Set the name of the main configuration file to use (Default: '" + config_file.string() + "').");
    flags.AddOption('h', "help", [this](){ PrintHelp(); },
      "Get additional information about options.");
    flags.AddOption('i', "interactive", [this](){ mode = Mode::Interactive; },
      "Interactively fix file problems");
    flags.AddOption('r', "replace_file", [this](emp::String filename){ replace_file = filename.str(); },
      "Specify the word file to use for spell checks (Default: '" + replace_file.string() + "').");
    flags.AddOption('v', "verbose", [this](){ mode = Mode::Verbose; },
      "Provide more detailed output");
    flags.AddOption('w', "word_file", [this](emp::String filename){ word_file = filename.str(); },
      "Specify the word file to use for spell checks (Default: '" + word_file.string() + "').");
  }

  // Check the default key press options that should work from any menu.
  void AddDefaultMenuOptions(emp::ANSIOptionMenu & menu) {
    menu.AddSilent('h', "Help", [](){ emp::PrintLn("Help info goes here..."); return false; });
    menu.AddSilent('s', "Save", [this](){ SaveAll(); return false; });
    menu.AddSilent('q', "Quit", [this](){ Quit(false); return false; }).AddAlias('x');
    menu.AddSilent('Q', "Save & Quit", [this](){ Quit(true); return false; }).AddAlias('X');
    menu.AddSilent('v', "View code around token (5 lines per side)",
      [this](){ emp::PrintLn("----------"); File().PrintTokenRange(5); return false; });
    menu.AddSilent('V', "10 lines per side",
      [this](){ emp::PrintLn("----------"); File().PrintTokenRange(10); return false; }, true);
  }

  bool AskYesNo(emp::String question) {
    emp::ANSIOptionMenu menu;
    menu.SetQuestion(question + " (" + ToOptionSet("yn") + ")");
    bool result = false;

    menu.AddSilent('y', "Accept", [&result](){ result=true; return true; });
    menu.AddSilent('n', "Reject", [&result](){ result=false; return true; });
    AddDefaultMenuOptions(menu);
    menu.Run();

    return result;
  }

  void AskRemoveToken() {
    if (IsInteractive() && AskYesNo("Remove? ")) {
      File().ClearLexeme();
    }
    if (IsVerbose()) { emp::PrintRepeatLn('-',79); }
  }

  void MakeEmpiricalDir(fs::path common_path) {
    emp_assert(IsInteractive());

    fs::path option_path = common_path;
    if (!fs::is_directory(option_path)) option_path = option_path.parent_path();

    emp::PrintLn("No config folder found in any parent directory.");

    emp::ANSIOptionMenu menu;
    menu.SetQuestion("Where should .Empirical/ be created?");

    for (size_t opt_id = 0;
        !option_path.empty() && opt_id < 10 && emp::CanWriteToDirectory(option_path);
        ++opt_id) {
      menu.AddOption('0' + opt_id, "create " + option_path.string(), [this,option_path](){
        emp_dir = option_path / ".Empirical";
        emp::PrintLn("Creating directory: ", *emp_dir);
        std::filesystem::create_directory(*emp_dir);
        return true;
      });
      if (option_path == option_path.parent_path()) break;
      option_path = option_path.parent_path();
    }
    menu.AddOption('q', "Quit", [this](){ Quit(false); return false; }).AddAlias('x');
    menu.Run();
  }

  // Find the sub-path that all test files have in common.
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
      if (!emp_dir) MakeEmpiricalDir(common_path);
  }

  void LoadWords() {
    // Try loading the default word_file.
    std::ifstream in_file(word_file);

    // If word_file fails to load, look for (or make) a .Empirical/ directory.
    if (!in_file) {
      SetEMPDir();
      word_file = *emp_dir / word_file;
      in_file.open(word_file);

      // If we have still failed, report the error and abort.
      if (!in_file) {
        std::cerr << ToBoldRed("Error:") << " Unable to open file '" << word_file << "'.\n";
        exit(1);
      }
    }

    // Now that we have the file set up, actually load the words!
    emp::String word;
    while (std::getline(in_file, word)) {
      project_words.insert(word);
    }

    emp::PrintLn("Loaded word file: '", ToFilename(word_file), "'.");

    std::ifstream in_replace_file(replace_file);
    if (!in_replace_file) {
      SetEMPDir();
      replace_file = *emp_dir / replace_file;
      in_replace_file.open(replace_file);

      // If we have still failed, let the user know, but keep going.
      if (!in_replace_file) {
        emp::PrintLn("No word replacement list file found at ", replace_file, ".");
      }
    }

    if (in_replace_file) {
      emp::String translation;
      while (in_replace_file) {
        in_replace_file >> word >> translation;
        // std::getline(in_replace_file, translation); // Grab entire translation, maybe more than one word.
        suggest_map[word] = translation;
      }
      emp::PrintLn("Loaded replacement list file: '", ToFilename(replace_file),
              "' with ", suggest_map.size(), " suggestions.");
    }


  }

  void SaveCurrentFile() {
    if (active_file < files.size()) File().Save();
    else emp::PrintLn("No active file to save.");
  }

  void SaveProjectConfig() {
    if (!project_changed) {
      emp::PrintLn("No changes need to be saved in '", ToFilename(word_file), "'.");
    } else {
      emp::PrintLn("Saving '", ToFilename(word_file), "'.");

      // Move the project_words to a vector and sort them.
      emp::vector<emp::String> out_words;
      out_words.reserve(project_words.size());
      for (emp::String word : project_words) out_words.emplace_back(word);
      std::sort(out_words.begin(), out_words.end());

      // Print the new file
      std::ofstream file(word_file);
      for (emp::String word : out_words) file << word << '\n';
    }

    if (suggest_map.size() || replacement_map.size()) {
      emp::PrintLn("Saving '", ToFilename(replace_file), "'.");
      std::ofstream file(replace_file);
      for (auto [from, to] : suggest_map) {
        if (to.HasWhitespace()) continue; // Skip saving any suggestions with multiple words.
        file << from << " " << to << '\n';
      }
      // Also record anything in the replacement map that was NOT in the suggestion map.
      for (auto [from, to] : replacement_map) {
        if (suggest_map.contains(from) || to.HasWhitespace()) continue;
        file << from << " " << to << '\n';
      }
    } else {
      emp::PrintLn("No suggestions to save.");
    }
  }

  void SaveAll() {
    SaveCurrentFile();
    SaveProjectConfig();
  }

  [[noreturn]] void Quit(bool save_before_quitting) {
    if (save_before_quitting) SaveAll();
    exit(0);
  }

  void AddProjectWord(emp::String word) {
    emp_assert(!emp::Has(project_words, word), word);
    emp::PrintLn("Added '", word.AsANSICyan(), "' to project dictionary.");
    project_words.insert(word);
    project_changed = true;
  }

  void DoReplace(emp::String new_word, bool change_all=false) {
    const emp::String old_word = GetLexeme();
    suggest_map[old_word] = new_word;
    if (change_all) replacement_map[old_word] = new_word;
    File().SetLexeme(new_word);
    if (IsVerbose()) {
      emp::PrintLn("Line ", GetLineID(),
        ": Replacing '", old_word.AsANSICyan(), "' with '", new_word.AsANSICyan(), "'.");
    }
  }

  // DoReplace with no replacement word uses the replacement_map.
  void DoReplace() {
    const emp::String old_word = GetLexeme();
    emp_assert(replacement_map.contains(old_word));
    DoReplace(replacement_map[old_word]);
  }

  void QueryReplace(bool change_all) {
    emp::String word = GetLexeme();

    emp::Print("Enter replacement word: ");
    emp::String new_word;
    std::cin >> new_word;

    // Make change to THIS token.
    File().SetLexeme(new_word);

    // Record change for future tokens.
    suggest_map[word] = new_word;
    if (change_all) replacement_map[word] = new_word;

    emp::Print("Replacing ");
    if (change_all) emp::Print(" all instances of ");
    emp::PrintLn("'", word.AsANSICyan(), "' with '", new_word.AsANSICyan(), "'.");
  }

  // Search through available dictionaries to try to find misspellings.
  emp::vector<emp::String> FindWordMatches(emp::String target_word, size_t max_size=5) {
    emp_assert(max_size > 0);

    constexpr double max_word_diff = 3.0;
    const int target_size = target_word.ssize();

    std::vector<emp::String> matches; // Set of matches to suggest to the user.

    // If the target word already has a suggestion associated with it, make sure that comes first.
    emp::String suggestion = suggest_map.contains(target_word) ? suggest_map[target_word] : "";

    // Score all of the other words in the dictionary (if they're close enough)
    std::vector<std::pair<double, std::string>> scored;
    const emp::String lower_word = target_word.AsLower();

    for (const emp::String & word : project_words) {
      // Don't consider words that are too different in length.
      if (std::abs(word.ssize() - target_size) >= max_word_diff) continue;

      // Don't reconsider an existing suggestion.
      if (word == suggestion) continue;

      // Save all other words with their distance.
      double dist = emp::calc_word_distance(word.AsLower(), lower_word);
      if (dist <= max_word_diff) scored.emplace_back(dist, word);
    }

    std::sort(scored.begin(), scored.end());

    if (suggestion.size()) matches.push_back(suggestion);
    for (auto [score, word] : scored) {
      matches.push_back(word);
      if (matches.size() >= max_size) break;
    }

    return matches;
  }

  // Should we automatically allow the provided word in the file?
  bool IsWordAllowed(const emp::String & word) {
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

    return false; // Word not found as an automatically allowed option.
  }

  // If we have a disallowed word that we need to interactively deal with, do so.
  void ManageWord_Interactive(const emp::String & word) {
    emp::String lower_word = word.AsLower();

    emp::ANSIOptionMenu menu;
    menu.AddOption('a', "Add '" + lower_word.AsANSICyan() + "' to main PROJECT dictionary",
      [this,lower_word](){ AddProjectWord(lower_word); return true; });
    if (word.HasUpper()) {
      menu.AddOption('A', "add case-sensitive '" + word.AsANSICyan() + "'",
        [this,word](){ AddProjectWord(word); return true; }, true);
    }

    menu.AddOption('f', "Add '" + lower_word.AsANSICyan() + "' to this FILE's dictionary",
      [this,lower_word](){ File().AddWord(lower_word); return true; });
    if (word.HasUpper()) {
      menu.AddOption('F', "add case-sensitive '" + word.AsANSICyan() + "'",
        [this,word](){ File().AddWord(word); return true; }, true);
    }

    menu.AddOption('i', "Ignore this instance of '" + word.AsANSICyan() + "'",
      [word](){ emp::PrintLn("Skipping '", word.AsANSICyan(), "'!"); return true; });
    menu.AddOption('I', "ignore ALL instances",
      [this,word](){ emp::PrintLn("Ignoring all instances of '", word.AsANSICyan(), "'!");
        skip_words.insert(word); return true; }, true);

    // Come up with word matches (and keep case the same as the original word)
    emp::vector<emp::String> matches = FindWordMatches(word);
    if (word.OnlyUpper()) { for (auto & match : matches) match.SetUpper(); }
    else if (word.HasUpperAt(0)) { for (auto & match : matches) match.SetUpperAt(0); }

    // Put the match suggestions into the menu.
    for (int i = 0; i < std::ssize(matches); ++i) {
      menu.AddOption('0'+i, "Replace with '" + matches[i].AsANSICyan() + "'",
        [this,new_word=matches[i]](){ DoReplace(new_word, false); return true; });
      menu.AddOption('0'+i+5, "replace ALL instances",
        [this,new_word=matches[i]](){ DoReplace(new_word, true); return true; }, true);
    }
    menu.AddOption('r', "Provide replacement for this instance of '" + word.AsANSICyan() + "'",
      [this](){ QueryReplace(false); return true; });
    menu.AddOption('R', "for ALL instances",
      [this](){ QueryReplace(true); return true; }, true);

    AddDefaultMenuOptions(menu);

    menu.Run();
  }

  // Look at the current token and test if it is allowed, possibly interactively asking user.
  bool TestWordToken() {
    emp::String word(GetLexeme());

    if (IsWordAllowed(word)) return true;

    // See if we already have a replacement set up for this word.
    if (replacement_map.contains(word)) { DoReplace(); return false; }

    // Report unknown word.
    File().ReportIssue(emp::MakeString("Unknown word '", word.AsANSICyan(), "'"));

    // If interactive, allow user to decide how to handle the unknown word.
    if (IsInteractive()) { ManageWord_Interactive(word); }

    return false;
  }

public:
  Empecable(int argc, char * argv[]) : flags(argc, argv) {
    ConfigureSettings();
    SetupOptionFlags();
    flags.Process();

    // Collect the filenames
    emp::vector<emp::String> filenames = flags.GetExtras();
    if (filenames.size() == 0) {
      emp::PrintLn("No files listed.");
      PrintUsage();
      exit(0);
    }

    // Validate the filenames, set up full paths, and save as ReviewFile objects.
    size_t err_count = 0;
    for (const std::string & name : filenames) {
      files.emplace_back(name, mode);
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
      File().Save(); // Will save only if a change has occurred.
    }
    SaveProjectConfig();
    std::cout << "\nTotal Issues = " << total_issues << std::endl;
  }

  ~Empecable() { }

  bool IsVerbose() const { return mode >= Mode::Verbose; }
  bool IsInteractive() const { return mode == Mode::Interactive; }
  size_t GetNumIssues() const { return total_issues; }

  void PrintUsage() const {
    emp::PrintLn("Usage: ", flags[0], " {options ...} files ...");
    emp::PrintLn("Type `", flags[0], " -h` for more detailed help.");
  }

  void PrintVersion() const {
    emp::PrintLn("File formatter version 0.1.");
  }

  void PrintHelp() const {
    PrintVersion();
    emp::PrintLn();
    PrintUsage();
    emp::PrintLn();
    flags.PrintOptions();
    exit(0);
  }

  void ProcessFile() {
    emp::PrintLn("=== File: ", File().GetName().AsANSIBrightCyan(), " ==");

    if (!File().Load(lexer, project_words)) {
      return; // File failed to load.
    }

    while (File().NextToken()) {
      switch (GetToken()) {
        using namespace emplex;
      case Lexer::ID_WORD:
        if (!TestWordToken() && IsVerbose()) emp::PrintRepeatLn('-',79);
        break;
      case Lexer::ID_ERR_END_LINE_WS:
        File().ReportIssue("Extra whitespace at end of line:");
        AskRemoveToken();
        break;
      case Lexer::ID_ERR_WS:
        File().ReportIssue("Illegal whitespace:");
        AskRemoveToken();
        break;
      case Lexer::ID_PRAGMA_ONCE:
        if (File().GetPragmaOnce() > 0) {
          File().ReportIssue("Duplicate `#pragma once` found (original on line ",
                              File().GetPragmaOnce(), "):");
          AskRemoveToken();
        }
        else {
          File().SetPragmaOnce();
        }
      }

    }

    emp::PrintLn(ToBoldRed("=== ", File().GetNumIssues(), " issues found ==="));
    total_issues += File().GetNumIssues();
    if (IsVerbose()) emp::PrintLn();
  }
};

int main(int argc, char * argv[])
{
  class Empecable formatter(argc, argv);
  return formatter.GetNumIssues();
}

// Local settings for Empecable file checker.
// empecable_words: formatter eol
