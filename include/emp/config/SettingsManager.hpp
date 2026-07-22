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
 * SettingsManager maintains a collection of named settings, each bound to callback functions
 * that are triggered when it is get or set.  These are usually associated with C++ variables
 * whose values are maintained, but can also trigger more complex functions (for example, setting
 * a random number seed might call ResetSeed on the random number generator.)
 * 
 * Supported setting types are `emp::String`, `bool`, `int64_t`, `uint64_t`, and `double`.
 *
 * Settings can be organized into scopes using dot-notation names (e.g. `"robot1.speed"`), which
 * simplifies configuring multiple objects of the same type without name collisions.
 * (Internally all settings are stored with their full dotted key.)
 * Each name component must begin with a letter or underscore and contain only letters, digits,
 * and underscores. A name cannot be both a setting and a scope containing other settings.
 *
 * In addition to settings, users can also specify "Keywords", which trigger an arbitrary callback,
 * receiving the remaining tokens on the line as arguments.  They are useful for arbitrary
 * directives that do not follow the `name = value` pattern (e.g. `include other_file.cfg;`).
 * Keywords are always global and are recognized only at the top level, outside any scope.
 *
 * === Config-file format ===
 *
 * Files are parsed line-by-line.  Each line has one of the following forms:
 *
 *     # This is a comment (ignored)
 *
 *     setting_name = value;          # assign a literal value to a setting
 *     setting_name = other_name;     # copy the current value of another setting
 *     keyword arg1 arg2 ...;         # invoke a keyword with zero or more arguments
 *
 *     scope.setting = value;         # dot-notation: assign a scoped setting directly
 *
 *     scope_name {                   # brace-block: enter a scope
 *       setting = value;             #   all names inside are prefixed with scope_name
 *       inner { setting = value; }   #   scopes may nest arbitrarily
 *     }
 *
 * Dot notation and brace-block notation may be freely mixed in the same file.
 * A trailing '\' at the end of a line continues onto the next line (the newline is ignored).
 * Booleans accept 'On', 'Off', 'True', 'False', '1', or '0' (case-insensitive).
 * Numbers accept optional signs, decimal points, and scientific notation.
 * Strings may be single- or double-quoted literals with standard C escape sequences.
 * Lines may be terminated by ';' or a newline; blank lines and comment-only lines are skipped.
 *
 * === Basic usage ===
 *
 *     std::string name = "World";
 *     uint64_t    reps = 10;
 *     bool        verbose = false;
 *     int64_t     r1_speed = 0, r2_speed = 0;
 *
 *     emp::SettingsManager cfg;
 *     // AddSetting can take the setting name, the C++ variable, a description, and an optional
 *     // single-character flag (for quick setting on command line).
 *     cfg.AddSetting("name",         name,     "Name to greet",      'n');
 *     cfg.AddSetting("reps",         reps,     "Number of repeats",  'r');
 *     cfg.AddSetting("verbose",      verbose,  "Enable verbose mode",'v');
 *     cfg.AddSetting("robot1.speed", r1_speed, "Robot 1 speed");
 *     cfg.AddSetting("robot2.speed", r2_speed, "Robot 2 speed");
 *
 *     cfg.Load("my_config.cfg");    // updates variables from file
 *     cfg.Save("my_config.cfg");    // writes default values with description comments
 *
 *     // Apply settings from command-line arguments (e.g. --reps 5)
 *     emp::vector<emp::String> args(argv, argv + argc);
 *     cfg.LoadArgs(args);
 *
 * === Main methods ===
 *
 *  - AddSetting(name, var, desc [, flag ]) – register a setting bound to 'var';
 *     'flag' is a one-character CLI flag (optional).
 *  - AddKeyword(keyword, fun, desc) – register a keyword that invokes 'fun' with its args.
 *  - Load(istream&) or Load(filename) – parse settings; errors are immediately fatal.
 *  - LoadArgs(args) – scan a 'vector<emp::String>' of command-line arguments, deleting those used:
 *       '-x val' or '--setting val' set specified setting (registered with AddSetting)
 *       '-k arg...' or '--keyword arg...' trigger a keyword (registered with AddKeyword)
 *  - Save(ostream&) or Save(filename) – write all settings as config file using defaults.
 *  - SaveCurrent(ostream&) or SaveCurrent(filename) – same but uses current (live) values.
 *  - Get<T>(name) or Set(name, value) – programmatic get/set.
 *  - HasSetting(name) or HasKeyword(name) or HasIdentifier(name) – query if name is registered.
 *  - SetVerbose() – enable diagnostic printing during Load/Save.
 *
 * DEVELOPER NOTES:
 * - Consider allowing types to be more dynamic, perhaps set in a template.
 * - Consider collecting multiple errors before aborting.
 */

#pragma once

#ifndef INCLUDE_EMP_CONFIG_SETTINGS_MANAGER_HPP_GUARD
#define INCLUDE_EMP_CONFIG_SETTINGS_MANAGER_HPP_GUARD

#include <cerrno>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <limits>
#include <map>
#include <print>
#include <cstdint>
#include <stddef.h>

#include "../base/notify.hpp"
#include "../base/vector.hpp"
#include "../compiler/Lexer.hpp"
#include "../io/io_utils.hpp"
#include "../serialize/SerialPod.hpp"
#include "../tools/String.hpp"

namespace emp {

  class SettingsManager {
  private:
    using keyword_fun_arg_t = emp::vector<emp::String>;
    using keyword_fun_t = std::function<void(keyword_fun_arg_t)>;
    using Iterator = emp::TokenStream::Iterator;

    /// Class to manage a single setting.
    class SettingInfo {
    private:
      emp::String name;           ///< Label for this setting in config files
      emp::String desc   = "";    ///< Description of setting
      emp::String default_val;    ///< Default value as a string (for SaveTemplate)
      char flag          = '\0';  ///< Command-line flag ('\0' for none)

      enum class Type { ERROR=0, STRING, BOOL, INT64, UINT64, DOUBLE };
      Type type;

      std::function<void(const emp::String &)> set_string;
      std::function<void(bool)>                set_bool;
      std::function<void(int64_t)>             set_int64;
      std::function<void(uint64_t)>            set_uint64;
      std::function<void(double)>              set_double;

      std::function<emp::String()> get_string;
      std::function<bool()>        get_bool;
      std::function<int64_t()>     get_int64;
      std::function<uint64_t()>    get_uint64;
      std::function<double()>      get_double;

      template <typename T> static constexpr Type ToTypeEnum() {
        using base_t = std::remove_cv_t<T>;
        if constexpr (std::same_as<base_t, bool>)               return Type::BOOL;
        else if constexpr (std::signed_integral<base_t>)        return Type::INT64;
        else if constexpr (std::unsigned_integral<base_t>)      return Type::UINT64;
        else if constexpr (std::floating_point<base_t>)         return Type::DOUBLE;
        else if constexpr (std::same_as<base_t, emp::String> ||
                           std::same_as<base_t, std::string>)   return Type::STRING;
        else return Type::ERROR;
      }

      template <typename T>
      [[nodiscard]] static T ParseNumber(const emp::String & in,
                                         const emp::String & setting_name) {
        if (!in.IsNumber(std::floating_point<T>)) {
          emp::notify::Error(
            "Invalid numeric value '", in, "' for setting '", setting_name, "'."
          );
        }

        emp::String unparsed = in;
        errno = 0;

        if constexpr (std::signed_integral<T>) {
          const long long value = unparsed.PopSigned();
          if (unparsed.size() || errno == ERANGE ||
              value < std::numeric_limits<T>::lowest() ||
              value > std::numeric_limits<T>::max()) {
            emp::notify::Error(
              "Invalid or out-of-range value '", in, "' for setting '", setting_name, "'."
            );
          }
          return static_cast<T>(value);
        } else if constexpr (std::unsigned_integral<T>) {
          const unsigned long long value = unparsed.PopUnsigned();
          if (unparsed.size() || errno == ERANGE || value > std::numeric_limits<T>::max()) {
            emp::notify::Error(
              "Invalid or out-of-range value '", in, "' for setting '", setting_name, "'."
            );
          }
          return static_cast<T>(value);
        } else {
          const double value = unparsed.PopFloat();
          const double magnitude = std::abs(value);
          if (unparsed.size() || errno == ERANGE ||
              value < std::numeric_limits<T>::lowest() ||
              value > std::numeric_limits<T>::max() ||
              (value != 0.0 && magnitude < std::numeric_limits<T>::denorm_min())) {
            emp::notify::Error(
              "Invalid or out-of-range value '", in, "' for setting '", setting_name, "'."
            );
          }
          return static_cast<T>(value);
        }
      }

      // Dynamic conversion from a STRING type.
      template <typename TO_T>
      [[nodiscard]] static TO_T Convert(const emp::String & in,
                                        const emp::String & setting_name = "") {
        constexpr Type to_type = ToTypeEnum<TO_T>();

        if constexpr (to_type == Type::STRING) return in;
        else if constexpr (to_type == Type::BOOL) {
          const emp::String lower = in.AsLower();
          if (lower.IsOneOf("on", "true", "1")) return true;
          if (lower.IsOneOf("off", "false", "0")) return false;
          emp::notify::Error(
            "Invalid boolean value '", in, "' for setting '", setting_name,
            "'. Expected On, Off, True, False, 1, or 0."
          );
        }
        else if constexpr (to_type == Type::INT64 ||
                           to_type == Type::UINT64 ||
                           to_type == Type::DOUBLE) {
          return ParseNumber<TO_T>(in, setting_name);
        }
        else static_assert(false, "Cannot convert from string to unknown type.");
      }

      // Allow conversions from std::string.
      template <typename TO_T>
      [[nodiscard]] static TO_T Convert(const std::string & in,
                                        const emp::String & setting_name = "") {
        return Convert<TO_T>(emp::String{in}, setting_name);
      }

      // Dynamic conversion from a NUMERICAL type.
      template <typename TO_T, typename FROM_T>
      [[nodiscard]] static TO_T Convert(const FROM_T & in) {
        constexpr Type to_type = ToTypeEnum<TO_T>();

        if constexpr (to_type == Type::STRING) return emp::MakeString(in);
        else return static_cast<TO_T>(in);
      }


    public:
      SettingInfo()                    = delete;
      SettingInfo(const SettingInfo &) = default;
      SettingInfo(SettingInfo &&)      = default;

      // Create from a bound variable.
      template <typename VAR_T>
      SettingInfo(emp::String name, VAR_T & var, emp::String desc, char flag = '\0',
                  emp::String explicit_default = "")
        : name(name), desc(desc)
        , default_val(explicit_default.empty() ? Convert<emp::String>(var) : explicit_default)
        , flag(flag), type(ToTypeEnum<VAR_T>())
        , set_string([&var, setting_name=name](const emp::String & in){
            var = Convert<VAR_T>(in, setting_name);
          })
        , set_bool(  [&var](bool in)               { var = Convert<VAR_T>(in); })
        , set_int64( [&var](int64_t in)            { var = Convert<VAR_T>(in); })
        , set_uint64([&var](uint64_t in)           { var = Convert<VAR_T>(in); })
        , set_double([&var](double in)             { var = Convert<VAR_T>(in); })
        , get_string([&var]() { return Convert<emp::String>(var); })
        , get_bool(  [&var]() { return Convert<bool>(var); })
        , get_int64( [&var]() { return Convert<int64_t>(var); })
        , get_uint64([&var]() { return Convert<uint64_t>(var); })
        , get_double([&var]() { return Convert<double>(var); }) {}

      // Create from getter/setter functions; T is deduced from the getter's return type.
      template <typename GETTER_T, typename SETTER_T,
                typename T = std::remove_cvref_t<std::invoke_result_t<GETTER_T>>>
        requires std::invocable<GETTER_T>
      SettingInfo(emp::String name, GETTER_T getter, SETTER_T setter,
                  emp::String desc, char flag = '\0', emp::String explicit_default = "")
        : name(name), desc(desc)
        , default_val(explicit_default.empty() ? Convert<emp::String>(getter()) : explicit_default)
        , flag(flag), type(ToTypeEnum<T>())
        , set_string([setter, setting_name=name](const emp::String & in){
            setter(Convert<T>(in, setting_name));
          })
        , set_bool(  [setter](bool in)               { setter(Convert<T>(in)); })
        , set_int64( [setter](int64_t in)            { setter(Convert<T>(in)); })
        , set_uint64([setter](uint64_t in)           { setter(Convert<T>(in)); })
        , set_double([setter](double in)             { setter(Convert<T>(in)); })
        , get_string([getter]() { return Convert<emp::String>(getter()); })
        , get_bool(  [getter]() { return Convert<bool>(getter()); })
        , get_int64( [getter]() { return Convert<int64_t>(getter()); })
        , get_uint64([getter]() { return Convert<uint64_t>(getter()); })
        , get_double([getter]() { return Convert<double>(getter()); }) {}


      [[nodiscard]] bool IsString() const { return type == Type::STRING; }
      [[nodiscard]] bool IsBool() const { return type == Type::BOOL; }
      [[nodiscard]] bool IsInt64() const { return type == Type::INT64; }
      [[nodiscard]] bool IsUInt64() const { return type == Type::UINT64; }
      [[nodiscard]] bool IsDouble() const { return type == Type::DOUBLE; }

      [[nodiscard]] const emp::String & GetName() const { return name; }
      [[nodiscard]] const emp::String & GetDescription() const { return desc; }
      [[nodiscard]] char GetFlag() const { return flag; }
      [[nodiscard]] const emp::String & GetDefault() const { return default_val; }
      void SetDefault(const emp::String & val) { default_val = val; }

      /// Return the default in a form suitable for writing to a config file.
      [[nodiscard]] emp::String GetDefaultLiteral() const {
        if (IsString()) return default_val.AsLiteral();
        return default_val;
      }

      template <typename T>
      [[nodiscard]] T GetValue() const {
        if constexpr (std::same_as<T, emp::String>)  return get_string();
        else if constexpr (std::same_as<T, bool>)    return get_bool();
        else if constexpr (std::same_as<T, double>)  return get_double();
        else if constexpr (std::signed_integral<T>)  return static_cast<T>(get_int64());
        else if constexpr (std::unsigned_integral<T>) return static_cast<T>(get_uint64());
        else {
          static_assert(false, "unsupported type");
          return T{};
        }
      }

      /// Set the value from a string (parsed to the setting's native type).
      void SetValue(const emp::String & val) { set_string(val); }
      void SetValue(const std::string & val) { set_string(val); }
      void SetValue(const char * val) { set_string(val); }

      /// Set the value from a typed argument; dispatches by concept like GetValue.
      template <typename T>
      void SetValue(T val) {
        if constexpr (std::same_as<T, bool>)           set_bool(val);
        else if constexpr (std::same_as<T, double>)    set_double(val);
        else if constexpr (std::signed_integral<T>)    set_int64(static_cast<int64_t>(val));
        else if constexpr (std::unsigned_integral<T>)  set_uint64(static_cast<uint64_t>(val));
        else static_assert(false, "unsupported type for SetValue");
      }

      [[nodiscard]] emp::String AsString() const { return get_string(); }

      [[nodiscard]] emp::String AsLiteral() const {
        if (IsString()) { return AsString().AsLiteral(); }
        return AsString();
      }

      /// Check the type (return std::string for compatibility with type manager)
      [[nodiscard]] std::string GetTypeName() const {
        switch (type) {
          case Type::STRING: return "emp::String";
          case Type::BOOL:   return "bool";
          case Type::INT64:  return "int64_t";
          case Type::UINT64: return "uint64_t";
          case Type::DOUBLE: return "double";
          default: return "error";
        }
      }
    }; // END OF SettingInfo definition

    struct KeywordInfo {
      emp::String name;                  ///< Label for this keyword in config files
      keyword_fun_t fun;                 ///< Function to call when keyword is triggered
      emp::String desc = "";             ///< Description of keyword
      char flag = '\0';                  ///< Command-line flag ('\0' for none)
      size_t max_args = emp::MAX_SIZE_T; ///< Max number of command-line args to send.
    };

    // === MEMBER VARIABLES ===

    std::filesystem::path config_dir{"../config"};  // Directory with configuration files
    emp::String exe_name;
    std::map<emp::String, SettingInfo> setting_map;
    std::map<emp::String, KeywordInfo> keyword_map;
    std::map<char, emp::String> flag_map;  ///< Flag char -> setting/keyword name
    emp::vector<emp::String> cur_scopes{};
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
      if (!self.HasSetting(name)) emp::notify::Error("SettingsManager: unknown setting '", name, "'.");
      return self.setting_map.find(self.AppendScope(name))->second;
    }

    auto & GetKeywordInfo(this auto & self, const emp::String & name) {
      if (!self.HasKeyword(name)) emp::notify::Error("SettingsManager: unknown keyword '", name, "'.");
      return self.keyword_map.find(name)->second;
    }

    // === Direct parsing helpers ===

    // Count the number of non-option arguments available from a starting index.
    size_t CountFlagArgs(emp::vector<emp::String> & args,
                         size_t start_index,
                         size_t max_args = emp::MAX_SIZE_T)
    {
      max_args = std::min(max_args, args.size() - start_index);
      const size_t max_index = start_index + max_args;
      size_t end_index = start_index;
      while (end_index < max_index &&
             args[end_index].size() > 0 &&
             args[end_index][0] != '-') { ++end_index; }
      return end_index - start_index;
    }

    /// Apply a setting value at args[i]
    /// Called after the flag/option token itself has already been consumed.
    void LoadArgSetting(emp::vector<emp::String> & args, size_t & i,
                        SettingInfo & info, const emp::String & flag_desc) {
      if (i >= args.size()) {
        emp::notify::Error("Expected arg value after '", flag_desc, "'.");
      }
      emp::notify::Message("Setting '", info.GetName(), "' to '", args[i], "'.");
      info.SetValue(args[i]);
      args.erase(args.begin() + i);
      --i;
    }

    /// Apply a keyword at args[i..]
    /// Called after the flag/option token itself has already been consumed.
    void LoadArgKeyword(emp::vector<emp::String> & args, size_t & i, const KeywordInfo & info) {
      size_t args_found = CountFlagArgs(args, i, info.max_args);
      emp::vector<emp::String> keyword_vars(args_found);
      for (size_t arg_id = 0; arg_id < args_found; ++arg_id) {
        keyword_vars[arg_id] = args[i + arg_id];
      }
      info.fun(keyword_vars);
      args.erase(args.begin()+i, args.begin()+i+args_found);
      --i;
    }

    [[nodiscard]] bool IsEndLine(int id) const { return id == ';' || id == '\n'; }

    /// Use the next token if it's the right type; abort if not.
    void RequireToken(Iterator & it, int token_id, const emp::String & name) {
      const Token cur_token = it.Use();
      if (cur_token != token_id) {
        emp::notify::Error(
          "UnexpectedToken '", cur_token.lexeme, "' on line ", cur_token.line_id,
          "; expected ", name, "."
        );
      }
    }

    /// We found a keyword during a load; load all arguments and trigger it.
    void LoadKeyword(Iterator & it, const emp::String & keyword) {
      if (verbose) emp::PrintLn("...identified as keyword!");
      // Grab the rest of the line.
      emp::vector<emp::String> keyword_vars;
      while (it.IsValid() && !(IsEndLine(it.Peek()))) {
        keyword_vars.push_back(it.Use().lexeme);
      }
      GetKeywordInfo(keyword).fun(keyword_vars);
    }

    emp::String TokenToStringValue(const Token & token) {
      // If numeric literal value, use it directly.
      if (token.IsOneOf(bool_value_ID, int_ID, double_ID)) {
        return token.lexeme;
      }

      // If string literal, convert it to a regular string.
      if (token == string_ID) {
        return token.lexeme.ConvertStringFromLiteral("\"'");
      }

      // If identifier, look it up.
      if (token == ident_ID) {
        if (HasSetting(token.lexeme)) {
          return GetSettingInfo(token.lexeme).AsString();
        }
        emp::notify::Error("Identifier '", token.lexeme, "' UNKNOWN!");
      }

      // If we made it this far, we don't know how to do the conversion.
      emp::notify::Error("UnexpectedToken '", token.lexeme, "'; expected value.");
    }

    // We have found a setting name at the beginning of a line; load it!
    void LoadSetting(Iterator & it, const emp::String & name) {
      if (verbose) emp::PrintLn("...identified as setting!");

      // setting name must be followed by an '='
      RequireToken(it, '=', "assignment");
      GetSettingInfo(name).SetValue(TokenToStringValue(it.Use()));
    }

    void LoadScope(Iterator & it, const emp::String & name) {
      // Scopes must be opened with a '{'
      RequireToken(it, '{', "open scope");
      PushScope(name);

      while (it.Any() && it.Peek() != '}') {
        LoadLine(it);
      }

      PopScope();
      RequireToken(it, '}', "close scope");
    }

    // Load a single line starting from the current token iterator.
    void LoadLine(Iterator & it) {
      // Skip any extra lines.
      if (IsEndLine(it.Peek())) { ++it; return; }

      const Token name_token = it.Use();
      if (verbose) emp::PrintLn("Found initial line token '", name_token.lexeme, "'.");

      if (name_token != ident_ID) {
        emp::notify::Error(
          "UnexpectedToken '", name_token.lexeme, "'; expected keyword or parameter name."
        );
      }
      const emp::String name = name_token.lexeme;
      if (cur_scopes.empty() && HasKeyword(name)) { LoadKeyword(it, name); }
      else if (HasSetting(name)) { LoadSetting(it, name); }
      else { LoadScope(it, name); } // Unknown id must be a new scope.
    }

    /// Is prefix a complete scope prefix of name? (For example, "robot" of "robot.speed".)
    [[nodiscard]] static bool IsScopePrefix(const emp::String & prefix,
                                            const emp::String & name) {
      return prefix.size() < name.size()
        && name.compare(0, prefix.size(), prefix) == 0
        && name[prefix.size()] == '.';
    }

    /// Validate and fully qualify a new setting name before registration.
    [[nodiscard]] emp::String ValidateSettingName(const emp::String & name) const {
      if (!name.IsIdentifierChain()) {
        emp::notify::Error(
          "SettingsManager: invalid setting name '", name,
          "'. Expected dot-separated identifiers containing only letters, digits, and underscores."
        );
      }

      const emp::String full_name = AppendScope(name);
      if (setting_map.contains(full_name) || keyword_map.contains(full_name)) {
        emp::notify::Error("SettingsManager: identifier '", full_name, "' is already registered.");
      }

      for (const auto & [other_name, info] : setting_map) {
        (void) info;
        if (IsScopePrefix(full_name, other_name) || IsScopePrefix(other_name, full_name)) {
          emp::notify::Error(
            "SettingsManager: setting names '", full_name, "' and '", other_name,
            "' are structurally ambiguous; a name cannot be both a setting and a scope."
          );
        }
      }

      for (const auto & [keyword, info] : keyword_map) {
        (void) info;
        if (keyword.find('.') == emp::String::npos && IsScopePrefix(keyword, full_name)) {
          emp::notify::Error(
            "SettingsManager: global keyword '", keyword,
            "' conflicts with the first component of setting name '", full_name, "'."
          );
        }
      }

      return full_name;
    }

    /// Validate a new global keyword name before registration.
    void ValidateKeywordName(const emp::String & keyword) const {
      if (!keyword.IsIdentifierChain()) {
        emp::notify::Error(
          "SettingsManager: invalid keyword name '", keyword,
          "'. Expected dot-separated identifiers containing only letters, digits, and underscores."
        );
      }
      if (keyword_map.contains(keyword) || setting_map.contains(keyword)) {
        emp::notify::Error("SettingsManager: identifier '", keyword, "' is already registered.");
      }

      if (keyword.find('.') == emp::String::npos) {
        for (const auto & [setting_name, info] : setting_map) {
          (void) info;
          if (IsScopePrefix(keyword, setting_name)) {
            emp::notify::Error(
              "SettingsManager: global keyword '", keyword,
              "' conflicts with the first component of setting name '", setting_name, "'."
            );
          }
        }
      }
    }

    void SetupFlag(char & flag, emp::String option_name) {
      // Give a warning if we are trying to add the same flag twice (but allow run to continue).
      if (HasFlag(flag)) {
        emp::notify::Warning(
          "Duplicate CLI flag in SettingsManager '", flag, "'; used as shortcut for both '--",
          flag_map[flag], "' and '--", option_name, "'.");
        flag = '\0';
      }
      if (flag != '\0') flag_map[flag] = option_name;
    }

  public:
    SettingsManager()
      : bool_value_ID(lexer.AddToken("bool_val", "[Oo][Nn]|[Tt][Rr][Uu][Ee]|[Oo][Ff][Ff]|[Ff][Aa][Ll][Ss][Ee]"))
      , ident_ID(lexer.AddToken("identifier", "[a-zA-Z_][a-zA-Z0-9_.]*"))
      , int_ID(lexer.AddToken("int", "[-+]?[0-9]+"))
      , double_ID(lexer.AddToken(
          "double",
          "[-+]?([0-9]+(\\.[0-9]+)?|\\.[0-9]+)([eE][-+]?[0-9]+)?"
        ))
      , string_ID(lexer.AddToken("string", "(\\\"([^\"\\\\]|(\\\\.))*\\\")|(\\'([^'\\\\]|(\\\\.))*\\')"))
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

    [[nodiscard]] bool HasFlag(char flag) const { return flag && flag_map.contains(flag); }
    [[nodiscard]] char GetFlag(const emp::String & name) const { return GetSettingInfo(name).GetFlag(); }

    template <typename T>
    void Set(const emp::String & name, T && value) {
      GetSettingInfo(name).SetValue(std::forward<T>(value));
    }

    const std::filesystem::path & GetConfigDir() const { return config_dir; }
    void SetConfigDir(const emp::String & in) { config_dir = in.str(); }

    template <typename T>
    SettingsManager & AddSetting(const emp::String & name,
                                 T & value,
                                 emp::String desc,
                                 char flag = '\0',
                                 emp::String default_val = "") {
      const emp::String full_name = ValidateSettingName(name);
      SetupFlag(flag, full_name);
      setting_map.emplace(full_name, SettingInfo{full_name, value, desc, flag, default_val});
      return *this;
    }

    /// Register a setting controlled by explicit getter/setter functions.
    /// The value type T is deduced from the getter's return type.
    template <typename GETTER_T, typename SETTER_T>
      requires std::invocable<GETTER_T>
    SettingsManager & AddSetting(const emp::String & name,
                                 GETTER_T getter,
                                 SETTER_T setter,
                                 emp::String desc,
                                 char flag = '\0',
                                 emp::String default_val = "") {
      const emp::String full_name = ValidateSettingName(name);
      SetupFlag(flag, full_name);
      setting_map.emplace(full_name, SettingInfo{full_name, getter, setter, desc, flag, default_val});
      return *this;
    }

    /// A specified keyword can be added to the settings; it will call a provided
    /// function with the remaining tokens before the next semicolon.
    SettingsManager & AddKeyword(const emp::String & keyword,
                                 keyword_fun_t fun,
                                 emp::String desc,
                                 char flag = '\0',
                                 size_t max_args = emp::MAX_SIZE_T) {
      ValidateKeywordName(keyword);
      SetupFlag(flag, keyword);
      keyword_map.emplace(keyword, KeywordInfo{keyword, fun, desc, flag, max_args});
      return *this;
    }

    // Print out all info on the currently known settings.
    void PrintKeywords(std::ostream & os=std::cout) {
      std::println(os, "General Options:");
      for (const auto & [name, info] : keyword_map) {
        std::print(os, "  {} : {}", name, info.desc);
        if (info.flag) std::println(os, "; (flag: -{})", info.flag);
        else std::println(os, "");
      }
    }

    // Print out all info on the currently known settings.
    void PrintSettings(std::ostream & os=std::cout) {
      std::println(os, "Available settings:");
      for (const auto & [name, info] : setting_map) {
        std::print(os, "  --{}", name);
        if (info.GetFlag()) std::print(os, " or -{}", info.GetFlag());
        std::println(os, " : {} (Default: {})", info.GetDescription(), info.GetDefaultLiteral());
      }
    }

    // Print help info.
    void PrintHelp(const emp::vector<emp::String> & args, std::ostream & os=std::cout) {
      if (args.size()) {
        for (const emp::String & arg : args) {
          if (arg == "options") PrintKeywords(os);
          else if (arg == "settings") PrintSettings(os);
          else std::println(os, "Unknown help argument '{}'", arg);
        }
        return;
      }

      // Default to printing all current options that were given flags.
      std::println(os, "Usage: {} [options...]", exe_name);
      std::println(os, "\nFlag Options:");

      std::map<emp::String, emp::String> flag_map;
      for (const auto & [name, info] : keyword_map) {
        if (!info.flag) continue; // Only include keywords with flags.
        flag_map[name] = std::format("  -{} --{} : {}", info.flag, name, info.desc);
      }
      for (const auto & [name, info] : setting_map) {
        if (!info.GetFlag()) continue; // Only include settings with flags.
        flag_map[name] = std::format("  -{} --{} : {}", info.GetFlag(), name, info.GetDescription());
      }

      for (auto [_, line] : flag_map) {
        std::println(os, "{}", line);
      }

      std::println(os, "");
      std::println(os, "Use `{} --help settings` for a full list of variables to set", exe_name);
      std::println(os, "Use `{} --help options` for a full list of all other options", exe_name);
    }

    /// Write all settings to a config file, grouping dot-prefixed keys into scoped blocks.
    /// @param value_fn  Optional lambda: (const SettingInfo &) -> emp::String.
    ///   Defaults to returning each setting's registered default value.
    ///   Pass a custom lambda to select a different value (e.g. the live value).
    ///
    /// Settings whose keys share a dot-separated prefix are grouped under a brace block.
    /// For example, `grid.height` and `grid.width` produce:
    /// @code
    ///   grid {
    ///     height = 100;
    ///     width  = 100;
    ///   }
    /// @endcode
    bool Save(std::ostream & ofs,
              std::function<emp::String(const SettingInfo &)> value_fn =
                  [](const SettingInfo & info) { return info.GetDefaultLiteral(); }) {
      emp_assert(ofs);

      emp::vector<emp::String> open_scopes;  // currently open scope names (outermost first)
      bool pending_blank = false;            // whether to emit a blank line before the next item

      auto emit_pending_blank = [&]() {
        if (pending_blank) { ofs << "\n"; pending_blank = false; }
      };

      // Close scopes until only `keep` remain open.
      auto close_scopes_to = [&](size_t keep) {
        while (open_scopes.size() > keep) {
          pending_blank = false;
          std::string indent(( open_scopes.size() - 1) * 2, ' ');
          ofs << indent << "}\n";
          open_scopes.pop_back();
          pending_blank = true;
        }
      };

      for (const auto & [key, info] : setting_map) {
        // Split "a.b.c" into parts; last part is the local name, rest are scopes.
        emp::vector<emp::String> parts = key.Slice(".");
        emp::String local_name = parts.back();
        parts.pop_back();  // parts is now the scope chain

        // Find how many leading scopes are shared with the current open_scopes.
        size_t common = 0;
        while (common < open_scopes.size() && common < parts.size() &&
               open_scopes[common] == parts[common]) {
          ++common;
        }

        // Close diverging scopes.
        close_scopes_to(common);

        // Open any new scopes needed for this key.
        for (size_t i = common; i < parts.size(); ++i) {
          emit_pending_blank();
          std::string indent(open_scopes.size() * 2, ' ');
          ofs << indent << parts[i] << " {\n";
          open_scopes.push_back(parts[i]);
        }

        // Write the setting entry.
        emit_pending_blank();
        std::string indent(open_scopes.size() * 2, ' ');
        const auto lines = info.GetDescription().Slice("\n");
        for (const auto & line : lines) { ofs << indent << "# " << line << "\n"; }
        ofs << indent << local_name << " = " << value_fn(info) << ";\n";
        pending_blank = true;
      }

      // Close any scopes still open after the last setting.
      close_scopes_to(0);
      if (pending_blank) { ofs << "\n"; }

      return true;
    }

    bool Save(const emp::String & filename,
              std::function<emp::String(const SettingInfo &)> value_fn =
                  [](const SettingInfo & info) { return info.GetDefaultLiteral(); }) {
      std::ofstream ofs{filename};
      if (!ofs) {
        notify::Error("Failed to open config file for saving: ", filename);
      }
      return Save(ofs, value_fn);
    }

    /// Write all settings using their current (live) values rather than defaults.
    bool SaveCurrent(std::ostream & ofs) {
      return Save(ofs, [](const SettingInfo & info) { return info.AsLiteral(); });
    }

    bool SaveCurrent(const emp::String & filename) {
      return Save(filename, [](const SettingInfo & info) { return info.AsLiteral(); });
    }

    // Load settings from a stream; return true on success and abort on error.
    bool Load(std::istream & is) {
      emp::TokenStream tokens = lexer.Tokenize(is);
      Iterator it = tokens.begin();
      while (it.Any()) LoadLine(it);

      return true;
    }

    // Load settings from a file; return true on success and abort on error.
    bool Load(const emp::String & filename) {
      std::ifstream is(filename);
      if (!is) emp::notify::Error("Failed to open config file for loading: ", filename);
      return Load(is);
    }
    
    /// Scan command-line arguments and apply recognised settings options.
    ///
    /// The following argument forms are handled:
    ///  - `-x val`        – short flag registered via AddSetting; `val` is the
    ///                      value string for that setting.
    ///  - `-k arg ...`    – short flag registered via AddKeyword; remaining
    ///                      non-option arguments (up to max_args) are passed to
    ///                      the keyword callback.
    ///  - `--setting val` – same as `setting = val` in a config file.
    ///  - `--keyword arg ...` – triggers a keyword by name with the following
    ///                      non-option arguments.
    ///
    /// Each matched flag/option and its value are removed from `args`;
    /// all other arguments are left untouched.
    /// Errors are immediately fatal; returns true on success.
    bool LoadArgs(emp::vector<emp::String> & args) {
      if (args.size() == 0) return true;
      exe_name = args[0];

      for (size_t i = 1; i < args.size(); ++i) {
        const emp::String test_arg = args[i];

        // Short flag: -x  (single character, registered via AddSetting or AddKeyword)
        if (test_arg.size() == 2 && test_arg[0] == '-' && test_arg[1] != '-') {
          const char flag_char = test_arg[1];
          if (HasFlag(flag_char)) {
            args.erase(args.begin() + i); // Remove the used argument.
            const emp::String & id = flag_map.at(flag_char);
            if (setting_map.contains(id)) {
              LoadArgSetting(args, i, setting_map.at(id), emp::MakeString('-',flag_char));
            } else {
              LoadArgKeyword(args, i, keyword_map.at(id));
            }
            continue;
          }
          emp::notify::Error("Unknown flag '", test_arg, "'.");
        }

        // Long option: --name  (setting value or keyword arguments)
        if (test_arg.size() > 2 && test_arg[0] == '-' && test_arg[1] == '-') {
          const emp::String opt = test_arg.substr(2);
          if (setting_map.contains(opt)) {
            args.erase(args.begin() + i); // Remove the used argument.
            LoadArgSetting(args, i, setting_map.at(opt), "--"+opt);
            continue;
          } else if (keyword_map.contains(opt)) {
            args.erase(args.begin() + i); // Remove the used argument.
            LoadArgKeyword(args, i, keyword_map.at(opt));
            continue;
          }
          emp::notify::Error("Unknown option '", test_arg, "'.");
        }
      }
      return true;
    }

    void PrintStatus(std::ostream & os = std::cout) {
      bool any_changed = false;
      for (const auto & [name, info] : setting_map) {
        if (info.AsString() != info.GetDefault()) {
          if (!any_changed) {
            std::println(os, "Changed settings:");
            any_changed = true;
          }
          std::println(os, "  {} = {}  (default: {})",
                       name, info.AsLiteral(), info.GetDefaultLiteral());
        }
      }
      if (!any_changed) std::println(os, "All settings are at their default values.");
    }

    /// Save all current setting values to a SerialPod.
    /// Only values are stored (not descriptions, defaults, or callbacks).
    /// The full dotted key is saved alongside each value so that SerialLoad can
    /// match them up even if the registration order later changes.
    void SerialSave(emp::SerialPod & pod) const {
      pod.Save(setting_map.size());
      for (const auto & [key, info] : setting_map) {
        pod.Save(key);
        pod.Save(info.AsString());
      }
    }

    /// Restore setting values from a SerialPod.
    /// Settings present in the pod but not currently registered emit a warning and are skipped,
    /// which allows save files to remain usable after settings are added or removed.
    void SerialLoad(emp::SerialPod & pod) {
      const size_t count = pod.LoadValue<size_t>();
      for (size_t i = 0; i < count; ++i) {
        emp::String key   = pod.LoadValue<emp::String>();
        emp::String value = pod.LoadValue<emp::String>();
        if (setting_map.contains(key)) {
          setting_map.at(key).SetValue(value);
        } else {
          emp::notify::Warning("SettingsManager::SerialLoad: setting '", key, "' not found; skipping.");
        }
      }
    }

  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_CONFIG_SETTINGS_MANAGER_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: ofs ident
