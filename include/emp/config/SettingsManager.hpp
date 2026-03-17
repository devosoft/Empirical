/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025-2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/config/SettingsManager.hpp
 * @brief A flexible manager for named configuration settings with file I/O.
 * @note An older version of this class became SettingCombos.hpp
 * @note Status: Alpha
 *
 * ## Overview
 *
 * `emp::SettingsManager` maintains a collection of named settings, each bound
 * directly to an external variable.  When a setting is loaded from a file or
 * set programmatically, the bound variable is updated automatically via a
 * stored action callback.
 *
 * Settings can be organized into **scopes** using dot-notation names
 * (e.g. `"robot1.speed"`).  This makes it straightforward to configure
 * multiple objects of the same type without name collisions.  Internally
 * all settings are stored with their full dotted key.
 *
 * Two kinds of identifiers are supported:
 *
 *  - **Settings** – named variables with a fixed type and a single value.
 *    Supported types are `emp::String`, `bool`, `int`, `size_t`, and `double`.
 *
 *  - **Keywords** – special command words that trigger an arbitrary callback
 *    and receive the remaining tokens on the line as arguments.  They are
 *    useful for directives that do not follow the `name = value` pattern
 *    (e.g. `include other_file.cfg;`).  Keywords are always global; they are
 *    not affected by the current scope.
 *
 * ## Config-file format
 *
 * Files are parsed line-by-line.  Each line has one of the following forms:
 *
 * ```
 * # This is a comment (ignored)
 *
 * setting_name = value;          # assign a literal value to a setting
 * setting_name = other_name;     # copy the current value of another setting
 * keyword arg1 arg2 ...;         # invoke a keyword with zero or more arguments
 *
 * scope.setting = value;         # dot-notation: assign a scoped setting directly
 *
 * scope_name {                   # brace-block: enter a scope
 *   setting = value;             #   all names inside are prefixed with scope_name
 *   inner { setting = value; }   #   scopes may nest arbitrarily
 * }
 * ```
 *
 * Dot notation and brace-block notation may be freely mixed in the same file.
 * A trailing `\` at the end of a line continues onto the next line (the
 * newline is ignored).  Booleans accept `On`/`Off`/`True`/`False`/`1`/`0`
 * (case-insensitive).  Strings may be bare identifiers or double-quoted
 * literals with standard C escape sequences.  Lines may be terminated by `;`
 * or a newline; blank lines and comment-only lines are skipped.
 *
 * ## Basic usage
 *
 * ```cpp
 * std::string name = "World";
 * size_t      reps = 10;
 * bool        verbose = false;
 * int         r1_speed = 0, r2_speed = 0;
 *
 * emp::SettingsManager cfg;
 * cfg.AddSetting("name",         name,     "Name to greet",      'n', "name");
 * cfg.AddSetting("reps",         reps,     "Number of repeats",  'r', "reps");
 * cfg.AddSetting("verbose",      verbose,  "Enable verbose mode",'v', "verbose");
 * cfg.AddSetting("robot1.speed", r1_speed, "Robot 1 speed");
 * cfg.AddSetting("robot2.speed", r2_speed, "Robot 2 speed");
 *
 * cfg.Load("my_config.cfg");    // updates variables from file
 * cfg.Save("my_config.cfg");    // writes current values with description comments
 *
 * // Apply settings from command-line arguments (e.g. -s "reps = 5")
 * emp::vector<emp::String> args(argv, argv + argc);
 * cfg.LoadArgs(args);
 * ```
 *
 * ## Key methods
 *
 *  - `AddSetting(name, var, desc [, flag [, option]])` – register a setting
 *    bound to `var`; `flag` is a one-character CLI flag (optional),
 *    `option` a long-form name (optional).  Returns `*this` for chaining.
 *  - `AddKeyword(keyword, fun, desc)` – register a keyword that invokes `fun`
 *    with its argument tokens.  Returns `*this` for chaining.
 *  - `Load(istream&)` / `Load(filename)` – parse settings; returns `false` on
 *    error.  Multiple calls accumulate; later values override earlier ones.
 *  - `LoadArgs(args [, erase_on_use])` – scan a `vector<emp::String>` of
 *    command-line arguments.  Per-setting short flags (`-x val`) and long
 *    options (`--opt val`) registered via `AddSetting` are applied directly;
 *    `-s`/`--set` applies the next argument as a bulk config string.  When
 *    `erase_on_use` is `true` each matched flag and its value are removed.
 *  - `Save(ostream&)` / `Save(filename)` – write all settings as a commented
 *    config file; returns `false` on error.
 *  - `Get<T>(name)` / `Set(name, value)` – programmatic get/set.
 *  - `HasSetting(name)` / `HasKeyword(name)` / `HasIdentifier(name)` – query
 *    whether a name is registered.
 *  - `HasError()` / `GetError()` – inspect the last error message.
 *  - `SetVerbose()` – enable diagnostic printing during Load/Save.
 *
 * DEVELOPER NOTES:
 * - Consider allowing types to be more dynamic, perhaps set in a template.
 */

#pragma once

#ifndef INCLUDE_EMP_CONFIG_SETTINGS_MANAGER_HPP_GUARD
#define INCLUDE_EMP_CONFIG_SETTINGS_MANAGER_HPP_GUARD

#include <fstream>
#include <functional>
#include <map>
#include <optional>
#include <stddef.h>
#include <variant>

#include "../base/notify.hpp"
#include "../base/vector.hpp"
#include "../compiler/Lexer.hpp"
#include "../io/io_utils.hpp"
#include "../tools/String.hpp"

namespace emp {

  class SettingsManager {
  private:
    using val_t = std::variant<emp::String, bool, int, size_t, double>;
    using keyword_fun_arg_t = emp::vector<emp::String>;
    using keyword_fun_t = std::function<void(keyword_fun_arg_t)>;
    using Iterator = emp::TokenStream::Iterator;

    /// Class to manage a single setting.
    class SettingInfo {
    private:
      emp::String name;           ///< Label for this setting in config files
      val_t value;                ///< Current value of this setting.
      emp::String desc   = "";    ///< Description of setting
      char flag          = '\0';  ///< Command-line flag ('\0' for none)
      emp::String option = "";    ///< Command-line longer option ("" for none)
      std::function<void(const SettingInfo &)> action;  ///< Action to take when set.

      // Helper: convert from string to val_t of current type.
      val_t ParseFromString(emp::String str) const {
        if (IsString()) {
          if (str.size() && str[0] == '\"') { return emp::MakeStringFromLiteral(str); }
          else { return str; }
        }
        if (IsBool()) {
          str.SetLower();
          return str == "on" || str == "true" || str == "1";
        }
        if (IsInt()) { return std::stoi(str); }
        if (IsULL()) { return static_cast<size_t>(std::stoull(str)); }
        if (IsDouble()) { return std::stod(str); }
        emp_assert(false, "Invalid type in Setting Info", name);
        return 0.0;
      }

    public:
      SettingInfo()                    = delete;
      SettingInfo(const SettingInfo &) = default;
      SettingInfo(SettingInfo &&)      = default;

      template <typename T>
      SettingInfo(emp::String name,
                  T & var,
                  emp::String desc,
                  char flag          = '\0',
                  emp::String option = "")
        : name(name), value(var), desc(desc), flag(flag), option(option)
        , action([&var](const SettingInfo & info) { var = info.GetValue<T>(); }) {}

      template <typename T>
      [[nodiscard]] bool IsType() const {
        return std::holds_alternative<T>(value);
      }

      template <typename T>
      [[nodiscard]] bool IsUsableType() const {
        return IsType<std::remove_cvref_t<T>>();
      }

      [[nodiscard]] bool IsString() const { return IsType<emp::String>(); }
      [[nodiscard]] bool IsBool() const { return IsType<bool>(); }
      [[nodiscard]] bool IsInt() const { return IsType<int>(); }
      [[nodiscard]] bool IsULL() const { return IsType<size_t>(); }
      [[nodiscard]] bool IsDouble() const { return IsType<double>(); }

      [[nodiscard]] const emp::String & GetName() const { return name; }
      [[nodiscard]] const emp::String & GetDescription() const { return desc; }
      [[nodiscard]] char GetFlag() const { return flag; }
      [[nodiscard]] const emp::String & GetOption() const { return option; }

      template <typename T>
      [[nodiscard]] const T & GetValue() const {
        return std::get<T>(value);
      }

      /// Set the value; must maintain current type.
      template <typename T>
      void SetValue(T && val) {
        emp_assert(IsUsableType<T>(), name);
        value = std::forward<T>(val);
        if (action) { action(*this); }
      }

      /// If input is a string, convert it to the correct type.
      void SetFromString(const emp::String & input) {
        value = ParseFromString(input);
        if (action) { action(*this); }
      }

      /// Convert to string for display
      [[nodiscard]] emp::String AsString() const {
        if (IsString()) { return std::get<emp::String>(value); }
        if (IsBool()) { return std::get<bool>(value) ? "1" : "0"; }
        if (IsInt()) { return emp::MakeString(std::get<int>(value)); }
        if (IsULL()) { return emp::MakeString(std::get<size_t>(value)); }
        if (IsDouble()) { return emp::MakeString(std::get<double>(value)); }
        emp_assert(false, "Invalid type in Setting Info", name);
        return "";
      }

      [[nodiscard]] emp::String AsLiteral() const {
        if (IsString()) { return std::get<emp::String>(value).AsLiteral(); }
        if (IsBool()) { return std::get<bool>(value) ? "On" : "Off"; }
        if (IsInt()) { return emp::MakeString(std::get<int>(value)); }
        if (IsULL()) { return emp::MakeString(std::get<size_t>(value)); }
        if (IsDouble()) { return emp::MakeString(std::get<double>(value)); }
        emp_assert(false, "Invalid type in Setting Info", name);
        return "";
      }

      /// Check the type (return std::string for compatibility with type manager)
      [[nodiscard]] std::string GetTypeName() const {
        if (IsString()) { return "emp::String"; }
        if (IsBool()) { return "bool"; }
        if (IsInt()) { return "int"; }
        if (IsULL()) { return "size_t"; }
        if (IsDouble()) { return "double"; }
        return "error";
      }
    }; // END OF SettingInfo definition

    struct KeywordInfo {
      emp::String name;           ///< Label for this keyword in config files
      keyword_fun_t fun;          ///< Function to call when keyword is triggered
      emp::String desc   = "";    ///< Description of keyword
    };

    // === MEMBER VARIABLES ===

    std::map<emp::String, SettingInfo> setting_map;
    std::map<emp::String, KeywordInfo> keyword_map;
    std::map<char, emp::String>        flag_map;    ///< Short flag char  -> full setting name
    std::map<emp::String, emp::String> option_map;  ///< Long option name -> full setting name
    emp::vector<emp::String> cur_scopes{};
    emp::String error_note;
    bool verbose = false;

    // Build the lexer to load the file.
    Lexer lexer;
    const int bool_value_ID;
    const int ident_ID;
    const int int_ID;
    const int double_ID;
    const int string_ID;

    // === HELPER FUNCTIONS ===

    void PushScope(const emp::String & scope) {
      emp_assert(scope.size() > 0);
      cur_scopes.push_back(scope);
    }

    emp::String PopScope() {
      emp_assert(cur_scopes.size() > 0);
      emp::String back = std::move(cur_scopes.back());
      cur_scopes.pop_back();
      return back;
    }

    emp::String AppendScope(const emp::String & name) const {
      emp_assert(name.size() > 0);
      emp::String out;
      for (const emp::String & scope : cur_scopes) {
        out += scope + '.';
      }
      out += name;
      return out;
    }

    auto & GetSettingInfo(this auto & self, const emp::String & name) {
      emp_assert(self.HasSetting(name), "Invalid setting name", name);
      return self.setting_map.find(self.AppendScope(name))->second;
    }

    auto & GetKeywordInfo(this auto & self, const emp::String & name) {
      emp_assert(self.HasKeyword(name), "Invalid setting name", name);
      return self.keyword_map.find(name)->second;
    }

    // === Direct parsing helpers ===

    [[nodiscard]] bool IsEndLine(int id) const { return id == ';' || id == '\n'; }

    /// Trigger an error that prints of verbose and stores an error note.
    bool IOError(auto... args) {
      error_note = emp::MakeString(args...);
      if (verbose) emp::PrintLn(error_note);
      return false;
    }

    /// Use the next token if it's the right type; return false if not.
    [[nodiscard]] bool RequireToken(Iterator & it, int token_id, const emp::String & name) {
      const Token cur_token = it.Use();
      if (cur_token != token_id) {
        return IOError("UnexpectedToken '", cur_token.lexeme, "'; expected ", name, ".");
      }

      return true;
    }

    /// We found a keyword during a load; load all arguments and trigger it.
    [[nodiscard]] bool LoadKeyword(Iterator & it, const emp::String & keyword) {
      if (verbose) emp::PrintLn("...identified as keyword!");
      // Grab the rest of the line.
      emp::vector<emp::String> keyword_vars;
      while (it.IsValid() && !(IsEndLine(it.Peek()))) {
        keyword_vars.push_back(it.Use().lexeme);
      }
      GetKeywordInfo(keyword).fun(keyword_vars);
      return true;
    }

    std::optional<emp::String> TokenToStringValue(const Token & token) {
      // If literal value, use it directly.
      if (token.IsOneOf(bool_value_ID, int_ID, double_ID, string_ID)) {
        return token.lexeme;
      }

      // If identifier, look it up.
      if (token == ident_ID) {
        if (HasSetting(token.lexeme)) {
          return GetSettingInfo(token.lexeme).AsLiteral();
        }
        IOError("Identifier '", token.lexeme, "' UNKNOWN!");
        return std::nullopt;
      }

      // If we made it this far, we don't know how to do the conversion.
      IOError("UnexpectedToken '", token.lexeme, "'; expected value.");
      return std::nullopt;
    }

    // We have found a setting name at the beginning of a line; load it!
    [[nodiscard]] bool LoadSetting(Iterator & it, const emp::String & name) {
      if (verbose) emp::PrintLn("...identified as setting!");

      // setting name must be followed by an '='
      if (!RequireToken(it, '=', "assignment")) return false;

      std::optional<emp::String> string_val = TokenToStringValue(it.Use());
      if (string_val) {
        GetSettingInfo(name).SetFromString(*string_val);
        return true;
      }
      return false;
    }

    [[nodiscard]] bool LoadScope(Iterator & it, const emp::String & name) {
      // Scopes must be opened with a '{'
      if (!RequireToken(it, '{', "open scope")) return false;
      PushScope(name);

      while (it.Any() && it.Peek() != '}') {
        if (!LoadLine(it)) {
          PopScope();
          return false;
        }
      }

      PopScope();
      if (!RequireToken(it, '}', "close scope")) return false;

      return true;
    }

    // Load a single line starting from the current token iterator.
    bool LoadLine(Iterator & it) {
      // Skip any extra lines.
      if (IsEndLine(it.Peek())) { ++it; return true; }

      const Token name_token = it.Use();
      if (verbose) emp::PrintLn("Found initial line token '", name_token.lexeme, "'.");

      if (name_token != ident_ID) {
        return IOError("UnexpectedToken '", name_token.lexeme, "'; expected keyword or parameter name.");
      }
      const emp::String name = name_token.lexeme;
      if (HasKeyword(name)) { return LoadKeyword(it, name); }
      if (HasSetting(name)) { return LoadSetting(it, name); }
      return LoadScope(it, name); // Unknown id must be a new scope.
    }

  public:
    SettingsManager()
      : bool_value_ID(lexer.AddToken("bool_val", "[Oo][Nn]|[Tt][Rr][Uu][Ee]|[Oo][Ff][Ff]|[Ff][Aa][Ll][Ss][Ee]"))
      , ident_ID(lexer.AddToken("identifier", "[a-zA-Z_][a-zA-Z0-9_.]*"))
      , int_ID(lexer.AddToken("int", "[0-9]+"))
      , double_ID(lexer.AddToken("double", "[0-9]+\\.[0-9]+"))
      , string_ID(lexer.AddToken("string", "(\\\"([^\"\\\\]|(\\\\.))*\\\")"))
    {
      lexer.IgnoreToken("whitespace", "[ \\t\\r]+");
      lexer.IgnoreToken("comment", "#.+");
      lexer.IgnoreToken("continue_line", "\\\\[ ]*\\n");
    }

    void SetVerbose(bool in=true) { verbose = in; }

    [[nodiscard]] bool HasSetting(const emp::String & name) const {
      return setting_map.contains(AppendScope(name));
    }

    [[nodiscard]] bool HasKeyword(const emp::String & name) const {
      return keyword_map.contains(name);
    }

    [[nodiscard]] bool HasIdentifier(const emp::String & name) const {
      return HasSetting(name) || HasKeyword(name);
    }

    template <typename T>
    [[nodiscard]] T Get(const emp::String & name) const {
      return GetSettingInfo(name).GetValue<T>();
    }

    [[nodiscard]] const emp::String & GetDesc(const emp::String & name) const {
      return GetSettingInfo(name).GetDescription();
    }

    [[nodiscard]] char GetFlag(const emp::String & name) const { return GetSettingInfo(name).GetFlag(); }

    [[nodiscard]] const emp::String & GetOption(const emp::String & name) const {
      return GetSettingInfo(name).GetOption();
    }

    template <typename T>
    void Set(const emp::String & name, T && value) {
      GetSettingInfo(name).SetValue(std::forward<T>(value));
    }

    [[nodiscard]] bool HasError() const { return error_note.size(); }

    [[nodiscard]] const emp::String & GetError() const { return error_note; }

    template <typename T>
    SettingsManager & AddSetting(const emp::String & name,
                                 T & value,
                                 emp::String desc,
                                 char flag          = '\0',
                                 emp::String option = "") {
      emp_assert(!HasSetting(name), "Trying to add a SettingsManager setting that already exists",
                 AppendScope(name));
      emp_assert(flag == '\0' || !flag_map.contains(flag),
                 "Duplicate CLI flag in SettingsManager", flag);
      emp_assert(option.empty() || !option_map.contains(option),
                 "Duplicate CLI option in SettingsManager", option);
      const emp::String full_name = AppendScope(name);
      setting_map.emplace(full_name, SettingInfo{full_name, value, desc, flag, option});
      if (flag != '\0')    flag_map[flag]     = full_name;
      if (!option.empty()) option_map[option] = full_name;
      return *this;
    }

    /// A specified keyword can be added to the settings; it will call a provided
    /// function with the remaining tokens before the next semicolon.
    SettingsManager & AddKeyword(const emp::String & keyword,
                                 keyword_fun_t fun,
                                 emp::String desc) {
      emp_assert(!HasKeyword(keyword), "Trying to add a keyword that already exists", keyword);
      keyword_map.emplace(keyword, KeywordInfo{keyword, fun, desc});
      return *this;
    }

    bool Save(std::ostream & ofs) {
      emp_assert(ofs);
      error_note.clear();

      for (const auto & [key, info] : setting_map) {
        const auto lines = info.GetDescription().Slice("\n");
        for (const auto & line : lines) { ofs << "# " << line << "\n"; }
        ofs << key << " = " << info.AsLiteral() << ";\n\n";
      }

      return true;
    }

    bool Save(const emp::String & filename) {
      std::ofstream ofs{filename};
      if (!ofs) {
        error_note.Set("Failed to open config file for saving: ", filename);
        notify::Error(error_note);
        return false;
      }
      return Save(ofs);
    }

    // Load settings from a stream; return success.
    bool Load(std::istream & is) {
      emp::TokenStream tokens = lexer.Tokenize(is);
      Iterator it = tokens.begin();
      error_note.clear();

      while (it.Any()) {
        if (!LoadLine(it)) return false;
      }

      return true;
    }

    // Load settings from a file; return success.
    bool Load(const emp::String & filename) {
      std::ifstream is(filename);
      if (!is) return IOError("Failed to open config file for loading: ", filename);
      return Load(is);
    }

    /// Scan command-line arguments and apply recognised settings options.
    ///
    /// Three argument forms are handled:
    ///  - `-x val`      – short flag registered via AddSetting; `val` is the
    ///                    value string for that setting.
    ///  - `--option val` – long option registered via AddSetting; `val` is the
    ///                    value string for that setting.
    ///  - `-s "x=5; y=10"` / `--set "..."` – bulk config string tokenized and
    ///                    loaded exactly as if it were a config file.
    ///
    /// All other arguments are left untouched.  When `erase_on_use` is true
    /// each matched flag/option and its value are removed from `args`.
    /// Returns false (and sets the error note) on the first parse error.
    bool LoadArgs(emp::vector<emp::String> & args, bool erase_on_use=false) {
      error_note.clear();
      for (size_t i = 0; i < args.size(); ++i) {
        const emp::String & arg = args[i];

        // Per-setting short flag: -x val
        if (arg.size() == 2 && arg[0] == '-' && arg[1] != '-') {
          const char flag_char = arg[1];
          if (flag_map.contains(flag_char)) {
            if (erase_on_use) args.erase(args.begin() + i);
            else ++i;
            if (i >= args.size()) {
              return IOError("Expected value after '-", flag_char, "'.");
            }
            setting_map.at(flag_map.at(flag_char)).SetFromString(args[i]);
            if (erase_on_use) { args.erase(args.begin() + i); --i; }
            continue;
          }
        }

        // Per-setting long option: --option val
        if (arg.size() > 2 && arg[0] == '-' && arg[1] == '-') {
          const emp::String opt = arg.substr(2);
          if (option_map.contains(opt)) {
            if (erase_on_use) args.erase(args.begin() + i);
            else ++i;
            if (i >= args.size()) {
              return IOError("Expected value after '--", opt, "'.");
            }
            setting_map.at(option_map.at(opt)).SetFromString(args[i]);
            if (erase_on_use) { args.erase(args.begin() + i); --i; }
            continue;
          }
        }

        // Bulk config string: -s "x = 5; y = 10" or --set "..."
        if (arg == "-s" || arg == "--set") {
          if (erase_on_use) args.erase(args.begin() + i);
          else ++i;
          if (i >= args.size()) {
            return IOError("Expected config string after '--set'.");
          }
          emp::TokenStream tokens = lexer.Tokenize(args[i]);
          Iterator it = tokens.begin();
          while (it.Any()) {
            if (!LoadLine(it)) return false;
          }
          if (erase_on_use) {
            args.erase(args.begin() + i);
            --i; // May wrap around to max, but immediately reset on loop.
          }
        }
      }
      return true;
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_CONFIG_SETTINGS_MANAGER_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: ofs ident
