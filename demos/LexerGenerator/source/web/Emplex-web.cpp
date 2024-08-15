/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2024
*/
/**
 *  @file
 */


#include "emp/base/notify.hpp"
#include "emp/base/vector.hpp"
#include "emp/compiler/Lexer.hpp"
#include "emp/io/CPPFile.hpp"
#include "emp/tools/String.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;
constexpr size_t MAX_TOKENS = 100;

UI::Document doc("emp_base");


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
    name_text.SetText(name);
    regex_text.SetText(regex);
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

struct LexerInfo {
  emp::vector<TokenInput> token_info;
  emp::String lexer_name{"Lexer"};
  emp::String out_filename{"lexer.hpp"};
  emp::String inc_guards{"EMPLEX_LEXER_HPP_INCLUDE_"};
  emp::String name_space{"emplex"};
};

LexerInfo lexer_info;

UI::Div intro_div;
UI::Div button_div;
UI::Div output_div;
UI::Table token_table(1, 4, "token_table");
UI::Text output_text;

void SwapTableRows(size_t row1, size_t row2);
void RemoveTableRow(size_t id);

// Add a row to the bottom of the token table.
void AddTableRow() {
  size_t token_id = token_table.GetNumRows() - 1; // Top row is labels, not token
  if (token_id >= MAX_TOKENS) {
    emp::notify::Warning("Maximum ", MAX_TOKENS, " token types allowed!");
    return;
  }
  auto new_row = token_table.AddRow();
  emp_assert(token_id <= lexer_info.token_info.size());
  if (token_id == lexer_info.token_info.size()) {
    // emp::notify::Message("Token_id = ", token_id, "; lexer_info.token_info.size() = ", lexer_info.token_info.size());
    lexer_info.token_info.emplace_back(TokenInput(token_id));
  }     
  auto & row_info = lexer_info.token_info[token_id];
  new_row[0] << row_info.GetNameWidget();
  new_row[1] << row_info.GetRegexWidget();
  new_row[2] << "&nbsp;&nbsp;&nbsp;" << row_info.GetIgnoreWidget();
  new_row[3] << UI::Button([token_id](){ RemoveTableRow(token_id); doc.Redraw(); }, "X").SetColor("red");
  new_row[3] << UI::Button([token_id](){ SwapTableRows(token_id, token_id-1); doc.Redraw(); }, "&uarr;").SetColor("blue");
  new_row[3] << UI::Button([token_id](){ SwapTableRows(token_id, token_id+1); doc.Redraw(); }, "&darr;").SetColor("blue");
}

void SwapTableRows(size_t row1, size_t row2) {
  [[maybe_unused]] const size_t num_rows = token_table.GetNumRows() - 1;
  if (row1 >= num_rows || row2 >= num_rows) return; // No place to move to.

  lexer_info.token_info[row1].Swap(lexer_info.token_info[row2]);
}

// Remove a specified row from the table.
void RemoveTableRow(size_t id) {
  const size_t num_rows = token_table.GetNumRows() - 1;
  emp_assert(id < num_rows);  // Make sure row to be deleted actually exists.
  // Swap rows to move deleted row to the end.
  while (id < num_rows-1) {
    SwapTableRows(id, id+1);
    ++id;
  }
  // Remove last row
  lexer_info.token_info[id].Clear();
  auto new_row = token_table.RemoveRow();
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
  lexer.WriteCPP(file, lexer_info.lexer_name);

  std::stringstream ss;
  file.Write(ss);
  output_text.Clear();
  output_text.SetBorder("20px");
  output_text << "<pre style=\"padding: 10px; border-radius: 5px; overflow-x: auto;\">\n" << emp::MakeWebSafe(ss.str()) << "\n</pre>\n";
  doc.Redraw();
}

void UpdateIntro(emp::String mode) {
  intro_div.Clear();
  intro_div.SetBackground("#E0E0FF").SetPadding(5).SetCSS("border-radius", "10px", "border", "1px", "border-style", "solid");
  if (mode == "home") {
    intro_div <<
      "<big><big><b>Overview</b></big></big><br>\n"
      "Emplex uses a set of <b>token names</b> and associated <b>regular expressions</b> to "
      "generate a fast, table-driven lexer in C++.<br><br>"
      "Click on the buttons above to learn more about how Emplex works, or just try it out below.<br><br>";
  } else if (mode == "lexer") {
    intro_div <<
      "<H2>Lexical analysis</H2>\n"
      "A lexer will read a stream of input characters (typically a text file) and"
      "break it into tokens that each form an atomic input unit.  For example, if "
      "we consider the following code where we might be calculating the area of a triangle:\n"
      "<p>&nbsp;&nbsp;&nbsp;<code style=\"background-color: #FFFFE0; padding:10px\">double area3 = base * height / 2.0;</code></p>"
      "<p>We could convert this statement into the series of tokens:</p>"
      "&nbsp;&nbsp;&nbsp;<table cellpadding=2px border=2px style=\"background-color: #FFFFE0;\">"
      "<tr><th width=150px>Token Type</th><th width=150px>Lexeme</th></tr>"
      "<tr><td>TYPE</td> <td><code>double</code></td></tr>"
      "<tr><td>IDENTIFIER</td> <td><code>area3</code></td></tr>"
      "<tr><td>OPERATOR</td> <td><code>=</code></td></tr>"
      "<tr><td>IDENTIFIER</td> <td><code>base</code></td></tr>"
      "<tr><td>OPERATOR</td> <td><code>*</code></td></tr>"
      "<tr><td>IDENTIFIER</td> <td><code>height</code></td></tr>"
      "<tr><td>OPERATOR</td> <td><code>/</code></td></tr>"
      "<tr><td>FLOAT</td> <td><code>2.0</code></td></tr>"
      "<tr><td>ENDLINE</td> <td><code>;</code></td></tr>"
      "</table>\n"
      "<p>In order to build a lexer, we define the set of token types that we want to use and "
      "build a <i>regular expression</i> for each that can identify the associated tokens.</p>\n"
      "<p>The lexer will always find the <i>longest</i> token that can be fully matched from the"
      "beginning of the input. If there is a tie for longest, the lexer will match the <i>first</i>"
      "token type listed</p>\n"
      "<p>For example, if we define the following token types:</p><br>\n"
      "<table>\n"
      "  <tr><td>KEYWORD:   </td><td><code>(for)|(if)|(set)|(while)</code></td></tr>\n"
      "  <tr><td>IDENTIFIER:</td><td><code>[a-zA-Z_][a-zA-Z0-9_]*  </code></td></tr>\n"
      "  <tr><td>INTEGER:   </td><td><code>[0-9]+                  </code></td></tr>\n"
      "  <tr><td>WHITESPACE:</td><td><code>[ \\t\\n\\r]            </code></td></tr>\n"
      "</table>\n"
      "<p>Then if we were parsing \"<code>set formula_id 5</code>\", "
      "the first token would be \"set\" and it would be type KEYWORD because while both "
      "KEYWORD and IDENTIFIER could match this series fo characters, KEYWORD comes first in the list. "
      "The next token would be a single space of type WHITESPACE, though if we marked the "
      "WHITESPACE token as 'ignore' then its characters would be skipped over and the token would "
      "not be included in the returned vector. "
      "After that the characters \"for\" could be matched by KEYWORD, but IDENTIFIER would be able "
      "to match the longer \"formula_id\", and as such it would be chosen next.<p>\n"
      "<p>See the next tab if you want to learn about writing regular expressions in Emplex.\n"

      "<br><br>";
  } else if (mode == "regex") {
    intro_div <<
      "<H2>Regular Expressions</H2>\n"
      "A regular expression is a mechanism to describe a pattern of characters, "
      "and in this case can be used to specify tokens for lexical analysis."
      "<br><br>";
  } else if (mode == "cpp") {
    intro_div <<
      "<H2>Working with the Generated C++ Code</H2>\n"
      "<p>Emplex will generate C++ code below that you can either copy-and-paste to wherever "
      "you want to use it, or download the generated file that will have a name of your "
      "choosing (\"lexer.hpp\" by default.)</p>\n"
      "<p>Once you have the file set up, you can simply #include it into your code.  It will contain "
      "a lexer object (with the class name \"Lexer\" by default), held in a C++ namespace "
      "(\"emplex\" by default).</p>\n"
      "For example, if you make a lexer for a language called \"Cabbage\", you might want to compile "
      "the file \"mycode.cab\". To handle the lexical analysis on this file you might use code like:</p>\n"
      "<code style=\"background-color: #FFFFE0; padding:10px\">"
      "&nbsp;&nbsp;&nbsp;std::ifstream in_file(\"mycode.cab\"); // Load the input file<br>"
      "&nbsp;&nbsp;&nbsp;emplex::Lexer lexer;                   // Build the lexer object<br>"
      "&nbsp;&nbsp;&nbsp;auto tokens = lexer.Tokenize(<br>"
      "&nbsp;&nbsp;&nbsp;<br>"
      "</code><br>"
      "<br><br>";
  } else if (mode == "under_hood") {
    intro_div <<
      "<H2>Under the Hood</H2>\n"
      "This page is written in C++ using the Empirical Library"
      "<br><br>";
  } else if (mode == "examples") {
    intro_div <<
      "<H2>Examples</H2>\n"
      "Some examples..."
      "<br><br>";
  }
}

int emp_main()
{
  emp::notify::MessageHandlers().Add([](const std::string & msg){ emp::Alert(msg); return true; });
  emp::notify::WarningHandlers().Add([](const std::string & msg){ emp::Alert(msg); return true; });
  emp::notify::ErrorHandlers().Add([](const std::string & msg){ emp::Alert(msg); return true; });

  doc << "<h1>Emplex: A C++ Lexer Generator</h1>";

  UpdateIntro("home");
  button_div << UI::Button([](){ UpdateIntro("home"); doc.Redraw(); }, "Home", "home_but").SetBackground("#CCCCFF");
  button_div << UI::Button([](){ UpdateIntro("lexer"); doc.Redraw(); }, "Lexical Analysis", "lex_but").SetBackground("#CCCCFF");
  button_div << UI::Button([](){ UpdateIntro("regex"); doc.Redraw(); }, "Regular Expressions", "regex_but").SetBackground("#CCCCFF");
  button_div << UI::Button([](){ UpdateIntro("cpp"); doc.Redraw(); }, "Generated C++ Code", "cpp_but").SetBackground("#CCCCFF");
  button_div << UI::Button([](){ UpdateIntro("under_hood"); doc.Redraw(); }, "Under the Hood", "hood_but").SetBackground("#CCCCFF");
  button_div << UI::Button([](){ UpdateIntro("examples"); doc.Redraw(); }, "Examples", "example_but").SetBackground("#CCCCFF");
  doc << button_div;
  doc << "<small><small><br></small></small>";
  doc << intro_div;
  doc << "<br><br>\n";

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
    lexer_info.lexer_name = str;
  }, "set_class").SetText(lexer_info.lexer_name).SetWidth(250);

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
  doc.AddButton([](){ Generate(); }, "Generate", "gen_but").SetBackground("#CCCCFF");

  doc << "<br>";
  doc << "<H3>Output:</H3>";

  output_div.SetBackground("black").SetColor("white");
  output_div.SetBorder("20px").SetCSS("border-radius", "10px");

  doc << output_div;
  output_div << output_text;
}
