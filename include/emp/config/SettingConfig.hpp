/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025.
*/
/**
 *  @file
 *  @brief A simple tool for managing configuration settings.
 *  @note An older version of this class became SettingCombos.hpp
 *  @note Status: Alpha
 * 
 *  Settings can be either a string of a numerical value (size_t or double).
 * 
 *  DEVELOPER NOTES:
 *  - Consider allowing types to be more dynamic, perhaps set in a template.
 */

#ifndef EMP_CONFIG_SETTING_MANAGER_HPP_INCLUDE
#define EMP_CONFIG_SETTING_MANAGER_HPP_INCLUDE

#include <fstream>
#include <map>
#include <sstream>
#include <stddef.h>
#include <unordered_map>
#include <variant>

#include "../base/notify.hpp"
#include "../base/vector.hpp"
#include "../compiler/Lexer.hpp"
#include "../config/command_line.hpp"
#include "../io/File.hpp"
#include "../tools/String.hpp"

namespace emp {

  class SettingManager {
  private:
    using val_t = std::variant<emp::String, int, double>;

    /// Class to manage a single setting.
    class SettingInfo {
    private:
      emp::String name;       ///< Label for this setting in config files
      val_t value;            ///< Current value of this setting.
      emp::String desc="";    ///< Description of setting
      char flag='\0';         ///< Command-line flag ('\0' for none)
      emp::String option="";  ///< Command-line longer option ("" for none)

      // Helper: convert from string to val_t of current type.
      val_t ParseFromString(const emp::String& str) const {
        if (IsString()) { return MakeStringFromLiteral(str); }
        if (IsInt()) { return std::stoi(str); }
        if (IsDouble()) { return std::stod(str); }
        emp_assert(false, "Invalid type in Setting Info", name);
        return 0.0;
      }

    public:
      SettingInfo() = delete;
      SettingInfo(const SettingInfo &) = default;
      SettingInfo(SettingInfo &&) = default;

      SettingInfo(emp::String name, emp::String value, emp::String desc,
                  char flag='\0', emp::String option="")
        : name(name), value(value), desc(desc), flag(flag), option(option) { }
      SettingInfo(emp::String name, int value, emp::String desc,
                  char flag='\0', emp::String option="")
        : name(name), value(value), desc(desc), flag(flag), option(option) { }
      SettingInfo(emp::String name, double value, emp::String desc,
                  char flag='\0', emp::String option="")
        : name(name), value(value), desc(desc), flag(flag), option(option) { }

      template <typename T>
      [[nodiscard]] bool IsType() const { return std::holds_alternative<T>(value); }
      [[nodiscard]] bool IsString() const { return IsType<emp::String>(); }
      [[nodiscard]] bool IsInt() const { return IsType<int>(); }
      [[nodiscard]] bool IsDouble() const { return IsType<double>(); }

      [[nodiscard]] const emp::String& GetName() const { return name; }
      [[nodiscard]] const emp::String& GetDescription() const { return desc; }
      [[nodiscard]] char GetFlag() const { return flag; }
      [[nodiscard]] const emp::String& GetOption() const { return option; }

      template <typename T>
      [[nodiscard]] const T& GetValue() const { return std::get<T>(value); }

      /// Set the value; must maintain current type.
      template <typename T>
      void SetValue(T && val) {
        emp_assert(IsType<T>(), name);
        value = std::forward<T>(val);
      }

      /// Convert to string for display
      [[nodiscard]] emp::String AsString() const {
        if (IsString()) { return std::get<emp::String>(value); }
        if (IsInt()) { return emp::MakeString(std::get<int>(value)); }
        if (IsDouble()) { return emp::MakeString(std::get<double>(value)); }
        emp_assert(false, "Invalid type in Setting Info", name);
        return "";
      }

      /// If input is a string, convert it to the correct type.
      void SetFromString(const emp::String& input) {
        value = ParseFromString(input);
      }

      /// Check the type
      [[nodiscard]] std::string GetTypeName() const {
        if (IsString()) return "emp::String";
        if (IsInt()) return "int";
        if (IsDouble()) return "double";
        return "error";
      }
    };

    std::map<emp::String, SettingInfo> setting_map;

    SettingInfo & GetInfo(const emp::String & name) {
      emp_assert(Has(name), "Invalid setting name", name);
      return setting_map.find(name)->second;
    }
    const SettingInfo & GetInfo(const emp::String & name) const {
      emp_assert(Has(name), "Invalid setting name", name);
      return setting_map.find(name)->second;
    }
  public:

    [[nodiscard]] bool Has(const emp::String & name) const { return setting_map.contains(name); }

    template <typename T>
    [[nodiscard]] T Get(const emp::String & name) const {
      return GetInfo(name).GetValue<T>();
    }

    [[nodiscard]] const emp::String & GetDesc(const emp::String & name) const {
      return GetInfo(name).GetDescription();
    }
    [[nodiscard]] char GetFlag(const emp::String & name) const {
      return GetInfo(name).GetFlag();
    }
    [[nodiscard]] const emp::String & GetOption(const emp::String & name) const {
      return GetInfo(name).GetOption();
    }

    template <typename T>
    void Set(const emp::String & name, T && value) {
      GetInfo(name).SetValue(std::forward<T>(value));
    }

    template <typename T>
    void AddSetting(const emp::String & name, T && value, emp::String desc,
                    char flag='\0', emp::String option="") {
      emp_assert(!Has(name), "Trying to add a SettingManager setting that already exists", name);
      setting_map.emplace(name, name, std::forward<T>(value), desc, flag, option);
    }

    void Save(emp::String filename) const {
      std::ofstream ofs(filename);
      if (!ofs) {
        notify::Error("Failed to open config file for saving:", filename);
        return;
      }

      for (const auto& [key, info] : setting_map) {
        ofs << "# " << info.GetDescription() << "\n";
        ofs << key << " = " << info.AsString().AsLiteral() << "\n\n";
      }
    }

    void Load(emp::String filename) {
      Lexer lexer;
      const size_t ident_ID = lexer.AddToken("identifier", "[a-zA-Z_][a-zA-Z0-9_]*");
      const size_t int_ID = lexer.AddToken("int", "[0-9]+");
      const size_t double_ID = lexer.AddToken("double", "[0-9]+.[0-9]+");
      const size_t string_ID = lexer.AddToken("string", "(\\\"([^\"\\\\]|(\\\\.))*\\\")");
      const size_t assign_ID = lexer.AddToken("assign", "=");
      lexer.IgnoreToken("whitespace", "[ \\t\\r\\n]+");
      lexer.IgnoreToken("comment", "#.+");
      lexer.IgnoreToken("spacing", "\";\"");
      const size_t error_ID = lexer.AddToken("error", ".");

      auto tokens = lexer.TokenizeFile(filename);
      auto it = tokens.begin();
      while (it.Any()) {
        emp::String name =
          it.Use(ident_ID, "Expected config line to start with identifier, not '",
                 it.Peek().lexeme, "'.").lexeme;
        if (!setting_map.contains(name)) {
          notify::Error("Unknown configuration setting, '", name, "'.");
        }
        it.Use(assign_ID, "Expected assignment ('='), not '", it.Peek().lexeme, "'.").lexeme;
        Token value_token = it.Use();

        GetInfo(name).SetFromString(value_token.lexeme);
      }
    }

  };

}

#endif // #ifndef EMP_CONFIG_SETTING_MANAGER_HPP_INCLUDE
