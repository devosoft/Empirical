/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2024
*/

#pragma once

#include "emp/tools/String.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;

class TokenInput {
private:
  UI::TextArea name_text;
  UI::TextArea regex_text;
  UI::CheckBox ignore_toggle;

public:
  TokenInput(size_t row_id, emp::String name="", emp::String regex="", bool ignore=false)
    : name_text(emp::MakeString("token_table_name_", row_id+1))
    , regex_text(emp::MakeString("token_table_regex_", row_id+1))
    , ignore_toggle(emp::MakeString("token_table_ignore_", row_id+1))
  {
    name_text.SetText(name).SetCSS("width", "150px", "font-size", "14px");
    regex_text.SetText(regex).SetCSS("width", "450px", "font-size", "14px");
    ignore_toggle.SetChecked(ignore);
  }

  UI::TextArea GetNameWidget() { return name_text; }
  UI::TextArea GetRegexWidget() { return regex_text; }
  UI::CheckBox GetIgnoreWidget() { return ignore_toggle; }

  emp::String GetName() const { return name_text.GetText(); }
  emp::String GetRegex() const { return regex_text.GetText(); }
  bool GetIgnore() const { return ignore_toggle.IsChecked(); }

  void SetName(emp::String name) { name_text.SetText(name); }
  void SetRegex(emp::String regex) { regex_text.SetText(regex); }
  void SetIgnore(bool in) { ignore_toggle.SetChecked(in); }

  void Set(emp::String name, emp::String regex, bool ignore=false) {
    // emp::notify::Message("name=", name, " regex=", regex, " ignore=", ignore);
    SetName(name);
    SetRegex(regex);
    SetIgnore(ignore);
  }
  void Clear() { Set("", "", false); }

  void Swap(TokenInput & in) {
    auto name_bak = GetName();
    auto regex_bak = GetRegex();
    auto ignore_bak = GetIgnore();
    Set(in.GetName(), in.GetRegex(), in.GetIgnore());
    in.Set(name_bak, regex_bak, ignore_bak);
  }
};
