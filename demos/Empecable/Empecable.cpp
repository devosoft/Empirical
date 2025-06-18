/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2024-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file demos/Empecable/Empecable.cpp
 * @brief Load a series of filenames and clean up each file.
 *
 * Features:
 * - Check spelling for the code files (With suggestions, change all, and remembering
 *   changes to use as suggestions in future.)
 * - Remove illegal characters including \r, \t, and end-of-line spaces
 * - Ensure the file ends in a newline
 * - Track any special features about files or projects including custom spelling.
 * - Find the .Empirical/ directory for configurations
 * - Ensure include guards or #pragma once (or both) exist for header files.
 *
 * Some features will also be covered by clang-format
 * - Ensure 2 spaces are used for indent levels.
 * - Group and track include files
 *
 * Additional TODO:
 * + Ensure no merge conflict markers are in the file (and help with merge?)
 * + Create config files that don't already exist.
 * + Fully configure actions with a Empecable.cfg file in .Empirical/
 * + Allow configure overrides in individual files.
 * + Produce a levelization map
 * + Make sure emp/include/ header (.hpp) files have corresponding test files.
 * + Make sure test files are not empty (or effectively empty)
 * + Better suggestions where 'y' always gives you what Empecable thinks is correct action.
 * + remove local words if in project dictionary; remove 'replace' if in either local of project
 *
 * Master list of interface options for consistency.
 *  'a' - Add lowercase word to project dictionary (or 'A' to preserve current case)
 *  'i' - Ignore (or 'I' to Ignore All)
 *  'f' - Add lowercase word to file dictionary (or 'F' to preserve current case)
 *  'h' - Help
 *  'q'/'x' - Quit without saving (or 'Q' to quit and save)
 *  'r' - Replace with (or 'R' to Replace All)
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
    " * This file is part of Empirical, https://github.com/devosoft/Empirical\n"
    " * Copyright (C) ${year} Michigan State University\n"
    " * MIT Software license; see doc/LICENSE.md\n"
    " *\n"
    " * @file ${file_id}\n"
    " * @brief ${brief}\n";
  emp::String header_extensions = ".hpp|.h|.H|.hh";
  emp::String code_extensions = ".cpp|.C|.cc";
  emp::String header_protections = "Pragma|Guards";
  bool spell_check = true;
  bool use_project_dict = true;
  bool use_file_dict = true;
  bool track_replacements = true;
  bool interactive = false;
  bool remove_illegal_chars = true;
  bool map_levels = false;
  Mode mode = Mode::Normal;

  emp::FlagManager flags;         // Tracker for command-line flags that were set.
  FileHandler file_handler;       // Manage all file handing in Empecable.
  size_t total_issues = 0;

  // Settings for working with file headers.
  std::unordered_map<emp::String, emp::String> var_map; // Vars to use in heading
  size_t cur_year;                                      // Numerical year
  emp::String new_heading;                              // Heading for current file

  // Answer tracking (for "Yes" or "No" to ALL type questions).
  std::unordered_map<emp::String, bool> yes_no_answers;

  // === HELPER FUNCTIONS ===

  ReviewFile & File() { return file_handler.File(); }
  const ReviewFile & File() const { return file_handler.File(); }

  template <typename... Ts>
  void ReportIssue(Ts &&... args) {
    File().ReportIssue(std::forward<Ts>(args)...);
  }

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
    flags.AddOption('l', "map-levels", [this](){ map_levels = true; },
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

  bool AskYesNo(emp::String question, emp::String label, bool is_dangerous = false) {
    if (!IsInteractive()) return false; // Cannot ask, so assume false.

    emp::String extra = "";
    if (is_dangerous) {
      extra = "\n" + ToBoldYellow("Warning:")
              + " This action may be dangerous; consider editing by hand.";
    }

    // If we already have an answer locked in, use it.
    if (yes_no_answers.contains(label)) return yes_no_answers[label];

    emp::ANSIOptionMenu menu;
    menu.SetQuestion(question + " (" + ToOptionSet("yn") + ")" + extra);
    bool result = false;

    AddDefaultMenuOptions(menu);
    menu.AddSilent('y', "Accept suggestion", [&result](){ result=true; return true; });
    menu.AddSilent('n', "Reject suggestion", [&result](){ result=false; return true; });
    if (!is_dangerous && label != "") {
      menu.AddSilent('Y', "Accept suggestion for ALL", [this, &label, &result](){
        emp::PrintLn("Responding YES to all instances of this question.");
        yes_no_answers[label] = true;
        result=true;
        return true;
      });
      menu.AddSilent('N', "Reject suggestion for ALL", [this, &label, &result](){
        emp::PrintLn("Responding NO to all instances of this question.");
        yes_no_answers[label] = false;
        result=false;
        return true;
      });
    }
    menu.Run();

    return result;
  }

  void LineBreak() {
    if (IsVerbose()) { emp::PrintRepeatLn('-',79); }
  }

  void AskRemoveToken(emp::String label, bool is_dangerous=false) {
    if (AskYesNo("Remove? ", label, is_dangerous)) { File().ClearLexeme(); }
    LineBreak();
  }

  void AskInsertToken(emp::String lexeme,  emp::String label, bool is_dangerous=false) {
    if (AskYesNo("Insert ''? ", label, is_dangerous)) { File().InsertLexeme(lexeme); }
    LineBreak();
  }

  void TestExpected(const emp::String & expected_lexeme, emp::String test_name) {
    // Check differences other than in whitespace (clang-format will deal with that)
    if (File().GetLexeme().AsRemoveWhitespace() != expected_lexeme.AsRemoveWhitespace()) {
      ReportIssue("Found unexpected ", test_name, " statement/comment."
                  "\n...Found: ", ToBoldRed(File().GetLexeme()),
                  "\nExpected: ", ToBoldGreen(expected_lexeme));
      if (AskYesNo("Swap it?", "swap " + test_name)) { File().SetLexeme(expected_lexeme); }
    }
  }

  void AddHeaderModMenuOption(emp::ANSIOptionMenu & menu, emp::String name, char key,
                              emp::String desc) {
    menu.AddOption(key, desc, [this, name](){
      var_map[name] = GetInput("Enter value for '", name, "': ");
      new_heading = "/**\n" + copyright.AsReplaceVars(var_map)
                    + AdjustCommentStars(var_map["details"]) + " */";
      emp::PrintLn("New heading:");
      emp::PrintLn(new_heading.AsANSIGreen());
      emp::PrintLn("Do you want to insert it?");
      return false;
    });
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
        (file_handler.HasProjectChange() && AskYesNo("Save before quitting?", "quitsave"))) {
      file_handler.SaveAll();
    }

    exit(0);
  }

  void DoReplace(emp::String new_word, bool change_all=false) {
    const emp::String old_word = File().GetLexeme();
    file_handler.AddSuggestion(old_word, new_word);
    if (change_all) file_handler.AddReplacement(old_word, new_word);
    File().SetLexeme(new_word);
    if (IsVerbose()) {
      if (!change_all) emp::Print("Line ", File().GetLineID(), ": ");
      emp::Print("Replacing ");
      if (change_all) emp::Print(" all instances of ");
      emp::PrintLn("'", old_word.AsANSICyan(), "' with '", new_word.AsANSICyan(), "'.");
    }
  }

  // DoReplace with no replacement word uses the replacement_map.
  void DoReplace(bool change_all=false) {
    DoReplace(file_handler.GetReplacement(File().GetLexeme()), change_all);
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
    emp::String word(File().GetLexeme());

    if (IsWordAllowed(word)) return true;

    // See if we already have a replacement set up for this word.
    if (file_handler.HasReplacement(word)) { DoReplace(); return false; }

    // We do not have a record of this word.
    ReportIssue("Unknown word '", word.AsANSICyan(), "'");

    // If interactive, allow user to decide how to handle the unknown word.
    if (IsInteractive()) { ManageWord_Interactive(word); }

    return false;
  }

public:
  Empecable(int argc, char * argv[]) : flags(argc, argv) {
    // Determine the current year.
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm * tm_ptr = std::localtime(&t);
    cur_year = tm_ptr->tm_year + 1900;

    // Load all of the configuration files needed to fun Empecable
    LoadConfig();

    // Step through all of the files, processing them.
    for (file_handler.ResetActiveFile(); file_handler.HasActiveFile(); file_handler.NextFile()) {
      ProcessFile();
    }

    if (map_levels) file_handler.MapLevels();

    file_handler.SaveProjectConfig();
    if (!IsSilent()) emp::PrintLn("\nTotal Issues = ", total_issues);
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

  void ResetVarMap() {
    // Initialize map of specific variables that we might need to substitute in.
    var_map["brief"] = "";
    var_map["details"] = "";
    var_map["year"].Set(cur_year);

    // Each 'file_id' is a path/name relative to the project.
    // For example, this file is: 'demos/Empecable/Empecable.cpp'
    var_map["file_id"] = file_handler.RelativePath(File());

    // Assume no heading to build from unless one is found.
    var_map["old_heading"] = "";
  }

  // Scan through the CURRENT file heading to mine information and return its end token.
  [[nodiscard]] size_t ScanFileHeading() {
    using namespace emplex;

    const size_t start_pos = File().GetTokenPos();

    // Make sure the initial comment block is good.
    size_t end_pos = File().FindTokenPos(Lexer::ID_COMMENT_END, start_pos);
    if (end_pos == ReviewFile::npos) {
      ReportIssue("No close comment");
      return start_pos;
    }
    const size_t brief_pos = File().FindTokenPos(Lexer::ID_DOX_BRIEF, start_pos);

    // If there is a follow-up comment block with an @brief, include it too.
    if (brief_pos > end_pos) {
      // If there is an "@brief" and its after this comment, check if another comment follows.
      if (File().GetToken(end_pos+1) == Lexer::ID_END_LINE &&
          File().GetToken(end_pos+2) == Lexer::ID_COMMENT_START) {
        end_pos = File().FindTokenPos(Lexer::ID_COMMENT_END, end_pos+2);
        if (end_pos == ReviewFile::npos) {
          ReportIssue("No close comment");
          return start_pos;
        }
      }
    }

    // Now that we know the start and end of the comment blocks, scan for a year
    // or year range.
    for (size_t pos = start_pos; pos < end_pos; ++pos) {
      if (File().GetToken(pos) == Lexer::ID_NUM) {
        const size_t start_year = File().GetLexeme(pos).AsULL();
        // Test if we have a likely candidate for year!
        if (start_year > 2000 && start_year <= cur_year) {
          // Now test if it's a range of years.
          if (File().GetLexeme(pos+1) == "-" && File().GetToken(pos+2) == Lexer::ID_NUM) {
            size_t end_year = File().GetLexeme(pos+2).AsULL();
            if (end_year < 100) end_year += 2000; // E.g., make 2013-15 into 2013-2015
            var_map["year"].Set(start_year, '-', end_year);
          } else {
            var_map["year"].Set(start_year);
          }

          break;  // We have a year; exit the for loop.
        }
      }
    }

    // Set the @brief description, if available.
    if (brief_pos < end_pos) {
      var_map["brief"] = File().GetLineFrom(brief_pos+2);

      // Save everything after @brief as "details".
      size_t brief_end_pos = File().FindTokenPos(Lexer::ID_END_LINE, brief_pos+2);

      size_t detail_start_line = File().GetLineID(brief_end_pos+1);
      size_t detail_end_line = File().GetLineID(end_pos)-1;

      var_map["details"] = File().GetLines(detail_start_line, detail_end_line);
    }

    return end_pos;
  }

  // Make sure the file doesn't begin with any blank lines.
  // Return new start token position.
  void ValidateFileHeading_RemoveLeadingNewlines() {
    // Offer to delete blank lines at the beginning of a file.
    while (File().GetToken() == emplex::Lexer::ID_END_LINE) File().NextToken();
    if (!File().AtFront()) {
      ReportIssue("File begins with ", File().GetTokenPos(), " blank lines.");
      if (AskYesNo("Remove? ", "remove_leading_endlines")) {
        File().RemoveToken(0, File().GetTokenPos());
        File().ResetTokens();
      }
      LineBreak();
    }
  }

  void ValidateFileHeading_UpdateCopyrightComment() {
    ResetVarMap();  // Prepare the var_map for the current file.
    if (File().GetToken() == emplex::Lexer::ID_COMMENT_START) {
      size_t copyright_end_pos = ScanFileHeading();
      var_map["old_heading"] = File().GetRangeToPos(copyright_end_pos+1);
      File().SetTokenPos(copyright_end_pos+1);
    } else {
      ReportIssue("No file heading (with copyright) found!");
    }

    // Generate what the copyright SHOULD look like.
    new_heading = "/**\n" + copyright.AsReplaceVars(var_map)
      + AdjustCommentStars(var_map["details"]) + " */";

    if (IsInteractive()) {
      if (var_map["old_heading"] == "") { // No old heading!
        emp::PrintLn("Do you want to insert this suggested heading?");
        emp::PrintLn(new_heading.AsANSIGreen());

        emp::ANSIOptionMenu menu;
        AddDefaultMenuOptions(menu);

        AddHeaderModMenuOption(menu, "brief", 'b', "Update brief description.");
        AddHeaderModMenuOption(menu, "year", 'd', "Update date.");

        menu.AddOption('n', "Ignore suggestion and move on.", [](){
          emp::PrintLn("Leaving file without a heading.");
          return true;
        });
        menu.AddOption('y', "Insert suggestion.", [this](){
          emp::PrintLn("Inserting heading!");
          File().InsertLexeme(0, new_heading+"\n\n");
          return true;
        });

        menu.Run();
      }

      // If these IS a heading, we need to compare to it.
      else if (var_map["old_heading"] != new_heading) {
        emp::PrintLn("Current file heading:\n", var_map["old_heading"].AsANSIRed());
        emp::PrintLn("Suggested file heading:\n", new_heading.AsANSIGreen());

        emp::PrintLn("Do you want to do the suggested header replacement?");

        emp::ANSIOptionMenu menu;
        AddDefaultMenuOptions(menu);

        AddHeaderModMenuOption(menu, "brief", 'b', "Update brief description.");
        AddHeaderModMenuOption(menu, "year", 'd', "Update date.");

        menu.AddOption('n', "Ignore suggestion and move on.", [](){
          emp::PrintLn("Leaving file with previous heading.");
          return true;
        });
        menu.AddOption('y', "Replace with suggestion.", [this](){
          emp::PrintLn("Replacing heading!");
          File().RemoveToken(0, File().GetTokenPos());
          File().InsertLexeme(0, new_heading);
          return true;
        });

        menu.Run();
      }

    }
  }

  emp::String GenerateGuardName() {
    emp::String guard_name = file_handler.RelativePath(File()).string();
    guard_name.SetPascalToCaps();                            // Change to ALL_CAPS

    // If this filename begins with '_', assume it is an implementation file.
    if (File().GetPath().filename().string()[0] == '_') {
      guard_name += "_impl"; // This will be the only lowercase portion of the guard name.
    }

    guard_name.ReplaceSet(!emp::AlphanumericCharSet(), '_')  // Change all punctuation to '_'
              .Append("_GUARD")                              // Make name end in GUARD
              .Compress('_');                                // Compress multiple '_' to just one
    return guard_name;
  }

  // Helper for recording include guard names (to prevent duplicates)
  void RecordGuardName(const emp::String & guard_name) {
    bool ok = file_handler.AddIncludeGuard(guard_name);

    if (!ok) {
      ReportIssue("Include guard '", ToBoldGreen(guard_name), "' already used!");
    }
  }

  // Make sure the appropriate include guards / pragma once are in place.
  void ValidateFileHeading_UpdateGuards() {
    using namespace emplex;

    // If this file is not a header, do not worry about guards.
    if (File().GetPath().extension() != ".hpp") { return; }

    // Check for #pragma once
    File().SkipNewLines();
    if (File().GetToken() != Lexer::ID_PRAGMA_ONCE) {
      ReportIssue("Did not find '#pragma once' after heading.");
      AskInsertToken("#pragma once\n\n", "insert #pragma once");
    } else {
      File().NextToken();
    }

    // Check for include guards.
    File().SkipNewLines();
    emp::String guard_name = GenerateGuardName();
    if (File().GetToken() != Lexer::ID_PP_IFNDEF) {
      ReportIssue("Did not find include guard");
      if (AskYesNo("Insert guard '" + ToBoldGreen(guard_name) + "'?", "insert_guards")) {
        File().InsertLexeme("#ifndef " + guard_name + "\n#define " + guard_name + "\n\n");
        File().InsertBack("\n#endif // #ifndef" + guard_name + "\n");
        RecordGuardName(guard_name);
      }
      LineBreak();
    }
    else {  // There ARE existing include guards.
      // Determine the current guard.
      emp::String cur_guard = File().GetLexeme();
      cur_guard.PopWord();

      // If guard is different from expected, offer to change it.
      if (cur_guard != guard_name) {
        ReportIssue("Mis-named include guard."
                    "\n...Found: ", ToBoldRed(cur_guard),
                    "\nExpected: ", ToBoldGreen(guard_name));
        if (AskYesNo("Swap it?", "swap_guards")) {
          File().SetLexeme("#ifndef " + guard_name);

          // Since we are changing the include guard, also change the #define on the next line.
          File().NextToken();
          File().SkipNewLines();
          if (File().GetToken() != Lexer::ID_PP_DEFINE) {
            ReportIssue("Missing #define in include guard.");
            AskInsertToken("#define " + guard_name + "\n\n", "insert_guard_define", true);
          } else {
            File().SetLexeme("#define " + guard_name);
          }
  
          RecordGuardName(guard_name); // Changed to suggested name.
        }
        else RecordGuardName(cur_guard); // Choose to not change; record current name.

      }
      else RecordGuardName(guard_name); // Name was already correct.
    }

  }

  void ValidateFileHeading() {
    using namespace emplex;

    File().ResetTokens();
    ValidateFileHeading_RemoveLeadingNewlines();
    ValidateFileHeading_UpdateCopyrightComment();
    ValidateFileHeading_UpdateGuards();
  }

  // Make sure all tokens throughout the body of a file are valid.
  void ValidateFileTokens() {
    using namespace emplex;
    file_handler.RefreshActiveFile();  // Incorporate all previous changes.

    // Track if's and ends in the pre-processor to provide useful comments.
    emp::vector<Token> pp_stack;

    for (File().ResetTokens(); File().GetToken() != 0; File().NextToken()) {
      switch (File().GetToken()) {
      case Lexer::ID_WORD:
        if (!TestWordToken()) LineBreak();
        break;
      case Lexer::ID_ERR_END_LINE_WS:
        ReportIssue("Extra whitespace at end of line:");
        AskRemoveToken("end-of-line spaces");
        break;
      case Lexer::ID_ERR_WS:
        ReportIssue("Illegal whitespace:");
        AskRemoveToken("illegal whitespace");
        break;
      case Lexer::ID_GIT_MERGE_CONFLICT:
        ReportIssue("Git merge conflict marker");
        if (IsVerbose()) emp::PrintLn("...Please resolve outside of Empecable (for now).");
        break;
      case Lexer::ID_EMP_META_START_OLD:
        ReportIssue("Old-style EMP meta-data:");
        AskRemoveToken("old meta data");
        break;
      case Lexer::ID_PP_IF:
      case Lexer::ID_PP_IFDEF:
      case Lexer::ID_PP_IFNDEF:
        pp_stack.push_back(File().GetToken());
        break;

      case Lexer::ID_PP_ELSE:
      case Lexer::ID_PP_ELIF:
      case Lexer::ID_PP_ELIFDEF:
      case Lexer::ID_PP_ELIFNDEF:
      case Lexer::ID_PP_ENDIF:
      {
        PPLine line(File().GetLexeme());

        // Make sure we are continuing an if-chain
        if (pp_stack.size() == 0) {
          ReportIssue(line.command, " found without opening #if* or #el*");
          AskRemoveToken(line.command + " with no #if*", true);
          break;
        }

        // Construct what this line SHOULD look like and test it.
        line.ChainComment(pp_stack.back().lexeme);
        TestExpected(line.AsLexeme(), line.command);

        // Replace the top of the stack with a new message.
        pp_stack.pop_back();
        if (line.command != "#endif") pp_stack.push_back(File().GetToken());
        break;
      }

      case Lexer::ID_PRAGMA_ONCE:
        if (File().GetPragmaOnce() > 0) {
          ReportIssue("Duplicate `#pragma once` found (original on line ",
                      File().GetPragmaOnce(), "):");
          AskRemoveToken("duplicate #pragma once");
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
      return; // File failed to load file.
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
