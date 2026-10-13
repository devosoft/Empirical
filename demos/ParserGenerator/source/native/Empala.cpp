/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University; MIT license; see doc/LICENSE.md
 * @brief Empala: command-line generator of standalone C++ lexers and parsers.
 */
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include "../Grammar.hpp"

namespace {
void PrintHelp(std::ostream & out) {
  out << "Usage: Empala [options] grammar.empala\n"
      "Generate a standalone C++ lexer and parser from an Empala grammar.\n\n"
      "  -f, -o, --output FILE     Output header (default: parser.hpp; '-' for stdout)\n"
      "  -c, --parser NAME         Parser class (default: Parser)\n"
      "  -a, --node NAME           AST node class, nested in parser (default: ASTNode)\n"
      "  -l, --lexer NAME          Lexer class (default: Lexer)\n"
      "  -d, --dfa NAME            DFA class (default: DFA)\n"
      "  -t, --token NAME          Token class (default: Token)\n"
      "  -n, --namespace NAME      Namespace (default: empala; empty for global)\n"
      "  -g, --guards NAME         Include guard (default: EMPALA_PARSER_HPP_INCLUDE_)\n"
      "  -V, --value-type TYPE     AST result type (default: double)\n"
      "      --set KEY VALUE      Override any :setting, including token storage flags\n"
      "  -v, --verbose            Print progress to stderr\n"
      "  -h, --help               Show this help\n\n"
      "CLI settings override settings in the grammar file. Use '--' before a filename\n"
      "beginning with '-'. Token lines are NAME regex; prefix NAME with '-' to ignore.\n"
      "Rule lines are >name [@handler_method] [!no_ast] [!note=\"text\"] pattern.\n"
      "The first rule is the start rule. Prefix an element with '-' to omit its AST.\n"
      "!no_ast passes through zero or one child and cannot have a handler or note.\n"
      "Lines beginning with '#' are comments; :key value lines configure output.\n";
}
}  // namespace

int main(int argc, char * argv[]) {
  empala::Grammar grammar;
  emp::String input_filename;
  std::map<emp::String, emp::String> overrides;
  bool verbose = false;
  bool positional = false;
  const std::map<emp::String, emp::String> options{
    {"-f", "out_filename"}, {"-o", "out_filename"}, {"--output", "out_filename"},
    {"-c", "parser_name"}, {"--parser", "parser_name"},
    {"-a", "node_name"}, {"--node", "node_name"},
    {"-l", "lexer_name"}, {"--lexer", "lexer_name"},
    {"-d", "dfa_name"}, {"--dfa", "dfa_name"},
    {"-t", "token_name"}, {"--token", "token_name"},
    {"-n", "name_space"}, {"--namespace", "name_space"},
    {"-g", "inc_guards"}, {"--guards", "inc_guards"},
    {"-V", "value_type"}, {"--value-type", "value_type"}
  };
  auto fail = [](const auto & message) {
    std::cerr << "Empala: " << message << '\n';
    return 1;
  };
  for (int i = 1; i < argc; ++i) {
    const emp::String arg(argv[i]);
    if (!positional && (arg == "-h" || arg == "--help")) { PrintHelp(std::cout); return 0; }
    if (!positional && arg == "--") { positional = true; continue; }
    if (!positional && (arg == "-v" || arg == "--verbose")) { verbose = true; continue; }
    if (!positional && arg == "--set") {
      if (i + 2 >= argc) return fail("--set requires a setting name and value.");
      const emp::String key(argv[++i]);
      overrides[key] = argv[++i];
    } else if (!positional && options.contains(arg)) {
      if (i + 1 >= argc) return fail(emp::MakeString(arg, " requires a value."));
      overrides[options.at(arg)] = argv[++i];
    } else if (!positional && !arg.empty() && arg[0] == '-') {
      return fail(emp::MakeString("Unknown option '", arg, "'. Use --help for usage."));
    } else {
      if (!input_filename.empty()) return fail("Expected exactly one grammar file.");
      input_filename = arg;
    }
  }
  if (input_filename.empty()) { PrintHelp(std::cerr); return 1; }
  std::ifstream input(input_filename);
  if (!input) return fail(emp::MakeString("Unable to open grammar file '", input_filename, "'."));
  grammar.Read(input);
  for (const auto & [key, value] : overrides) {
    const auto error = grammar.settings.Set(key, value);
    if (!error.empty()) grammar.errors.push_back(error);
  }
  grammar.Validate();
  for (const auto & error : grammar.errors) fail(error);
  if (!grammar.errors.empty()) return 1;

  bool had_error = false;
  emp::notify::AddHandler("ERROR", [&](emp::notify::id_arg_t, emp::notify::message_arg_t message) -> bool {
    had_error = true;
    fail(message);
    throw empala::DiagnosticError{};
  });
  emp::notify::AddHandler("WARNING", [](emp::notify::id_arg_t, emp::notify::message_arg_t message) {
    std::cerr << "Empala: warning: " << message << '\n';
    return true;
  });
  emp::Lexer lexer;
  emp::Parser parser(lexer);
  if (!grammar.Build(lexer, parser) || had_error) return 1;

  emp::CPPFile file;
  grammar.settings.WriteCPP(file, lexer, parser);
  if (had_error) return 1;
  std::ostringstream code;
  code << "// Generated by Empala. MIT license; see https://opensource.org/licenses/MIT.\n";
  file.Write(code);

  const auto & output_filename = grammar.settings.out_filename;
  if (output_filename == "-") {
    std::cout << code.str();
    std::cout.flush();
    if (!std::cout) return fail("Unable to write generated code to stdout.");
  } else {
    // Do not accidentally truncate the grammar itself, including through a symlink.
    std::error_code ec;
    if (std::filesystem::equivalent(std::filesystem::path(input_filename.c_str()),
                                    std::filesystem::path(output_filename.c_str()), ec)) {
      return fail("Output file must differ from the input grammar file.");
    }
    std::ofstream output(output_filename);
    if (!output) return fail(emp::MakeString("Unable to open output file '", output_filename, "'."));
    output << code.str();
    output.close();
    if (!output) return fail(emp::MakeString("Unable to write output file '", output_filename, "'."));
  }
  if (verbose) {
    std::cerr << "Empala: generated " << output_filename << " from " << grammar.tokens.size()
              << " tokens and " << grammar.rules.size() << " rule rows.\n";
  }
  return 0;
}
