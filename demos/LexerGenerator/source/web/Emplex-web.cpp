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
    name_text.SetText(name).SetCSS("max-width", "250px", "font-size", "14px");
    regex_text.SetText(regex).SetCSS("max-width", "250px", "font-size", "14px");
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
  const emp::String text_color = "white";
  const emp::String active_color = "#0000AA";
  const emp::String button_color = "#000044";
  const emp::String table_color = "white"; // "#FFFFE0";
  const emp::String link_color = "#C0C0FF";
  intro_div.SetColor(text_color).SetBackground(button_color).SetPadding(5).SetCSS("border-radius", "10px", "border", "1px", "border-style", "solid", "padding", "10px", "max-width", "800px");
  doc.Button("home_but").SetBackground(button_color);
  doc.Button("lexer_but").SetBackground(button_color);
  doc.Button("regex_but").SetBackground(button_color);
  doc.Button("cpp_but").SetBackground(button_color);
  doc.Button("example_but").SetBackground(button_color);
  doc.Button("about_but").SetBackground(button_color);

  UI::Style table_style;
  table_style.Set("background-color", table_color)
             .Set("color", "white")
             .Set("padding", "10px")
             .Set("border", "1px solid black")
             .Set("text_align", "center")
  ;

  if (mode == "home") {
    doc.Button("home_but").SetBackground(active_color);
    intro_div <<
      "<big><big><b>Overview</b></big></big><br>\n"
      "<p>Emplex uses a set of <b>token names</b> and associated <b>regular expressions</b> to "
      "generate code for a fast, table-driven lexer in C++.</p>"
      "<p>Click on the buttons above to learn more about how Emplex works, or just try it out below.</p>";
  } else if (mode == "lexer") {
    doc.Button("lexer_but").SetBackground(active_color);
    intro_div <<
      "<big><big><b>Lexical analysis</b></big></big><br>\n"
      "<p>A <a href=\"https://en.wikipedia.org/wiki/Lexical_analysis\" style=\"color: " << link_color << ";\">lexical analyzer</a> "
      "(commonly called a \"lexer\", \"tokenizer\", or \"scanner\") reads a stream of input "
      "characters, typically from a text file, and breaks it into tokens that each form an "
      "atomic input unit.  For example, if we consider the following code where we might be "
      "calculating the area of a triangle:</p>\n"
      "<p>&nbsp;&nbsp;<code style=\"background-color: " << table_color << "; color: black; padding:10px; border: 1px solid black\">double area3 = base * height / 2.0;</code></p>"
      "<p>We could convert this statement into the series of tokens:</p>"
      "<p><table cellpadding=2px border=2px style=\"background-color: " << table_color << "; color: black; text-align: center;\">"
      "<tr><th width=150px>Lexeme</th><th width=150px>Token Type</th></tr>"
      "<tr><td><code>double</code></td> <td>TYPE</td>       </tr>"
      "<tr><td><code>area3</code></td>  <td>IDENTIFIER</td> </tr>"
      "<tr><td><code>=</code></td>      <td>OPERATOR</td>   </tr>"
      "<tr><td><code>base</code></td>   <td>IDENTIFIER</td> </tr>"
      "<tr><td><code>*</code></td>      <td>OPERATOR</td>   </tr>"
      "<tr><td><code>height</code></td> <td>IDENTIFIER</td> </tr>"
      "<tr><td><code>/</code></td>      <td>OPERATOR</td>   </tr>"
      "<tr><td><code>2.0</code></td>    <td>FLOAT</td>      </tr>"
      "<tr><td><code>;</code></td>      <td>ENDLINE</td>    </tr>"
      "</table></p>\n"
      "<p>In order to build a lexer, we define the set of token types that we want to use and "
      "build a <i>regular expression</i> for each that can identify the associated tokens.</p>\n"
      "<p>The lexer will always find the <i>longest</i> token that can be fully matched from the "
      "beginning of the input. If there is a tie for longest, the lexer will match the <i>first</i> "
      "token type listed</p>\n"
      "<p>For example, we could define the following token types:</p>\n"
      "<p><table cellpadding=2px border=2px style=\"background-color: " << table_color << "; color: black; text-align: center;\">\n"
      "  <tr><td width=150px>KEYWORD</td> <td width=200px><code>(for)|(if)|(set)|(while)</code></td></tr>\n"
      "  <tr><td>IDENTIFIER</td>          <td><code>[a-zA-Z_][a-zA-Z0-9_]*</code></td>              </tr>\n"
      "  <tr><td>INTEGER   </td>          <td><code>[0-9]+                </code></td>              </tr>\n"
      "  <tr><td>WHITESPACE</td>          <td><code>[ \\t\\n\\r]          </code></td>              </tr>\n"
      "</table></p>\n"
      "<p>Then if we were parsing \"<code>set formula_id 5</code>\", "
      "the first token would be \"set\" and it would be type KEYWORD because while both "
      "KEYWORD and IDENTIFIER could match this series fo characters, KEYWORD comes first in the list. "
      "The next token would be a single space of type WHITESPACE, though if we marked the "
      "WHITESPACE token as 'ignore' then its characters would be skipped over and the token would "
      "not be included in the returned vector. "
      "After that the characters \"for\" could be matched by KEYWORD, but IDENTIFIER would be able "
      "to match the longer \"formula_id\", and as such it would be chosen next.<p>\n"
      "<p>See the next tab if you want to learn about writing regular expressions in Emplex.</p>\n";
  } else if (mode == "regex") {
    doc.Button("regex_but").SetBackground(active_color);
    intro_div <<
      "<big><big><b>Regular Expressions</b></big></big><br>\n"
      "<p>A <a href=\"https://en.wikipedia.org/wiki/Regular_expression\" style=\"color: " << link_color << ";\">regular expression</a> "
      "(or \"regex\") is a mechanism to describe a pattern of characters "
      "and, in particular, they can be used to describe tokens for lexical analysis.</p> "
      "<p>In a regular expression, letters and digits always directly match themselves, but other "
      "characters often have a special function.  The following regular expression techniques are "
      "implemented in emplex:</p>"
      "<p><table border=\"2\" cellpadding=\"3\" style=\"background: white; color: black\">"
      "<tr><th>Symbol</th><th>Description</th><th>Example</th><th>Explanation</th>\n"
      "<tr><th>|</th>       <td>A logical \"or\" (match just one side)</td>"
                           "<td><code>this|that</code></td>"
                           "<td>Match the words \"this\" or \"that\", but nothing else</td>\n"
      "<tr><th>( ... )</th> <td>Specify grouping</td>"
                           "<td><code>th(is|at)</code></td>"
                           "<td>Also match just the words \"this\" or \"that\"</td>\n"
      "<tr><th>\"</th>      <td>Quotes (directly match symbols inside)</td>"
                           "<td><code>\"|\"</code></td>"
                           "<td>Match the pipe symbol</td>\n"
      "<tr><th>?</th>       <td>The previous match is optional</td>"
                           "<td><code>a?b</code></td>"
                           "<td>Match \"ab\" or just \"b\"; the 'a' is optional</td>\n"
      "<tr><th>*</th>       <td>The previous match can be made zero, one, or multiple times</td>"
                           "<td><code>c*d</code></td>"
                           "<td>Match \"d\", \"cd\", \"ccd\", \"cccccd\" or with any other number of c's</td>\n"
      "<tr><th>+</th>       <td>The previous match can be made one or more times</td>"
                           "<td><code>(ab)+</code></td>"
                           "<td>Match \"ab\", \"abab\", \"ababababab\", with any non-zero number of ab's</td>\n"
      "<tr><th>[ ... ]</th> <td>Match any single character between the brackets; ranges of characters are allowed using a dash ('-'). If the first character is a caret ('^') match any character EXCEPT those listed.</td>"
                           "<td><code>[0-9]</code></td>"
                           "<td>Match any single digit.</td>\n"
      "<tr><th>.</th>       <td>Match any single character <i>except</i> a newline ('\\n'); same as <code>[^\\n]</code></td>"
                           "<td><code>.*</code></td>"
                           "<td>Match all characters until the end of the current line.</td>\n"      
      "</table></p>";
  } else if (mode == "cpp") {
    doc.Button("cpp_but").SetBackground(active_color);
    intro_div <<
      "<big><big><b>Working with the Generated C++ Code</b></big></big><br>\n"
      "<p>Emplex will generate C++ code that you can either copy-and-paste to wherever "
      "you need it, or download the generated file that will have a name of your "
      "choosing (\"lexer.hpp\" by default.)</p>\n"
      "<p>Once you have the file set up, you can simply #include it into your code.  It will contain "
      "a lexer object (with the class name \"Lexer\" by default), held in a C++ namespace "
      "(\"emplex\" by default).</p>\n"
      "For example, if you make a lexer for a language called \"Cabbage\", you might want to compile "
      "the file \"mycode.cab\". To handle the lexical analysis on this file you might use code like:</p>\n"
      "<pre style=\"background-color: " << table_color << "; color: black; padding:10px\">\n"
      "   std::ifstream in_file(\"mycode.cab\");    // Load the input file\n"
      "   emplex::Lexer lexer;                   // Build the lexer object\n"
      "   std::vector<emplex::Token> tokens = lexer.Tokenize(in_file);\n"
      "   // ... Use the vector of tokens ...\n"
      "</pre>\n"
      "<p>In practice, any input stream can be fed into a generated lexer to produce the vector of tokens. "
      "Once you do, each token is a simple <code>struct</code> for you to use:</p>\n"
      "<pre style=\"background-color: " << table_color << "; color: black; padding:10px\">\n"
      "   struct Token {\n"
      "     int id;              // Type ID for token\n"
      "     std::string lexeme;  // Sequence matched by token\n"
      "     size_t line_id;      // Line token started on\n"
      "   };\n"
      "</pre>\n"
      "<p>If you want to translate a token ID back to a token type, you can use "
      "\"<code>lexer.GetTokenName(id);</code>\".  The <code>lexeme</code> field provides "
      "the specific set of characters that were matched in the input stream. "
      "The <code>line_id</code> gives the line number on which the token was found in "
      "the input stream and can be useful for error reporting."
      "<br><br>";
  } else if (mode == "about") {
    doc.Button("about_but").SetBackground(active_color);
    intro_div <<
      "<big><big><b>About</b></big></big><br>\n"
      "<p>Emplex is written in C++ using the "
      "<a href=\"https://github.com/devosoft/Empirical\" style=\"color: " << link_color << ";\">Empirical Library</a>, "
      "and then compiled into "
      "<a href=\"https://webassembly.org/\" style=\"color: " << link_color << ";\">WebAssembly</a> "
      "with the <a href=\"https://emscripten.org/\" style=\"color: " << link_color << ";\">Emscripten</a> "
      "LLVM compiler.</p>"
      "<p>Emplex takes in the set of token types and associated regular expressions.  Each regular expression is then converted into a non-deterministic finite automaton (NFA).  The set of automata are merged together, while keeping track of which token type each end condition is associated with (using the first end condition listed when more than one can apply).  The resulting NFA is then converted into a DFA, which can be implemented as a table.  That table is then hard-coded into the C++ output that is generated, along with the associated helper functions.</p>"
      "<p>The Emplex software and most of the associated tools in the underlying "
      "Empirical library were written by "
      "<a href=\"https://ofria.com/\" style=\"color: " << link_color << ";\">Dr. Charles Ofria</a> "
      "at <a href=\"https://msu.edu/\" style=\"color: " << link_color << ";\">Michigan State University</a>."
      "<br><br>";
  } else if (mode == "examples") {
    doc.Button("example_but").SetBackground(active_color);
    intro_div <<
      "<big><big><b>Examples</b></big></big><br>\n"
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
  UI::Style button_style;
  button_style.Set("padding", "10px 15px")
    .Set("background-color", "#000066") // Dark Blue background
    .Set("color", "white")              // White text
    .Set("border", "1px solid white")   // Thin white border
    .Set("border-radius", "5px")        // Rounded corners
    .Set("cursor", "pointer")
    .Set("font-size", "14px")
    .Set("transition", "background-color 0.3s ease, transform 0.3s ease"); // Smooth transition

  button_div << UI::Button([](){ UpdateIntro("home"); doc.Redraw(); }, "Home", "home_but").SetCSS(button_style).SetBackground("#0000AA");
  button_div << UI::Button([](){ UpdateIntro("lexer"); doc.Redraw(); }, "Lexical Analysis", "lexer_but").SetCSS(button_style);
  button_div << UI::Button([](){ UpdateIntro("regex"); doc.Redraw(); }, "Regular Expressions", "regex_but").SetCSS(button_style);
  button_div << UI::Button([](){ UpdateIntro("cpp"); doc.Redraw(); }, "Generated C++ Code", "cpp_but").SetCSS(button_style);
  button_div << UI::Button([](){ UpdateIntro("examples"); doc.Redraw(); }, "Examples", "example_but").SetCSS(button_style);
  button_div << UI::Button([](){ UpdateIntro("about"); doc.Redraw(); }, "About", "about_but").SetCSS(button_style);
  doc << button_div;
  doc << "<small><small><br></small></small>";
  doc << intro_div;
  doc << "<br><br>\n";

  // token_table.SetCSS("border-collapse", "collapse");
  UI::Div token_div;
  token_div.SetBackground("lightgrey").SetCSS("border-radius", "10px", "border", "1px solid black", "padding", "15px", "width", "fit-content");
  token_div << "<big><big><b>Tokens</b></big></big><br><br>\n";

  token_table.SetColor("#000044");
  token_table.GetCell(0,0).SetHeader() << "Token Name";
  token_table.GetCell(0,1).SetHeader() << "Regular Expression";
  token_table.GetCell(0,2).SetHeader() << "Ignore?";

  // Start table with three rows?
  AddTableRow();
  AddTableRow();
  AddTableRow();

  token_div << token_table;

  token_div << "<p>";
  token_div << UI::Button([](){
    AddTableRow();
    doc.Redraw();
  }, "Add Row", "row_but").SetCSS(button_style);

  token_div << UI::Button([](){
    AddTableRow();
    AddTableRow();
    AddTableRow();
    AddTableRow();
    AddTableRow();
    doc.Redraw();
  }, "+5 Rows", "5row_but").SetCSS(button_style);

  token_div << UI::Button([](){
    doc.Div("settings_div").ToggleActive();
  }, "Advanced Settings", "settings_but").SetCSS(button_style);

  token_div << UI::Button([](){
    Generate();
  }, "Generate Output", "generate_but").SetCSS(button_style).SetBackground("#330066");

  doc << token_div;
  doc << "<p>";

  UI::Div settings_div("settings_div");
  settings_div.SetBackground("tan").SetCSS("border-radius", "10px", "border", "1px solid black", "padding", "15px", "width", "fit-content");
  settings_div << "<big><big><b>Advanced Settings</b></big></big><br>\n";

  UI::Table settings_table(4, 2, "settings_table");

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

  settings_div << settings_table;
  doc << settings_div;
  settings_div.Deactivate();

  doc << "<br>";
  doc << "<H3>Output:</H3>";

  output_div.SetBackground("black").SetColor("white");
  output_div.SetBorder("20px").SetCSS("border-radius", "10px");

  doc << output_div;
  output_div << output_text;
}
