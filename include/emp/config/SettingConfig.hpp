/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  SettingConfig.hpp
 *  @brief A tool for collecting settings, including from files and the command line.
 *  @note Status: DEPRECATED!
 */

#ifndef EMP_SETTING_CONFIG_H
#define EMP_SETTING_CONFIG_H

#include <unordered_map>
#include <sstream>
#include <string>
#include <string_view>

#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../config/command_line.hpp"
#include "../datastructs/map_utils.hpp"
#include "../datastructs/vector_utils.hpp"
#include "../math/math.hpp"
#include "../tools/string_utils.hpp"

namespace emp {

  /// Class to take a set of value for each "setting" and then step through all combinations of
  /// those values for a factorial analysis.

  class SettingConfig {
  private:
    /// Base class to describe information about a single setting.
    struct SettingBase {
      std::string name;          ///< Name for this setting
      std::string desc;          ///< Description of setting
      char flag;                 ///< Command-line flag ('\0' for none)
      std::string option;        ///< Command-line longer option.
      std::string args_label;    ///< Label for option arguments (used in --help)

      SettingBase(const std::string & _name, const std::string & _desc,
                  const char _flag, const std::string & _args_label)
        : name(_name), desc(_desc), flag(_flag), option(emp::to_string("--",_name))
        , args_label(_args_label) { }
      virtual ~SettingBase() { }

      virtual size_t GetSize() const = 0;                    ///< How many values are available?
      virtual std::string AsString() const = 0;              ///< All values, as a single string.
      virtual std::string AsString(size_t) const = 0;        ///< A specified value as a string.
      virtual bool FromString(const std::string_view &) = 0; ///< Convert string to set of settings.
      virtual bool SetValueID(size_t) {return false; }       ///< Setup cur value in linked variable
      virtual bool IsComboSetting() { return false; }        ///< Do we have a combo setting?
      virtual size_t GetID() const { return (size_t) -1; }   ///< Combination ID for this setting.

      bool IsOptionMatch(const std::string & test_option) const { return test_option == option; }
      bool IsFlagMatch(const char test_flag) const { return test_flag == flag; }
    };

    /// Full details about a single setting, including type information and values.
    template <typename T>
    struct SettingInfo : public SettingBase {
      T value;
      emp::Ptr<T> var_ptr = nullptr;

      SettingInfo(const std::string & _name,  ///< Unique name for this setting.
                  const std::string & _desc,  ///< Description of this setting (for help)
                  const char _flag,           ///< Single char flag for easy access (e.g., "-h")
                  const std::string & _arg,   ///< Label for option argument (for help)
                  emp::Ptr<T> _var=nullptr)   ///< Pointer to variable to set (optional)
        : SettingBase(_name, _desc, _flag, _arg), var_ptr(_var) { }

      size_t GetSize() const override { return 1; }
      std::string AsString() const override { return emp::to_string(value); }
      std::string AsString(size_t id) const override {
        emp_assert(id == 0);
        return emp::to_string(value);
      }

      bool FromString(const std::string_view & input) override {
        value = emp::from_string<T>(input);
        // @CAO: Could do more tests to make sure whole string was used.
        if (!var_ptr.IsNull()) *var_ptr = value;
        return true;
      }
    };

    /// Allow a single setting to have multiple values specified that should be stepped through.
    template <typename T>
    struct ComboSettingInfo : public SettingBase {
      emp::vector<T> values;                       ///< Set of values to use for this setting.
      emp::Ptr<T> var_ptr = nullptr;               ///< Pointer to variable to set as combos change.
      size_t id;                                   ///< Unique ID/position for this setting.

      ComboSettingInfo(const std::string & _name,  ///< Unique name for this setting.
                       const std::string & _desc,  ///< Description of this setting (for help)
                       const char _flag,           ///< Char flag for easy access (e.g., "-h")
                       const std::string & _args,  ///< Label for option arguments (for help)
                       emp::Ptr<T> _var=nullptr)   ///< Pointer to variable to set (optional)
        : SettingBase(_name, _desc, _flag, _args), var_ptr(_var) { }

      size_t GetSize() const override { return values.size(); }
      std::string AsString() const override {
        std::stringstream ss;
        for (size_t i=0; i < values.size(); i++) {
          if (i) ss << ',';
          ss << values[i];
        }
        return ss.str();
      }
      std::string AsString(size_t id) const override {
        return emp::to_string(values[id]);
      }

      bool FromString(const std::string_view & input) override {
        // Divide up inputs into comma-separated units.
        auto slices = emp::slice(input, ',');

        // Clear out the values to set, one at a time (in most cases, aleady clear)
        values.resize(0);

        // Process each slice into one or more values.
        for (auto & cur_str : slices) {
          // If we are working with an arithmetic type, check if this is a range.
          if constexpr (std::is_arithmetic<T>::value) {
            auto r_slices = emp::slice(cur_str, ':');
            if (r_slices.size() > 3) return false; // Error!  Too many slices!!!
            T start = emp::from_string<T>( r_slices[0] );
            T end = emp::from_string<T>( r_slices.back() ); // Same as start if one value.
            T step = (r_slices.size() == 3) ? emp::from_string<T>( r_slices[1] ) : 1;
            while (start <= end) {
              values.push_back(start);
              start += step;
            }
          }

          // Otherwise do a direct conversion.
          else {
            values.push_back( emp::from_string<T>(cur_str) );
          }
        }

        if (!var_ptr.IsNull() && values.size()) *var_ptr = values[0];
        return values.size();  // Must result in at least one value.
      }

      bool SetValueID(size_t id) override { if (var_ptr) *var_ptr = values[id]; return true; }
      bool IsComboSetting() override { return true; }
      size_t GetID() const override  { return id; }
    };

    /// A setting that is just a flag with an action function to run if it's called.
    struct ActionFlag {
      std::string name;           ///< Name for this flag
      std::string desc;           ///< Description of flag
      char flag;                  ///< Command-line flag ('\0' for none)
      std::function<void()> fun;  ///< Function to be called if flag is set.
    };

    std::string exe_name = "";

    std::map<std::string, emp::Ptr<SettingBase>> setting_map;  ///< All settings by name
    std::map<std::string, ActionFlag> action_map;              ///< Action flags

    emp::vector<emp::Ptr<SettingBase>> combo_settings;         ///< Multi-value settings (in order)
    emp::vector<size_t> cur_combo;    ///< Which combo settings are we currently using?
    size_t combo_id = 0;              ///< Unique value indicating which combination we are on.

    emp::vector<std::string> unused_args;  // Arguments that we were unable to process.
    std::string errors;

  public:
    SettingConfig() = default;

    ~SettingConfig() {
      for (auto [name,ptr] : setting_map) ptr.Delete();
    }

    const std::string & GetExeName() const { return exe_name; }
    size_t GetComboID() const { return combo_id; }
    const emp::vector<std::string> & GetUnusedArgs() const { return unused_args; }
    const std::string & GetErrors() const { return errors; }

    bool HasUnusedArgs() const { return unused_args.size(); }
    bool HasErrors() const { return errors.size(); }

    /// Get the current value of a specified setting.
    template <typename T>
    const T & GetValue(const std::string & name) const {
      emp_assert(emp::Has(setting_map, name), name);
      emp::Ptr<SettingBase> base_ptr = setting_map.find(name)->second;

      // If we have a combo setting, determine the current value.
      if (base_ptr->IsComboSetting()) {
        emp::Ptr<ComboSettingInfo<T>> ptr = base_ptr.Cast<ComboSettingInfo<T>>();
        size_t id = cur_combo[ptr->id];
        return ptr->values[id];
      }

      // Otherwise we have a regular setting.
      return base_ptr.Cast<SettingInfo<T>>()->value;
    }

    /// Scan through all values and return the maximum.
    template <typename T>
    T MaxValue(const std::string & name) const {
      emp_assert(emp::Has(setting_map, name), name);
      emp::Ptr<SettingBase> base_ptr = setting_map.find(name)->second;

      // If we have a combo setting, determine the current value.
      if (base_ptr->IsComboSetting()) {
        emp::Ptr<ComboSettingInfo<T>> ptr = base_ptr.Cast<ComboSettingInfo<T>>();
        return emp::FindMax(ptr->values);
      }

      // Otherwise we have a regular setting with just one value.
      return base_ptr.Cast<SettingInfo<T>>()->value;
    }

    /// Add a new setting of a specified type.  Returns the (initially empty) vector of values
    /// to allow easy setting.
    /// Example:
    ///   config.AddSetting("num_runs") = 200;

    template <typename T>
    T & AddSetting(const std::string & name,
                   const std::string & desc,
                   const char option_flag,
                   T & var,
                   const std::string & args_label="Value")
    {
      emp_assert(!emp::Has(setting_map, name));
      auto new_ptr = emp::NewPtr<SettingInfo<T>>(name, desc, option_flag, args_label, &var);
      setting_map[name] = new_ptr;
      return new_ptr->value;
    }

    /// Add a new setting of a specified type.  Returns the (initially empty) vector of values
    /// to allow easy setting.
    /// Example:
    ///   config.AddComboSetting("pop_size") = { 100,200,400,800 };

    template <typename T>
    emp::vector<T> & AddComboSetting(const std::string & name,
                                     const std::string & desc="",
                                     const char option_flag='\0') {
      emp_assert(!emp::Has(setting_map, name));
      auto new_ptr = emp::NewPtr<ComboSettingInfo<T>>(name, desc, option_flag, "Values...");
      new_ptr->id = combo_settings.size();
      combo_settings.push_back(new_ptr);
      setting_map[name] = new_ptr;
      cur_combo.push_back(0);
      return new_ptr->values;
    }

    /// A setting can also be linked to a value that is kept up-to-date.
    template <typename T>
    emp::vector<T> & AddComboSetting(const std::string & name,
                                     const std::string & desc,
                                     const char option_flag,
                                     T & var,
                                     const std::string & args_label="Values...")
    {
      emp_assert(!emp::Has(setting_map, name));
      auto new_ptr = emp::NewPtr<ComboSettingInfo<T>>(name, desc, option_flag, args_label, &var);
      new_ptr->id = combo_settings.size();
      combo_settings.push_back(new_ptr);
      setting_map[name] = new_ptr;
      cur_combo.push_back(0);
      return new_ptr->values;
    }

    void AddAction(const std::string & name,
                   const std::string & desc,
                   const char flag,
                   std::function<void()> fun)
    {
      std::string name_option = emp::to_string("--", name);
      std::string flag_option = emp::to_string("-", flag);
      emp_assert(!emp::Has(action_map, name_option));
      emp_assert(!emp::Has(action_map, flag_option));
      action_map[name_option] = ActionFlag{ name, desc, flag, fun };
      action_map[flag_option] = ActionFlag{ name, desc, flag, fun };
    }

    /// Access ALL values for a specified setting, to be modified freely.
    template <typename T>
    emp::vector<T> & ComboValues(const std::string & name) {
      emp_assert(emp::Has(setting_map, name), name);
      emp_assert(setting_map[name]->IsComboSetting());
      return setting_map[name].DynamicCast<ComboSettingInfo<T>>()->values;
    }

    /// Start over stepping through all combinations of parameter values.
    void ResetCombos() {
      // Setup as base combo.
      for (size_t & x : cur_combo) x = 0;
      combo_id = 0;

      // Setup all linked values.
      for (auto x : combo_settings) x->SetValueID(0);
    }

    /// Add a single new value to the specified setting.
    template <typename T>
    void AddComboValue(const std::string & name, T && val) {
      emp_assert(emp::Has(setting_map, name), name);
      emp_assert(setting_map[name]->IsComboSetting());
      auto ptr = setting_map[name].DynamicCast<ComboSettingInfo<T>>();
      ptr->values.emplace_back(std::forward<T>(val));
    }

    /// Set all values for the specified setting.
    template <typename T1, typename... Ts>
    void SetComboValues(const std::string & name, T1 && val1, Ts &&... vals) {
      emp_assert(emp::Has(setting_map, name));
      auto ptr = setting_map[name].DynamicCast<ComboSettingInfo<T1>>();
      emp::Append(ptr->values, std::forward<T1>(val1), std::forward<Ts>(vals)...);
    }

    /// Determine how many unique combinations there currently are.
    size_t CountCombos() {
      size_t result = 1;
      for (auto ptr : combo_settings) result *= ptr->GetSize();
      return result;
    }

    /// Set the next combination of settings to be active.  Return true if successful
    /// or false if we ran through all combinations and reset.
    bool NextCombo() {
      combo_id++;
      for (size_t i = 0; i < cur_combo.size(); i++) {
        cur_combo[i]++;

        // Check if this new combo is valid.
        if (cur_combo[i] < combo_settings[i]->GetSize()) {
          combo_settings[i]->SetValueID( cur_combo[i] );    // Set value in linked variable.
          return true;
        }

        // Since it's not, prepare to move on to the next one.
        cur_combo[i] = 0;
        combo_settings[i]->SetValueID(0);
      }

      // No valid combo found.
      combo_id = 0;
      return false;
    }

    /// Get the set of headers used for the CSV file.
    std::string GetSettingHeaders(const std::string & separator=",") {
      std::string out_str;
      for (auto [name,ptr] : setting_map) {
        if (out_str.size()) out_str += separator;
        out_str += ptr->name;
      }
      return out_str;
    }

    /// Convert all of the current values into a comma-separated string.
    std::string CurSettings(const std::string & separator=",") const {
      std::string out_str;
      for (auto [name,ptr] : setting_map) {
        if (ptr) {
          if (out_str.size()) out_str += separator;
          out_str += ptr->IsComboSetting() ? ptr->AsString(cur_combo[ptr->GetID()]) : ptr->AsString();
        }
      }
      return out_str;
    }

    /// Get the set of headers used for the CSV file.
    std::string GetComboHeaders(const std::string & separator=",") {
      std::string out_string;
      for (size_t i = 0; i < combo_settings.size(); i++) {
        if (i) out_string += separator;
        out_string += combo_settings[i]->name;
      }
      return out_string;
    }

    /// Convert all of the current values into a comma-separated string.
    std::string CurComboString(const std::string & separator=",",
                               bool use_labels=false,   ///< Print name with each value?
                               bool multi_only=false    ///< Only print values that can change?
                              ) const {
      std::string out_str;
      for (size_t i = 0; i < cur_combo.size(); i++) {
        if (multi_only && combo_settings[i]->GetSize() == 1) continue;
        if (out_str.size()) out_str += separator;
        if (use_labels) out_str += emp::to_string(combo_settings[i]->name, '=');
        out_str += combo_settings[i]->AsString(cur_combo[i]);
      }
      return out_str;
    }

    /// Scan through all settings for a match option and return ID.
    emp::Ptr<SettingBase> FindOptionMatch(const std::string & option_name) {
      for (const auto & [name, ptr] : setting_map) {
        if (ptr->IsOptionMatch(option_name)) return ptr;
      }
      return nullptr;
    }

    /// Scan through all settings for a match option and return ID.
    emp::Ptr<SettingBase>  FindFlagMatch(const char symbol) {
      for (const auto & [name, ptr] : setting_map) {
        if (ptr->IsFlagMatch(symbol)) return ptr;
      }
      return nullptr;
    }

    /// Take an input set of config options, process them, and track set of unprocessed ones.
    bool ProcessOptions(const emp::vector<std::string> & args) {
      exe_name = args[0];

      for (size_t i = 1; i < args.size(); i++) {
        const std::string & cur_arg = args[i];
        if (cur_arg.size() < 2 || cur_arg[0] != '-') {
          unused_args.push_back(cur_arg);
          continue;  // If isn't an option, continue.
        }

        // See if this is a fully spelled-out option.
        auto setting_ptr = FindOptionMatch(cur_arg);
        if (!setting_ptr.IsNull()) {
          if (++i >= args.size()) {
            errors += "ERROR: Must provide args for option '--";
            errors += setting_ptr->name;
            errors += "' to use!\n";
            std::cerr << errors;
            return false;
          }
          setting_ptr->FromString(args[i]);
          // @CAO: Should make sure string translated correctly.
        }

        // See if we have a flag option.
        setting_ptr = FindFlagMatch(cur_arg[1]);
        if (!setting_ptr.IsNull()) {
          // Check if the flag is followed by the values without whitespace.
          if (cur_arg.size() > 2) {
            setting_ptr->FromString( emp::view_string(cur_arg,2) );
          }
          else if (++i >= args.size()) {
            std::cout << "ERROR: Must provide args to use!\n";
            return false;
          }
          else {
            setting_ptr->FromString(args[i]);
          }
        }

        // Or see of this is a flag trigger.
        else if (Has(action_map, cur_arg)) {
          action_map[cur_arg].fun();
        }

        // Otherwise this argument will go unused; send it back.
        else unused_args.push_back(cur_arg);
      }

      return true;
    }

    bool ProcessOptions(int argc, char* argv[]) {
      return ProcessOptions(emp::cl::args_to_strings(argc, argv));
    }


    template <typename... Ts>
    void PrintHelp(const Ts &... examples) const {

      std::cout << "Format: " << exe_name << " [OPTIONS...]\n"
                << "\nSetting Options:\n";
      for (auto [name, ptr] : setting_map) {
        std::string spacing(emp::Max(1, 12 - (int) ptr->args_label.size()), ' ');
        std::cout << " -" << ptr->flag << " [" << ptr->args_label << "]" << spacing << ": "
                  << ptr->desc << " (--" << name << ") ["
                  << ptr->AsString() << "]\n";
      }

      std::cout << "\nAction Options:\n";
      for (auto [name, action] : action_map) {
        if (name.size() == 2) continue;  // Skip flag entries.
        std::cout << " -" << action.flag << " : "
                  << action.desc << " (" << name << ")\n";
      }

      if constexpr (sizeof...(examples) > 0) {
        std::cout << "\nExample: " << emp::to_string(examples...) << std::endl;
      }

      std::cout.flush();
    }
  };

}

#endif
