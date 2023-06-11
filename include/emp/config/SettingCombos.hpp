/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file SettingCombos.hpp
 *  @brief A tool for exploring all parameter combinations
 *  @note Status: ALPHA
 */

#ifndef EMP_CONFIG_SETTINGCOMBOS_HPP_INCLUDE
#define EMP_CONFIG_SETTINGCOMBOS_HPP_INCLUDE

#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../datastructs/vector_utils.hpp"
#include "../math/math.hpp"
#include "../tools/string_utils.hpp"

namespace emp {

  /// Class to take a set of value for each "setting" and then step through all combinations of
  /// those values for a factorial analysis.

  class SettingCombos {
  private:
    /// Base class to describe information about a single setting.
    struct SettingBase {
      size_t id;                 ///< Unique ID/position for this setting.
      std::string name;          ///< Name for this setting
      std::string desc;          ///< Description of setting
      char flag;                 ///< Command-line flag ('\0' for none)
      std::string option;        ///< Command-line longer option.
      std::string args_label;    ///< Label for option arguments (used in --help)
      size_t cap = (size_t) -1;  ///< Max number of settings allowed in combo

      SettingBase(const std::string & _name, const std::string & _desc,
                  const char _flag, const std::string & _args_label, const size_t _cap)
        : name(_name), desc(_desc), flag(_flag), option(emp::to_string("--",_name))
        , args_label(_args_label), cap(_cap) { }
      virtual ~SettingBase() { }

      virtual size_t GetSize() const = 0;                    ///< How many values are available?
      virtual std::string AsString() const = 0;              ///< All values, as a single string.
      virtual std::string AsString(size_t) const = 0;        ///< A specified value as a string.
      virtual bool FromString(const std::string_view &) = 0; ///< Convert string to set of settings.
      virtual void SetValueID(size_t) = 0;                   ///< Setup cur value in linked variable
      virtual bool OK() const = 0;                           ///< Any problems with this setting?

      bool IsOptionMatch(const std::string & test_option) const { return test_option == option; }
      bool IsFlagMatch(const char test_flag) const { return test_flag == flag; }
    };

    /// Full details about a single setting, including type information and values.
    template <typename T>
    struct SettingInfo : public SettingBase {
      emp::vector<T> values;
      emp::Ptr<T> var_ptr = nullptr;

      SettingInfo(const std::string & _name,
                  const std::string & _desc,
                  const char _flag,
                  const std::string & _args_label,
                  const size_t _cap,
                  emp::Ptr<T> _var=nullptr)
        : SettingBase(_name, _desc, _flag, _args_label, _cap), var_ptr(_var) { }

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

      bool OK() const override {
        return values.size() > 0 && values.size() <= cap;
      }

      bool FromString(const std::string_view & input) override {
        values = emp::from_strings<T>(emp::slice(input, ','));
        return OK();
      }

      void SetValueID(size_t id) override {
        if (var_ptr) *var_ptr = values[id];
      }
    };

    /// A setting that is just a flag with an action function to run if it's called.
    struct ActionFlag {
      std::string name;           ///< Name for this flag
      std::string desc;           ///< Description of flag
      char flag;                  ///< Command-line flag ('\0' for none)
      std::function<void()> fun;  ///< Function to be called if flag is set.
    };

    std::string exe_name = "";

    emp::vector<emp::Ptr<SettingBase>> settings;               ///< Order to be varied.
    std::map<std::string, emp::Ptr<SettingBase>> setting_map;  ///< Settings by name.
    std::map<std::string, ActionFlag> action_map;              ///< Available flags

    emp::vector<size_t> cur_combo;    ///< Which settings are we currently using?
    size_t combo_id = 0;              ///< Unique value indicating which combination we are on.

  public:
    SettingCombos() = default;

    ~SettingCombos() {
      for (auto ptr : settings) ptr.Delete();
    }

    size_t GetComboID() const { return combo_id; }

    /// Start over stepping through all combinations of parameter values.
    void Reset() {
      // Setup as base combo.
      for (size_t & x : cur_combo) x = 0;
      combo_id = 0;

      // Setup all linked values.
      for (auto x : settings) x->SetValueID(0);
    }

    /// Get the current value of a specified setting.
    template <typename T>
    const T & GetValue(const std::string & name) const {
      emp_assert(emp::Has(setting_map, name), name);
      emp::Ptr<SettingBase> base_ptr = setting_map.find(name)->second;
      emp::Ptr<SettingInfo<T>> ptr = base_ptr.Cast<SettingInfo<T>>();
      size_t id = cur_combo[ptr->id];
      return ptr->values[id];
    }

    /// Scan through all values and return the maximum.
    template <typename T>
    T MaxValue(const std::string & name) const {
      emp_assert(emp::Has(setting_map, name), name);
      emp::Ptr<SettingBase> base_ptr = setting_map.find(name)->second;
      emp::Ptr<SettingInfo<T>> ptr = base_ptr.Cast<SettingInfo<T>>();
      return emp::FindMax(ptr->values);
    }

    /// Add a new setting of a specified type.  Returns the (initially empty) vector of values
    /// to allow easy setting.
    /// Example:
    ///   combos.AddSetting("pop_size") = {100,200,400,800};

    template <typename T>
    emp::vector<T> & AddSetting(const std::string & name,
                                const std::string & desc="",
                                const char option_flag='\0') {
      emp_assert(!emp::Has(setting_map, name));
      emp::Ptr<SettingInfo<T>> new_ptr =
        emp::NewPtr<SettingInfo<T>>(name, desc, option_flag, "Values...", (size_t) -1);
      new_ptr->id = settings.size();
      settings.push_back(new_ptr);
      setting_map[name] = new_ptr;
      cur_combo.push_back(0);
      return new_ptr->values;
    }

    /// A setting can also be linked to a value that is kept up-to-date.
    template <typename T>
    emp::vector<T> & AddSetting(const std::string & name,
                                const std::string & desc,
                                const char option_flag,
                                T & var,
                                const std::string & args_label="Values...",
                                size_t cap=(size_t) -1)
    {
      emp_assert(!emp::Has(setting_map, name));
      emp::Ptr<SettingInfo<T>> new_ptr =
        emp::NewPtr<SettingInfo<T>>(name, desc, option_flag, args_label, cap, &var);
      new_ptr->id = settings.size();
      new_ptr->cap = cap;
      settings.push_back(new_ptr);
      setting_map[name] = new_ptr;
      cur_combo.push_back(0);
      return new_ptr->values;
    }

    /// A SingleSetting must have exactly one value, not multiple.
    template <typename T>
    emp::vector<T> & AddSingleSetting(const std::string & name,
                                const std::string & desc,
                                const char option_flag,
                                T & var,
                                const std::string & args_label="Value")
    {
      return AddSetting<T>(name, desc, option_flag, var, args_label, 1);
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
    emp::vector<T> & Values(const std::string & name) {
      emp_assert(emp::Has(setting_map, name));
      emp::Ptr<SettingInfo<T>> ptr = setting_map[name].DynamicCast<SettingInfo<T>>();
      return ptr->values;
    }

    /// Add a single new value to the specified setting.
    template <typename T>
    void AddValue(const std::string & name, T && val) {
      emp_assert(emp::Has(setting_map, name));
      emp::Ptr<SettingInfo<T>> ptr = setting_map[name].DynamicCast<SettingInfo<T>>();
      ptr->values.emplace_back(std::forward<T>(val));
    }

    /// Set all values for the specified setting.
    template <typename T1, typename... Ts>
    void SetValues(const std::string & name, T1 && val1, Ts &&... vals) {
      emp_assert(emp::Has(setting_map, name));
      emp::Ptr<SettingInfo<T1>> ptr = setting_map[name].DynamicCast<SettingInfo<T1>>();
      emp::Append(ptr->values, std::forward<T1>(val1), std::forward<Ts>(vals)...);
    }

    /// Determine how many unique combinations there currently are.
    size_t CountCombos() {
      size_t result = 1;
      for (auto ptr : settings) result *= ptr->GetSize();
      return result;
    }

    /// Set the next combination of settings to be active.  Return true if successful
    /// or false if we ran through all combinations and reset.
    bool Next() {
      combo_id++;
      for (size_t i = 0; i < cur_combo.size(); i++) {
        cur_combo[i]++;

        // Check if this new combo is valid.
        if (cur_combo[i] < settings[i]->GetSize()) {
          settings[i]->SetValueID( cur_combo[i] );    // Set value in linked variable.
          return true;
        }

        // Since it's not, prepare to move on to the next one.
        cur_combo[i] = 0;
        settings[i]->SetValueID(0);
      }

      // No valid combo found.
      combo_id = 0;
      return false;
    }

    /// Get the set of headers used for the CSV file.
    /// By default, don't include settings capped at one value.
    std::string GetHeaders(const std::string & separator=",", bool include_fixed=false) {
      std::string out_string;
      for (size_t i = 0; i < settings.size(); i++) {
        if (!include_fixed && settings[i]->cap == 1) continue;
        if (i) out_string += separator;
        out_string += settings[i]->name;
      }
      return out_string;
    }

    /// Convert all of the current values into a comma-separated string.
    std::string CurString(const std::string & separator=",", bool include_fixed=false) const {
      std::string out_str;
      for (size_t i = 0; i < cur_combo.size(); i++) {
        if (!include_fixed && settings[i]->cap == 1) continue;
        if (i) out_str += separator;
        out_str += settings[i]->AsString(cur_combo[i]);
      }
      return out_str;
    }

    /// Scan through all settings for a match option and return ID.
    size_t FindOptionMatch(const std::string & option_name) {
      for (const auto & setting : settings) {
        if (setting->IsOptionMatch(option_name)) return setting->id;
      }
      return (size_t) -1;
    }

    /// Scan through all settings for a match option and return ID.
    size_t FindFlagMatch(const char symbol) {
      for (const auto & setting : settings) {
        if (setting->IsFlagMatch(symbol)) return setting->id;
      }
      return (size_t) -1;
    }

    /// Take an input set of config options, process them, and return set of unprocessed ones.
    emp::vector<std::string> ProcessOptions(const emp::vector<std::string> & args) {
      emp::vector<std::string> out_args;
      exe_name = args[0];

      for (size_t i = 1; i < args.size(); i++) {
        const std::string & cur_arg = args[i];
        if (cur_arg.size() < 2 || cur_arg[0] != '-') continue;  // If isn't an option, continue.

        // See if this is a fully spelled-out option.
        size_t id = FindOptionMatch(cur_arg);
        if (id < settings.size()) {
          if (++i >= args.size()) {
            std::cout << "ERROR: Must provide args to use!\n";
            // @CAO Need to signal error...
            return args;
          }
          settings[id]->FromString(args[i]);
          if (!settings[id]->OK()) {
            std::cout << "ERROR: Invalid arguments for option " << cur_arg << "!\n";
            // @CAO Need to signal error...
            return args;
          }
        }

        // See if we have a flag option.
        id = FindFlagMatch(cur_arg[1]);
        if (id < settings.size()) {
          // Check if the flag is followed by the values without whitespace.
          if (cur_arg.size() > 2) {
            settings[id]->FromString( emp::view_string(cur_arg,2) );
          }
          else if (++i >= args.size()) {
            std::cout << "ERROR: Must provide args to use!\n";
            // @CAO Need to signal error...
            return args;
          }
          else {
            settings[id]->FromString(args[i]);
          }

          if (!settings[id]->OK()) {
            std::cout << "ERROR: Invalid arguments for flag -" << cur_arg[1] << "!\n";
            // @CAO Need to signal error...
            return args;
          }
        }

        // Or see of this is a flag trigger.
        else if (Has(action_map, cur_arg)) {
          action_map[cur_arg].fun();
        }

        // Otherwise this argument will go unused; send it back.
        else out_args.push_back(cur_arg);
      }

      return out_args;
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

#endif // #ifndef EMP_CONFIG_SETTINGCOMBOS_HPP_INCLUDE
