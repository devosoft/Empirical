/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2026.
*/
/**
 *  @file
 *  @brief Empala: A C++ Parser Generator -- web interface.
 *
 *  A companion tool to Emplex.  Users define lexer tokens (as in Emplex) plus a set of
 *  grammar rules; Empala builds a recursive-descent parser that produces an abstract
 *  syntax tree, provides a live sandbox to explore parses, and generates standalone
 *  C++ code for the lexer + parser.
 */

#include <chrono>
#include <format>
#include <sstream>

#include "emp/base/notify.hpp"
#include "emp/base/vector.hpp"
#include "emp/compiler/Lexer.hpp"
#include "emp/compiler/Parser.hpp"
#include "emp/io/CPPFile.hpp"
#include "emp/tools/String.hpp"
#include "emp/web/web.hpp"

#include "../../../LexerGenerator/source/web/TokenInput.hpp"
#include "RuleInput.hpp"
#include "../Grammar.hpp"

namespace Palette = emp::Palette;
namespace UI = emp::web;

class Empala : private empala::Settings {
private:
  static constexpr size_t MAX_TOKENS = 100;
  static constexpr size_t MAX_RULES  = 100;

  UI::Document doc{"emp_base"};
  emp::CPPFile file;
  emp::vector<emp::String> errors;
  emp::vector<emp::String> warnings;
  emp::Lexer lexer;
  emp::Lexer preview_lexer;
  bool lexer_ok = false;
  emp::String generated_filename;
  UI::Div lexer_errors{"lexer_errors"};
  UI::Div lexer_sandbox{"lexer_sandbox"};
  UI::TextArea lexer_input{"lexer_input"};
  UI::Text lexer_text{"lexer_text"};
  bool lexer_info = false, lexer_types = false, lexer_lines = false, lexer_ignored = false;
  emp::Parser parser{lexer};
  bool grammar_ok = false;   // Did the most recent grammar build succeed?

  // Messages captured from emp::notify (parser errors/warnings are reported there).
  emp::vector<emp::String> notify_errors;
  emp::vector<emp::String> notify_warnings;

  // Token and grammar-rule information
  emp::vector<TokenInput> token_info;
  emp::vector<RuleInput> rule_info;
  size_t active_token = 1024;
  size_t active_rule = 1024;

  // Sections of web page
  UI::Div intro_div{"intro_div"};
  UI::Div button_div{"button_div"};
  UI::Div token_div{"token_div"};
  UI::Div rule_div{"rule_div"};
  UI::Div settings_div{"settings_div"};
  UI::Div error_div{"error_div"};
  UI::Div sandbox_div{"sandbox_div"};
  UI::Div output_div{"output_div"};
  UI::Div footer_div{"footer_div"};

  UI::Table token_table{1, 4, "token_table"};
  UI::Table rule_table{1, 6, "rule_table"};
  UI::Table settings_table{14, 3, "settings_table"};
  UI::Text output_text{"output_text"};
  UI::TextArea sandbox_input{"sandbox_input"};
  UI::Text sandbox_text{"sandbox_text"};

  // Sandbox state
  bool sandbox_show_tokens = false;   // Should we show the token stream above the AST?
  emp::vector<emp::String> sandbox_colors; // Foreground colors to use for tokens

  emp::Color highlight_color{"#ddddff"};

  UI::Style button_style {
    "padding", "10px 15px",
    "background-color", "#000066",   // Dark Blue background
    "color", "white",                // White text
    "border", "1px solid white",     // Thin white border
    "border-radius", "5px",          // Rounded corners
    "cursor", "pointer",
    "font-size", "16px",
    "transition", "background-color 0.3s ease, transform 0.3s ease" // Smooth transition
  };

  UI::Style div_style {
    "border-radius", "10px",
    "border",        "1px solid black",
    "padding",       "15px",
    "width",         "800px",
    "margin-top",    "10pt"
  };

  UI::Style sandbox_but_style {
    "padding", "5px 10px",
    "background-color", "#220022",   // Dark Purple background
    "color", "white",                // White text
    "border", "1px solid white",     // Thin white border
    "border-radius", "5px",          // Rounded corners
    "cursor", "pointer",
    "font-size", "12px",
    "transition", "background-color 0.3s ease, transform 0.3s ease" // Smooth transition
  };

  // ---- HELPER FUNCTIONS ----

  // Move any messages captured from emp::notify into the main error/warning lists.
  void DrainNotify() {
    for (auto & msg : notify_errors) errors.push_back(msg);
    for (auto & msg : notify_warnings) warnings.push_back(msg);
    notify_errors.resize(0);
    notify_warnings.resize(0);
  }

  // Throw away any messages captured from emp::notify (e.g., expected sandbox errors).
  void DiscardNotify() {
    notify_errors.resize(0);
    notify_warnings.resize(0);
  }

  void UpdateErrors() {
    if (errors.size()) {
      // Clear any stale generated code and remove access to it.
      output_text.Clear();
      doc.Button("download_icon").Hide();
      doc.Button("copy_icon").Hide();
      doc.Button("close_icon").Hide();
      output_div.Redraw();
    }

    error_div.Clear();
    error_div.SetCSS("color", "red");
    for (emp::String & error : errors)  {
      error_div << emp::MakeWebSafe(error) << "<br>\n";
    }
    for (emp::String & warning : warnings)  {
      error_div << "<span style=\"color: #B05000\">Warning: "
                << emp::MakeWebSafe(warning) << "</span><br>\n";
    }
    error_div.Redraw();
  }

  // == Token table management ==

  // Highlight a specified token-table row.
  void ActivateTokenRow(size_t row_id) {
    if (active_token < token_info.size()) {
      token_info[active_token].GetNameWidget().SetBackground(Palette::WHITE);
      token_info[active_token].GetRegexWidget().SetBackground(Palette::WHITE);
    }
    token_info[row_id].GetNameWidget().SetBackground(highlight_color);
    token_info[row_id].GetRegexWidget().SetBackground(highlight_color);
    active_token = row_id;
  }

  // Set up the callback functions for a token-table row.
  void SetupTokenRowCallbacks(size_t row_id) {
    token_info[row_id].GetNameWidget().SetCallback([this](std::string){
      GenerateGrammar();
      UpdateSandbox();
    });
    token_info[row_id].GetRegexWidget().SetCallback([this](std::string){
      GenerateGrammar();
      UpdateSandbox();
    });
    token_info[row_id].GetIgnoreWidget().SetCallback([this](bool){
      GenerateGrammar();
      UpdateSandbox();
    });
    token_info[row_id].GetRemoveButton().SetCallback([this, row_id](){
      RemoveTokenRow(row_id); doc.Div("token_div").Redraw();
      GenerateGrammar();
      UpdateSandbox();
    });
  }

  // Add a row to the bottom of the token table.
  void AddTokenRow() {
    size_t token_id = token_table.GetNumRows() - 1; // Top row is labels, not token
    if (token_id >= MAX_TOKENS) {
      emp::notify::Warning("Maximum ", MAX_TOKENS, " token types allowed!");
      return;
    }
    auto new_row = token_table.AddRow();
    new_row.OnMouseDown([this,token_id](){ ActivateTokenRow(token_id); });

    emp_assert(token_id <= token_info.size());
    // Grow the table if we need to.
    if (token_id == token_info.size()) {
      token_info.emplace_back(token_id);
      SetupTokenRowCallbacks(token_id);
    }
    auto & row_info = token_info[token_id];
    new_row[0] << row_info.GetNameWidget();
    new_row[1] << row_info.GetRegexWidget();
    new_row[2] << "&nbsp;&nbsp;&nbsp;" << row_info.GetIgnoreWidget();
    new_row[3] << row_info.GetRemoveButton();
  }

  void AddTokenRow(emp::String name, emp::String regex, bool ignore=false) {
    const size_t row_id = token_table.GetNumRows()-1;
    if (row_id >= MAX_TOKENS) return;
    AddTokenRow();
    token_info[row_id].Set(name, regex, ignore);
  }

  bool SwapTokenRows(size_t row1, size_t row2) {
    [[maybe_unused]] const size_t num_rows = token_table.GetNumRows() - 1;
    if (row1 >= num_rows || row2 >= num_rows) return false; // No place to move to.

    token_info[row1].Swap(token_info[row2]);
    return true;
  }

  // Remove a specified row from the token table.
  void RemoveTokenRow(size_t id) {
    const size_t num_rows = token_table.GetNumRows() - 1;
    emp_assert(id < num_rows);  // Make sure row to be deleted actually exists.
    // Swap rows to move deleted row to the end.
    while (id < num_rows-1) {
      SwapTokenRows(id, id+1);
      ++id;
    }
    // Remove last row
    token_info[id].Clear();
    token_table.RemoveRow();
  }

  // Remove all rows in the token table.
  void ClearTokenTable() {
    for (auto & row : token_info) row.Clear();
    token_table.Rows(1);
    active_token = MAX_TOKENS;
  }

  // == Grammar rule table management ==

  // Highlight a specified rule-table row.
  void ActivateRuleRow(size_t row_id) {
    if (active_rule < rule_info.size()) {
      rule_info[active_rule].GetNameWidget().SetBackground(Palette::WHITE);
      rule_info[active_rule].GetPatternWidget().SetBackground(Palette::WHITE);
      rule_info[active_rule].GetFunctionWidget().SetBackground(Palette::WHITE);
    }
    rule_info[row_id].GetNameWidget().SetBackground(highlight_color);
    rule_info[row_id].GetPatternWidget().SetBackground(highlight_color);
    rule_info[row_id].GetFunctionWidget().SetBackground(highlight_color);
    active_rule = row_id;
  }

  // Set up the callback functions for a rule-table row.
  void SetupRuleRowCallbacks(size_t row_id) {
    rule_info[row_id].GetNameWidget().SetCallback([this](std::string){
      GenerateGrammar();
      UpdateSandbox();
    });
    rule_info[row_id].GetPatternWidget().SetCallback([this](std::string){
      GenerateGrammar();
      UpdateSandbox();
    });
    rule_info[row_id].GetFunctionWidget().SetCallback([this](std::string){
      GenerateGrammar();
      UpdateSandbox();
    });
    rule_info[row_id].GetNoteWidget().SetCallback([this](std::string){
      GenerateGrammar(); UpdateSandbox();
    });
    rule_info[row_id].GetNoASTWidget().SetCallback([this, row_id](bool){
      rule_info[row_id].RefreshDisabled();
      GenerateGrammar(); UpdateSandbox();
    });
    rule_info[row_id].GetRemoveButton().SetCallback([this, row_id](){
      RemoveRuleRow(row_id); doc.Div("rule_div").Redraw();
      GenerateGrammar();
      UpdateSandbox();
    });
  }

  // Add a row to the bottom of the rule table.
  void AddRuleRow() {
    size_t rule_id = rule_table.GetNumRows() - 1; // Top row is labels, not a rule
    if (rule_id >= MAX_RULES) {
      emp::notify::Warning("Maximum ", MAX_RULES, " grammar rules allowed!");
      return;
    }
    auto new_row = rule_table.AddRow();
    new_row.OnMouseDown([this,rule_id](){ ActivateRuleRow(rule_id); });

    emp_assert(rule_id <= rule_info.size());
    // Grow the table if we need to.
    if (rule_id == rule_info.size()) {
      rule_info.emplace_back(rule_id);
      SetupRuleRowCallbacks(rule_id);
    }
    auto & row_info = rule_info[rule_id];
    new_row[0] << row_info.GetNameWidget();
    new_row[1] << row_info.GetPatternWidget();
    new_row[2] << row_info.GetNoASTWidget();
    new_row[3] << row_info.GetFunctionWidget();
    new_row[4] << row_info.GetNoteWidget();
    new_row[5] << row_info.GetRemoveButton();
    ApplyRuleColumns();
  }

  void AddRuleRow(emp::String name, emp::String pattern, emp::String function="",
                  bool no_ast=false, emp::String note="") {
    const size_t row_id = rule_table.GetNumRows()-1;
    if (row_id >= MAX_RULES) return;
    AddRuleRow();
    rule_info[row_id].Set(name, pattern, function, no_ast, note);
  }

  bool SwapRuleRows(size_t row1, size_t row2) {
    [[maybe_unused]] const size_t num_rows = rule_table.GetNumRows() - 1;
    if (row1 >= num_rows || row2 >= num_rows) return false; // No place to move to.

    rule_info[row1].Swap(rule_info[row2]);
    return true;
  }

  // Remove a specified row from the rule table.
  void RemoveRuleRow(size_t id) {
    const size_t num_rows = rule_table.GetNumRows() - 1;
    emp_assert(id < num_rows);  // Make sure row to be deleted actually exists.
    // Swap rows to move deleted row to the end.
    while (id < num_rows-1) {
      SwapRuleRows(id, id+1);
      ++id;
    }
    // Remove last row
    rule_info[id].Clear();
    rule_table.RemoveRow();
  }

  // Remove all rows in the rule table.
  void ClearRuleTable() {
    for (auto & row : rule_info) row.Clear();
    rule_table.Rows(1);
    active_rule = MAX_RULES;
  }

  // == Saving and loading ==

  void ApplyRuleColumns() {
    for (size_t row = 0; row < rule_table.GetNumRows(); ++row) {
      if (row) {
        rule_info[row-1].GetPatternWidget().SetCSS("width",
          show_notes && show_functions ? "300px" : "430px");
      }
      rule_table.GetCell(row, 2).SetCSS("display", show_no_ast ? "" : "none");
      rule_table.GetCell(row, 3).SetCSS("display", show_functions ? "" : "none");
      rule_table.GetCell(row, 4).SetCSS("display", show_notes ? "" : "none");
    }
  }

  empala::Grammar ReadTables() const {
    empala::Grammar grammar;
    grammar.settings = static_cast<const empala::Settings &>(*this);
    for (const auto & token : token_info) {
      if (token.GetName().empty() && token.GetRegex().empty()) continue;
      grammar.tokens.push_back({token.GetName(), token.GetRegex(), token.GetIgnore()});
    }
    for (const auto & rule : rule_info) {
      if (rule.GetName().empty() && rule.GetPattern().empty() && rule.GetFunction().empty() &&
          rule.GetNote().empty() && !rule.GetNoAST()) continue;
      grammar.rules.push_back({rule.GetName(), rule.GetPattern(), rule.GetFunction(),
                               rule.GetNoAST(), rule.GetNote()});
    }
    return grammar;
  }

  void SaveTables(bool lexer_only = false) {
    std::stringstream ss;
    ReadTables().Write(ss, lexer_only);
    emp::DownloadFile(lexer_only ? "lexer.emplex" : "parser.empala", ss.str());
  }

  void LoadTables(emp::File & load_file, bool lexer_only=false) {
    std::stringstream input;
    for (const auto & line : load_file) input << line << '\n';
    empala::Grammar grammar;
    grammar.settings = static_cast<const empala::Settings &>(*this);
    grammar.Read(input);
    if (lexer_only) {
      if (!grammar.rules.empty()) grammar.errors.push_back("Load Lexer expects an Emplex file without grammar rules.");
      auto imported = ReadTables();
      imported.ImportLexer(grammar);
      imported.errors = grammar.errors;
      grammar = std::move(imported);
    }
    grammar.Validate(lexer_only);
    if (grammar.tokens.size() > MAX_TOKENS || grammar.rules.size() > MAX_RULES) {
      grammar.errors.push_back("The web editor supports at most 100 tokens and 100 rule rows.");
    }
    if (!grammar.errors.empty()) {
      errors = grammar.errors;
      warnings.clear();
      UpdateErrors();
      return;
    }
    static_cast<empala::Settings &>(*this) = grammar.settings;
    doc.TextArea("set_filename").SetText(out_filename);
    doc.TextArea("set_lexer_class").SetText(lexer_name);
    doc.TextArea("set_token_class").SetText(token_name);
    doc.TextArea("set_dfa_class").SetText(dfa_name);
    doc.TextArea("set_parser_class").SetText(parser_name);
    doc.TextArea("set_node_class").SetText(node_name);
    doc.TextArea("set_value_type").SetText(value_type);
    doc.TextArea("set_includes").SetText(inc_guards);
    doc.TextArea("set_namespace").SetText(name_space);
    doc.CheckBox("checkbox_lexemes").SetChecked(use_token_lexemes).Redraw();
    doc.CheckBox("checkbox_line_nums").SetChecked(use_token_line_num).Redraw();
    doc.CheckBox("checkbox_cols").SetChecked(use_token_column).Redraw();
    ClearTokenTable();
    ClearRuleTable();
    for (const auto & token : grammar.tokens) AddTokenRow(token.name, token.regex, token.ignore);
    for (const auto & rule : grammar.rules) AddRuleRow(rule.name, rule.pattern, rule.function, rule.no_ast, rule.note);
    doc.CheckBox("show_functions").SetChecked(show_functions).Redraw();
    doc.CheckBox("show_notes").SetChecked(show_notes).Redraw();
    doc.CheckBox("show_no_ast").SetChecked(show_no_ast).Redraw();
    ApplyRuleColumns();
    doc.Div("token_div").Redraw();
    doc.Div("rule_div").Redraw();

    // Now that the new tables are in place, rebuild the grammar and refresh the sandbox.
    GenerateGrammar();
    UpdateSandbox();
  }

  // Count how many grammar rule rows are (at least partially) filled in.
  size_t CountRuleRows() const {
    size_t count = 0;
    for (const auto & r_info : rule_info) {
      if (r_info.GetName().size() || r_info.GetPattern().size()) ++count;
    }
    return count;
  }

  // The lexer has its own validation and instance so unfinished grammar rules
  // never prevent token testing or generating a standalone lexer.
  bool GenerateLexer() {
    auto grammar = ReadTables();
    lexer_ok = false;
    DiscardNotify();
    grammar.Validate(true);
    if (grammar.errors.empty()) {
      try { grammar.BuildLexer(preview_lexer); lexer_ok = true; }
      catch (const empala::DiagnosticError &) { lexer_ok = false; }
    }
    lexer_errors.Clear();
    lexer_errors.SetColor(Palette::RED);
    for (const auto & error : grammar.errors) lexer_errors << emp::MakeWebSafe(error) << "<br>";
    for (const auto & error : notify_errors) lexer_errors << emp::MakeWebSafe(error) << "<br>";
    for (const auto & warning : notify_warnings) lexer_errors << emp::MakeWebSafe(warning) << "<br>";
    DiscardNotify();
    lexer_errors.Redraw();
    UpdateLexerSandbox();
    return lexer_ok;
  }

  void GenerateLexerCPP() {
    ClearOutput();
    if (!GenerateLexer()) return;
    ReadTables().WriteLexerCPP(file, preview_lexer);
    generated_filename = "lexer.hpp";
    ShowCode();
  }

  void InitializeLexerSandbox() {
    lexer_sandbox.SetBackground(Palette::BLACK).SetColor(Palette::WHITE).SetCSS(div_style);
    lexer_sandbox << HeadingName("Lexer Sandbox");
    lexer_sandbox << UI::Button([this](){ GenerateLexer(); }, "Refresh", "lexer_refresh").SetCSS(sandbox_but_style);
    lexer_sandbox << UI::Button([this](){
      lexer_info = !lexer_info;
      doc.Button("lexer_info").SetLabel(lexer_info ? "Token Info: ON" : "Token Info: OFF");
      for (const auto & id : {"lexer_types", "lexer_lines", "lexer_ignored"}) {
        doc.Button(id).SetDisabled(!lexer_info);
      }
      UpdateLexerSandbox();
    }, "Token Info: OFF", "lexer_info").SetCSS(sandbox_but_style);
    lexer_sandbox << UI::Button([this](){
      lexer_types = !lexer_types;
      doc.Button("lexer_types").SetLabel(lexer_types ? "Types: ON" : "Types: OFF");
      UpdateLexerSandbox();
    }, "Types: OFF", "lexer_types").SetCSS(sandbox_but_style).SetDisabled(true);
    lexer_sandbox << UI::Button([this](){
      lexer_lines = !lexer_lines;
      doc.Button("lexer_lines").SetLabel(lexer_lines ? "Line Nums: ON" : "Line Nums: OFF");
      UpdateLexerSandbox();
    }, "Line Nums: OFF", "lexer_lines").SetCSS(sandbox_but_style).SetDisabled(true);
    lexer_sandbox << UI::Button([this](){
      lexer_ignored = !lexer_ignored;
      doc.Button("lexer_ignored").SetLabel(lexer_ignored ? "Ignored: VISIBLE" : "Ignored: HIDDEN");
      UpdateLexerSandbox();
    }, "Ignored: HIDDEN", "lexer_ignored").SetCSS(sandbox_but_style).SetDisabled(true);
    lexer_input.SetText(sandbox_input.GetText()).SetSize(750, 115);
    lexer_input.SetCallback([this](std::string){ UpdateLexerSandbox(); });
    lexer_sandbox << "<p>" << lexer_input << "</p><pre style=\"white-space: pre-wrap\">" << lexer_text << "</pre>";
  }

  void UpdateLexerSandbox() {
    if (lexer_sandbox.IsInactive()) return;
    lexer_text.Freeze();
    lexer_text.Clear();
    if (!lexer_ok) lexer_text << "Define valid token types above to try the lexer.";
    else {
      const auto tokens = preview_lexer.Tokenize(lexer_input.GetText(), "Empala Lexer Sandbox", lexer_ignored || !lexer_info);
      if (tokens.size() == 0) lexer_text << "NO VISIBLE TOKENS.";
      for (const auto & token : tokens) {
        if (lexer_info) {
          lexer_text << "[";
          if (lexer_types) lexer_text << emp::MakeWebSafe(preview_lexer.GetTokenName(token.id)) << ":";
          if (lexer_lines) lexer_text << token.line_id << ":";
        }
        const auto color = token.id == -1 ? "#FF6666" : sandbox_colors[(255-token.id) % sandbox_colors.size()];
        lexer_text << "<span style=\"color:" << color << "\">";
        lexer_text << (lexer_info ? token.lexeme.AsEscaped(false) : token.lexeme).AsWebSafe(true);
        lexer_text << "</span>";
        if (lexer_info) lexer_text << "]";
      }
    }
    lexer_text.Activate();
  }

  /// Try to build the lexer and parser based on the current tables.
  /// @return success
  bool GenerateGrammar() {
    errors.resize(0);
    warnings.resize(0);
    DiscardNotify();
    grammar_ok = false;

    // Every edit invalidates previously generated code, even if the new grammar is valid.
    ClearOutput();
    GenerateLexer();
    auto grammar = ReadTables();
    grammar.Validate();
    errors = grammar.errors;
    if (!errors.empty()) { UpdateErrors(); return false; }
    if (grammar.rules.empty()) { UpdateErrors(); return false; }

    // Run a full grammar analysis: undefined rules, left recursion, repetition of
    // empty-matching rules, and character literals hidden by the lexer.
    const bool valid = grammar.Build(lexer, parser);
    DrainNotify();          // Move any parser errors/warnings into our lists.
    UpdateErrors();
    grammar_ok = valid && errors.empty();
    return grammar_ok;
  }

  /// Try to generate CPP code based on the current tokens and grammar.
  /// @return success
  bool GenerateCPP() {
    if (!GenerateGrammar()) {
      if (errors.empty() && CountRuleRows() == 0) {
        errors.push_back("Cannot generate a parser until grammar rules have been added.");
        UpdateErrors();
      }
      return false;
    }

    empala::Settings::WriteCPP(file, lexer, parser);
    generated_filename = out_filename;
    ShowCode();
    return true;
  }

  void ShowCode() {
    std::stringstream ss;
    ss << "/**************************************************************************\n"
          " * This file was autogenerated by Empala on "
       << std::format("{:%B %d, %Y}", std::chrono::system_clock::now()) << ".\n"
          " * You may freely use, modify, and distribute this code under the terms of\n"
          " * the MIT Public License (see https://opensource.org/licenses/MIT).\n"
          " * Empala was developed at Michigan State University by Dr. Charles Ofria.\n"
          " * To use Empala, go to https://www.cse.msu.edu/~cse450/Empala.html\n"
          " **************************************************************************/\n"
          "\n";
    file.Write(ss);
    output_text.Clear();
    output_text.SetBorder("20px");
    output_text << "<pre style=\"padding: 10px; border-radius: 5px; overflow-x: auto;\">\n" << emp::MakeWebSafe(ss.str()) << "\n</pre>\n";
    output_div.Redraw();

    doc.Button("download_icon").Hide(false);
    doc.Button("copy_icon").Hide(false);
    doc.Button("close_icon").Hide(false);

  }

  void ClearOutput() {
    output_text.Clear();
    doc.Button("download_icon").Hide();
    doc.Button("copy_icon").Hide();
    doc.Button("close_icon").Hide();
    output_div.Redraw();
  }

  void CopyCode() {
    std::stringstream ss;
    file.Write(ss);
    emp::CopyText(ss.str());
    doc.Button("copy_icon").SetLabel("<img src=\"../../LexerGenerator/web/Icons/ICON-Copied.png\" width=\"50px\">").Redraw();
    emp::DelayCall([this](){
      doc.Button("copy_icon").SetLabel("<img src=\"../../LexerGenerator/web/Icons/ICON-Copy.png\" width=\"50px\">").Redraw();
    }, 1500);
  }

  void DownloadCode() {
    std::stringstream ss;
    file.Write(ss);
    emp::DownloadFile(generated_filename, ss.str());
  }

  void ToggleSandbox() {
    sandbox_div.ToggleActive();
    if (sandbox_div.IsInactive()) {
      doc.Button("sandbox_but").SetLabel("Open Sandbox");
    } else {
      doc.Button("sandbox_but").SetLabel("Close Sandbox");
    }
  }

  emp::String HeadingName(emp::String name) {
    return emp::MakeString("<big><big><b>", name, "</b></big></big><br>\n");
  }

  emp::String MakeLink(emp::String text, emp::String link) {
    return emp::MakeHTMLLink(text, link, "#C0C0FF");
  }

  emp::String MakeTrigger(emp::String text, std::function<void()> fun) {
    return emp::MakeHTMLTrigger(text, fun, "#C0C0FF");
  }

  // Load an example lexer + grammar for a small programming language.
  void LoadExampleGrammar() {
    ClearTokenTable();
    AddTokenRow("WHITESPACE",  "[ \\t\\n\\r]+", true);
    AddTokenRow("COMMENT",     "#.*", true);
    AddTokenRow("FLOAT",       "([0-9]+\\.[0-9]*)|(\\.[0-9]+)");
    AddTokenRow("INTEGER",     "[0-9]+");
    AddTokenRow("IF",          "if");
    AddTokenRow("ELSE",        "else");
    AddTokenRow("WHILE",       "while");
    AddTokenRow("TYPE",        "int|double|string|char");
    AddTokenRow("IDENTIFIER",  "[a-zA-Z_][a-zA-Z0-9_]*");
    AddTokenRow("STRING",      "\\\"([^\"\\\\]|(\\\\.))*\\\"");
    AddTokenRow("COMPARE",     "\"==\"|\"!=\"|\"<=\"|\">=\"|\"<\"|\">\"");
    doc.Div("token_div").Redraw();

    ClearRuleTable();
    AddRuleRow("program", "statement*", "DoProgram", false, "");
    AddRuleRow("statement", "block | declare | if_else | while_loop | expr_stmt", "", true, "");
    AddRuleRow("block", "-'{' statement* -'}'", "DoBlock", false, "");
    AddRuleRow("declare", "TYPE IDENTIFIER (-'=' expr)? -';'", "DoDeclare", false, "");
    AddRuleRow("if_else", "-IF -'(' expr -')' statement (-ELSE statement)?", "DoIf", false, "");
    AddRuleRow("while_loop", "-WHILE -'(' expr -')' statement", "DoWhile", false, "");
    AddRuleRow("expr_stmt", "expr -';'", "", true, "");
    AddRuleRow("expr", "IDENTIFIER -'=' expr", "DoAssign", false, "assignment");
    AddRuleRow("expr", "compare", "", true, "");
    AddRuleRow("compare", "sum (COMPARE sum)+", "DoCompare", false, "");
    AddRuleRow("compare", "sum", "", true, "");
    AddRuleRow("sum", "term (('+' | '-') term)+", "DoSum", false, "");
    AddRuleRow("sum", "term", "", true, "");
    AddRuleRow("term", "unit (('*' | '/' | '%') unit)+", "DoTerm", false, "");
    AddRuleRow("term", "unit", "", true, "");
    AddRuleRow("unit", "-'-' unit", "DoNegate", false, "negation");
    AddRuleRow("unit", "-'(' expr -')'", "", true, "");
    AddRuleRow("unit", "IDENTIFIER -'(' args? -')'", "DoCall", false, "call");
    AddRuleRow("unit", "IDENTIFIER | INTEGER | FLOAT | STRING", "DoValue", false, "value");
    AddRuleRow("args", "expr (-',' expr)*", "", false, "");
    doc.Div("rule_div").Redraw();

    GenerateGrammar();
    UpdateSandbox();
  }

  void UpdateIntro(emp::String mode) {
    intro_div.Clear();
    const emp::String text_color = "white";
    const emp::String active_color = "#0000AA";
    const emp::String button_color = "#000044";
    const emp::String table_color = "white";
    intro_div.SetColor(text_color).SetBackground(button_color).SetCSS(div_style);
    doc.Button("home_but").SetBackground(button_color);
    doc.Button("guide_but").SetBackground(button_color);
    doc.Button("parsing_but").SetBackground(button_color);
    doc.Button("grammar_but").SetBackground(button_color);
    doc.Button("about_but").SetBackground(button_color);

    if (mode == "home") {
      doc.Button("home_but").SetBackground(active_color);
      static emp::String trigger_text =
        MakeTrigger("<b>load an example</b>", [this](){ LoadExampleGrammar(); });
      intro_div << HeadingName("Overview") <<
          "<p>Empala uses a set of <b>token types</b> (with regular expressions, as in its "
          "sibling tool " << MakeLink("Emplex", "https://www.cse.msu.edu/~cse450/Emplex.html") <<
          ") plus a set of <b>grammar rules</b> to generate C++ code for a recursive-descent "
          "parser that builds an <b>abstract syntax tree</b> (AST) from input text. "
          "Each grammar rule can name a C++ function for the generated code to call when the "
          "matching AST node is run, making it easy to attach behavior to your language. "
          "Click on the buttons above to learn more or " << trigger_text << ".</p>"
          "<p>Open the <b>sandbox</b> to type input and watch, live, how it parses into an "
          "AST -- or where the parse goes wrong.</p>";
    } else if (mode == "parsing") {
      doc.Button("parsing_but").SetBackground(active_color);
      intro_div << HeadingName("Parsing") <<
        "<p>While a lexer groups input characters into <i>tokens</i>, a " <<
        MakeLink("parser", "https://en.wikipedia.org/wiki/Parsing") <<
        " groups tokens into the nested structures of a language -- expressions, statements, "
        "declarations, and so on -- according to a " <<
        MakeLink("grammar", "https://en.wikipedia.org/wiki/Context-free_grammar") <<
        ".  The result is an " <<
        MakeLink("abstract syntax tree", "https://en.wikipedia.org/wiki/Abstract_syntax_tree") <<
        " (AST): a tree where each node identifies which grammar rule matched a section of "
        "the input, with child nodes for each nested piece.</p>"
        "<p>A grammar is a set of named <b>rules</b>.  Each rule lists one or more "
        "<b>patterns</b> that it can match; each pattern is a sequence of tokens and other "
        "rules.  For example, a rule for an <code>if</code> command might be:</p>"
        "<p>&nbsp;&nbsp;<code style=\"background-color: " << table_color << "; color: black; "
        "padding:10px; border: 1px solid black\">"
        "if_else &nbsp;::=&nbsp; IF '(' expr ')' statement (ELSE statement)?</code></p>"
        "<p>Empala parsers use <b>recursive descent</b>: each rule becomes a function that "
        "tries its patterns on the upcoming tokens, calling the functions of any rules "
        "inside the pattern.  Three properties of this approach are important to keep in "
        "mind when you design a grammar:</p>"
        "<p><table cellpadding=4px border=2px style=\"background-color: " << table_color <<
        "; color: black;\">"
        "<tr><th width=170px>Ordered choice</th><td>When a rule has multiple patterns "
        "(separate rows or alternatives split by '|'), they are tried IN THE ORDER GIVEN and "
        "the first full match wins.  Put longer patterns before their prefixes: list "
        "<code>IDENTIFIER '(' args? ')'</code> before plain <code>IDENTIFIER</code>.</td></tr>"
        "<tr><th>Backtracking</th><td>If a pattern matches part-way and then fails, the "
        "parser rewinds and tries the next alternative, so partial matches never corrupt "
        "the result.</td></tr>"
        "<tr><th>No left recursion</th><td>A rule must not be able to reach itself before "
        "consuming at least one token.  <code>expr ::= expr '+' term</code> would recurse "
        "forever; write <code>expr ::= term ('+' term)*</code> instead.  Empala detects "
        "left recursion (along with undefined rules and repetition of empty matches) and "
        "reports it as an error.</td></tr>"
        "</table></p>"
        "<p><b>Operator precedence</b> falls out of rule layering.  Give each precedence "
        "level its own rule, from loosest to tightest binding:</p>"
        "<p><table cellpadding=4px border=2px style=\"background-color: " << table_color <<
        "; color: black; text-align: left;\">"
        "<tr><td><code>sum</code></td><td><code>term (('+' | '-') term)*</code></td></tr>"
        "<tr><td><code>term</code></td><td><code>unit (('*' | '/') unit)*</code></td></tr>"
        "<tr><td><code>unit</code></td><td><code>INTEGER | '(' sum ')'</code></td></tr>"
        "</table></p>"
        "<p>With this layout, <code>1+2*3</code> parses as <code>1+(2*3)</code>: the "
        "<code>sum</code> rule can only combine <code>term</code>s, so multiplication binds "
        "tighter automatically.  Try it in the sandbox!</p>";
    } else if (mode == "grammar") {
      doc.Button("grammar_but").SetBackground(active_color);
      intro_div << HeadingName("Grammar Rule Syntax") <<
        "<p>Each row of the grammar table defines a rule: a <b>name</b>, a <b>pattern</b> to "
        "match, and (optionally) the name of a <b>C++ function</b> for generated code to run "
        "at that AST node.  The rule on the FIRST row is the <b>start rule</b> -- it must "
        "match the entire input.  A pattern is a space-separated sequence of symbols:</p>"
        "<p><table border=\"2\" cellpadding=\"3\" style=\"background: white; color: black\">\n"
        "<tr><th>Symbol</th> <th>Description</th><th>Example</th></tr>\n"
        "<tr><th>TOKEN_NAME</th><td>Match one token defined in the token table.</td>"
            "<td><code>IDENTIFIER '=' expr ';'</code></td></tr>\n"
        "<tr><th>rule_name</th><td>Match another grammar rule (which may be defined on a "
            "later row, or be this same rule when not left-recursive).</td>"
            "<td><code>'(' expr ')'</code></td></tr>\n"
        "<tr><th>'c'</th><td>Match a single-character token, with normal escapes "
            "('\\n', '\\t', '\\\\', '\\'').  Use these for punctuation the lexer does not "
            "already claim in some other token.</td>"
            "<td><code>'{' statement* '}'</code></td></tr>\n"
        "<tr><th>( ... )</th><td>Group symbols; a group may contain alternatives split "
            "by '|'.</td><td><code>(ELSE statement)?</code></td></tr>\n"
        "<tr><th>|</th><td>Separate alternatives, tried in order.  Works at the top level "
            "of a pattern or inside a group.</td>"
            "<td><code>block | declare | expr_stmt</code></td></tr>\n"
        "<tr><th>-element</th><td>Match an element but omit its entire subtree from the AST. "
            "Works with tokens, rules, groups, and repetitions; quoted '-' still matches a minus.</td>"
            "<td><code>-'{' statement* -'}'</code></td></tr>\n"
        "<tr><th>?</th><td>The previous symbol or group is optional.</td>"
            "<td><code>('=' expr)?</code></td></tr>\n"
        "<tr><th>?:text</th><td>An optional named token inserts default text when absent. "
            "The text must lex as exactly one visible token of that type. "
            "Use double quotes for spaces or grammar punctuation. Defaults consume no input "
            "and have source coordinates zero; rules and groups cannot have defaults.</td>"
            "<td><code>INTEGER?:1 'd' INTEGER</code></td></tr>\n"
        "<tr><th>*</th><td>Match the previous symbol or group zero or more times "
            "(as many as possible).</td><td><code>statement*</code></td></tr>\n"
        "<tr><th>+</th><td>Match the previous symbol or group one or more times.</td>"
            "<td><code>INTEGER+</code></td></tr>\n"
        "</table></p>\n"
        "<p><b>No AST</b> applies to one row: zero retained children omit the node; one child "
        "replaces it. A row that could retain more than one child is an error. Such rows cannot "
        "have a C++ function or note (uncheck No AST to edit those fields). A promoted child can "
        "be the root; a successful parse without a root displays a warning.</p>"
        "<p><b>Note</b> attaches text to the retained node for the matched alternative. Generated "
        "nodes have a string <code>note</code> only if any grammar rows use notes. Column visibility "
        "changes only the display; hidden functions, notes, and No AST settings still apply.</p>"
        "<p>Saved rows support <code>&gt;name !no_ast pattern</code> or "
        "<code>&gt;name @Function !note=\"text\" pattern</code>.</p>"
        "<p>Additional details:</p>"
        "<ul>"
        "<li>Using the SAME rule name on multiple rows adds extra alternatives to that rule "
        "(tried top to bottom).  Each row can name its own C++ function, so different "
        "alternatives can trigger different behaviors.</li>\n"
        "<li>Modifiers must directly follow their symbol with no space: "
        "<code>statement*</code>, not <code>statement *</code>.</li>\n"
        "<li>To let a rule match NOTHING as one of its options, end a pattern with '|' "
        "(e.g., <code>args |</code> tries <code>args</code>, then accepts an empty "
        "match).</li>\n"
        "<li>A character literal like <code>'&lt;'</code> only works if the lexer leaves "
        "that character on its own.  If a token's regular expression would consume it, "
        "Empala warns you and the token name must be used instead.</li>\n"
        "<li>In the resulting AST, groups are spliced into their parent, so "
        "<code>sum ::= term (('+' | '-') term)*</code> produces one <code>sum</code> node "
        "whose children alternate term, operator, term, operator, term...</li>\n"
        "</ul>";
    } else if (mode == "guide") {
      doc.Button("guide_but").SetBackground(active_color);
      intro_div << HeadingName("User Guide: Working with the Generated C++ Code") <<
        "<p>Empala will generate a C++ file that you can either copy or download "
        "(as \"parser.hpp\" by default) and simply <code>#include</code> into your own code. "
        "The generated file contains a complete lexer (identical to Emplex output) plus a "
        "parser class (called \"Parser\" by default) in a shared namespace (\"empala\" by "
        "default).  Tokenize your input, then parse the tokens:</p>\n"
        "<pre style=\"background-color: " << table_color << "; color: black; padding:10px\">\n"
        "   std::ifstream in_file(\"mycode.cab\");    // Load the input file\n"
        "   empala::Lexer lexer;                    // Build the lexer object\n"
        "   empala::Parser parser;                  // Build the parser object\n"
        "   auto &amp; tokens = lexer.Tokenize(in_file);\n"
        "   auto ast_root = parser.Parse(tokens);   // Returns a unique_ptr to the root\n"
        "   if (!ast_root) { parser.PrintError(); exit(1); }\n"
        "</pre>\n"
        "<p>Each AST node is a simple <code>struct</code>:</p>\n"
        "<pre style=\"background-color: " << table_color << "; color: black; padding:10px\">\n"
        "   struct ASTNode {\n"
        "     int rule_id;         // Which grammar rule matched? (-1 = leaf token)\n"
        "     int pattern_id;      // Which alternative of that rule matched?\n"
        "     Token token;         // The token matched (leaf nodes only)\n"
        "     std::vector&lt;std::unique_ptr&lt;ASTNode&gt;&gt; children;  // Sub-nodes, in order\n"
        "   };\n"
        "</pre>\n"
        "<p>Nodes provide helpers like <code>IsLeaf()</code>, <code>NumChildren()</code>, "
        "<code>GetChild(i)</code>, and <code>GetChildLexeme(i)</code>, and the parser "
        "provides <code>Parser::RuleName(rule_id)</code> for debugging.  The "
        "<code>rule_id</code> values match constants like "
        "<code>empala::Parser::RULE_expr</code>.</p>"
        "<p><b>Running the tree.</b>  To execute (or evaluate, or compile) an AST, define a "
        "<b>handler</b> class with one member function for each function name you provided "
        "in the grammar table, then call <code>Run</code>:</p>"
        "<pre style=\"background-color: " << table_color << "; color: black; padding:10px\">\n"
        "  struct Handler {\n"
        "    using node_t = empala::Parser::ASTNode;\n"
        "    double DoSum(node_t &amp; node) {\n"
        "      double out = node.RunChild(0, *this);       // Evaluate the first term\n"
        "      for (size_t i = 1; i &lt; node.NumChildren(); i += 2) {\n"
        "        if (node.GetChild(i).token.id == '+') out += node.RunChild(i+1, *this);\n"
        "        else out -= node.RunChild(i+1, *this);\n"
        "      }\n"
        "      return out;\n"
        "    }\n"
        "    // ... one function for each name used in the grammar table ...\n"
        "  };\n"
        "\n"
        "  Handler handler;\n"
        "  double result = ast_root-&gt;Run(handler);\n"
        "</pre>\n"
        "<p>Inside a handler function, <code>node.RunChild(i, *this)</code> evaluates child "
        "<code>i</code> -- children are evaluated only when you ask, so control flow (like "
        "running the body of a <code>while</code> repeatedly, or skipping the unused branch "
        "of an <code>if</code>) works naturally.  Nodes for rules with NO function name "
        "simply run each child rule in order and return the last result, so pass-through "
        "rules (like <code>statement</code>) need no code at all.</p>"
        "<p>The return type of handler functions (and <code>Run</code>) is the <b>AST value "
        "type</b> from Advanced Options (default <code>double</code>).  Set it to a value "
        "class, a pointer, or <code>void</code> to suit your project.  Note that the "
        "handler must define EVERY function named in the grammar; if one is missing, the "
        "C++ compiler will tell you exactly which.</p>";
    } else if (mode == "about") {
      doc.Button("about_but").SetBackground(active_color);
      intro_div << HeadingName("About") <<
        "<p>Empala is written in C++ using the "
        << MakeLink("Empirical Library", "https://github.com/devosoft/Empirical") <<
        " and then compiled into "
        << MakeLink("WebAssembly", "https://webassembly.org/") << " with the "
        << MakeLink("Emscripten", "https://emscripten.org/") << " LLVM compiler. "
        "The name is short for 'Empirical Parser' -- and is a play on Impala, itself an "
        "homage to the hoofed-animal naming tradition of "
        << MakeLink("Yacc", "https://en.wikipedia.org/wiki/Yacc") << " and "
        << MakeLink("Bison", "https://en.wikipedia.org/wiki/GNU_Bison") << ". "
        "It is the companion tool to the "
        << MakeLink("Emplex", "https://www.cse.msu.edu/~cse450/Emplex.html") <<
        " lexer generator.</p>"

        "<p>Empala takes in a set of grammar rules, each built from tokens and references "
        "to other rules.  Patterns are compiled into an internal rule table; groups become "
        "anonymous sub-rules that are spliced back into their parents when an AST is built. "
        "The full grammar is then analyzed: rules are checked for definitions, nullability "
        "is computed to fixed point, and the 'can-start-with' relation is searched for "
        "cycles to detect left recursion before any parsing is attempted.  Parsing itself "
        "is performed by recursive descent with ordered alternatives and full "
        "backtracking (PEG-style semantics).  The generated C++ output contains the same "
        "table-driven lexer that Emplex produces, plus one parse function per grammar "
        "rule and an AST class whose <code>Run()</code> dispatches to user-provided "
        "handler functions.</p>"
        "<p>The Empala software and most of the associated tools in the underlying "
        "Empirical library were written by:<br><br> "
        << "<b>" << MakeLink("Dr. Charles Ofria", "https://ofria.com/") << "</b><br>"
        << MakeLink("Michigan State University", "https://msu.edu/") << "<br>"
        "<a href=\"mailto:ofria@cse.msu.edu\" style=\"color: #C0C0FF\">ofria@cse.msu.edu</a><br><br>\n"

        "<a href=\"https://scholar.google.com/citations?user=nYLuKDAAAAAJ\" target=\"_blank\" rel=\"noopener noreferrer\">"
        "  <img src=\"https://img.shields.io/badge/Google%20Scholar-Follow-blue?style=social&logo=google-scholar\" alt=\"Follow on Google Scholar\">"
        "</a><br>  "
        "<a href=\"https://bsky.app/profile/ofria.bsky.social\">"
        "  <img src=\"https://img.shields.io/badge/Bluesky-0285FF?logo=bluesky&logoColor=fff&label=@ofria\" alt=\"Follow on Bluesky\">"
        "</a><br>"
        "<a href=\"https://github.com/mercere99\" target=\"_blank\" rel=\"noopener noreferrer\">"
        "  <img src=\"https://img.shields.io/github/followers/mercere99?label=Github&style=social\" alt=\"Follow on GitHub\">"
        "</a><br>\n"

        "<h3>My current To-Do list for the site includes:</h3>\n"
        "<ul>\n"
        "<li>Import grammars written in BNF / EBNF notation.</li>\n"
        "<li>Automatically rewrite direct left recursion into repetition.</li>\n"
        "<li>Operator-precedence shorthand for expression grammars.</li>\n"
        "<li>Optionally generate a matching skeleton handler class with the C++ output.</li>\n"
        "<li>Step-through parsing mode in the sandbox to watch choices and backtracking.</li>\n"
        "<li>Set up alternate languages to generate to, including Python, Java, C, and Rust.</li>\n"
        "</ul>\n"
        "<br><br>"
        ;
    }
  }

  void InitializeButtonDiv() {
    button_div << UI::Button([this](){
      UpdateIntro("home"); intro_div.Redraw();
    }, "Home", "home_but").SetCSS(button_style).SetBackground(emp::Color("#0000AA")).SetCSS("width", "159px");
    button_div << UI::Button([this](){
      UpdateIntro("guide"); intro_div.Redraw();
    }, "User Guide", "guide_but").SetCSS(button_style).SetCSS("width", "159px");
    button_div << UI::Button([this](){
      UpdateIntro("parsing"); intro_div.Redraw();
    }, "Parsing", "parsing_but").SetCSS(button_style).SetCSS("width", "159px");
    button_div << UI::Button([this](){
      UpdateIntro("grammar"); intro_div.Redraw();
    }, "Grammar Syntax", "grammar_but").SetCSS(button_style).SetCSS("width", "180px");
    button_div << UI::Button([this](){
      UpdateIntro("about"); intro_div.Redraw();
    }, "About", "about_but").SetCSS(button_style).SetCSS("width", "159px");
  }

  void InitializeTokenDiv() {
    token_div.SetBackground(emp::Color{"lightgrey"})
             .SetCSS("margin-top", "10pt")
             .SetCSS("border-radius", "10px")
             .SetCSS("border", "1px solid black")
             .SetCSS("padding", "15px")
             .SetCSS("width", "800px");
    token_div << HeadingName("Token Types");

    token_table.SetColor(emp::Color{"#000044"});
    token_table.GetCell(0,0).SetHeader() << "Token Name";
    token_table.GetCell(0,1).SetHeader() << "Regular Expression";
    token_table.GetCell(0,2).SetHeader() << "Ignore?";

    // Start table with three rows.
    AddTokenRow();
    AddTokenRow();
    AddTokenRow();

    token_div << UI::Button([this](){
      ClearTokenTable(); AddTokenRow(); AddTokenRow(); AddTokenRow();
      token_div.Redraw(); GenerateGrammar(); UpdateSandbox();
    }, "Reset", "reset_lexer").SetCSS(button_style).SetTitle("Reset token types only; keep all grammar rules and settings.");
    token_div << UI::Button([this](){ SaveTables(true); }, "Save Lexer", "save_lexer").SetCSS(button_style);
    token_div << UI::FileInput([this](emp::File input){ LoadTables(input, true); }, "load_lexer_input").SetCSS("display", "none");
    token_div << UI::Button([this](){ doc.FileInput("load_lexer_input").DoClick(); }, "Load Lexer", "load_lexer").SetCSS(button_style);
    token_div << "<p>";
    token_div << UI::Button([this](){ GenerateLexerCPP(); }, "Generate C++ Lexer", "generate_lexer").SetCSS(button_style);
    token_div << UI::Button([this](){
      lexer_sandbox.ToggleActive();
      doc.Button("lexer_sandbox_but").SetLabel(lexer_sandbox.IsInactive() ? "Open Lexer Sandbox" : "Close Lexer Sandbox");
      GenerateLexer();
    }, "Open Lexer Sandbox", "lexer_sandbox_but").SetCSS(button_style);
    token_div << "</p>";
    token_div << token_table;

    token_div << "<p>";
    token_div << UI::Button([this](){
      AddTokenRow();
      doc.Div("token_div").Redraw();
    }, "Add Row", "token_row_but").SetCSS(button_style)
    .SetTitle("Add an additional line for defining token types.");

    token_div << UI::Button([this](){
      AddTokenRow();
      AddTokenRow();
      AddTokenRow();
      AddTokenRow();
      AddTokenRow();
      doc.Div("token_div").Redraw();
    }, "+5 Rows", "token_5row_but").SetCSS(button_style)
    .SetTitle("Add five more lines for defining additional tokens.");

    token_div << UI::Button{[this](){
      if (SwapTokenRows(active_token, active_token+1)) {
        ActivateTokenRow(active_token+1);
      }
      doc.Div("token_div").Redraw();
      GenerateGrammar();
      UpdateSandbox();
    }, "Row &darr;"}
      .SetCSS(button_style).SetCSS("float", "right", "padding", "5px 10px")
      .SetTitle("Move active token row DOWN.");

    token_div << UI::Button{[this](){
      if (SwapTokenRows(active_token, active_token-1)) {
        ActivateTokenRow(active_token-1);
      }
      doc.Div("token_div").Redraw();
      GenerateGrammar();
      UpdateSandbox();
    }, "Row &uarr;"}
      .SetCSS(button_style).SetCSS("float", "right", "padding", "5px 10px")
      .SetTitle("Move active token row UP.");
  }

  void InitializeRuleDiv() {
    rule_div.SetBackground(emp::Color{"lightgrey"})
            .SetCSS("margin-top", "10pt")
            .SetCSS("border-radius", "10px")
            .SetCSS("border", "1px solid black")
            .SetCSS("padding", "15px")
            .SetCSS("width", "800px");
    rule_div << HeadingName("Grammar Rules");
    rule_div << "<p style=\"margin-top: 0px\">The <b>first</b> rule is the start rule and must match the "
      "full input.  Re-using a rule name adds more alternatives to that rule.  The C++ function "
      "(optional) is called by the <i>generated</i> code when that rule's AST node is run.</p>";

    rule_table.SetColor(emp::Color{"#000044"});
    rule_table.GetCell(0,0).SetHeader() << "Rule Name";
    rule_table.GetCell(0,1).SetHeader() << "Pattern";
    rule_div << "<p>Show columns: "
      << UI::CheckBox([this](bool in){ show_no_ast=in; ApplyRuleColumns(); }, "show_no_ast").SetChecked(show_no_ast) << "<label for=\"show_no_ast\">No AST</label> "
      << UI::CheckBox([this](bool in){ show_functions=in; ApplyRuleColumns(); }, "show_functions").SetChecked(show_functions) << "<label for=\"show_functions\">C++ Function</label> "
      << UI::CheckBox([this](bool in){ show_notes=in; ApplyRuleColumns(); }, "show_notes").SetChecked(show_notes) << "<label for=\"show_notes\">Note</label> "
      << "</p>";
    rule_table.GetCell(0,2).SetHeader() << "No AST";
    rule_table.GetCell(0,3).SetHeader() << "C++ Function";
    rule_table.GetCell(0,4).SetHeader() << "Note";
    rule_div.SetCSS("overflow-x", "auto");

    // Start table with three rows.
    AddRuleRow();
    AddRuleRow();
    AddRuleRow();

    rule_div << rule_table;

    rule_div << "<p>";
    rule_div << UI::Button([this](){
      AddRuleRow();
      doc.Div("rule_div").Redraw();
    }, "Add Row", "rule_row_but").SetCSS(button_style)
    .SetTitle("Add an additional line for defining grammar rules.");

    rule_div << UI::Button([this](){
      AddRuleRow();
      AddRuleRow();
      AddRuleRow();
      AddRuleRow();
      AddRuleRow();
      doc.Div("rule_div").Redraw();
    }, "+5 Rows", "rule_5row_but").SetCSS(button_style)
    .SetTitle("Add five more lines for defining additional grammar rules.");

    rule_div << UI::Button([this](){
      ClearTokenTable();
      AddTokenRow();
      AddTokenRow();
      AddTokenRow();
      doc.Div("token_div").Redraw();
      ClearRuleTable();
      AddRuleRow();
      AddRuleRow();
      AddRuleRow();
      doc.Div("rule_div").Redraw();
      GenerateGrammar();
      UpdateSandbox();
    }, "Reset", "reset_but").SetCSS(button_style)
    .SetTitle("Reset tokens and rules back to the starting setup.");

    rule_div << UI::Button([this](){
      SaveTables();
    }, "Save Grammar")
      .SetCSS(button_style)
      .SetTitle("Save tokens, grammar rules, and settings to a file.");

    // Create a fake, hidden input for uploading a file to be triggered by the load button.
    rule_div << UI::FileInput([this](emp::File load_file){
      LoadTables(load_file);
    }, "load_input").SetCSS("display", "none");

    rule_div << UI::Button([this](){
      // Just open the dialog; loading is async and LoadTables() handles the refresh.
      doc.FileInput("load_input").DoClick();
    }, "Load Grammar", "load_but")
      .SetCSS(button_style)
      .SetTitle("Load previously saved tokens and grammar rules from file.");

    rule_div << UI::Button{[this](){
      if (SwapRuleRows(active_rule, active_rule+1)) {
        ActivateRuleRow(active_rule+1);
      }
      doc.Div("rule_div").Redraw();
      GenerateGrammar();
      UpdateSandbox();
    }, "Row &darr;"}
      .SetCSS(button_style).SetCSS("float", "right", "padding", "5px 10px")
      .SetTitle("Move active rule row DOWN. (Order matters: alternatives are tried top to bottom.)");

    rule_div << UI::Button{[this](){
      if (SwapRuleRows(active_rule, active_rule-1)) {
        ActivateRuleRow(active_rule-1);
      }
      doc.Div("rule_div").Redraw();
      GenerateGrammar();
      UpdateSandbox();
    }, "Row &uarr;"}
      .SetCSS(button_style).SetCSS("float", "right", "padding", "5px 10px")
      .SetTitle("Move active rule row UP. (Order matters: alternatives are tried top to bottom.)");

    rule_div << "<br>";

    rule_div << UI::Button([this](){
      GenerateCPP();
    }, "Generate C++ Code", "generate_but").SetCSS(button_style).SetBackground(emp::Color{"#330066"})
    .SetTitle("Generate a lexer + parser using the tokens and grammar defined above.");

    rule_div << UI::Button([this](){
      ToggleSandbox();
      GenerateGrammar();
      UpdateSandbox();
    }, "Open Sandbox", "sandbox_but").SetCSS(button_style).SetBackground(emp::Color{"#330066"})
    .SetTitle("Try out the current grammar live and explore the resulting syntax trees.");

    rule_div << UI::Button([this](){
      doc.Div("settings_div").ToggleActive();
    }, "Advanced Options", "settings_but")
      .SetCSS(button_style).SetCSS("float", "right", "border-radius", "15px", "font-size", "12px")
      .SetTitle("Adjust naming details for generated code.");
  }

  void InitializeSettingsDiv() {
    settings_div.SetBackground(Palette::TAN).SetCSS(div_style);
    settings_div << HeadingName("Advanced Options");

    size_t row_id = 0;
    settings_table[row_id][0] << "&nbsp;";
    settings_table[row_id][1].SetCSS("font-weight", "bold").SetBackground(Palette::TAN) << "Generated Filename:";
    settings_table[row_id][2] << UI::TextArea([this](const std::string & str) {
      out_filename = str;
      ClearOutput();
    }, "set_filename").SetText(out_filename).SetWidth(250)
      .SetTitle("Filename to use if you download the generated parser.");
    ++row_id;

    settings_table[row_id][0].SetColSpan(3).SetColor(emp::Color{"darkblue"})
      << "<big><b>Token Data to Store</b></big>";
    ++row_id;

    settings_table[row_id][1].SetCSS("font-weight", "bold") << "Store lexemes?";
    settings_table[row_id][2] << UI::CheckBox([this](bool in){ use_token_lexemes = in; ClearOutput(); }, "checkbox_lexemes")
      .SetChecked(use_token_lexemes)
      .SetTitle("Should we store found lexemes as part of the generated Token class?");
    ++row_id;

    settings_table[row_id][1].SetCSS("font-weight", "bold") << "Store line numbers?";
    settings_table[row_id][2] << UI::CheckBox([this](bool in){ use_token_line_num = in; ClearOutput(); }, "checkbox_line_nums")
      .SetChecked(use_token_line_num)
      .SetTitle("Should we store the line number where a token was found as part of the generated Token class?");
    ++row_id;

    settings_table[row_id][1].SetCSS("font-weight", "bold") << "Store columns?";
    settings_table[row_id][2] << UI::CheckBox([this](bool in){ use_token_column = in; ClearOutput(); }, "checkbox_cols")
      .SetChecked(use_token_column)
      .SetTitle("Should we store the column where a token was found as part of the generated Token class?");
    ++row_id;

    settings_table[row_id][0].SetColSpan(3).SetColor(emp::Color{"darkblue"})
      << "<big><b>Names to use in the generated C++ code</b></big>";
    ++row_id;

    settings_table[row_id][1].SetCSS("font-weight", "bold") << "Include Guards: ";
    settings_table[row_id][2] << UI::TextArea([this](const std::string & str) {
      inc_guards = str;
      ClearOutput();
    }, "set_includes").SetText(inc_guards).SetWidth(250)
      .SetTitle("Unique name of include guards at top and bottom of generated C++ file.");
    ++row_id;

    settings_table[row_id][1].SetCSS("font-weight", "bold") << "Namespace: ";
    settings_table[row_id][2] << UI::TextArea([this](const std::string & str) {
      name_space = str;
      ClearOutput();
    }, "set_namespace").SetText(name_space).SetWidth(250)
      .SetTitle("Namespace where generated classes should be placed.");
    ++row_id;

    settings_table[row_id][1].SetCSS("font-weight", "bold") << "Lexer class Name: ";
    settings_table[row_id][2] << UI::TextArea([this](const std::string & str) {
      lexer_name = str;
      ClearOutput();
    }, "set_lexer_class").SetText(lexer_name).SetWidth(250)
      .SetTitle("Identifier name to use for the generated C++ Lexer class.");
    ++row_id;

    settings_table[row_id][1].SetCSS("font-weight", "bold") << "Token class Name: ";
    settings_table[row_id][2] << UI::TextArea([this](const std::string & str) {
      token_name = str;
      ClearOutput();
    }, "set_token_class").SetText(token_name).SetWidth(250)
      .SetTitle("Identifier name to use for the generated C++ Token class.");
    ++row_id;

    settings_table[row_id][1].SetCSS("font-weight", "bold") << "DFA class Name: ";
    settings_table[row_id][2] << UI::TextArea([this](const std::string & str) {
      dfa_name = str;
      ClearOutput();
    }, "set_dfa_class").SetText(dfa_name).SetWidth(250)
      .SetTitle("Identifier name to use for the generated C++ DFA class.");
    ++row_id;

    settings_table[row_id][1].SetCSS("font-weight", "bold") << "Parser class Name: ";
    settings_table[row_id][2] << UI::TextArea([this](const std::string & str) {
      parser_name = str;
      ClearOutput();
    }, "set_parser_class").SetText(parser_name).SetWidth(250)
      .SetTitle("Identifier name to use for the generated C++ Parser class.");
    ++row_id;

    settings_table[row_id][1].SetCSS("font-weight", "bold") << "AST Node class Name: ";
    settings_table[row_id][2] << UI::TextArea([this](const std::string & str) {
      node_name = str;
      ClearOutput();
    }, "set_node_class").SetText(node_name).SetWidth(250)
      .SetTitle("Identifier name to use for the generated C++ AST node class.");
    ++row_id;

    settings_table[row_id][1].SetCSS("font-weight", "bold") << "AST value type: ";
    settings_table[row_id][2] << UI::TextArea([this](const std::string & str) {
      value_type = str;
      ClearOutput();
    }, "set_value_type").SetText(value_type).SetWidth(250)
      .SetTitle("C++ type produced when AST nodes are Run (e.g. double, std::string, MyValue, void).");
    ++row_id;

    settings_div << settings_table;
  }

  // Build an HTML rendering of an AST for the sandbox.
  void RenderAST(const emp::Parser::node_t & node, emp::String & out) {
    if (node.IsLeaf()) {
      const auto & token = node.GetToken();
      out += "<div><span style=\"color: #FFDD66\">";
      out += emp::MakeWebSafe(lexer.GetTokenName(token.id));
      out += "</span> <span style=\"color: #99FF99\">&quot;";
      out += token.lexeme.AsEscaped(false).AsWebSafe(true);
      out += "&quot;</span></div>\n";
      return;
    }
    out += "<div><span style=\"color: #9999FF; font-weight: bold\">";
    out += emp::MakeWebSafe(parser.GetRuleName(node.GetRuleID()));
    out += "</span> <span style=\"color: #777777; font-size: 11px\">(pattern ";
    out += emp::MakeString(node.GetPatternID());
    out += ")</span>\n";
    if (!node.GetNote().empty()) out += " <span style=\"color: #FFDD66\">" + node.GetNote().AsEscaped(false).AsWebSafe(true) + "</span>";
    if (node.NumChildren()) {
      out += "<div style=\"margin-left: 14px; border-left: 1px solid #555555; padding-left: 10px\">\n";
      for (size_t i = 0; i < node.NumChildren(); ++i) {
        RenderAST(node.GetChild(i), out);
      }
      out += "</div>\n";
    }
    out += "</div>\n";
  }

  void InitializeSandboxDiv() {
    sandbox_input.SetText("# Sample text; replace with whatever you want to try parsing.\n"
                          "int countdown = 10;\n"
                          "while (countdown > 0) {\n"
                          "  print(countdown);\n"
                          "  countdown = countdown - 1;\n"
                          "}\n"
                          "print(\"Boom!\");\n");

    sandbox_div.SetBackground(Palette::BLACK).SetColor(Palette::WHITE).SetCSS(div_style);
    sandbox_div << UI::Button([this](){
      GenerateGrammar();
      UpdateSandbox();
    }, "Refresh", "sandbox_refresh_but").SetCSS(sandbox_but_style);
    sandbox_div << UI::Button([this](){
      sandbox_show_tokens = !sandbox_show_tokens;
      if (sandbox_show_tokens) doc.Button("sandbox_tokens_but").SetLabel("Tokens: ON");
      else doc.Button("sandbox_tokens_but").SetLabel("Tokens: OFF");
      UpdateSandbox();
    }, "Tokens: OFF", "sandbox_tokens_but").SetCSS(sandbox_but_style)
      .SetTitle("Also show the token stream that is sent into the parser.");
    sandbox_div << sandbox_input.SetSize(750, 115);
    sandbox_div << "<p>";
    sandbox_div << sandbox_text.SetWidth(750).SetBackground(Palette::BLACK).SetColor(Palette::WHITE);
    sandbox_div << "</p>";

    sandbox_input.SetCallback([this](std::string){ UpdateSandbox(); });

    sandbox_colors.push_back("#8888FF");
    sandbox_colors.push_back("#99FF99");
    sandbox_colors.push_back("#FFFF88");
    sandbox_colors.push_back("#FF88FF");
    sandbox_colors.push_back("#88FFFF");
    sandbox_colors.push_back("#f58231");
    sandbox_colors.push_back("#ffe119");
    sandbox_colors.push_back("#bfef45");
    sandbox_colors.push_back("#4363d8");
    sandbox_colors.push_back("#fabed4");
    sandbox_colors.push_back("#ffd8b1");
    sandbox_colors.push_back("#aaffc3");
    sandbox_colors.push_back("#dcbeff");
    sandbox_colors.push_back("#3cb44b");
  }

  void UpdateSandbox() {
    if (sandbox_div.IsInactive()) return;

    sandbox_text.Freeze();
    sandbox_text.Clear();

    // If we have grammar problems, redirect the user to the reported errors.
    if (!grammar_ok) {
      if (CountRuleRows() == 0) {
        sandbox_text << "<span style=\"color: #FFCCCC\">Define some grammar rules above "
          "(or load an example from the Home tab) to see input parsed here.</span>";
      } else {
        sandbox_text << "<span style=\"color: #FFCCCC\">Fix the grammar errors listed "
          "above to try out parsing.</span>";
      }
      sandbox_text.Activate();
      return;
    }

    emp::TokenStream tokens = lexer.Tokenize(sandbox_input.GetText(), "Empala Sandbox");

    // If requested, show the token stream being fed into the parser.
    if (sandbox_show_tokens) {
      sandbox_text << "<b>Tokens:</b> ";
      if (tokens.size() == 0) sandbox_text << "(no visible tokens)";
      for (const auto & token : tokens) {
        if (token.id == -1) {
          sandbox_text << "[<span style=\"background-color:#440000; color:#FFCCCC\">"
                       << token.lexeme.AsEscaped(false).AsWebSafe(true) << "</span>]";
        } else {
          size_t color_id = ((size_t) (255 - token.id)) % sandbox_colors.size();
          sandbox_text << "[<span style=\"color:" << sandbox_colors[color_id] << "\">"
                       << token.lexeme.AsEscaped(false).AsWebSafe(true) << "</span>]";
        }
      }
      sandbox_text << "<hr style=\"border-color: #333333\">";
    }

    // Try to parse the tokens and display the resulting AST (or the error).
    emp::Parser::ast_t ast;
    try {
      ast = parser.Parse(tokens);
    } catch (const empala::DiagnosticError &) {
      // The parser records GetError() before issuing its fatal notification.
    }
    DiscardNotify();  // Any parse error is expected; we show it in the sandbox itself.

    if (ast && !ast.HasRoot()) {
      sandbox_text << "<span style=\"color: #FFDD66\">Parse succeeded. Warning: this input produces no AST root.</span>";
    } else if (ast) {
      emp::String ast_html;
      RenderAST(ast.GetRoot(), ast_html);
      sandbox_text << "<div style=\"font-family: monospace; font-size: 13px\">\n"
                   << ast_html << "</div>\n";
    } else {
      sandbox_text << "<span style=\"background-color:#440000; color:#FFCCCC\">&nbsp;"
                   << emp::MakeWebSafe(parser.GetError()) << "&nbsp;</span>";
    }

    sandbox_text.Activate();
  }

  void InitializeOutputDiv() {
    UI::Style icon_style{
      "background-color", "black",
      "position", "absolute",
      "cursor", "pointer",
      "top", "10px",
      "right", "10px",
      "border", "none"
   };

    output_div.SetCSS("width", "830px", "position", "relative");
    output_div.SetBackground(Palette::BLACK).SetColor(Palette::WHITE);
    output_div.SetBorder("20px").SetCSS("border-radius", "10px");
    output_div << UI::Button([this](){ DownloadCode(); },
      "<img src=\"../../LexerGenerator/web/Icons/ICON-Save.png\" width=\"40px\">", "download_icon")
      .SetTitle("Download Code").SetCSS(icon_style).SetCSS("right", "120px").Hide();
    output_div << UI::Button([this](){ CopyCode(); },
      "<img src=\"../../LexerGenerator/web/Icons/ICON-Copy.png\" width=\"50px\">", "copy_icon")
      .SetTitle("Copy Code to Clipboard").SetCSS(icon_style).SetCSS("right", "60px").Hide();
    output_div << UI::Button([this](){ ClearOutput(); },
      "<img src=\"../../LexerGenerator/web/Icons/ICON-Close.png\" width=\"40px\">", "close_icon")
      .SetTitle("Close Code").SetCSS(icon_style).SetCSS("right", "10px").Hide();
    output_div << output_text;
  }

  void InitializeFooterDiv() {
    footer_div.SetBackground(emp::Color{"#000044"}).SetColor(Palette::WHITE).SetCSS(div_style);
    footer_div <<
      "Empala was developed by Dr. Charles Ofria at Michigan State University, 2026. "
      "See \"About\" for more information.";
  }

public:
  Empala() {
    // Capture parser errors and warnings (reported through emp::notify) so that they can
    // be shown on the page rather than halting the program.
    emp::notify::AddHandler("ERROR", [this](emp::notify::id_arg_t, emp::notify::message_arg_t msg) -> bool {
      notify_errors.push_back(msg);
      throw empala::DiagnosticError{};
    });
    emp::notify::AddHandler("WARNING", [this](emp::notify::id_arg_t, emp::notify::message_arg_t msg) {
      notify_warnings.push_back(msg);
      return true;
    });

    InitializeButtonDiv();
    InitializeTokenDiv();
    InitializeRuleDiv();
    InitializeSettingsDiv();
    error_div.SetBackground(Palette::WHITE).SetColor(Palette::RED);
    InitializeSandboxDiv();
    InitializeLexerSandbox();
    InitializeOutputDiv();
    InitializeFooterDiv();

    // Place all of the divs into the document.
    doc << "<h1>Empala: A C++ Parser Generator</h1>";
    doc << button_div;
    doc << intro_div;
    doc << token_div;
    doc << lexer_errors;
    doc << lexer_sandbox;
    doc << rule_div;
    doc << settings_div;
    doc << error_div;
    doc << sandbox_div;
    doc << output_div;
    doc << footer_div;

    UpdateIntro("home");
    settings_div.Deactivate();
    sandbox_div.Deactivate();
    lexer_sandbox.Deactivate();
  }
};


Empala empala_app{};

int emp_main() {
  return 0;
}
