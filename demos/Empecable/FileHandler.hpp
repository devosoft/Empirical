/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file demos/Empecable/FileHandler.hpp
 * @brief Class to manage both config files and analyze files in Empecable.
 */

#pragma once

#ifndef DEMOS_EMPECABLE_FILE_HANDLER_HPP_GUARD
#define DEMOS_EMPECABLE_FILE_HANDLER_HPP_GUARD

#include <filesystem>

#include "../../include/emp/base/assert.hpp"
#include "../../include/emp/io/ascii_utils.hpp"
#include "../../include/emp/io/io_utils.hpp"
#include "../../include/emp/tools/String.hpp"

#include "Levelizer.hpp"
#include "Lexer.hpp"
#include "ReviewFile.hpp"

namespace fs = std::filesystem;

class FileHandler {
private:
  emp::vector<ReviewFile> files;  // Files to be reviewed.
  size_t active_file = 0;         // Which file are we working with?

  fs::path base_path;     // Root directory for project; where .Empirical/ folder is found.
  fs::path emp_path;      // Main directory for config files.

  // Locations of important config files (with default names)
  fs::path config_path = "Empecable.cfg";      // Main config file for project.
  fs::path word_path = "word_list.txt";        // Project-level words
  fs::path replace_path = "replace_list.txt";  // Suggested word replacements

  using word_set_t = std::unordered_set<emp::String>;
  using word_map_t = std::map<emp::String, emp::String>;
  word_set_t project_words;   // Dictionary of legal words for this project.
  word_set_t skip_words;      // Words to skip over for now.
  word_map_t replacement_map; // Track replacement words to always use.
  word_map_t suggest_map;     // Track replacement words to suggest.

  word_set_t guard_names;     // Set of all include guards used so far.

  Mode mode;                    // Output level to use.
  bool project_changed = false; // Have there been any project-level changes requiring save?

  emplex::Lexer lexer;

  // === Helper Functions ===
  // Look for a particular Empecable file, first locally, then in the .Empirical/ directory.
  fs::path ResolvePath(fs::path & in_path, bool required=true) const {
    // If the current path works, keep it.
    if (fs::exists(in_path)) return in_path;

    // Otherwise search in the project (.Empirical) folder.
    fs::path project_path = emp_path / in_path;
    if (fs::exists(project_path)) return in_path = project_path;

    if (required) {
      PrintError("Unable to open file '", in_path, "'.\n");
      exit(1);
    }

    return {}; // return empty path if not found
  }

  // Look for an include file, first relative, then search path if available.
  fs::path IncludePath(const fs::path & target_path, const ReviewFile & call_file, emp::String search_path="") const {
    // Try the path relative to the calling file.
    fs::path relative_path = call_file.GetPath().parent_path() / target_path;
    if (fs::exists(relative_path)) return relative_path;

    // Try other search path options, in order.
    if (search_path.size() > 0) {
      auto paths = search_path.Slice(':');
      for (const emp::String & test_path : paths) {
        fs::path other_path = fs::path(test_path.str()) / target_path;
        if (fs::exists(other_path)) return other_path;
      }
    }

    // Nothing found; return an empty path.
    return {};
  }

  public:
  void ResetActiveFile() { active_file = 0; }
  [[nodiscard]] bool HasActiveFile() const { return active_file < files.size(); }
  bool NextFile() { ++active_file; return HasActiveFile(); }
  [[nodiscard]] ReviewFile & File() { return files[active_file]; }
  [[nodiscard]] const ReviewFile & File() const { return files[active_file]; }

  [[nodiscard]] const fs::path & BasePath() const { return base_path; };
  [[nodiscard]] const fs::path & EMPPath() const { return emp_path; };

  [[nodiscard]] const fs::path & ConfigPath() const { return config_path; }
  void ConfigPath(const std::string & filename) { config_path = filename; }
  [[nodiscard]] std::ifstream ConfigStream() { return std::ifstream(ResolvePath(config_path)); }

  [[nodiscard]] const fs::path & WordPath() const { return word_path; }
  void WordPath(const std::string & filename) { word_path = filename; }

  [[nodiscard]] const fs::path & ReplacePath() const { return replace_path; }
  void ReplacePath(const std::string & filename) { replace_path = filename; }

  [[nodiscard]] fs::path RelativePath(const fs::path & abs_path) const {
    return fs::relative(abs_path, base_path);
  }
  [[nodiscard]] fs::path RelativePath(const ReviewFile & file) const {
    return RelativePath(file.GetPath());
  }

  // Check if a word is currently listed somewhere as a legal word.
  [[nodiscard]] bool HasWord(const emp::String & word) const {
    return project_words.contains(word) || File().HasWord(word) || skip_words.contains(word);
  }

  void AddWord(const emp::String & word) {
    emp_assert(!project_words.contains(word), word);
    emp::PrintLn("Added '", word.AsANSICyan(), "' to project dictionary.");
    project_words.insert(word);
    MarkProjectChange();
  }

  void AddSkipWord(const emp::String & word) {
    emp_assert(!project_words.contains(word), word);
    emp::PrintLn("Added '", word.AsANSICyan(), "' to skip list.");
    skip_words.insert(word);
  }

  [[nodiscard]] const word_set_t & GetProjectWords() const { return project_words; }

  void AddSuggestion(const emp::String & old_word, const emp::String & new_word) {
    suggest_map[old_word] = new_word;
    MarkProjectChange();
  }

  [[nodiscard]] const emp::String & GetSuggestion(const emp::String & old_word,
                                                  const emp::String & default_word="") {
    return suggest_map.contains(old_word) ? suggest_map[old_word] : default_word;
  }

  [[nodiscard]] bool HasReplacement(const emp::String & word) const  {
    return replacement_map.contains(word);
  }

  // Add an automatic replacement for every future occurrence.
  void AddReplacement(const emp::String & old_word, const emp::String & new_word) {
    replacement_map[old_word] = new_word;
    MarkProjectChange();
  }

  [[nodiscard]] const emp::String & GetReplacement(const emp::String old_word) {
    emp_assert(replacement_map.contains(old_word));
    return replacement_map[old_word];
  }

  // Track another include guard.  Return false if it's already used.
  [[nodiscard]] bool AddIncludeGuard(const emp::String & in_name) {
    if (guard_names.contains(in_name)) return false;
    guard_names.insert(in_name);
    return true;
  }

  [[nodiscard]] bool HasProjectChange() const { return project_changed; }
  void MarkProjectChange() { project_changed = true; }

  // Load (and tokenize) the active file; return success.
  bool LoadActiveFile() {
    File().Load(lexer, project_words);
    return File().IsValid();
  }

  // Re-tokenize the active file based on its current state.
  bool RefreshActiveFile() {
    File().UpdateTokenize(lexer);
    return File().IsValid();
  }

  void SaveCurrentFile() {
    if (HasActiveFile()) File().Save();
    else emp::PrintLn("No active file to save.");
  }

  void SaveProjectConfig() {
    if (project_changed) {
      if (mode != Mode::Silent) {
        emp::PrintLn("Saving '", ToFilename(word_path), "'.");
      }

      // Move the project_words to a vector and sort them.
      emp::vector<emp::String> out_words;
      out_words.reserve(project_words.size());
      for (emp::String word : project_words) out_words.emplace_back(word);
      std::sort(out_words.begin(), out_words.end());

      // Print the new file
      std::ofstream file(word_path);
      for (emp::String word : out_words) file << word << '\n';
    } else {
      if (mode != Mode::Silent) {
        emp::PrintLn("No changes need to be saved in '", ToFilename(word_path), "'.");
      }
    }

    if (project_changed && (suggest_map.size() || replacement_map.size())) {
      if (mode != Mode::Silent) {
        emp::PrintLn("Saving '", ToFilename(replace_path), "'.");
      }
      std::ofstream file(replace_path);
      for (auto [from, to] : suggest_map) {
        if (to.HasWhitespace()) continue; // Skip saving any suggestions with multiple words.
        file << from << " " << to << '\n';
      }
      // Also record anything in the replacement map that was NOT in the suggestion map.
      for (auto [from, to] : replacement_map) {
        if (suggest_map.contains(from) || to.HasWhitespace()) continue;
        file << from << " " << to << '\n';
      }
    } else if (mode != Mode::Silent) {
      emp::PrintLn("No suggestions to save.");
    }

    project_changed = false;
  }

  void SaveAll() {
    SaveCurrentFile();
    SaveProjectConfig();
  }

  void MakeEmpiricalDir(fs::path common_path) {
    fs::path option_path = common_path;
    if (!fs::is_directory(option_path)) option_path = option_path.parent_path();

    emp::PrintLn("No config folder found in any parent directory.");

    emp::ANSIOptionMenu menu;
    menu.SetQuestion("Where should .Empirical/ be created?");

    for (size_t opt_id = 0;
        !option_path.empty() && opt_id < 10 && emp::CanWriteToDirectory(option_path);
        ++opt_id) {
      menu.AddOption('0' + opt_id, "create " + option_path.string(), [this,option_path](){
        base_path = option_path;
        emp_path = option_path / ".Empirical";
        emp::PrintLn("Creating directory: ", emp_path);
        std::filesystem::create_directory(emp_path);
        return true;
      });
      if (option_path == option_path.parent_path()) break;
      option_path = option_path.parent_path();
    }
    menu.AddOption('q', "Quit", [](){ exit(0); return false; }).AddAlias('x');
    menu.Run();
  }

  // Find the sub-path that all test files have in common.
  [[nodiscard]] fs::path FindCommonPath() {
    if (files.size() == 0) return {};

    fs::path common_path = files[0].GetPath();
    for (size_t i=1; i < files.size(); ++i) {
      common_path = emp::FindCommonPath(common_path, files[i].GetPath());
    }

    return common_path;
  }

  // Find the base folder for project and make sure we have a .Empirical in it.
  void Init(const emp::vector<emp::String> & filenames, Mode in_mode) {
    mode = in_mode;
    // Validate the filenames, set up full paths, and save as ReviewFile objects.
    size_t err_count = 0;
    for (const std::string & name : filenames) {
      files.emplace_back(name, mode);
      if (!files.back().IsValid()) ++err_count;
    }

    if (err_count) {
      std::cerr << err_count << " errors opening files. Exiting.\n";
      exit(err_count);
    }

    // Identify the common path of all analyzed files; they must be inside the project folder.
    fs::path common_path = FindCommonPath();
    if (mode == Mode::DEBUG) { emp::PrintLn("Found common path: ", common_path); }

    // Back up further, if needed, to find the .Empirical folder.
    auto found_emp = emp::FindFolderInPath(".Empirical", common_path);

    // If the .Empirical/ folder exists, save its location; it not, create it.
    if (found_emp) { emp_path = *found_emp; }
    else { MakeEmpiricalDir(common_path); }

    // Assume that .Empirical is inside of the base folder for the project.
    base_path = emp_path.parent_path();
  }

  void LoadWords() {
    // Load the dictionary of words for spell checking.
    std::ifstream word_stream = std::ifstream(ResolvePath(word_path));
    emp::String word;
    while (std::getline(word_stream, word)) {
      project_words.insert(word);
    }
    if (mode != Mode::Silent) {
      emp::PrintLn("Loaded word file '", ToFilename(word_path), "' with ",
                   project_words.size(), "words.");
    }

    // Load the replacement suggestions based on prior fixes.
    std::ifstream replace_stream = std::ifstream(ResolvePath(replace_path));
    emp::String translation;
    while (replace_stream) {
      std::getline(replace_stream, translation); // Grab entire translation, maybe more than one word.
      word = translation.PopWord();
      suggest_map[word] = translation;
    }

    if (mode != Mode::Silent) {
      emp::PrintLn("Loaded replacement list file: '", ToFilename(replace_path), "' with ",
                   suggest_map.size(), " suggestions.");
    }
  }

  void MapLevels() {
    if (mode != Mode::Silent) {
      emp::PrintLn("Constructing Levelization Map.");
    }
    std::map<emp::String, int> system_include_count;  // System include counts.
    std::map<fs::path, LevelInfo> info_map;           // Info about project file includes.
    
    // Loop through all of the files collecting their include information.
    for (const ReviewFile & file : files) {
      const fs::path path = RelativePath(file);
      LevelInfo & info = info_map[path];
      info.path = path;

      // Update global counts for system includes.
      for (const emp::String & sys_include : file.GetSystemIncludes()) {
        system_include_count[sys_include]++;
      }

      // Update network for local includes.
      for (const emp::String & local_include : file.GetLocalIncludes()) {
        const fs::path inc_path = RelativePath(IncludePath(local_include.str(), file));
        LevelInfo & inc_info = info_map[inc_path];
        inc_info.path = inc_path;
        info.local_includes.insert(&inc_info);
        inc_info.included_from.insert(&info);
      }
    }

    // Now that we have the full network, print out the info for all of the files.
    std::ofstream level_map(emp_path / "level-map.txt");

    // TOC
    level_map << "This file was auto generated by Empecable.  Below are:\n"
                 "- A summary, by level of files and what they include.\n"
                 "- A count of how many times each standard library file is used.\n"
                 "- A detailed layout of each file and how it fits in to the levelization.\n\n";

    level_map << "========================================================================\n\n";

    level_map << "LEVELIZATION SUMMARY:\n";
    int cur_level = 0;
    while (true) {    
      int count = 0;
      for (auto & [path, info] : info_map) {
        if (info.GetLevel() == cur_level) {
          if (count == 0) level_map << "\n=== LEVEL " << cur_level << " ===\n";
          level_map << path.filename().string() << " :";
          for (const auto & inc_info : info.local_includes) {
            level_map << " " << inc_info->path.filename().string()
                      << "(" <<  inc_info->GetLevel() << ")";
          }
          level_map << "\n";
          ++count;
        }
      }
      if (count == 0) break; // Once we have an empty level, we are done.
      ++cur_level;
    }
    
    // Give the gory details about each file type.
    level_map << "\n========================================================================\n\n";
    level_map << "STANDARD LIBRARY SUMMARY:\n\n";

    for (auto & [filename, count] : system_include_count) {
      level_map << filename << " : " << count << " includes.\n";
    }

    // Give the gory details about each file type.
    level_map << "\n========================================================================\n\n";
    level_map << "LEVELIZATION DETAILS:\n\n";

    for (auto & [path, info] : info_map) {
      level_map << "=== " << path << " ===\n"
                << "Level: " << info.GetLevel() << '\n'
                << "Includes: " << info.local_includes.size() << "\n";
      for (const auto & inc_info : info.local_includes) {
        level_map << "    " << inc_info->path << " (level: " <<  inc_info->GetLevel() << ")\n";
      }
      level_map << "Included FROM: " << info.included_from.size() << "\n";
      for (const auto & inc_info : info.included_from) {
        level_map << "    " << inc_info->path << " (level: " <<  inc_info->GetLevel() << ")\n";
      }
      level_map << std::endl;
    }
  }
};

#endif
