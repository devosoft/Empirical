/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025 Michigan State University
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
 * Two kinds of identifiers are supported:
 *
 *  - **Settings** – named variables with a fixed type and a single value.
 *    Supported types are `emp::String`, `bool`, `int`, `size_t`, and `double`.
 *
 *  - **Keywords** – special command words that trigger an arbitrary callback
 *    and receive the remaining tokens on the line as arguments.  They are
 *    useful for directives that do not follow the `name = value` pattern
 *    (e.g. `include other_file.cfg;`).
 *
 * ## Config-file format
 *
 * Files are parsed line-by-line.  Each line has one of the following forms:
 *
 * ```
 * # This is a comment (ignored)
 *
 * setting_name = value;        # assign a literal value to a setting
 * setting_name = other_name;   # copy the current value of another setting
 * keyword arg1 arg2 ...;       # invoke a keyword with zero or more arguments
 * ```
 *
 * Booleans accept `On`/`Off`/`True`/`False`/`1`/`0` (case-insensitive).
 * Strings may be bare identifiers or double-quoted literals with standard
 * escape sequences.  Lines may be terminated by `;` or a newline; blank lines
 * are skipped.
 *
 * ## Basic usage
 *
 * ```cpp
 * std::string name = "World";
 * size_t      reps = 10;
 * bool        verbose = false;
 *
 * emp::SettingsManager cfg;
 * cfg.AddSetting("name",    name,    "Name to greet",      'n', "name");
 * cfg.AddSetting("reps",    reps,    "Number of repeats",  'r', "reps");
 * cfg.AddSetting("verbose", verbose, "Enable verbose mode",'v', "verbose");
 *
 * cfg.Load("my_config.cfg"); // updates name, reps, verbose from file
 * cfg.Save("my_config.cfg"); // writes current values with inline comments
 * ```
 *
 * ## Key methods
 *
 *  - `AddSetting(name, var, desc, flag, option)` – register a setting bound
 *    to `var`; `flag` is a one-character CLI flag, `option` a long-form name.
 *  - `AddKeyword(keyword, fun, desc)` – register a keyword that invokes `fun`
 *    with its argument tokens.
 *  - `Load(filename)` – parse a config file; returns `false` on error.
 *  - `Save(filename)` – write all settings as a commented config file.
 *  - `Get<T>(name)` / `Set(name, value)` – programmatic get/set.
 *  - `HasError()` / `GetError()` – inspect the last load error.
 *
 * DEVELOPER NOTES:
 * - Consider allowing types to be more dynamic, perhaps set in a template.
 * - Command-line argument parsing (using flag/option fields) is not yet
 *   implemented; those fields are stored but unused by Load/Save.
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

      SettingInfo(emp::String name,
                  emp::String & var,
                  emp::String desc,
                  char flag          = '\0',
                  emp::String option = "")
        : name(name), value(var), desc(desc), flag(flag), option(option)
        , action([&var](const SettingInfo & info) { var = info.GetValue<emp::String>(); }) {}

      SettingInfo(emp::String name,
                  bool & var,
                  emp::String desc,
                  char flag          = '\0',
                  emp::String option = "")
        : name(name), value(var), desc(desc), flag(flag), option(option)
        , action([&var](const SettingInfo & info) { var = info.GetValue<bool>(); }) {}

      SettingInfo(emp::String name,
                  int & var,
                  emp::String desc,
                  char flag          = '\0',
                  emp::String option = "")
        : name(name), value(var), desc(desc), flag(flag), option(option)
        , action([&var](const SettingInfo & info) { var = info.GetValue<int>(); }) {}

      SettingInfo(emp::String name,
                  size_t & var,
                  emp::String desc,
                  char flag          = '\0',
                  emp::String option = "")
        : name(name), value(var), desc(desc), flag(flag), option(option)
        , action([&var](const SettingInfo & info) { var = info.GetValue<size_t>(); }) {}

      SettingInfo(emp::String name,
                  double & var,
                  emp::String desc,
                  char flag          = '\0',
                  emp::String option = "")
        : name(name), value(var), desc(desc), flag(flag), option(option)
        , action([&var](const SettingInfo & info) { var = info.GetValue<double>(); }) {}

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

    emp::TokenStream tokens;
    emp::TokenStream::Iterator it;

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
    [[nodiscard]] bool RequireToken(int token_id, const emp::String & name) {
      const Token cur_token = it.Use();
      if (cur_token != token_id) {
        return IOError("UnexpectedToken '", cur_token.lexeme, "'; expected ", name, ".");
      }

      return true;
    }

    /// We found a keyword during a load; load all arguments and trigger it.
    [[nodiscard]] bool LoadKeyword(const emp::String & keyword) {
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
    [[nodiscard]] bool LoadSetting(const emp::String & name) {
      if (verbose) emp::PrintLn("...identified as setting!");

        // setting name must be followed by an '='
        if (!RequireToken('=', "assignment")) return false;

      std::optional<emp::String> string_val = TokenToStringValue(it.Use());
      if (string_val) {
        GetSettingInfo(name).SetFromString(*string_val);
        return true;
      }
      return false;
    }

    [[nodiscard]] bool LoadScope(const emp::String & name) {
      // Scopes must be opened with a '{'
      if (!RequireToken('{', "open scope")) return false;
      PushScope(name);

      while (it.Any() && it.Peek() != '}') {
        if (!LoadLine()) {
          PopScope();
          return false;
        }
      }

      PopScope();
      if (!RequireToken('}', "close scope")) return false;

      return true;
    }

    // Load a single line starting from the current token iterator.
    bool LoadLine() {
      // Skip any extra lines.
      if (IsEndLine(it.Peek())) { ++it; return true; }

      const Token name_token = it.Use();
      if (verbose) emp::PrintLn("Found initial line token '", name_token.lexeme, "'.");

      if (name_token != ident_ID) {
        return IOError("UnexpectedToken '", name_token.lexeme, "'; expected keyword or parameter name.");
      }
      const emp::String name = name_token.lexeme;
      if (HasKeyword(name)) { return LoadKeyword(name); }
      if (HasSetting(name)) { return LoadSetting(name); }
      return LoadScope(name); // Unknown id must be a new scope.
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
      const emp::String full_name = AppendScope(name);
      setting_map.emplace(full_name, SettingInfo{full_name, value, desc, flag, option});
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

    bool Save(const emp::String & filename) {
      std::ofstream ofs(filename);
      error_note.clear();
      if (!ofs) {
        error_note.Set("Failed to open config file for saving: ", filename);
        notify::Error(error_note);
        return false;
      }

      for (const auto & [key, info] : setting_map) {
        const auto lines = info.GetDescription().Slice("\n");
        for (const auto & line : lines) { ofs << "# " << line << "\n"; }
        ofs << key << " = " << info.AsLiteral() << ";\n\n";
      }

      return true;
    }

    // Load a specified file; return success.
    bool Load(const emp::String & filename) {
      tokens = lexer.TokenizeFile(filename);
      it = tokens.begin();
      error_note.clear();

      while (it.Any()) {
        if (!LoadLine()) return false;
      }

      return true;
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_CONFIG_SETTINGS_MANAGER_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: ofs ident
