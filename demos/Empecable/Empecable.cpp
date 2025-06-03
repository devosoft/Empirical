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
 *  blue    - duplicates of highlighted words
 *
 * Config options:
 * - HeaderExtensions: .hpp|.h|.H|.hh
 * - ProtectHeaders: None|Pragma|Guards|Both
 */

#include <chrono>
#include <filesystem>

#include "../../include/emp/base/assert.hpp"
#include "../../include/emp/base/vector.hpp"
#include "../../include/emp/config/FlagManager.hpp"
#include "../../include/emp/config/SettingsManager.hpp"
#include "../../include/emp/math/sequence_utils.hpp"
#include "../../include/emp/tools/String.hpp"

#include "FileHandler.hpp"
#include "helpers.hpp"
#include "Lexer.hpp"
#include "ReviewFile.hpp"

class Empecable {
private:
  emp::SettingsManager settings;
  emp::String copyright =
    "This file is part of Empirical, https://github.com/devosoft/Empirical\n"
    "Copyright (C) ${year} Michigan State University\n"
    "MIT Software license; see doc/LICENSE.md\n\n"
    "@file ${file_id}\n"
    "@brief [File Description]";
  emp::String header_extensions = ".hpp|.h|.H|.hh";
  emp::String code_extensions = ".cpp|.C|.cc";
  emp::String header_protections = "Pragma|Guards";
  bool spell_check = true;
  bool use_project_dict = true;
  bool use_file_dict = true;
  bool track_replacements = true;
  bool interactive = false;
  bool remove_illegal_chars = true;
  bool map_levels = true;
  Mode mode = Mode::Normal;

  emp::FlagManager flags;         // Tracker for command-line flags that were set.
  FileHandler file_handler;       // Manage all file handing in Empecable.
  size_t total_issues = 0;

  // === HELPER FUNCTIONS ===

  ReviewFile & File() { return file_handler.File(); }
  const ReviewFile & File() const { return file_handler.File(); }

  emplex::Token GetToken() const { return File().GetToken(); }
  emp::String GetLexeme() const { return File().GetLexeme(); }
  size_t GetLineID() const { return File().GetLineID(); }
  emplex::Token GetToken(size_t pos) const { return File().GetToken(pos); }
  emp::String GetLexeme(size_t pos) const { return File().GetLexeme(pos); }
  size_t GetLineID(size_t pos) const { return File().GetLineID(pos); }

  void SetupSettings() {
    settings.AddSetting("Copyright", copyright,
      "Full text of the copyright heading that should be at the top of each file.");

    settings.AddSetting("HeaderExtensions", header_extensions,
      "File extensions to assume are C++ headers.");

    settings.AddSetting("CodeExtensions", code_extensions,
      "File extensions to assume are C++ code files.");

    settings.AddSetting("HeaderProtections", header_protections,
      "Type of protections against multiple includes to use in headers (None|Pragma|Guards|Both)\n"
      "E.g., if you want either type of protection, but not both, use \"Pragma|Guards\"");

    settings.AddSetting("SpellCheck", spell_check,
      "Count spelling mistakes as errors in a file? (On/Off)");

    settings.AddSetting("ProjectDictionary", use_project_dict,
      "Track a master list of legal words in .Empirical/word_list.txt (On/Off)");

    settings.AddSetting("FileDictionary", use_file_dict,
      "Track a local list of legal words in comment at bottom of each file? (On/Off)");

    settings.AddSetting("TrackReplacements", track_replacements,
      "Track word replacements in .Empirical/replace_list.txt for future suggestions? (On/Off)");

    settings.AddSetting("Interactive", interactive,
      "Go into interactive mode by default? (On/Off; Use -i flag to activate from command line)");

    settings.AddSetting("RemoveIllegalChars", remove_illegal_chars,
      "Remove tabs ('\t') and carriage returns ('\r')? (On/Off)");

    settings.AddSetting("MapLevels", map_levels,
      "Create a levelization map of header files include each other? (On/Off)");
  }

  void SetupOptionFlags() {
    flags.AddGroup("Basic Operation");
    flags.AddOption('c', "config-file", [this](emp::String filename){
      file_handler.ConfigPath(filename); },
      "Name of main configuration file (Default: '" + file_handler.ConfigPath().string() + "').");
    flags.AddOption('d', "debug", [this](){ mode = Mode::DEBUG; },
      "Print extra information for debugging problems.");
    flags.AddOption('h', "help", [this](){ PrintHelp(); },
      "Get additional information about options.");
    flags.AddOption('i', "interactive", [this](){ mode = Mode::Interactive; },
      "Interactively fix file problems");
    // -r : recursive
    flags.AddOption('s', "suggest_filename",
      [this](emp::String filename){ file_handler.ReplacePath(filename); },
      "Name of file for replacement suggestions (Default: '" +
      file_handler.ReplacePath().string() + "').");
    flags.AddOption('S', "silent", [this](){ mode = Mode::Silent; },
      "Do not allow any output from program.");
    flags.AddOption('v', "verbose", [this](){ mode = Mode::Verbose; },
      "Provide more detailed output");
    flags.AddOption('w', "word_filename",
      [this](emp::String filename){ file_handler.WordPath(filename); },
      "Name of word file for spell checks (Default: '" + file_handler.WordPath().string() + "').");
  }

  // Check the default key press options that should work from any menu.
  void AddDefaultMenuOptions(emp::ANSIOptionMenu & menu) {
    menu.AddSilent('h', "Help", [&menu](){ menu.PrintHelp(); return false; }).AddAlias('?');
    menu.AddSilent('s', "Save", [this](){ file_handler.SaveAll(); return false; });
    menu.AddSilent('q', "Quit", [this](){ Quit(false); return false; }).AddAlias('x');
    menu.AddSilent('Q', "Save & Quit", [this](){ Quit(true); return false; }).AddAlias('X');
    menu.AddSilent('v', "View code around token (5 lines per side)",
      [this](){ emp::PrintLn("----------"); File().PrintTokenRange(5); return false; });
    menu.AddLinked('V', "10 lines per side",
      [this](){ emp::PrintLn("----------"); File().PrintTokenRange(10); return false; });
  }

  bool AskYesNo(emp::String question) {
    emp::ANSIOptionMenu menu;
    menu.SetQuestion(question + " (" + ToOptionSet("yn") + ")");
    bool result = false;

    AddDefaultMenuOptions(menu);
    menu.AddSilent('y', "Accept suggestion", [&result](){ result=true; return true; });
    menu.AddSilent('n', "Reject suggestion", [&result](){ result=false; return true; });
    menu.Run();

    return result;
  }

  void AskRemoveToken() {
    if (IsInteractive() && AskYesNo("Remove? ")) {
      File().ClearLexeme();
    }
    if (IsVerbose()) { emp::PrintRepeatLn('-',79); }
  }


  void LoadConfig() {
    SetupSettings();     // Set up the available configuration options that users can set.
    SetupOptionFlags();  // Set up the available command-line options.

    // Most flags come AFTER config file (to override); check a few that come first.
    flags.FindAndProcess("config-file");             // Allow overriding of config file name.

    flags.FindAndProcess("silent");
    flags.FindAndProcess("verbose");
    flags.FindAndProcess("interactive");
    flags.FindAndProcess("debug");

    const auto & filenames = flags.ProcessExtras();  // Collect files to analyze.

    // If there are no files, process the other options and then stop.
    if (filenames.size() == 0) {
      flags.Process();
      emp::PrintLn("No files listed.");
      emp::PrintLn("Type `", flags[0], " -h` for help.");
      exit(0);
    }

    file_handler.Init(filenames, mode);        // Set up test files and identify config folder
    settings.Load(file_handler.ConfigPath());  // Load base configuration settings.
    flags.Process();                           // Remaining flags; may override config settings.
    file_handler.LoadWords();                  // Load configuration word lists.
  }

  [[noreturn]] void Quit(bool save_before_quitting) {
    if (save_before_quitting ||
        (file_handler.HasProjectChange() && AskYesNo("Save before quitting?"))) {
      file_handler.SaveAll();
    }

    exit(0);
  }

  void DoReplace(emp::String new_word, bool change_all=false) {
    const emp::String old_word = GetLexeme();
    file_handler.AddSuggestion(old_word, new_word);
    if (change_all) file_handler.AddReplacement(old_word, new_word);
    File().SetLexeme(new_word);
    if (IsVerbose()) {
      if (!change_all) emp::Print("Line ", GetLineID(), ": ");
      emp::Print("Replacing ");
      if (change_all) emp::Print(" all instances of ");
      emp::PrintLn("'", old_word.AsANSICyan(), "' with '", new_word.AsANSICyan(), "'.");
    }
  }

  // DoReplace with no replacement word uses the replacement_map.
  void DoReplace(bool change_all=false) {
    DoReplace(file_handler.GetReplacement(GetLexeme()), change_all);
  }

  void QueryReplace(bool change_all) {
    emp::String new_word = GetInput("Enter replacement word: ");
    DoReplace(new_word, change_all);
  }

  // Search through available dictionaries to try to find misspellings.
  emp::vector<emp::String> FindWordMatches(emp::String target_word, size_t max_size=5) {
    emp_assert(max_size > 0);

    constexpr double max_word_diff = 3.0;

    std::vector<emp::String> matches; // Set of matches to suggest to the user.

    // If the target word already has a suggestion associated with it, make sure that comes first.
    emp::String suggestion = file_handler.GetSuggestion(target_word);

    // Score all of the other words in the dictionary (if they're close enough)
    std::vector<std::pair<double, std::string>> scored;
    const emp::String lower_word = target_word.AsLower();

    for (const emp::String & word : file_handler.GetProjectWords()) {
      // Don't consider words that are too different in length.
      if (std::abs(word.ssize() - target_word.ssize()) >= max_word_diff) continue;

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
    // Words are allowed if they are short (1 or 2 char), are whitelisted, or have
    // their lowercase versions whitelisted.
    return word.size() <= 2 || file_handler.HasWord(word) ||
      (word.HasUpper() && file_handler.HasWord(word.AsLower()));
  }

  // If we have a disallowed word that we need to interactively deal with, do so.
  void ManageWord_Interactive(const emp::String & word) {
    emp::String lower_word = word.AsLower();

    emp::ANSIOptionMenu menu;
    AddDefaultMenuOptions(menu);

    menu.AddOption('a', "Add '" + lower_word.AsANSICyan() + "' to main PROJECT dictionary",
      [this,lower_word](){ file_handler.AddWord(lower_word); return true; });
    if (word.HasUpper()) {
      menu.AddLinked('A', "add case-sensitive '" + word.AsANSICyan() + "'",
        [this,word](){ file_handler.AddWord(word); return true; });
    }

    menu.AddOption('f', "Add '" + lower_word.AsANSICyan() + "' to this FILE's dictionary",
      [this,lower_word](){ File().AddWord(lower_word); return true; });
    if (word.HasUpper()) {
      menu.AddLinked('F', "add case-sensitive '" + word.AsANSICyan() + "'",
        [this,word](){ File().AddWord(word); return true; });
    }

    menu.AddOption('i', "Ignore this instance of '" + word.AsANSICyan() + "'",
      [word](){ emp::PrintLn("Skipping '", word.AsANSICyan(), "'!"); return true; });
    menu.AddLinked('I', "ignore ALL instances",
      [this,word](){ emp::PrintLn("Ignoring all instances of '", word.AsANSICyan(), "'!");
        file_handler.AddSkipWord(word); return true; });

    // Come up with word matches (and keep case the same as the original word)
    emp::vector<emp::String> matches = FindWordMatches(word);
    if (word.OnlyUpper()) { for (auto & match : matches) match.SetUpper(); }
    else if (word.HasUpperAt(0)) { for (auto & match : matches) match.SetUpperAt(0); }

    // Put the match suggestions into the menu.
    for (int i = 0; i < std::ssize(matches); ++i) {
      menu.AddOption('0'+i, "Replace with '" + matches[i].AsANSICyan() + "'",
        [this,new_word=matches[i]](){ DoReplace(new_word, false); return true; });
      menu.AddLinked('0'+i+5, "replace ALL instances",
        [this,new_word=matches[i]](){ DoReplace(new_word, true); return true; });
    }
    menu.AddOption('r', "Provide replacement for this instance of '" + word.AsANSICyan() + "'",
      [this](){ QueryReplace(false); return true; });
    menu.AddLinked('R', "for ALL instances",
      [this](){ QueryReplace(true); return true; });

    menu.AddOption(emp::IOChar::LEFT, "Move to previous token", [this](){
      File().PrevToken();
      emp::PrintLn("===");
      File().PrintTokenRange();
      return false;
    });
    menu.AddLinked(emp::IOChar::RIGHT, "next token.", [this](){
      File().NextToken();
      emp::PrintLn("===");
      File().PrintTokenRange();
      return false;
    });

    menu.Run();
  }

  // Look at the current token and test if it is allowed, possibly interactively asking user.
  bool TestWordToken() {
    emp::String word(GetLexeme());

    if (IsWordAllowed(word)) return true;

    // See if we already have a replacement set up for this word.
    if (file_handler.HasReplacement(word)) { DoReplace(); return false; }

    // Report unknown word.
    File().ReportIssue(emp::MakeString("Unknown word '", word.AsANSICyan(), "'"));

    // If interactive, allow user to decide how to handle the unknown word.
    if (IsInteractive()) { ManageWord_Interactive(word); }

    return false;
  }

public:
  Empecable(int argc, char * argv[]) : flags(argc, argv) {
    // Load all of the configuration files needed to fun Empecable
    LoadConfig();

    // Step through all of the files, processing them.
    for (file_handler.ResetActiveFile(); file_handler.HasActiveFile(); file_handler.NextFile()) {
      ProcessFile();
    }
    file_handler.SaveProjectConfig();
    if (!IsSilent()) std::cout << "\nTotal Issues = " << total_issues << std::endl;
  }

  ~Empecable() { }

  bool IsSilent() const { return mode == Mode::Silent; }
  bool IsVerbose() const { return mode >= Mode::Verbose; }
  bool IsInteractive() const { return mode >= Mode::Interactive; }
  bool IsDebugging() const { return mode >= Mode::DEBUG; }

  size_t GetNumIssues() const { return total_issues; }

  void PrintUsage() const {
    emp::PrintLn("Usage: ", flags[0], " {options ...} files ...");
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

  // To be called after a new file is open to ensure that the file begins correctly.
  // Each file should begin with (in order):
  // - Copyright comment
  // - Description comment
  // - Pragma once (if activated)
  // - Include guards
  // - STD include block
  // - EMP include block
  // - Local include block
  // - namespace opening
  void ValidateFileHeading() {
    // Create a map of specific variables that we might need to substitute in.
    std::unordered_map<emp::String, emp::String> var_map;

    // Each 'file_id' is a path/name relative to the project.
    // For example, this file is: 'demos/Empecable/Empecable.cpp'
    var_map["file_id"] = fs::relative(File().GetPath(), file_handler.FindCommonPath()).string();

    // Default ${year} to the current year.
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm * tm_ptr = std::localtime(&t);
    var_map["year"] = emp::MakeString(tm_ptr->tm_year + 1900);

    using namespace emplex;
    // Delete any blank lines at the beginning of a file.
    size_t cur_pos = 0;
    while (File().GetToken(cur_pos) == Lexer::ID_END_LINE) ++cur_pos;
    if (cur_pos > 0) {
      File().ReportIssue("File begins with ", cur_pos, " blank lines.");
      if (IsInteractive() && AskYesNo("Remove? ")) {
        File().RemoveToken(0, cur_pos);
        cur_pos = 0;
      }
    }

    // Next should be the copyright.
    if (File().GetToken(0) != Lexer::ID_COMMENT_START) {
      File().ReportIssue("No open comment ('/*') found at beginning of file.");
      if (IsInteractive()) {
        emp::String file_copyright = copyright.ReplaceVars(var_map).AsANSIGreen();
        emp::PrintLn(file_copyright);
        File().PrintFront(3);
        if (AskYesNo("Should we insert the copyright block? ")) {
          File().InsertLexeme(0, file_copyright);
        }
      }
      if (IsVerbose()) { emp::PrintRepeatLn('-',79); }
    }
  }

  // Make sure all tokens throughout the body of a file are valid.
  void ValidateFileTokens() {
    using namespace emplex;

    for (File().ResetTokens(); GetToken() != 0; File().NextToken()) {
      switch (GetToken()) {
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
      case emplex::Lexer::ID_EMP_META_START_OLD:
        File().ReportIssue("Old-style EMP meta-data:");
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
  }

  void ProcessFile() {
    using namespace emplex;
    if (!IsSilent()) {
      emp::PrintLn("=== File: ", File().GetName().AsANSIBrightCyan(), " ===");
    }

    if (!file_handler.LoadActiveFile()) {
      ++total_issues;
      return; // File failed to load.
    }

    ValidateFileHeading();
    ValidateFileTokens();

    if (!IsSilent()) {
      emp::PrintLn(ToBoldRed("=== ", File().GetNumIssues(), " issues found ==="));
    }
    total_issues += File().GetNumIssues();
    if (IsVerbose()) emp::PrintLn();

    File().Save();   // Will save only if a change has occurred.
    File().Close();  // Clean up current file.
  }
};

int main(int argc, char * argv[])
{
  Empecable formatter(argc, argv);
  return std::min<int>(formatter.GetNumIssues(), 255);
}

// Local settings for Empecable file checker.
// empecable_words: formatter
