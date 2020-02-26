/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  SettingCombos.h
 *  @brief A tool for exploring all parameter combinations
 *  @note Status: ALPHA
 */

#ifndef EMP_SETTING_COMBOS_H
#define EMP_SETTING_COMBOS_H

#include <unordered_map>
#include <sstream>
#include <string>

#include "../base/Ptr.h"
#include "../base/vector.h"
#include "../tools/string_utils.h"
#include "../tools/map_utils.h"
#include "../tools/vector_utils.h"

namespace emp {

  /// Class to take a set of value for each "setting" and then step through all combinations of
  /// those values for a factorial analysis.

  class SettingCombos {
  private:
    struct SettingBase {
      size_t id;                                       ///< Unique ID/position for this setting.
      std::string name;
      std::string desc;
      std::string flag;
      std::string option;

      SettingBase(const std::string & _name, const std::string & _desc, const std::string & _flag)
        : name(_name), desc(_desc), flag(_flag), option(emp::to_string("--",_name)) { }
      virtual ~SettingBase() { }

      virtual size_t GetSize() const = 0;               ///< How many values are available?
      virtual std::string AsString() const = 0;         ///< All values, as a single string.
      virtual std::string AsString(size_t) const = 0;   ///< A specified value as a string.
      virtual void FromString(const std::string &) = 0; ///< Convert string to range of settings.

      bool IsMatch(const std::string & test_option) const {
        return test_option == flag || test_option == option;
      }
    };

    template <typename T>
    struct SettingInfo : public SettingBase {
      emp::vector<T> values;

      SettingInfo(const std::string & _name, const std::string & _desc, const std::string & _flag)
        : SettingBase(_name, _desc, _flag) { }

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

      void FromString(const std::string & input) override {
        values = emp::from_strings<T>(emp::slice(input, ','));
      }
    };

    using set_ptr_t = emp::Ptr<SettingBase>;

    emp::vector<set_ptr_t> settings;                           ///< Order to be varied.
    std::unordered_map<std::string, set_ptr_t> setting_map;  ///< Settings by name.

    emp::vector<size_t> cur_combo;    ///< Which settings are we currently using?

  public:
    SettingCombos() = default;

    ~SettingCombos() {
      for (auto ptr : settings) ptr.Delete();
    }

    /// Start over stepping through all combinations of parameter values.
    void Reset() { for (size_t & x : cur_combo) x = 0; }

    /// Get the current value of a specified setting.
    template <typename T>
    const T & GetValue(const std::string & name) const {
      emp_assert(emp::Has(setting_map, name), name);
      emp::Ptr<SettingBase> base_ptr = setting_map.find(name)->second;
      emp::Ptr<SettingInfo<T>> ptr = base_ptr.Cast<SettingInfo<T>>();
      size_t id = cur_combo[ptr->id];
      return ptr->values[id];
    }

    /// Add a new setting of a specified type.  Returns the (initially empty) vector of values 
    /// to allow easy setting.
    /// Example:
    ///   combos.AddSetting("pop_size") = {100,200,400,800};

    template <typename T>
    emp::vector<T> & AddSetting(const std::string & name,
                                const std::string & desc="",
                                const std::string & option_flag="") {
      emp_assert(!emp::Has(setting_map, name));
      emp::Ptr<SettingInfo<T>> new_ptr = emp::NewPtr<SettingInfo<T>>(name, desc, option_flag);
      new_ptr->id = settings.size();
      settings.push_back(new_ptr);
      setting_map[name] = new_ptr;
      cur_combo.push_back(0);
      return new_ptr->values;
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
      for (set_ptr_t ptr : settings) result *= ptr;
      return result;
    }

    /// Set the next combination of settings to be active.  Return true if successful
    /// or false if we ran through all combinations and reset.
    bool Next() {
    for (size_t i = 0; i < cur_combo.size(); i++) {
        cur_combo[i]++;

        // Check if this new combo is valid.
        if (cur_combo[i] < settings[i]->GetSize()) return true;

        // Since it's not, prepare to move on to the next one.
        cur_combo[i] = 0;
      }

      // No valid combo found.
      return false;
    }

    /// Get the set of headers used for the CSV file.
    std::string GetHeaders() {
      std::string out_string;
      for (size_t i = 0; i < settings.size(); i++) {
        if (i) out_string += ",";
        out_string += settings[i]->name;
      }
      return out_string;
    }

    /// Convert all of the current values into a comma-separated string.
    std::string CurString() const {
      std::string out_str;
      for (size_t i = 0; i < cur_combo.size(); i++) {
        if (i) out_str += ",";
        out_str += settings[i]->AsString(cur_combo[i]);
      }
      return out_str;
    }

    /// Scan through all settings for a match option and return ID.
    size_t FindOptionMatch(const std::string & option_name) {
      for (const auto & setting : settings) {
        if (setting->IsMatch(option_name)) return setting->id;
      }
      return (size_t) -1;
    }

    /// Take an input set of config options, process them, and return set of unpressed ones.
    emp::vector<std::string> ProcessOptions(const emp::vector<std::string> & args) {
      emp::vector<std::string> out_args;

      for (size_t i = 0; i < args.size(); i++) {
        size_t id = FindOptionMatch(args[i]);
        if (id < settings.size()) {
          if (++i >= args.size()) {
            std::cout << "ERROR: Must provide args to use!\n";          
            // @CAO Need to signal error...
            return args;
          }
          settings[id]->FromString(args[i]);
        }

        // Otherwise this argument will go unused; send it back.
        else out_args.push_back(args[i]);
      }

      return out_args;
    }
  };

}

#endif
