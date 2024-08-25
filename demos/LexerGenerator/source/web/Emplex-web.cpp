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
emp::CPPFile file;
emp::vector<emp::String> errors;


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

struct LexerInfo {
  emp::vector<TokenInput> token_info;
  emp::String lexer_name{"Lexer"};
  emp::String out_filename{"lexer.hpp"};
  emp::String inc_guards{"EMPLEX_LEXER_HPP_INCLUDE_"};
  emp::String name_space{"emplex"};
};

LexerInfo lexer_info;

UI::Div intro_div{"intro_div"};
UI::Div button_div{"button_div"};
UI::Div token_div{"token_div"};
UI::Div settings_div{"settings_div"};
UI::Div error_div{"error_div"};
UI::Div output_div{"output_div"};

UI::Table token_table{1, 4, "token_table"};
UI::Table settings_table{4, 2, "settings_table"};
UI::Text output_text{"output_text"};

UI::Style button_style;
UI::Style button_style_dark;

void UpdateErrors() {
  if (errors.size()) {
    output_text.Clear();
    output_div.Redraw();
    doc.Button("download_but").SetBackground("#606060").SetDisabled().SetTitle("Generate code to activate this button.");
  }

  error_div.Clear();
  for (emp::String & error : errors)  {
    error_div << emp::MakeWebSafe(error) << "<br>\n";
  }
  error_div.Redraw();
}

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
  new_row[3] << UI::Button([token_id](){ RemoveTableRow(token_id); doc.Div("token_div").Redraw(); }, "X").SetColor("red").SetTitle("Click to remove this row.");
  new_row[3] << UI::Button([token_id](){ SwapTableRows(token_id, token_id-1); doc.Div("token_div").Redraw(); }, "&uarr;").SetColor("blue").SetTitle("Click to swap this row with the one above it.");
  new_row[3] << UI::Button([token_id](){ SwapTableRows(token_id, token_id+1); doc.Div("token_div").Redraw(); }, "&darr;").SetColor("blue").SetTitle("Click to swap this row with the one below it.");
}

void AddTableRow(emp::String name, emp::String regex, bool ignore=false) {
  const size_t row_id = token_table.GetNumRows()-1;
  AddTableRow();
  lexer_info.token_info[row_id].Set(name, regex, ignore);
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

// Remove all rows in the table.
void ClearTable() {
  for (auto & row : lexer_info.token_info) row.Clear();
  token_table.Rows(1);
}

void SaveTable() {
  emp::String out;

  for (const auto & t_info : lexer_info.token_info) {
    emp::String name = t_info.GetName();
    emp::String regex = t_info.GetRegex();
    bool ignore = t_info.GetIgnore();
    if (name.empty()) continue;  // Unnamed token types can be skipped.

    if (ignore) out += "-";
    out += name + ' ' + regex + '\n';
  }

  emp::DownloadFile("lexer.emplex", out);
}

template <typename... Ts>
void Error(size_t line_num, Ts &&... args) {
  errors.push_back(emp::MakeString("Error (line ", line_num, ") - ", args...));
}

// Make sure that the token table contains only valid information.
bool TestValidTable() {
  // Make sure all of the token information is valid.
  errors.resize(0);
  size_t line_num = 0;
  std::unordered_set<emp::String> token_names;
  for (const auto & t_info : lexer_info.token_info) {
    ++line_num;
    emp::String name = t_info.GetName();
    emp::String regex = t_info.GetRegex();
    if (name.empty() && regex.empty()) continue;  // Completely empty slots can be skipped.

    if (name.empty()) Error(line_num, "No name provided for RegEx: ", regex);
    else if (regex.empty()) Error(line_num, "No regex provided for token '", name, "'");

    if (!name.OnlyIDChars()) {
      Error(line_num, "Invalid token name '", name, "'; only letters, digits, and '_' allowed.");
    }
    if (token_names.contains(name)) {
      Error(line_num, "Multiple token types named '", name, "'.");
    }
    token_names.insert(name);

    emp::RegEx regex_text(regex);
    for (const auto & note : regex_text.GetNotes()) {
      Error(line_num, "Invalid Regular expression: ", note);
    }
  }

  // Halt generation if any errors were triggered.
  UpdateErrors();
  return !errors.size();
}

void Generate() {
  if (!TestValidTable()) return;

  emp::Lexer lexer;

  // Load all of the tokens ino the lexer (since they passed tests, assume they are valid)
  for (const auto & t_info : lexer_info.token_info) {
    emp::String name = t_info.GetName();
    emp::String regex = t_info.GetRegex();
    bool ignore = t_info.GetIgnore();

    if (name.empty()) continue;

    if (ignore) lexer.IgnoreToken(name, regex);
    else lexer.AddToken(name, regex);
  }

  file.Clear();
  file.SetGuards(lexer_info.inc_guards);
  file.SetNamespace(lexer_info.name_space);
  lexer.WriteCPP(file, lexer_info.lexer_name);

  std::stringstream ss;
  file.Write(ss);
  output_text.Clear();
  output_text.SetBorder("20px");
  output_text << "<pre style=\"padding: 10px; border-radius: 5px; overflow-x: auto;\">\n" << emp::MakeWebSafe(ss.str()) << "\n</pre>\n";
  output_div.Redraw();

  doc.Button("download_but").SetDisabled(false).SetBackground("#330066").SetTitle("Click to download the generated code.");
}

void DownloadCode() {
  std::stringstream ss;
  file.Write(ss);
  emp::DownloadFile(lexer_info.out_filename, ss.str());
}

void UpdateIntro(emp::String mode) {
  intro_div.Clear();
  const emp::String text_color = "white";
  const emp::String active_color = "#0000AA";
  const emp::String button_color = "#000044";
  const emp::String table_color = "white"; // "#FFFFE0";
  const emp::String link_color = "#C0C0FF";
  intro_div.SetColor(text_color).SetBackground(button_color).SetCSS("border-radius", "10px", "border", "1px", "border-style", "solid", "padding", "15px", "width", "800px");
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
      "generate C++ code for a fast, table-driven lexer for ASCII input.</p>"
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
      "token type listed.</p>\n"
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
      "   emplex::Lexer lexer;                    // Build the lexer object\n"
      "   std::vector&lt;emplex::Token&gt; tokens = lexer.Tokenize(in_file);\n"
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
      "<p>If you want to translate a token <code>id</code> back to a token type, you can use "
      "\"<code>emplex::Lexer::TokenName(id);</code>\".  The <code>lexeme</code> field provides "
      "the specific set of characters that were matched in the input stream. "
      "The <code>line_id</code> gives the line number on which the token was found in "
      "the input stream and can be useful for error reporting.</p>"
      "<p>Finally, you can look up the ID value for a particular token type by finding its name "
      "in the Lexer, prepended with \"ID_\".  For example, if you had created a token type called \"INT\" that was assigned "
      "the value 248, then <code>emplex::ID_INT</code> would equal 248.</p>"
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
      "<p>Emplex takes in the set of token types and associated regular expressions. "
      "Each regular expression is then converted into a non-deterministic finite automaton (NFA). "
      "The set of automata are merged together, while keeping track of which token type each end "
      "condition was associated with. When an end condition could have come from two different "
      "regular expressions, the regex listed first (highest in the list) is used. "
      "The resulting NFA is then converted into a DFA, which can be implemented as a table. "
      "That table is then hard-coded into the C++ output that is generated, along with the "
      "associated helper functions.  When tokenization is performed, the longest possible input "
      "string is matched and the ID associated with that end condition is returned.</p>"
      "<p>The Emplex software and most of the associated tools in the underlying "
      "Empirical library were written by "
      "<a href=\"https://ofria.com/\" style=\"color: " << link_color << ";\">Dr. Charles Ofria</a> "
      "at <a href=\"https://msu.edu/\" style=\"color: " << link_color << ";\">Michigan State University</a>."
      "<br><br>";
  } else if (mode == "examples") {
    doc.Button("example_but").SetBackground(active_color);
    intro_div <<
      "<big><big><b>Examples</b></big></big><br>\n"
      "<p>When you are performing lexical analysis on input text, you must first decide "
      "what types of elements you are working with and make a corresponding token type "
      "for each.</p>"
      "<p>Click here to load some example tokens:</p>"
      << UI::Button([](){
          ClearTable();
          AddTableRow("whitespace", "[ \\t\\n\\r]+", true);
          AddTableRow("comment",    "#.*", true);
          AddTableRow("integer",    "[0-9]+");
          AddTableRow("float",      "([0-9]+.[0-9]*)|(.[0-9]+)");
          AddTableRow("keyword",    "(break)|(continue)|(else)|(for)|(if)|(return)|(while)");
          AddTableRow("identifier", "[a-zA-Z_][a-zA-Z0-9_]*");
          // AddTableRow("string",     "(\\\"([^\"\\\\]|(\\\\.))*\\\")|('([^'\\\\]|(\\\\.))*')");
          AddTableRow("symbol",     ".|\"::\"|\"==\"|\"!=\"|\"<=\"|\">=\"|\"->\"|\"&&\"|\"||\"|\"<<\"|\">>\"|\"++\"|\"--\"");
          doc.Div("token_div").Redraw();
        }, "Load Example", "example_load_but")
      <<
      "<p>NOTE: loading this example will clear all existing token information. "
      "You can save (or load) your own token types at any time. "
      "The save file uses a simple format with each line starting with a token name "
      "followed by whitespace and then the associated regular expression; "
      "you can prepend a minus sign ('-') to a token name to have that token ignored in output.<p>";
  }
}



int emp_main()
{
  doc << "<h1>Emplex: A C++ Lexer Generator</h1>";

  UpdateIntro("home");
  button_style.Set("padding", "10px 15px")
    .Set("background-color", "#000066") // Dark Blue background
    .Set("color", "white")              // White text
    .Set("border", "1px solid white")   // Thin white border
    .Set("border-radius", "5px")        // Rounded corners
    .Set("cursor", "pointer")
    .Set("font-size", "16px")
    .Set("transition", "background-color 0.3s ease, transform 0.3s ease"); // Smooth transition

  button_style_dark.Set("padding", "10px 15px")
    .Set("background-color", "#B2946C") // Dark Blue background
    .Set("color", "black")              // Black text
    .Set("border", "1px solid black")   // Thin black border
    .Set("border-radius", "5px")        // Rounded corners
    .Set("cursor", "pointer")
    .Set("font-size", "16px")
    .Set("transition", "background-color 0.3s ease, transform 0.3s ease"); // Smooth transition

  button_div << UI::Button([](){ UpdateIntro("home"); intro_div.Redraw(); }, "Home", "home_but").SetCSS(button_style).SetBackground("#0000AA").SetCSS("width", "106px");
  button_div << UI::Button([](){ UpdateIntro("lexer"); intro_div.Redraw(); }, "Lexical Analysis", "lexer_but").SetCSS(button_style); // .SetCSS("width", "170px");
  button_div << UI::Button([](){ UpdateIntro("regex"); intro_div.Redraw(); }, "Regular Expressions", "regex_but").SetCSS(button_style); // .SetCSS("width", "170px");
  button_div << UI::Button([](){ UpdateIntro("cpp"); intro_div.Redraw(); }, "Generated C++ Code", "cpp_but").SetCSS(button_style); // .SetCSS("width", "170px");
  button_div << UI::Button([](){ UpdateIntro("examples"); intro_div.Redraw(); }, "Examples", "example_but").SetCSS(button_style).SetCSS("width", "106px");
  button_div << UI::Button([](){ UpdateIntro("about"); intro_div.Redraw(); }, "About", "about_but").SetCSS(button_style).SetCSS("width", "106px");
  doc << button_div;
  doc << "<small><small><br></small></small>";
  doc << intro_div;
  doc << "<br>\n";

  // token_table.SetCSS("border-collapse", "collapse");
  token_div.SetBackground("lightgrey").SetCSS("border-radius", "10px", "border", "1px solid black", "padding", "15px", "width", "800px");
  token_div << "<big><big><b>Token Types</b></big></big><br><br>\n";

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
    doc.Div("token_div").Redraw();
  }, "Add Row", "row_but").SetCSS(button_style)
  .SetTitle("Add an additional line for defining token types.");

  token_div << UI::Button([](){
    AddTableRow();
    AddTableRow();
    AddTableRow();
    AddTableRow();
    AddTableRow();
    doc.Div("token_div").Redraw();
  }, "+5 Rows", "5row_but").SetCSS(button_style)
  .SetTitle("Add five more lines for defining additional tokens.");

  token_div << UI::Button([](){
    ClearTable();
    AddTableRow();
    AddTableRow();
    AddTableRow();
    doc.Div("token_div").Redraw();
  }, "Reset", "reset_but").SetCSS(button_style)
  .SetTitle("Reset tokens back to the starting setup.");

  token_div << UI::Button([](){ SaveTable(); }, "Save Token Types")
    .SetCSS(button_style)
    .SetTitle("Save token names and regular expressions to a file.");

  token_div << UI::FileInput([](emp::File file){
    file.RemoveIfBegins("#");  // Remove all lines that are comments
    file.RemoveEmpty();
    ClearTable();

    for (emp::String line : file) {
      bool ignore = line.PopIf('-');
      emp::String name = line.PopWord();  // First entry on a line is the token name.
      emp::String regex = line.Trim();    // Regex is remainder, minus start & end whitespace.
      AddTableRow(name, regex, ignore);
    }
    doc.Div("token_div").Redraw();

  }, "load_input").SetCSS("display", "none");

  token_div << UI::Button([](){ doc.FileInput("load_input").DoClick(); }, "Load Token Types", "load_but")
    .SetCSS(button_style)
    .SetTitle("Load previously save token types from file.");

  token_div << "<br>";

  token_div << UI::Button([](){
    Generate();
  }, "Generate C++ Code", "generate_but").SetCSS(button_style).SetBackground("#330066")
  .SetTitle("Generate a lexer using the token types defined above.");

  token_div << UI::Button([](){
    DownloadCode();
  }, "Download C++ Code", "download_but").SetCSS(button_style).SetBackground("#606060").SetDisabled()
  .SetTitle("Generate code to activate this button.");

  token_div << UI::Button([](){
    doc.Div("settings_div").ToggleActive();
  }, "Advanced Options", "settings_but")
    .SetCSS(button_style).SetCSS("float", "right", "border-radius", "15px", "font-size", "12px")
    .SetTitle("Adjust naming details for generated code.");

  doc << token_div;
  doc << "<p>";

  settings_div.SetBackground("tan")
    .SetCSS("border-radius", "10px", "border", "1px solid black", "padding", "15px", "width", "fit-content");
  settings_div << "<big><big><b>Advanced Options</b></big></big><br>\n";

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

  error_div.SetBackground("white").SetColor("red");
  doc << error_div;

  output_div.SetBackground("black").SetColor("white");
  output_div.SetBorder("20px").SetCSS("border-radius", "10px");

  doc << output_div;
  output_div << output_text;
}
