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
 * Additional TODO:
 * + Ensure that 2 spaces are used for indent levels (esp. after `{` at eol?})
 * + Group and track include files
 * + Ensure include guards or #pragma once (or both) exist.
 * + Ensure no merge conflict markers are in the file (and help with merge?)
 * + Create config files that don't already exist.
 *
 * - Fully configure actions with a code_format.cfg file in .Empirical/
 * - Allow configure overrides in individual files.
 * - Produce a levelization map
 * - Dynamic control over boilerplate (for easy scaling to other projects)
 * - Guided shifting of boilerplate to a new format
 * - Spacing must always shift by 0 or 2 OR somehow align with previous line?
 * - Make sure include files have corresponding test files.
 * - Make sure test files are not empty (or effectively empty)
 * - Better suggestions where 'y' always gives you what Empecable thinks is correct action.
 * - Cleanup words at the bottom of a file, removing those now in main project dictionary.
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
#include "../../include/emp/math/sequence_utils.hpp"
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
  Mode mode = Mode::Normal;

  emp::FlagManager flags;         // Tracker for command-line flags that were set.
  emp::vector<ReviewFile> files;  // Files to be reviewed.
  size_t active_file = 0;         // Which file are we working with?
  size_t total_issues = 0;

  // Lexer information.
  emplex::Lexer lexer;

  fs::path word_file = "word_list.txt";
  fs::path replace_file = "replace_list.txt";
  std::optional<fs::path> emp_dir = std::nullopt; // Put emp_dir here if one is found.

  using word_set_t = std::unordered_set<emp::String>;
  using word_map_t = std::unordered_map<emp::String, emp::String>;
  word_set_t project_words;   // Dictionary of legal words for this project.
  word_set_t skip_words;      // Words to skip over for now.
  word_map_t replacement_map; // Track replacement words to always use.
  word_map_t suggest_map;     // Track replacement words to suggest.

  bool project_changed = false; // Have there been any project-level changes requiring save?


  // === HELPER FUNCTIONS ===

  ReviewFile & File() { return files[active_file]; }
  const ReviewFile & File() const { return files[active_file]; }

  emplex::Token GetToken(size_t pos) const { return File().GetToken(pos); }
  emp::String GetLexeme(size_t pos) const { return File().GetLexeme(pos); }
  size_t GetLineID(size_t pos) const { return File().GetLineID(pos); }

  void SetupOptionFlags() {
    flags.AddGroup("Basic Operation");
    flags.AddOption('a', "all", [this](){ SetAll(true); },
      "Activate ALL fixes (except those explicitly excluded; see below)");
    flags.AddOption('h', "help", [this](){ PrintHelp(); },
      "Get additional information about options.");
    flags.AddOption('i', "interactive", [this](){ mode = Mode::Interactive; },
      "Interactively fix file problems");
    flags.AddOption('r', "replace_file", [this](emp::String filename){ replace_file = filename.str(); },
      "Specify the word file to use for spell checks.");
    flags.AddOption('v', "verbose", [this](){ mode = Mode::Verbose; },
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
      emp::PrintLn("Help info should go here...");
      return true;
    case 'q':
    case 'x':
      emp::PrintLn("Quitting!");
      exit(0);
    case 's':
      if (active_file < files.size()) File().Save();
      else emp::PrintLn("No active file to save.");
      SaveProjectConfig();
      return true;
    case 'Q':
    case 'X':
      emp::PrintLn("Saving and Quitting!");
      if (active_file < files.size()) File().Save();
      else emp::PrintLn("No active file to save.");
      SaveProjectConfig();
      exit(0);
    }
    return false; // Key not used.
  }

  bool TestDefaultKeyOptions(char key, size_t token_pos) {
    if (key == 'v') { emp::PrintLn("----------"); File().PrintTokenRange(token_pos, 5); return true; }
    if (key == 'V') { emp::PrintLn("----------"); File().PrintTokenRange(token_pos, 10); return true; }
    return TestDefaultKeyOptions(key);
  }

  bool AskYesNo(emp::String question, size_t token_pos) {
    emp::PrintLn(question, " (", ToOptionSet("yn"), ")");
    std::cout.flush();
    while (true) {
      char key = emp::GetIOChar();
      switch (key) {
      case 'y': case 'Y': return true;
      case 'n': case 'N': return false;
      default:
        if (!TestDefaultKeyOptions(key, token_pos)) {
          emp::PrintLn("Unknown option ", ToBoldRed("'", key, "'"));
        }
      }
    }
  }

  fs::path MakeEmpiricalDir(fs::path common_path) {
    emp_assert(IsInteractive());

    fs::path option_path = common_path;
    if (!fs::is_directory(option_path)) option_path = option_path.parent_path();
    emp::PrintLn("No .Empirical/ folder could be found in any parent directory.");
    emp::PrintLn("Where should it be created?");
    size_t opt_id = 0;
    while (!option_path.empty() && opt_id < 10 && emp::CanWriteToDirectory(option_path)) {
      emp::PrintLn(ToOption(std::to_string(opt_id)), " - ", option_path);
      ++opt_id;
      if (option_path == option_path.parent_path()) break;
      option_path = option_path.parent_path();
    }
    emp::Print(ToOption("q"), " - Quit.\n");
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
        emp::PrintLn("Creating directory: ", out_path);
        std::filesystem::create_directory(out_path);
        done = true;
        break;
      default:
        used = TestDefaultKeyOptions(key);
      }
      if (!done && !used) {
        emp::PrintLn("Unknown option ", ToBoldRed("'", key, "'"));
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

  void AddProjectWord(emp::String word) {
    emp_assert(!emp::Has(project_words, word), word);
    emp::PrintLn("Added '", word.AsANSICyan(), "' to project dictionary.");
    project_words.insert(word);
    project_changed = true;
  }

  void DoReplace(size_t token_pos, emp::String new_word, bool change_all=false) {
    const emp::String old_word = GetLexeme(token_pos);
    suggest_map[old_word] = new_word;
    if (change_all) replacement_map[old_word] = new_word;
    File().SetLexeme(token_pos, new_word);
    if (IsVerbose()) {
      emp::PrintLn("Line ", GetLineID(token_pos),
              ": Replacing '", old_word.AsANSICyan(),
              "' with '", new_word.AsANSICyan(), "'.");
    }
  }

  // DoReplace with no replacement word uses the replacement_map.
  void DoReplace(size_t token_pos) {
    const emp::String old_word = GetLexeme(token_pos);
    emp_assert(replacement_map.contains(old_word));
    DoReplace(token_pos, replacement_map[old_word]);
  }

  void QueryReplace(size_t token_pos, bool change_all) {
    emp::String word = GetLexeme(token_pos);

    emp::Print("Enter replacement word: ");
    emp::String new_word;
    std::cin >> new_word;

    // Make change to THIS token.
    File().SetLexeme(token_pos, new_word);

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
    if (replacement_map.contains(word)) {
      DoReplace(token_pos);
      return false;           // Indicate that we had an issue, even if we fixed it.
    }

    // We have an unknown word.
    File().ReportIssue(emp::MakeString("Unknown word '", word.AsANSICyan(), "'"), token_pos);

    if (IsInteractive()) {
      emp::String lower_word = word.AsLower();
      emp::Print(ToOption("a"), " - Add '", lower_word.AsANSICyan(), "' to main PROJECT dictionary");
      if (word.HasUpper()) {
        emp::Print(" or ", ToOption("A"), " to add case-sensitive '", word.AsANSICyan(), "'");
      }
      emp::PrintLn();

      emp::Print(ToOption("f"), " - Add '", lower_word.AsANSICyan(), "' to this FILE's dictionary");
      if (word.HasUpper()) {
        emp::Print("  or ", ToOption("F"), " to add case-sensitive '", word.AsANSICyan(), "'");
      }
      emp::PrintLn();

      emp::PrintLn(ToOption("i"), " - Ignore this instance of '", word.AsANSICyan(), "'  or ",
              ToOption("I"), " to ignore ALL instances");

      emp::vector<emp::String> matches = FindWordMatches(word);
      // Use the same case as the word being matched.
      if (word.OnlyUpper()) { for (auto & match : matches) match.SetUpper(); }
      else if (word.HasUpperAt(0)) { for (auto & match : matches) match.SetUpperAt(0); }
      for (size_t i = 0; i < matches.size(); ++i) {
        emp::PrintLn(ToOption(std::to_string(i)), " - Replace with '", matches[i].AsANSICyan(), "'",
                " or ", ToOption(std::to_string(i+5)), " to replace ALL instances");
      }
      if (matches.size() == 0) emp::PrintLn("(no replacement suggestions found)");
      emp::PrintLn(ToOption("r"), " - Provide replacement for this instance of '", word.AsANSICyan(), "' or ",
              ToOption("R"), " for ALL instances");

      bool done = false;
      while (!done) {
        bool change_all = false;  // Shortcut for below.
        char key = emp::GetIOChar();
        switch (key) {
          case 'a': AddProjectWord(lower_word);     done = true; break;
          case 'A': AddProjectWord(word);           done = true; break;
          case 'f': File().AddWord(lower_word);     done = true; break;
          case 'F': File().AddWord(word);           done = true; break;
          case 'r': QueryReplace(token_pos, false); done = true; break;
          case 'R': QueryReplace(token_pos, true);  done = true; break;
          case 'i':
            emp::PrintLn("Skipping '", word.AsANSICyan(), "'!");
            done = true;
            break;
          case 'I':
            emp::PrintLn("Ignoring all instances of '", word.AsANSICyan(), "'!");
            skip_words.insert(word);
            done = true;
            break;
          case '5': change_all = true; [[fallthrough]];
          case '0':
            if (matches.size() > 0) {
              DoReplace(token_pos, matches[0], change_all);
              done = true;
              break;
            } [[fallthrough]];
          case '6': change_all = true; [[fallthrough]];
          case '1':
            if (matches.size() > 1) {
              DoReplace(token_pos, matches[1], change_all);
              done = true;
              break;
            } [[fallthrough]];
          case '7': change_all = true; [[fallthrough]];
          case '2':
            if (matches.size() > 2) {
              DoReplace(token_pos, matches[2], change_all);
              done = true;
              break;
            } [[fallthrough]];
          case '8': change_all = true; [[fallthrough]];
          case '3':
            if (matches.size() > 3) {
              DoReplace(token_pos, matches[3], change_all);
              done = true;
              break;
            } [[fallthrough]];
          case '9': change_all = true; [[fallthrough]];
          case '4':
            if (matches.size() > 4) {
              DoReplace(token_pos, matches[4], change_all);
              done = true;
              break;
            } [[fallthrough]];
          default:
            bool used = TestDefaultKeyOptions(key, token_pos);
            if (!used) emp::PrintLn("Unknown key ", ToBoldRed("'", key, "'"));
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

  Empecable & SetAll(bool _in=true) { checks.SetAll(_in); return *this; }

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
        if (IsInteractive() && AskYesNo("Remove? ", token_pos)) File().ClearLexeme(token_pos);
        found_issue = true;
        break;
      case Lexer::ID_ERR_WS:
        File().ReportIssue("Illegal whitespace:", token_pos);
        if (IsInteractive() && AskYesNo("Remove? ", token_pos)) File().ClearLexeme(token_pos);
        found_issue = true;
        break;
      default:
        break;
      }

      // Skip a line between issues.
      if (found_issue && IsVerbose()) {
        emp::PrintLn("-------------------------------------------------------------------------------");
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


// Special info below for local control over the Empecable file checker.



// Special info below for local control over the Empecable file checker.
// empecable_words: esp eol formatter
