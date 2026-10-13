/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2026
*/

#pragma once

#include "emp/tools/String.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;

/// A single row of grammar-rule information in the Empala interface:
/// a rule name, the pattern to match, and (optionally) the name of the C++
/// function that generated code should call when the resulting AST node is run.
class RuleInput {
private:
  UI::TextArea name_text;
  UI::TextArea pattern_text;
  UI::TextArea function_text;
  UI::TextArea note_text;
  UI::CheckBox no_ast;
  UI::Button remove_but{[](){}, "X"};

public:
  RuleInput(size_t row_id, emp::String name="", emp::String pattern="", emp::String function="")
    : name_text(emp::MakeString("rule_table_name_", row_id+1))
    , pattern_text(emp::MakeString("rule_table_pattern_", row_id+1))
    , function_text(emp::MakeString("rule_table_function_", row_id+1))
    , note_text(emp::MakeString("rule_table_note_", row_id+1))
    , no_ast(emp::MakeString("rule_table_no_ast_", row_id+1))
  {
    name_text.SetText(name).SetCSS("width", "110px", "font-size", "14px");
    pattern_text.SetText(pattern).SetCSS("width", "430px", "font-size", "14px");
    function_text.SetText(function).SetCSS("width", "110px", "font-size", "14px");

    note_text.SetCSS("width", "150px", "font-size", "14px");
    no_ast.SetTitle("Pass through zero or one child; no function or note allowed.");
    remove_but.SetColor(emp::Palette::RED).SetTitle("Click to remove this row.");
  }

  UI::TextArea GetNameWidget() { return name_text; }
  UI::TextArea GetPatternWidget() { return pattern_text; }
  UI::TextArea GetFunctionWidget() { return function_text; }
  UI::TextArea GetNoteWidget() { return note_text; }
  UI::CheckBox GetNoASTWidget() { return no_ast; }
  UI::Button GetRemoveButton() { return remove_but; }

  emp::String GetName() const { return name_text.GetText(); }
  emp::String GetPattern() const { return pattern_text.GetText(); }
  emp::String GetFunction() const { return function_text.GetText(); }

  emp::String GetNote() const { return note_text.GetText(); }
  bool GetNoAST() const { return no_ast.IsChecked(); }

  void RefreshDisabled() {
    function_text.SetDisabled(GetNoAST());
    note_text.SetDisabled(GetNoAST());
  }

  void SetName(emp::String name) { name_text.SetText(name); }
  void SetPattern(emp::String pattern) { pattern_text.SetText(pattern); }
  void SetFunction(emp::String function) { function_text.SetText(function); }

  void Set(emp::String name, emp::String pattern, emp::String function="",
           bool transparent=false, emp::String note="") {
    SetName(name);
    SetPattern(pattern);
    SetFunction(function);
    no_ast.SetChecked(transparent);
    note_text.SetText(note);
    RefreshDisabled();
  }
  void Clear() { Set("", "", ""); }

  void Swap(RuleInput & in) {
    auto name_bak = GetName();
    auto pattern_bak = GetPattern();
    auto function_bak = GetFunction();
    auto note_bak = GetNote();
    bool no_ast_bak = GetNoAST();
    Set(in.GetName(), in.GetPattern(), in.GetFunction(), in.GetNoAST(), in.GetNote());
    in.Set(name_bak, pattern_bak, function_bak, no_ast_bak, note_bak);
  }
};
