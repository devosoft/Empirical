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
 *
 * In addition to settings, users can also specify "Keywords", which trigger an arbitrary callback,
 * receiving the remaining tokens on the line as arguments.  They are useful for arbitrary
 * directives that do not follow the `name = value` pattern (e.g. `include other_file.cfg;`).
 * Keywords are always global; they are not affected by the current scope.
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
 *     cfg.Save("my_config.cfg");    // writes current values with description comments
 *
 *     // Apply settings from command-line arguments (e.g. -S "reps = 5")
 *     emp::vector<emp::String> args(argv, argv + argc);
 *     cfg.LoadArgs(args);
 *
 * === Main methods ===
 *
 *  - AddSetting(name, var, desc [, flag ]) – register a setting bound to 'var';
 *     'flag' is a one-character CLI flag (optional).
 *  - AddKeyword(keyword, fun, desc) – register a keyword that invokes 'fun' with its args.
 *  - Load(istream&) or Load(filename) – parse settings; return success (true/false)
 *  - LoadArgs(args) – scan a 'vector<emp::String>' of command-line arguments, deleting those used:
 *       '-x val' or '--setting val' set specified setting (registered with AddSetting)
 *       '-k arg...' or '--keyword arg...' trigger a keyword (registered with AddKeyword)
 *       '-S "cfg"' or '--set "cfg"' apply a bulk config string (built-in keyword).
 *  - Save(ostream&) or Save(filename) – write all settings as config file; return success.
 *  - Get<T>(name) or Set(name, value) – programmatic get/set.
 *  - HasSetting(name) or HasKeyword(name) or HasIdentifier(name) – query if name is registered.
 *  - HasError() or GetError() – inspect the last error message.
 *  - SetVerbose() – enable diagnostic printing during Load/Save.
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
#include <print>
#include <cstdint>
#include <stddef.h>

#include "../base/notify.hpp"
#include "../base/vector.hpp"
#include "../compiler/Lexer.hpp"
#include "../io/io_utils.hpp"
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

      // Dynamic conversion from a STRING type.
      template <typename TO_T>
      [[nodiscard]] static TO_T Convert(const emp::String & in) {
        constexpr Type to_type = ToTypeEnum<TO_T>();

        if constexpr (to_type == Type::STRING) return in;
        else if constexpr (to_type == Type::BOOL) return !in.AsLower().IsOneOf("off", "false", "0");
        else if constexpr (to_type == Type::INT64) return static_cast<TO_T>(std::stoll(in));
        else if constexpr (to_type == Type::UINT64) return static_cast<TO_T>(in.AsULL());
        else if constexpr (to_type == Type::DOUBLE) return static_cast<TO_T>(in.AsDouble());
        else static_assert(false, "Cannot convert from string to unknown type.");
      }

      // Allow conversions from std::string.
      template <typename TO_T>
      [[nodiscard]] static TO_T Convert(const std::string & in) {
        return Convert<TO_T>(emp::String{in});
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
        , set_string([&var](const emp::String & in){ var = Convert<VAR_T>(in); })
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
        , set_string([setter](const emp::String & in){ setter(Convert<T>(in)); })
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
          static_assert(emp::dependent_false<T>(), "unsupported type");
          return T{};
        }
      }

      /// Set the value; must maintain current type.
      void SetValue(const emp::String & val) { set_string(val); }
      void SetValue(bool val)     { set_bool(val); }
      void SetValue(int64_t val)  { set_int64(val); }
      void SetValue(uint64_t val) { set_uint64(val); }
      void SetValue(double val)   { set_double(val); }

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

    emp::String exe_name;
    std::map<emp::String, SettingInfo> setting_map;
    std::map<emp::String, KeywordInfo> keyword_map;
    std::map<char, emp::String> flag_map;  ///< Flag char -> setting/keyword name
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
    bool LoadArgSetting(emp::vector<emp::String> & args, size_t & i,
                        SettingInfo & info, const emp::String & flag_desc) {
      if (i >= args.size()) {
        return IOError("Expected value after '", flag_desc, "'.");
      }
      info.SetValue(args[i]);
      args.erase(args.begin() + i);
      --i;
      return true;
    }

    /// Apply a keyword at args[i..]
    /// Called after the flag/option token itself has already been consumed.
    /// Returns false if the keyword callback set an error.
    bool LoadArgKeyword(emp::vector<emp::String> & args, size_t & i, const KeywordInfo & info) {
      size_t args_found = CountFlagArgs(args, i, info.max_args);
      emp::vector<emp::String> keyword_vars(args_found);
      for (size_t arg_id = 0; arg_id < args_found; ++arg_id) {
        keyword_vars[arg_id] = args[i + arg_id];
      }
      info.fun(keyword_vars);
      args.erase(args.begin()+i, args.begin()+i+args_found);
      --i;
      return !HasError();
    }

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
        GetSettingInfo(name).SetValue(*string_val);
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
      , string_ID(lexer.AddToken("string", "(\\\"([^\"\\\\]|(\\\\.))*\\\")|(\\'([^'\\\\]|(\\\\.))*\\')"))
    {
      lexer.IgnoreToken("whitespace", "[ \\t\\r]+");
      lexer.IgnoreToken("comment", "#.+");
      lexer.IgnoreToken("continue_line", "\\\\[ ]*\\n");

      // Built-in keyword: applies a bulk config string (same as a config file fragment).
      AddKeyword("set", [this](emp::vector<emp::String> kw_args) {
        if (kw_args.empty()) { IOError("Expected config string after '--set'."); return; }
        emp::TokenStream tokens = lexer.Tokenize(kw_args[0]);
        Iterator it = tokens.begin();
        while (it.Any()) { if (!LoadLine(it)) return; }
      }, "Apply a bulk config string", 'S', /*max_args=*/1);
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
                                 char flag = '\0',
                                 emp::String default_val = "") {
      emp_assert(!HasIdentifier(name), "Trying to add SettingsManager identifier that already exists",
                 AppendScope(name));
      emp_assert(flag == '\0' || !flag_map.contains(flag),
                 "Duplicate CLI flag in SettingsManager", flag);
      const emp::String full_name = AppendScope(name);
      setting_map.emplace(full_name, SettingInfo{full_name, value, desc, flag, default_val});
      if (flag != '\0')    flag_map[flag] = full_name;
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
      emp_assert(!HasIdentifier(name), "Trying to add SettingsManager identifier that already exists",
                 AppendScope(name));
      emp_assert(flag == '\0' || !flag_map.contains(flag),
                 "Duplicate CLI flag in SettingsManager", flag);
      const emp::String full_name = AppendScope(name);
      setting_map.emplace(full_name, SettingInfo{full_name, getter, setter, desc, flag, default_val});
      if (flag != '\0')    flag_map[flag] = full_name;
      return *this;
    }

    /// A specified keyword can be added to the settings; it will call a provided
    /// function with the remaining tokens before the next semicolon.
    SettingsManager & AddKeyword(const emp::String & keyword,
                                 keyword_fun_t fun,
                                 emp::String desc,
                                 char flag = '\0',
                                 size_t max_args = emp::MAX_SIZE_T) {
      emp_assert(!HasIdentifier(keyword), "Adding keyword with preexisting identifier", keyword);
      emp_assert(flag == '\0' || !flag_map.contains(flag), "Duplicate CLI flag in SettingsManager",
        keyword, flag);
      keyword_map.emplace(keyword, KeywordInfo{keyword, fun, desc, flag, max_args});
      if (flag != '\0') flag_map[flag] = keyword;
      return *this;
    }

    // Print out all info on the currently known settings.
    void PrintSettings(std::ostream & os=std::cout) {
      std::println(os, "Available settings:");
      for (const auto & [name, info] : setting_map) {
        std::print(os, "  {} : {} (Default: {}", name, info.GetDescription(), info.GetDefaultLiteral());
        if (info.GetFlag()) std::println(os, "; setting flag: -{})", info.GetFlag());
        else std::println(os, ")");
      }
    }

    // Print out all info on the current options.
    void PrintHelp(const emp::vector<emp::String> & args, std::ostream & os=std::cout) {
      if (args.size()) {
        for (const emp::String & arg : args) {
          if (arg == "settings") PrintSettings(os);
          else std::println(os, "Unknown help argument '{}'", arg);
        }
        return;
      }

      std::println(os, "Format: {} [flags]", exe_name);
      std::println(os, "Allowed flags include:");

      for (const auto & [name, info] : keyword_map) {
        if (info.flag) {
          std::println(os, "  --{} (or -{}) : {}", name, info.flag, info.desc);
        } else {
          std::println(os, "  --{} : {}", name, info.desc);
        }
      }

      std::println(os, "Use `{} --help settings` for a full list of settings", exe_name);
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

    /// Write a starter config file using each setting's default value.
    /// Useful for distributing a template that users can customize.
    bool SaveTemplate(std::ostream & ofs) {
      emp_assert(ofs);
      error_note.clear();

      for (const auto & [key, info] : setting_map) {
        const auto lines = info.GetDescription().Slice("\n");
        for (const auto & line : lines) { ofs << "# " << line << "\n"; }
        ofs << key << " = " << info.GetDefaultLiteral() << ";\n\n";
      }

      return true;
    }

    bool SaveTemplate(const emp::String & filename) {
      std::ofstream ofs{filename};
      if (!ofs) {
        error_note.Set("Failed to open config file for saving: ", filename);
        notify::Error(error_note);
        return false;
      }
      return SaveTemplate(ofs);
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
    /// The following argument forms are handled:
    ///  - `-x val`        – short flag registered via AddSetting; `val` is the
    ///                      value string for that setting.
    ///  - `-k arg ...`    – short flag registered via AddKeyword; remaining
    ///                      non-option arguments (up to max_args) are passed to
    ///                      the keyword callback.
    ///  - `--setting val` – same as `setting = val` in a config file.
    ///  - `--keyword arg ...` – triggers a keyword by name with the following
    ///                      non-option arguments.
    ///  - `-S "cfg"` / `--set "cfg"` – built-in "set" keyword; tokenizes the
    ///                      string and loads it exactly as a config file.
    ///
    /// Each matched flag/option and its value are removed from `args`;
    /// all other arguments are left untouched.
    /// Returns false (and sets the error note) on the first parse error.
    bool LoadArgs(emp::vector<emp::String> & args) {
      if (args.size() == 0) return true;
      exe_name = args[0];

      error_note.clear();
      for (size_t i = 1; i < args.size(); ++i) {
        const emp::String test_arg = args[i];

        // Short flag: -x  (single character, registered via AddSetting or AddKeyword)
        if (test_arg.size() == 2 && test_arg[0] == '-' && test_arg[1] != '-') {
          const char flag_char = test_arg[1];
          if (flag_map.contains(flag_char)) {
            args.erase(args.begin() + i); // Remove the used argument.
            const emp::String & id = flag_map.at(flag_char);
            if (setting_map.contains(id)) {
              if (!LoadArgSetting(args, i, setting_map.at(id), emp::MakeString('-',flag_char))) {
                return false;
              }
            } else {
              if (!LoadArgKeyword(args, i, keyword_map.at(id))) return false;
            }
            continue;
          }
        }

        // Long option: --name  (setting value or keyword arguments)
        if (test_arg.size() > 2 && test_arg[0] == '-' && test_arg[1] == '-') {
          const emp::String opt = test_arg.substr(2);
          if (setting_map.contains(opt)) {
            args.erase(args.begin() + i); // Remove the used argument.
            if (!LoadArgSetting(args, i, setting_map.at(opt), "--"+opt)) return false;
            continue;
          } else if (keyword_map.contains(opt)) {
            args.erase(args.begin() + i); // Remove the used argument.
            if (!LoadArgKeyword(args, i, keyword_map.at(opt))) return false;
            continue;
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
