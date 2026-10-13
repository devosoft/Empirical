/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file tests/compiler/lexer_codegen_fixture.cpp
 * @brief Emit matching C++, Python, and JavaScript lexers for test_codegen.py.
 */

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "emp/compiler/Lexer.hpp"

struct Scenario {
  emp::Lexer lexer;
  std::vector<emp::String> inputs;
};

int main(int argc, char ** argv) {
  if (argc != 2) { return 1; }
  const std::filesystem::path directory(argv[1]);
  std::vector<Scenario> scenarios(6);
  auto & basic = scenarios[0];
  basic.lexer.AddToken("IF", "if");
  basic.lexer.AddToken("WORD", "[a-zA-Z_][a-zA-Z0-9_]*");
  basic.lexer.AddToken("NUMBER", "[0-9]+");
  basic.lexer.IgnoreToken("SPACE", "[ \\t\\r\\n]+");
  basic.lexer.IgnoreToken("COMMENT", "#[^\\n]*");
  basic.inputs = {"", "if iff 123 ?\nif\t# ignored\nword", "  # all ignored\n", "abc123 99!"};

  auto & trailing = scenarios[1];
  trailing.lexer.AddToken("DECLARE", "[a-zA-Z_][a-zA-Z0-9_]*/[ ]*:");
  trailing.lexer.AddToken("WORD", "[a-zA-Z_][a-zA-Z0-9_]*");
  trailing.lexer.IgnoreToken("SPACE", "[ \\t\\n]+");
  trailing.lexer.AddToken("COLON", ":");
  trailing.inputs = {"a : b", "ab:abc ab x\n", "a   :b: c?", "a "};

  auto & anchored = scenarios[2];
  anchored.lexer.AddToken("START", "^START");
  anchored.lexer.AddToken("END", "END$");
  anchored.lexer.AddToken("WORD", "[A-Z]+");
  anchored.lexer.IgnoreToken("SPACE", "[ \\t\\n]+");
  anchored.inputs = {"START MID END\nSTART END", " START END!\nSTART END\n", "END", "STARTX"};

  auto & longest = scenarios[3];
  longest.lexer.AddToken("AB", "ab");
  longest.lexer.AddToken("ABC", "abc");
  longest.lexer.AddToken("ALSO_AB", "ab");
  longest.lexer.IgnoreToken("SPACE", "[ \\t\\n]+");
  longest.inputs = {"ab abc abcd !", "aba\nab", "abc"};

  scenarios[4].inputs = {"", "abc ?\n"};  // Empty DFA and fallback-only tokenization.
  auto & nullable     = scenarios[5];
  nullable.lexer.AddToken("AS", "a*");
  nullable.lexer.AddToken("BS", "b+");
  nullable.lexer.IgnoreToken("SPACE", "[ \\t\\n]+");
  nullable.inputs = {"", "bbb aa ?\n", "aaa", "?b?"};

  std::ofstream manifest(directory / "manifest.json");
  std::ofstream runner(directory / "reference.cpp");
  runner << "#include <iostream>\n#include <string>\n#include <vector>\n";
  manifest << "[\n";
  for (size_t s = 0; s < scenarios.size(); ++s) {
    for (int flags = 0; flags < 8; ++flags) {
      const emp::String name = emp::MakeString("case", s, "_", flags);
      const bool lexeme = flags & 1, line = flags & 2, column = flags & 4;
      emp::CPPFile cpp((directory / (name.str() + ".hpp")).string());
      cpp.SetGuards(name + "_HPP").SetNamespace(name);
      scenarios[s].lexer.WriteCPP(cpp,
                                  "CustomLexer",
                                  "CustomDFA",
                                  "CustomToken",
                                  lexeme,
                                  line,
                                  column);
      cpp.Write();
      emp::PythonFile python((directory / (name.str() + ".py")).string());
      // Generators must not depend on the destination file's substitution settings.
      python.SetVarSymbol("!").AddVarSetting("unrelated", 42);
      scenarios[s].lexer.WritePython(python,
                                     "CustomLexer",
                                     "CustomDFA",
                                     "CustomToken",
                                     lexeme,
                                     line,
                                     column);
      python.Write();
      emp::JSFile js((directory / (name.str() + ".mjs")).string());
      js.SetVarSymbol("");
      scenarios[s].lexer.WriteJS(js,
                                 "CustomLexer",
                                 "CustomDFA",
                                 "CustomToken",
                                 lexeme,
                                 line,
                                 column);
      js.Write();
      runner << "#include \"" << name << ".hpp\"\n";
      if (s || flags) { manifest << ",\n"; }
      manifest << "{\"name\":" << emp::internal::QuoteCodeString(name) << ",\"flags\":" << flags
               << ",\"inputs\":[";
      for (size_t i = 0; i < scenarios[s].inputs.size(); ++i) {
        if (i) { manifest << ','; }
        manifest << emp::internal::QuoteCodeString(scenarios[s].inputs[i]);
      }
      manifest << "]}";
    }
  }
  manifest << "\n]\n";
  runner << R"REFERENCE(template <typename TOKEN>
    void PrintToken(const TOKEN & token) {
      std::cout << '[' << token.id << ',';
      if constexpr (requires { token.lexeme; }) {
        std::cout << '[';
        bool first = true;
        for (unsigned char c : token.lexeme) {
          if (!first) { std::cout << ','; }
          first = false;
          std::cout << static_cast<int>(c);
        }
        std::cout << ']';
      } else {
        std::cout << "null";
      }
      std::cout << ',';
      if constexpr (requires { token.line_id; }) {
        std::cout << token.line_id;
      } else {
        std::cout << "null";
      }
      std::cout << ',';
      if constexpr (requires { token.col_id; }) {
        std::cout << token.col_id;
      } else {
        std::cout << "null";
      }
      std::cout << ']';
    }

    template <typename LEXER, typename DFA>
    void Run(const std::vector<std::string> & inputs) {
      LEXER lexer;
      std::cout << '[';
      bool first_input = true;
      for (const auto & input : inputs) {
        if (!first_input) { std::cout << ','; }
        first_input         = false;
        const auto & tokens = lexer.Tokenize(input);
        std::cout << "{\"tokens\":[";
        bool first = true;
        for (const auto & token : tokens) {
          if (!first) { std::cout << ','; }
          first = false;
          PrintToken(token);
        }
        while (lexer.Any()) { lexer.Use(); }
        std::cout << "],\"eof\":";
        PrintToken(lexer.Peek());
        std::cout << ",\"dfa\":" << DFA::Test(input) << '}';
      }
      std::cout << "]\n";
    }
    int main() {
  )REFERENCE";
  for (size_t s = 0; s < scenarios.size(); ++s) {
    for (int flags = 0; flags < 8; ++flags) {
      const emp::String name = emp::MakeString("case", s, "_", flags);
      runner << "Run<" << name << "::CustomLexer, " << name << "::CustomDFA>({";
      for (size_t i = 0; i < scenarios[s].inputs.size(); ++i) {
        if (i) { runner << ','; }
        runner << scenarios[s].inputs[i].AsLiteral();
      }
      runner << "});\n";
    }
  }
  runner << "}\n";
}
