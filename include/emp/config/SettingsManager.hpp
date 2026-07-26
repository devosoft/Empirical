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
 * SettingsManager maintains a collection of named values. Settings are writable, user-facing
 * values that appear in generated configuration files. Other values may be fixed constants,
 * dynamic getters, or writable values intentionally omitted from generated configuration.
 * Settings are usually associated with C++ variables, but can also trigger more complex functions
 * (for example, setting a random number seed might call ResetSeed on the random number generator.)
 * 
 * Supported value types are `emp::String`, `bool`, `int64_t`, `uint64_t`, and `double`.
 *
 * Values can be organized into scopes using dot-notation names (e.g. `"robot1.speed"`), which
 * simplifies configuring multiple objects of the same type without name collisions.
 * (Internally all named values are stored with their full dotted key.)
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
 *     setting_name = other_name;     # copy the current value of another named value
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
 * Values on the right-hand side use lexical lookup: the current scope is checked first, followed
 * by each parent scope and finally global scope.
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
 *     cfg.AddValue("PI", 3.141592653589793);              // fixed, read-only value
 *     cfg.AddValue("time", [] { return GetTime(); });     // dynamic, read-only value
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
 *  - AddValue(name, value [, desc ]) – register a fixed, read-only value.
 *  - AddValue(name, getter [, desc ]) – register a dynamic, read-only value.
 *  - AddValue(name, getter, setter [, desc ]) – register a writable value that is omitted from
 *     generated configuration, command-line options, help, status, and serialized setting state.
 *  - AddKeyword(keyword, fun, desc) – register a keyword that invokes 'fun' with its args.
 *  - AddOutputKeyword(keyword, fun, desc) – register a keyword whose ostream can be redirected.
 *  - Load(istream&) or Load(filename) – parse settings; errors are immediately fatal.
 *  - LoadArgs(args) – scan a 'vector<emp::String>' of command-line arguments, deleting those used:
 *       '-x val' or '--setting val' set specified setting (registered with AddSetting)
 *       '-k arg...' or '--keyword arg...' trigger a keyword (registered with AddKeyword)
 *  - Save(ostream&) or Save(filename) – write all settings as config file using defaults.
 *  - SaveCurrent(ostream&) or SaveCurrent(filename) – same but uses current (live) values.
 *  - Get<T>(name) or Set(name, value) – programmatic get/set; setting a read-only value is fatal.
 *  - HasValue(name), HasSetting(name), HasKeyword(name), or HasIdentifier(name) – query names.
 *  - SetVerbose() – enable diagnostic printing during Load/Save.
 *  - SetOutputPathResolver(fun) – customize paths used by output-keyword redirection.
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
#include <concepts>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <print>
#include <stddef.h>
#include <system_error>
#include <type_traits>
#include <utility>

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
    using output_keyword_fun_t = std::function<void(keyword_fun_arg_t, std::ostream &)>;
    using output_path_resolver_t =
      std::function<std::filesystem::path(const std::filesystem::path &)>;
    using Iterator = emp::TokenStream::Iterator;

    /// Class to manage a named value, including user-facing settings.
    class SettingInfo {
    private:
      emp::String name;            ///< Label for this value in config files
      emp::String desc   = "";     ///< Description of value
      emp::String default_val;     ///< Default value as a string (for SaveTemplate)
      char flag          = '\0';   ///< Command-line flag ('\0' for none)
      bool is_setting    = false;  ///< Should this value be exposed and saved as a setting?

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
        requires (!std::invocable<VAR_T>)
      SettingInfo(emp::String name, VAR_T & var, emp::String desc, char flag = '\0',
                  emp::String explicit_default = "")
        : name(name), desc(desc)
        , default_val(explicit_default.empty() ? Convert<emp::String>(var) : explicit_default)
        , flag(flag), is_setting(true), type(ToTypeEnum<VAR_T>())
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
                  emp::String desc, char flag = '\0', emp::String explicit_default = "",
                  bool is_setting = true)
        : name(name), desc(desc)
        , default_val(is_setting
                        ? (explicit_default.empty() ? Convert<emp::String>(getter()) : explicit_default)
                        : "")
        , flag(flag), is_setting(is_setting), type(ToTypeEnum<T>())
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

      // Create a read-only value from a getter. The getter is not called during registration.
      template <typename GETTER_T,
                typename T = std::remove_cvref_t<std::invoke_result_t<GETTER_T>>>
        requires std::invocable<GETTER_T>
      SettingInfo(emp::String name, GETTER_T getter, emp::String desc)
        : name(name), desc(desc), default_val(""), flag('\0'), is_setting(false)
        , type(ToTypeEnum<T>())
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
      [[nodiscard]] bool IsSetting() const { return is_setting; }
      [[nodiscard]] bool CanSet() const { return static_cast<bool>(set_string); }

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
      void SetValue(const emp::String & val) {
        if (!CanSet()) emp::notify::Error("Cannot assign to read-only value '", name, "'.");
        set_string(val);
      }
      void SetValue(const std::string & val) { SetValue(emp::String{val}); }
      void SetValue(const char * val) { SetValue(emp::String{val}); }

      /// Set the value from a typed argument; dispatches by concept like GetValue.
      template <typename T>
      void SetValue(T val) {
        if (!CanSet()) emp::notify::Error("Cannot assign to read-only value '", name, "'.");
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
    std::map<emp::String, SettingInfo> setting_map;  ///< All named values, including settings
    std::map<emp::String, KeywordInfo> keyword_map;
    std::map<char, emp::String> flag_map;  ///< Flag char -> setting/keyword name
    emp::vector<emp::String> cur_scopes{};
    bool verbose = false;
    output_path_resolver_t output_path_resolver =
      [](const std::filesystem::path & path) { return path; };

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

    emp::String AppendScope(const emp::String & name, size_t scope_count) const {
      emp_assert(name.size() > 0);
      emp::String out;
      for (size_t i = 0; i < scope_count; ++i) {
        out += cur_scopes[i] + '.';
      }
      out += name;
      return out;
    }

    emp::String AppendScope(const emp::String & name) const {
      return AppendScope(name, cur_scopes.size());
    }

    /// Resolve a value name lexically, starting in the current scope and walking outward.
    [[nodiscard]] emp::String ResolveValueName(const emp::String & name) const {
      for (size_t scope_count = cur_scopes.size(); ; --scope_count) {
        const emp::String candidate = AppendScope(name, scope_count);
        if (setting_map.contains(candidate)) return candidate;
        if (scope_count == 0) break;
      }
      return "";
    }

    auto & GetValueInfo(this auto & self, const emp::String & name) {
      if (!self.HasValue(name)) emp::notify::Error("SettingsManager: unknown value '", name, "'.");
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
        const emp::String resolved_name = ResolveValueName(token.lexeme);
        if (resolved_name.size()) {
          return setting_map.at(resolved_name).AsString();
        }
        emp::notify::Error("Identifier '", token.lexeme, "' UNKNOWN!");
      }

      // If we made it this far, we don't know how to do the conversion.
      emp::notify::Error("UnexpectedToken '", token.lexeme, "'; expected value.");
    }

    // We have found a writable value name at the beginning of a line; assign it!
    void LoadAssignment(Iterator & it, const emp::String & name) {
      if (verbose) emp::PrintLn("...identified as named value!");

      // The value name must be followed by an '='.
      RequireToken(it, '=', "assignment");
      GetValueInfo(name).SetValue(TokenToStringValue(it.Use()));
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

      const emp::String name = name_token.lexeme;
      const bool is_keyword = cur_scopes.empty() && HasKeyword(name);
      if (name_token != ident_ID && !is_keyword) {
        emp::notify::Error(
          "UnexpectedToken '", name_token.lexeme, "'; expected keyword or parameter name."
        );
      }
      if (is_keyword) { LoadKeyword(it, name); }
      else if (HasValue(name)) { LoadAssignment(it, name); }
      else { LoadScope(it, name); } // Unknown id must be a new scope.
    }

    /// Is prefix a complete scope prefix of name? (For example, "robot" of "robot.speed".)
    [[nodiscard]] static bool IsScopePrefix(const emp::String & prefix,
                                            const emp::String & name) {
      return prefix.size() < name.size()
        && name.compare(0, prefix.size(), prefix) == 0
        && name[prefix.size()] == '.';
    }

    /// Validate and fully qualify a new named value before registration.
    [[nodiscard]] emp::String ValidateValueName(const emp::String & name) const {
      if (!name.IsIdentifierChain()) {
        emp::notify::Error(
          "SettingsManager: invalid value name '", name,
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
            "SettingsManager: value names '", full_name, "' and '", other_name,
            "' are structurally ambiguous; a name cannot be both a value and a scope."
          );
        }
      }

      for (const auto & [keyword, info] : keyword_map) {
        (void) info;
        if (keyword.find('.') == emp::String::npos && IsScopePrefix(keyword, full_name)) {
          emp::notify::Error(
            "SettingsManager: global keyword '", keyword,
            "' conflicts with the first component of value name '", full_name, "'."
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
        for (const auto & [value_name, info] : setting_map) {
          (void) info;
          if (IsScopePrefix(keyword, value_name)) {
            emp::notify::Error(
              "SettingsManager: global keyword '", keyword,
              "' conflicts with the first component of value name '", value_name, "'."
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

    /// Control how filenames from output redirection are resolved.  By default paths are used
    /// unchanged.  Applications can use this hook to place relative paths in a data directory.
    SettingsManager & SetOutputPathResolver(output_path_resolver_t resolver) {
      emp_assert(resolver, "SettingsManager output path resolver cannot be empty.");
      output_path_resolver = std::move(resolver);
      return *this;
    }

    /// Does this scope contain a named value? Settings are also values.
    [[nodiscard]] bool HasValue(const emp::String & name) const {
      return setting_map.contains(AppendScope(name));
    }

    [[nodiscard]] bool HasSetting(const emp::String & name) const {
      const auto it = setting_map.find(AppendScope(name));
      return it != setting_map.end() && it->second.IsSetting();
    }

    [[nodiscard]] bool HasKeyword(const emp::String & name) const {
      return keyword_map.contains(name);
    }

    [[nodiscard]] bool HasIdentifier(const emp::String & name) const {
      return HasValue(name) || HasKeyword(name);
    }

    template <typename T>
    [[nodiscard]] T Get(const emp::String & name) const {
      return GetValueInfo(name).GetValue<T>();
    }

    [[nodiscard]] const emp::String & GetDesc(const emp::String & name) const {
      return GetValueInfo(name).GetDescription();
    }

    [[nodiscard]] bool HasFlag(char flag) const { return flag && flag_map.contains(flag); }
    [[nodiscard]] char GetFlag(const emp::String & name) const { return GetValueInfo(name).GetFlag(); }

    template <typename T>
    void Set(const emp::String & name, T && value) {
      GetValueInfo(name).SetValue(std::forward<T>(value));
    }

    const std::filesystem::path & GetConfigDir() const { return config_dir; }
    void SetConfigDir(const emp::String & in) { config_dir = in.str(); }

    template <typename T>
    SettingsManager & AddSetting(const emp::String & name,
                                 T & value,
                                 emp::String desc,
                                 char flag = '\0',
                                 emp::String default_val = "") {
      const emp::String full_name = ValidateValueName(name);
      SetupFlag(flag, full_name);
      setting_map.emplace(full_name, SettingInfo{full_name, value, desc, flag, default_val});
      return *this;
    }

    /// Register a setting controlled by explicit getter/setter functions.
    /// The value type T is deduced from the getter's return type.
    template <typename GETTER_T, typename SETTER_T>
      requires std::invocable<GETTER_T>
        && std::invocable<
             SETTER_T,
             std::remove_cvref_t<std::invoke_result_t<GETTER_T>>
           >
    SettingsManager & AddSetting(const emp::String & name,
                                 GETTER_T getter,
                                 SETTER_T setter,
                                 emp::String desc,
                                 char flag = '\0',
                                 emp::String default_val = "") {
      const emp::String full_name = ValidateValueName(name);
      SetupFlag(flag, full_name);
      setting_map.emplace(full_name, SettingInfo{full_name, getter, setter, desc, flag, default_val});
      return *this;
    }

    /// Register an immutable value. The value is owned by the manager and is not evaluated again.
    template <typename T>
      requires (!std::invocable<std::remove_cvref_t<T>>)
    SettingsManager & AddValue(const emp::String & name, T && value, emp::String desc = "") {
      using value_t = std::conditional_t<
        std::is_convertible_v<T, const char *>, emp::String, std::decay_t<T>
      >;
      return AddValue(
        name,
        [stored_value = value_t(std::forward<T>(value))]() { return stored_value; },
        desc
      );
    }

    /// Register a dynamic, read-only value supplied by a getter.
    template <typename GETTER_T>
      requires std::invocable<GETTER_T>
    SettingsManager & AddValue(const emp::String & name,
                               GETTER_T getter,
                               emp::String desc = "") {
      const emp::String full_name = ValidateValueName(name);
      setting_map.emplace(full_name, SettingInfo{full_name, getter, desc});
      return *this;
    }

    /// Register a writable value that remains hidden from generated setting interfaces.
    template <typename GETTER_T, typename SETTER_T>
      requires std::invocable<GETTER_T>
        && std::invocable<
             SETTER_T,
             std::remove_cvref_t<std::invoke_result_t<GETTER_T>>
           >
    SettingsManager & AddValue(const emp::String & name,
                               GETTER_T getter,
                               SETTER_T setter,
                               emp::String desc = "") {
      const emp::String full_name = ValidateValueName(name);
      setting_map.emplace(
        full_name,
        SettingInfo{full_name, getter, setter, desc, '\0', "", false}
      );
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

    /// Register a keyword whose output can be redirected with a trailing `> filename` or
    /// `>> filename`.  Redirection is opt-in so ordinary keywords can preserve a nested command's
    /// redirection tokens for later execution.
    SettingsManager & AddOutputKeyword(const emp::String & keyword,
                                       output_keyword_fun_t fun,
                                       emp::String desc,
                                       char flag = '\0',
                                       size_t max_args = emp::MAX_SIZE_T) {
      return AddKeyword(
        keyword,
        [this, keyword, fun=std::move(fun)](keyword_fun_arg_t args) {
          size_t redirect_pos = args.size();
          bool append = false;

          const bool missing_filename = !args.empty() && (
            args.back() == ">" || args.back() == ">>" ||
            (args.size() >= 2 && args[args.size()-2] == ">" && args.back() == ">")
          );
          if (missing_filename) {
            emp::notify::Error(keyword, " output redirection is missing its filename.");
          }

          // The config lexer produces two `>` tokens, while CLI parsing may preserve `>>`.
          if (args.size() >= 3 && args[args.size()-3] == ">" && args[args.size()-2] == ">") {
            redirect_pos = args.size() - 3;
            append = true;
          } else if (args.size() >= 2 && args[args.size()-2] == ">>") {
            redirect_pos = args.size() - 2;
            append = true;
          } else if (args.size() >= 2 && args[args.size()-2] == ">") {
            redirect_pos = args.size() - 2;
          }

          if (redirect_pos == args.size()) {
            fun(std::move(args), std::cout);
            return;
          }

          emp::String filename = args.back();
          if (filename.IsLiteralString("\"'")) {
            filename = filename.ConvertStringFromLiteral("\"'");
          }
          if (filename.empty()) {
            emp::notify::Error(keyword, " output redirection requires a non-empty filename.");
          }
          args.resize(redirect_pos);

          const std::filesystem::path output_path = output_path_resolver(filename.str());
          const std::filesystem::path parent_path = output_path.parent_path();
          if (!parent_path.empty()) {
            std::error_code error;
            std::filesystem::create_directories(parent_path, error);
            if (error) {
              emp::notify::Error(
                "Unable to create output directory '", parent_path.string(), "' for ", keyword,
                ": ", error.message(), "."
              );
            }
          }

          const auto mode = std::ios::out | (append ? std::ios::app : std::ios::trunc);
          std::ofstream output{output_path, mode};
          if (!output) {
            emp::notify::Error(
              "Unable to open '", output_path.string(), "' for ", keyword, " output."
            );
          }
          fun(std::move(args), output);
          output.flush();
          if (!output) {
            emp::notify::Error(
              "Failed while writing ", keyword, " output to '", output_path.string(), "'."
            );
          }
        },
        std::move(desc), flag, max_args
      );
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
        if (!info.IsSetting()) continue;
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
        if (!info.IsSetting()) continue;
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
        if (!info.IsSetting()) continue;

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
            if (setting_map.contains(id) && setting_map.at(id).IsSetting()) {
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
          if (setting_map.contains(opt) && setting_map.at(opt).IsSetting()) {
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
        if (!info.IsSetting()) continue;
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
      size_t setting_count = 0;
      for (const auto & [key, info] : setting_map) {
        (void) key;
        if (info.IsSetting()) ++setting_count;
      }

      pod.Save(setting_count);
      for (const auto & [key, info] : setting_map) {
        if (!info.IsSetting()) continue;
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
        if (setting_map.contains(key) && setting_map.at(key).IsSetting()) {
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
