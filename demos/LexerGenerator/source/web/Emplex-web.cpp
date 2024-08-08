/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2024
*/
/**
 *  @file
 */


#include "emp/base/array.hpp"
#include "emp/base/notify.hpp"
#include "emp/compiler/Lexer.hpp"
#include "emp/io/CPPFile.hpp"
#include "emp/tools/String.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;
constexpr size_t MAX_TOKENS = 100;

UI::Document doc("emp_base");


class TokenInput {
private:
  emp::String name;
  emp::String regex;
  bool ignore = false;

public:
  const emp::String & GetName() const { return name; }
  const emp::String & GetRegex() const { return regex; }
  bool GetIgnore() const { return ignore; }

  void SetName(emp::String in_name) { name = in_name; }
  void SetRegex(emp::String in_regex) { regex = in_regex; }
  void SetIgnore(bool in) { ignore = in; }
};

struct LexerInfo {
  emp::array<TokenInput, MAX_TOKENS> token_info;
  emp::String class_name{"Lexer"};
  emp::String out_filename{"lexer.hpp"};
  emp::String inc_guards{"EMPLEX_LEXER_HPP_INCLUDE_"};
  emp::String name_space{"emplex"};
};

LexerInfo lexer_info;

UI::Table token_table(4, 4, "token_table");
UI::Div output_div;
UI::Text output_text;

// Add a row to the bottom of the token table.
void AddTableRow() {
  size_t row_id = token_table.GetNumRows();
  if (row_id > MAX_TOKENS) {
    emp::notify::Warning("Maximum ", MAX_TOKENS, " token types allowed!");
    return;
  }
  auto new_row = token_table.AddRow();
  UI::TextArea name_area( [row_id](const std::string & str) {
    lexer_info.token_info[row_id-1].SetName(str);
  }, emp::MakeString("token_table_name_", row_id) );
  UI::TextArea regex_area( [row_id](const std::string & str) {
    lexer_info.token_info[row_id-1].SetRegex(str);
  }, emp::MakeString("token_table_regex_", row_id) );
  UI::CheckBox ignore_toggle( [row_id](bool setting) {
    lexer_info.token_info[row_id-1].SetIgnore(setting);
  }, emp::MakeString("token_table_ignore_", row_id)  );
  new_row[0] << name_area;
  new_row[1] << regex_area;
  new_row[2] << ignore_toggle;
}

// void SwapTableRows(size_t row1, size_t row2) {
//   const size_t num_rows = token_table.GetNumRows() - 1;
//   emp_assert(row1 < num_rows && row2 < num_rows);

//   /// Collect the names of text areas to look them up and swap values.
//   emp::String name1 = emp::MakeString("token_table_name_", row1+1);
//   emp::String regex1 = emp::MakeString("token_table_regex_", row1+1);
//   emp::String ignore1 = emp::MakeString("token_table_ignore_", row1+1);

//   emp::String name2 = emp::MakeString("token_table_name_", row2+1);
//   emp::String regex2 = emp::MakeString("token_table_regex_", row2+1);
//   emp::String ignore2 = emp::MakeString("token_table_ignore_", row2+1);

//   lexer_info.token_info[row_id-1].name
//   emp::String t1 = doc.TextArea(name1).GetText();
// }

// Remove a specified row from the table.
void RemoveTableRow(size_t id) {
  const size_t num_rows = token_table.GetNumRows() - 1;
  emp_assert(id < num_rows);
  while (id < num_rows) {
    
    ++id;
  }
}

void Generate() {
  emp::Lexer lexer;

  // Load all of the tokens ino the lexer.
  for (const auto & t_info : lexer_info.token_info) {
    emp::String name = t_info.GetName();
    emp::String regex = t_info.GetRegex();
    bool ignore = t_info.GetIgnore();

    if (name.empty() && regex.empty()) continue;

    if (!name.size()) {
      emp::notify::Message("Empty token name has regular expression '", name, "'; must supply name.");
      return;
    }
    if (!regex.size()) {
      emp::notify::Message("Token '", name, "' does not have an associated regex.");
      return;
    }

    if (ignore) lexer.IgnoreToken(name, regex);
    else lexer.AddToken(name, regex);
  }

  emp::CPPFile file;
  file.SetGuards(lexer_info.inc_guards);
  file.SetNamespace(lexer_info.name_space);
  lexer.WriteCPP(file, lexer_info.class_name);

  std::stringstream ss;
  file.Write(ss);
  output_text.Clear();
  output_text.SetBorder("20px");
  output_text << "<pre style=\"padding: 10px; border-radius: 5px; overflow-x: auto;\">\n" << emp::MakeWebSafe(ss.str()) << "\n</pre>\n";
  doc.Redraw();
}

int main()
{
  emp::notify::MessageHandlers().Add([](const std::string & msg){ emp::Alert(msg); return true; });
  emp::notify::WarningHandlers().Add([](const std::string & msg){ emp::Alert(msg); return true; });
  emp::notify::ErrorHandlers().Add([](const std::string & msg){ emp::Alert(msg); return true; });

  doc << "<h2>Emplex: A C++ Lexer Generator</h2>";

  doc << "Emplex will take a series of token names and associated regular expressions and\n"
         "generate a fast, table-driven lexer in C++.\n"
      << "<br><br>\n";


  // token_table.SetCSS("border-collapse", "collapse");
  token_table.SetBackground("lightgrey");
  token_table.GetCell(0,0).SetHeader() << "Token Name";
  token_table.GetCell(0,1).SetHeader() << "Regular Expression";
  token_table.GetCell(0,2).SetHeader() << "Ignore?";

  // Start table with three rows?
  AddTableRow();
  AddTableRow();
  AddTableRow();

  doc << token_table;

  doc << "<p>";
  doc.AddButton([](){
    AddTableRow();
    doc.Redraw();
  }, "Add Row", "row_but").SetBackground("#CCCCFF");

  doc << "<p>";

  auto settings_table = doc.AddTable(4, 2, "settings_table");
  settings_table.SetBackground("tan");

  settings_table[0][0].SetHeader().SetCSS("padding-bottom", "15px") << "<br>Class Name: ";
  settings_table[0][1] << UI::TextArea([](const std::string & str) {
    lexer_info.class_name = str;
  }, "set_class").SetText(lexer_info.class_name).SetWidth(250);

  settings_table[1][0].SetHeader().SetCSS("padding-bottom", "15px") << "<br>Filename: ";
  settings_table[1][1] << UI::TextArea([](const std::string & str) {
    lexer_info.out_filename = str;
  }, "set_filename").SetText(lexer_info.out_filename).SetWidth(250);

  settings_table[2][0].SetHeader().SetCSS("padding-bottom", "15px") << "<br>Include Guards: ";
  settings_table[2][1] << UI::TextArea([](const std::string & str) {
    lexer_info.inc_guards = str;
  }, "set_includes").SetText(lexer_info.inc_guards).SetWidth(250);

  settings_table[3][0].SetHeader().SetCSS("padding-bottom", "15px") << "<br>Namespace: ";
  settings_table[3][1] << UI::TextArea([](const std::string & str) {
    lexer_info.name_space = str;
  }, "set_namespace").SetText(lexer_info.name_space).SetWidth(250);

  doc << "<p>";
  doc.AddButton([](){ Generate(); }, "Generate", "gen_but").SetBackground("#CCCCFF");;

  doc << "<br>";
  doc << "<H3>Output:</H3>";

  output_div.SetBackground("black");
  output_div.SetColor("white");
  output_div.SetBorder("20px");

  doc << output_div;
  output_div << output_text;
}
