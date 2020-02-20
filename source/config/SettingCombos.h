/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  SettingCombos.h
 *  @brief A tool for exploring all parameter combinations
 *  @note Status: PLANNING
 */

#ifndef EMP_SETTING_COMBOS_H
#define EMP_SETTING_COMBOS_H

#include <unordered_map>
#include <sstream>
#include <string>

#include "base/Ptr.h"
#include "base/vector.h"

namespace emp {

  class SettingCombos {
  private:
    struct SettingBase {
      virtual size_t GetSize() const = 0;              ///< How many values are available?
      virtual std::string AsString() const = 0;        ///< All values, as a single string.
      virtual std::string AsString(size_t) const = 0;  ///< A specified value as a string.
    };

    template <typename T>
    struct SettingInfo : public SettingBase {
      emp::vector<T> values;

      size_t GetSize() const override { return values.size(); }
      std::string AsString() const override {
        std::stringstream ss;
        for (size_t i; i < values.size(); i++) {
          if (i) ss << ',';
          ss << value[i];
        }
        return ss.str();
      }
      std::string AsString(size_t id) const override {
        return emp::to_string(values[id]);
      }
    };

    using set_ptr_t = emp::Ptr<SettingBase>;

    emp::vector<set_ptr_t> settings;                           ///< Order to be varied.
    std::unordered_map<std::string &, set_ptr_t> setting_map;  ///< Settings by name.

    emp::vector<size_t> cur_combo;    ///< Which settings are we currently using?

  public:
    SettingCombos() = default;

    void Reset() { for (size_t & x : cur_combo) x = 0; }

    template <typename T>
    void AddSetting(const std::string & name) {
      emp_assert(!emp::Has(setting_map, name));
      set_ptr_t new_ptr = emp::NewPtr<SettingInfo<T>>;
      settings.push_back(new_ptr);
      setting_map[name] = new_ptr;
      cur_combo.push_back(0);
    }

    /// Determine how many unique combinations there currently are.
    size_t CountCombos() {
      size_t result = 1;
      for (set_ptr_t ptr : settings) result *= ptr;
      return result;
    }

    /// Set the next combination of settings to be active.  Return true if successful
    /// or false if we ran through all combinations and reset.
    bool NextCombo() {
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
  };

}

#endif
