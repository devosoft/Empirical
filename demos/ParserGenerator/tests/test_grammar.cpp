// Exercise the shared save/load path used by the web editor, including lexer
// imports that must remain independent of unfinished grammar definitions.
#include <cassert>
#include <sstream>
#include "../source/Grammar.hpp"

int main() {
  empala::Grammar grammar;
  grammar.tokens = {{"SPACE", "[ ]+", true}, {"NUMBER", "[0-9]+", false}};
  grammar.rules = {{"start", "value", "", true, ""},
                   {"value", "NUMBER", "Number", false, "quotes\" slash\\ tab\t line\n"}};
  grammar.settings.show_notes = true;
  grammar.settings.show_functions = false;
  std::stringstream saved;
  grammar.Write(saved);
  empala::Grammar loaded;
  assert(loaded.Read(saved));
  assert(loaded.Validate());
  assert(loaded.rules.size() == 2 && loaded.rules[0].no_ast);
  assert(loaded.rules[1].note == grammar.rules[1].note);
  assert(loaded.rules[1].function == "Number");
  assert(loaded.settings.show_notes && !loaded.settings.show_functions);
  emp::Lexer lexer;
  emp::Parser parser(lexer);
  assert(loaded.Build(lexer, parser));
  auto ast = parser.Parse("42");
  assert(ast.GetRoot().note == grammar.rules[1].note);

  // Emplex settings are saved without any parser-only extensions.
  std::stringstream emplex;
  grammar.Write(emplex, true);
  const std::string text = emplex.str();
  assert(text.find('>') == std::string::npos);
  assert(text.find(":show_notes") == std::string::npos);
  assert(text.find(":parser_name") == std::string::npos);
  empala::Grammar imported;
  assert(imported.Read(emplex));
  imported.settings.lexer_name = "Scanner";
  imported.settings.token_name = "Lexeme";
  imported.settings.dfa_name = "Machine";
  imported.settings.use_token_lexemes = false;
  imported.settings.name_space = "different";
  imported.settings.out_filename = "different.hpp";
  imported.settings.inc_guards = "DIFFERENT";
  const auto settings = grammar.settings;
  grammar.ImportLexer(imported);
  assert(grammar.settings.out_filename == settings.out_filename);
  assert(grammar.settings.name_space == settings.name_space);
  assert(grammar.settings.inc_guards == settings.inc_guards);
  assert(grammar.settings.parser_name == settings.parser_name);
  assert(grammar.settings.lexer_name == "Scanner");
  assert(grammar.settings.token_name == "Lexeme" && grammar.settings.dfa_name == "Machine");
  assert(!grammar.settings.use_token_lexemes);
  assert(grammar.rules.size() == 2 && grammar.rules[1].note == loaded.rules[1].note);
  grammar.rules[0].pattern = "undefined";
  grammar.settings.parser_name = "bad-name";
  assert(grammar.Validate(true));
  grammar.BuildLexer(lexer);
  assert(lexer.Tokenize("42").size() == 1);
  emp::CPPFile file;
  grammar.WriteLexerCPP(file, lexer);
  std::ostringstream cpp;
  file.Write(cpp);
  assert(cpp.str().find("class Scanner") != std::string::npos);
  assert(cpp.str().find("ASTNode") == std::string::npos);
  assert(!grammar.Validate());
}
