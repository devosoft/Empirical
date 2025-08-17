/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/config/SettingsManager.hpp
 * @brief A simple tool for managing configuration settings.
 * @note An older version of this class became SettingCombos.hpp
 * @note Status: Alpha
 *
 * Settings can be either a string of a numerical value (size_t or double).
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
#include <sstream>
#include <stddef.h>
#include <unordered_map>
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
      std::function<void(const SettingInfo &)> action;  /// Action to take when set.

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
        emp_assert(IsType<T>(), name);
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

      /// Check the type
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
      emp::String name;           ///< Label for this setting in config files
      keyword_fun_t fun;          ///< Function to call when keyword is triggered
      emp::String desc   = "";    ///< Description of setting
    };

    // === MEMBER VARIABLES ===

    std::map<emp::String, SettingInfo> setting_map;
    std::map<emp::String, KeywordInfo> keyword_map;
    emp::String assign_op = "=";
    emp::String error_note;
    bool verbose = false;

    // === HELPER FUNCTIONS ===

    // template<typename THIS_T>
    // auto & GetSettingInfo(this THIS_T & self, const emp::String & name) {
    //   emp_assert(self.HasSetting(name), "Invalid setting name", name);
    //   return self.setting_map.find(name)->second;
    // }

    // template<typename THIS_T>
    // auto & GetKeywordInfo(this THIS_T & self, const emp::String & name) {
    //   emp_assert(self.HasKeyword(name), "Invalid setting name", name);
    //   return self.keyword_map.find(name)->second;
    // }

    auto & GetSettingInfo(const emp::String & name) {
      emp_assert(HasSetting(name), "Invalid setting name", name);
      return setting_map.find(name)->second;
    }

    auto & GetKeywordInfo(const emp::String & name) {
      emp_assert(HasKeyword(name), "Invalid setting name", name);
      return keyword_map.find(name)->second;
    }

    const auto & GetSettingInfo(const emp::String & name) const {
      emp_assert(HasSetting(name), "Invalid setting name", name);
      return setting_map.find(name)->second;
    }

    const auto & GetKeywordInfo(const emp::String & name) const {
      emp_assert(HasKeyword(name), "Invalid setting name", name);
      return keyword_map.find(name)->second;
    }

  public:
    void SetVerbose(bool in=true) { verbose = in; }

    [[nodiscard]] bool HasSetting(const emp::String & name) const {
      return setting_map.contains(name);
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
      emp_assert(!HasSetting(name), "Trying to add a SettingsManager setting that already exists", name);
      setting_map.emplace(name, SettingInfo{name, value, desc, flag, option});
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

    void Save(emp::String filename) const {
      std::ofstream ofs(filename);
      if (!ofs) {
        notify::Error("Failed to open config file for saving:", filename);
        return;
      }

      for (const auto & [key, info] : setting_map) {
        const auto lines = info.GetDescription().Slice("\n");
        for (const auto & line : lines) { ofs << "# " << line << "\n"; }
        ofs << key << " = " << info.AsLiteral() << ";\n\n";
      }
    }

    // Load a specified file; return success.
    bool Load(const std::string & filename) {
      // Build the lexer to load the file.
      Lexer lexer;
      const int bool_on_ID  = lexer.AddToken("bool_on", "[Oo][Nn]|[Tt][Rr][Uu][Ee]");
      const int bool_off_ID = lexer.AddToken("bool_off", "[Oo][Ff][Ff]|[Ff][Aa][Ll][Ss][Ee]");
      const int ident_ID    = lexer.AddToken("identifier", "[a-zA-Z_][a-zA-Z0-9_]*");
      const int int_ID      = lexer.AddToken("int", "[0-9]+");
      const int double_ID   = lexer.AddToken("double", "[0-9]+\\.[0-9]+");
      const int string_ID   = lexer.AddToken("string", "(\\\"([^\"\\\\]|(\\\\.))*\\\")");
      const int assign_ID   = lexer.AddToken("assign", assign_op.AsLiteral());
      const int endline_ID  = lexer.AddToken("endline", "[;\n]");
      lexer.IgnoreToken("whitespace", "[ \\t\\r]+");
      lexer.IgnoreToken("comment", "#.+");
      // const int error_ID = lexer.AddToken("error", ".");

      // Load in the tokens from the provided file.
      auto tokens = lexer.TokenizeFile(filename);
      auto it     = tokens.begin();
      error_note.clear();

      // Loop line-by-line processing the input file.
      while (it.Any()) {
        // Skip any extra lines.
        if (it.Is(endline_ID)) { ++it; continue; }

        const Token name_token = it.Use();
        if (verbose) emp::PrintLn("Found initial line token '", name_token.lexeme, "'.");

        if (name_token != ident_ID) {
        if (verbose) emp::PrintLn("Error: Not identifier.");
          error_note = "UnexpectedToken '" + name_token.lexeme
                     + "'; expected keyword or parameter name.";
          return false;
        }
        const emp::String name = name_token.lexeme;

        // If this line triggering a KEYWORD ?
        if (HasKeyword(name)) {
          if (verbose) emp::PrintLn("...identified as keyword!");
          // Grab the rest of the line.
          emp::vector<emp::String> keyword_vars;
          while (it.IsValid() && !it.Is(endline_ID)) {
            keyword_vars.push_back(it.Use().lexeme);
          }
          GetKeywordInfo(name).fun(keyword_vars);
        }

        // Is this line configuring a SETTING ?
        else if (HasSetting(name)) {
          if (verbose) emp::PrintLn("...identified as setting!");

          const Token op_token = it.Use();

          // setting name must be followed by an '='
          if (op_token != assign_ID) {
            if (verbose) emp::PrintLn("...expected assignment, but found '", op_token.lexeme, "'.");
            error_note = "UnexpectedToken '" + op_token.lexeme + "'; expected '" + assign_op + "'.";
            return false;
          }

          // Next, we must have the value for that setting.
          const Token value_token = it.Use();
          if (value_token == ident_ID) {
            if (verbose) emp::PrintLn("...being set to identifier '", value_token.lexeme, "'.");
            // Set equal to another identifier.
            if (!setting_map.contains(value_token.lexeme)) {
              if (verbose) emp::PrintLn("Identifier '", value_token.lexeme, "' UNKNOWN!");
              error_note = "Setting to unknown configuration variable, '" + value_token.lexeme + "'.";
              return false;
            }
            GetSettingInfo(name).SetFromString(GetSettingInfo(value_token.lexeme).AsLiteral());
          }
          else if (value_token.IsOneOf(bool_on_ID, bool_off_ID, int_ID, double_ID, string_ID)) {
            if (verbose) emp::PrintLn("...being set from literal '", value_token.lexeme, "'.");
            GetSettingInfo(name).SetFromString(value_token.lexeme);
            continue;
          }
          else {
            // If we made it this far, we don't know how to do the assignment.
            if (verbose) emp::PrintLn("...being set to something unknown '", value_token.lexeme, "'.");
            error_note = "UnexpectedToken '" + value_token.lexeme + "'; expected assignment value.";
            return false;
          }
        }
        else {
          if (verbose) emp::PrintLn("...unknown!");
          error_note = "Unknown keyword or configuration setting, '" + name + "'.";
          return false;
        }

        // Each line must end in a newline (or end of file)
        if (it.Any()) {
          const Token el_token = it.Use();
          if (el_token != endline_ID) {
            if (verbose) emp::PrintLn("...does not terminate in an endline!");
            error_note = "UnexpectedToken '" + el_token.lexeme + "'; expected end of line.";
            return false;
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
