/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University; MIT license; see doc/LICENSE.md
 * @brief Shared grammar-file format and validation for Empala's native and web interfaces.
 */
#pragma once

#include <cctype>
#include <istream>
#include <ostream>
#include <set>

#include "emp/compiler/Lexer.hpp"
#include "emp/compiler/Parser.hpp"
#include "emp/io/CPPFile.hpp"

namespace empala {

// notify::Error always exits after notifying handlers. Throw from our handler to
// unwind to the current operation instead, so an invalid grammar can be corrected.
struct DiagnosticError {};

struct Settings {
  emp::String out_filename = "parser.hpp";
  emp::String lexer_name = "Lexer";
  emp::String token_name = "Token";
  emp::String dfa_name = "DFA";
  emp::String parser_name = "Parser";
  emp::String node_name = "ASTNode";
  emp::String value_type = "double";
  emp::String inc_guards = "EMPALA_PARSER_HPP_INCLUDE_";
  emp::String name_space = "empala";
  bool use_token_lexemes = true;
  bool use_token_line_num = true;
  bool use_token_column = true;
  bool show_functions = true;
  bool show_notes = false;
  bool show_no_ast = true;

  // Return a diagnostic rather than silently discarding an unknown setting.
  emp::String Set(const emp::String & name, const emp::String & value) {
    if (name == "out_filename") out_filename = value;
    else if (name == "lexer_name") lexer_name = value;
    else if (name == "token_name") token_name = value;
    else if (name == "dfa_name") dfa_name = value;
    else if (name == "parser_name") parser_name = value;
    else if (name == "node_name") node_name = value;
    else if (name == "value_type") value_type = value;
    else if (name == "inc_guards") inc_guards = value;
    else if (name == "name_space") name_space = value;
    else {
      bool * flag = nullptr;
      if (name == "use_token_lexemes") flag = &use_token_lexemes;
      else if (name == "use_token_line_num") flag = &use_token_line_num;
      else if (name == "use_token_column") flag = &use_token_column;
      else if (name == "show_functions") flag = &show_functions;
      else if (name == "show_notes") flag = &show_notes;
      else if (name == "show_no_ast") flag = &show_no_ast;
      else return emp::MakeString("Unknown setting '", name, "'.");
      if (value == "1" || value == "true") *flag = true;
      else if (value == "0" || value == "false") *flag = false;
      else return emp::MakeString("Setting '", name, "' requires 0, 1, false, or true.");
    }
    return "";
  }

  void Write(std::ostream & out) const {
    out << ":out_filename " << out_filename << '\n'
        << ":lexer_name " << lexer_name << '\n'
        << ":token_name " << token_name << '\n'
        << ":dfa_name " << dfa_name << '\n'
        << ":parser_name " << parser_name << '\n'
        << ":node_name " << node_name << '\n'
        << ":value_type " << value_type << '\n'
        << ":inc_guards " << inc_guards << '\n'
        << ":name_space " << name_space << '\n'
        << ":use_token_lexemes " << use_token_lexemes << '\n'
        << ":use_token_line_num " << use_token_line_num << '\n'
        << ":use_token_column " << use_token_column << '\n'
        << ":show_functions " << show_functions << '\n'
        << ":show_notes " << show_notes << '\n'
        << ":show_no_ast " << show_no_ast << '\n';
  }

  void WriteCPP(emp::CPPFile & file, const emp::Lexer & lexer, const emp::Parser & parser) const {
    file.Clear();
    file.SetGuards(inc_guards);
    file.SetNamespace(name_space);
    lexer.WriteCPP(file, lexer_name, dfa_name, token_name,
                   use_token_lexemes, use_token_line_num, use_token_column);
    parser.WriteCPP(file, parser_name, node_name, token_name, value_type,
                    use_token_lexemes, use_token_line_num);
  }
};

struct Token {
  emp::String name;
  emp::String regex;
  bool ignore = false;
};

struct Rule {
  emp::String name;
  emp::String pattern;
  emp::String function;
  bool no_ast = false;
  emp::String note;
};

inline bool IsIdentifier(const emp::String & name) {
  return !name.empty() && name.OnlyIDChars() && !emp::is_digit(name[0]);
}

struct Grammar {
  Settings settings;
  emp::vector<Token> tokens;
  emp::vector<Rule> rules;
  emp::vector<emp::String> errors;

  /// Read a complete .empala file. Leading whitespace and CRLF are accepted.
  bool Read(std::istream & in) {
    tokens.clear();
    rules.clear();
    errors.clear();
    std::string text;
    size_t line_number = 0;
    while (std::getline(in, text)) {
      ++line_number;
      emp::String line(text);
      line.Trim();
      if (line.empty() || line[0] == '#') continue;
      if (line.PopIf(':')) {
        auto name = line.PopWord();
        auto error = settings.Set(name, line.Trim());
        if (!error.empty()) errors.push_back(emp::MakeString("Line ", line_number, ": ", error));
      } else if (line.PopIf('>')) {
        Rule rule;
        rule.name = line.PopWord();
        line.Trim();
        // Row metadata precedes the pattern; quoted notes use C++-style escapes.
        while (!line.empty() && (line[0] == '@' || line[0] == '!')) {
          if (line.PopIf('@')) {
            rule.function = line.PopWord();
            if (rule.function.empty()) errors.push_back("Missing function after '@'.");
          } else if (line.HasPrefix("!no_ast") &&
                     (line.size() == 7 || std::isspace(static_cast<unsigned char>(line[7])))) {
            line.erase(0, 7);
            rule.no_ast = true;
          } else if (line.HasPrefix("!note=\"")) {
            line.erase(0, 7);
            bool closed = false;
            while (!line.empty()) {
              char c = line[0];
              line.erase(0, 1);
              if (c == '"') { closed = true; break; }
              if (c == '\\') {
                if (line.empty()) break;
                c = line[0]; line.erase(0, 1);
                switch (c) {
                  case 'n': c = '\n'; break;
                  case 'r': c = '\r'; break;
                  case 't': c = '\t'; break;
                  case '\\': case '"': break;
                  default: errors.push_back("Unknown escape in note."); break;
                }
              }
              rule.note += c;
            }
            if (!closed) errors.push_back("Unterminated quoted note.");
            if (!line.empty() && !std::isspace(static_cast<unsigned char>(line[0]))) {
              errors.push_back("Expected whitespace after note.");
            }
          } else {
            errors.push_back(emp::MakeString("Line ", line_number, ": Unknown rule metadata: ", line.PopWord()));
          }
          line.Trim();
        }
        rule.pattern = line.Trim();
        rules.push_back(rule);
      } else {
        Token token;
        token.ignore = line.PopIf('-');
        token.name = line.PopWord();
        token.regex = line.Trim();
        tokens.push_back(token);
      }
    }
    if (in.bad()) errors.push_back("Unable to read grammar file.");
    return errors.empty();
  }

  /// Check definitions before passing them to APIs with assertion preconditions.
  bool Validate(bool lexer_only = false) {
    const auto & config = settings;
    emp::vector<emp::String> identifiers{config.lexer_name, config.token_name, config.dfa_name};
    if (!lexer_only) { identifiers.push_back(config.parser_name); identifiers.push_back(config.node_name); }
    std::set<emp::String> class_names(identifiers.begin(), identifiers.end());
    if (class_names.size() != identifiers.size()) errors.push_back("Generated class names must be distinct.");
    identifiers.push_back(lexer_only ? "EMPALA_LEXER_HPP_INCLUDE_" : config.inc_guards);
    for (const auto & name : identifiers) {
      if (!IsIdentifier(name)) errors.push_back(emp::MakeString("Invalid C++ identifier '", name, "'."));
    }
    if (!lexer_only && config.out_filename.empty()) errors.push_back("Output filename cannot be empty.");
    if (!lexer_only && config.value_type.empty()) errors.push_back("AST value type cannot be empty.");
    if (!config.name_space.empty()) {
      const std::string ns = config.name_space;
      size_t start = 0;
      while (true) {
        const size_t end = ns.find("::", start);
        if (!IsIdentifier(ns.substr(start, end == std::string::npos ? end : end - start))) {
          errors.push_back(emp::MakeString("Invalid namespace '", ns, "'."));
          break;
        }
        if (end == std::string::npos) break;
        start = end + 2;
      }
    }
    if (tokens.empty() && (lexer_only || !rules.empty())) {
      errors.push_back("Define at least one token (for example, -WHITESPACE [ \\t\\n\\r]+).");
    }
    std::set<emp::String> names;
    for (size_t i = 0; i < tokens.size(); ++i) {
      const auto & token = tokens[i];
      auto error = [&](auto... parts) {
        errors.push_back(emp::MakeString("Token row ", i + 1, ": ", parts...));
      };
      if (!IsIdentifier(token.name)) error("Invalid token name '", token.name, "'.");
      if (!names.insert(token.name).second) error("Duplicate token name '", token.name, "'.");
      if (token.regex.empty()) error("No regex for token '", token.name, "'.");
      else {
        emp::RegEx regex(token.regex);
        for (const auto & note : regex.GetNotes()) error("Invalid regular expression: ", note);
      }
    }
    // Keep token IDs above the ASCII literal range (0..127).
    if (tokens.size() > 128) errors.push_back("Too many token definitions.");
    if (lexer_only) return errors.empty();
    for (size_t i = 0; i < rules.size(); ++i) {
      const auto & rule = rules[i];
      auto error = [&](auto... parts) {
        errors.push_back(emp::MakeString("Rule row ", i + 1, ": ", parts...));
      };
      if (!IsIdentifier(rule.name)) error("Invalid rule name '", rule.name, "'.");
      if (rule.name.size() > 1 && names.contains(rule.name)) {
        error("Rule name '", rule.name, "' is already used as a token name.");
      }
      if (rule.pattern.empty()) error("No pattern for rule '", rule.name, "'.");
      if (rule.no_ast && (!rule.function.empty() || !rule.note.empty())) {
        error("No AST cannot have a function or note.");
      }
      if (!rule.function.empty() && !IsIdentifier(rule.function)) {
        error("Invalid function name '", rule.function, "'.");
      }
    }
    return errors.empty();
  }

  void Write(std::ostream & out, bool lexer_only = false) const {
    out << "# Saved by Empala\n";
    if (!lexer_only) settings.Write(out);
    else {
      out << ":out_filename lexer.hpp\n:inc_guards EMPALA_LEXER_HPP_INCLUDE_\n"
          << ":name_space " << settings.name_space << '\n'
          << ":lexer_name " << settings.lexer_name << '\n'
          << ":token_name " << settings.token_name << '\n'
          << ":dfa_name " << settings.dfa_name << '\n'
          << ":use_token_lexemes " << settings.use_token_lexemes << '\n'
          << ":use_token_line_num " << settings.use_token_line_num << '\n'
          << ":use_token_column " << settings.use_token_column << '\n';
    }
    for (const auto & token : tokens) {
      out << (token.ignore ? "-" : "") << token.name << ' ' << token.regex << '\n';
    }
    if (lexer_only) return;
    for (const auto & rule : rules) {
      out << '>' << rule.name;
      if (rule.no_ast) out << " !no_ast";
      if (!rule.function.empty()) out << " @" << rule.function;
      if (!rule.note.empty()) out << " !note=" << rule.note.AsLiteral();
      out << ' ' << rule.pattern << '\n';
    }
  }

  // Import only lexer-owned settings. Parser filenames, namespace, guards, and
  // all grammar rows remain the responsibility of the receiving grammar.
  void ImportLexer(const Grammar & other) {
    tokens = other.tokens;
    settings.lexer_name = other.settings.lexer_name;
    settings.token_name = other.settings.token_name;
    settings.dfa_name = other.settings.dfa_name;
    settings.use_token_lexemes = other.settings.use_token_lexemes;
    settings.use_token_line_num = other.settings.use_token_line_num;
    settings.use_token_column = other.settings.use_token_column;
  }

  void BuildLexer(emp::Lexer & lexer) const {
    lexer.Reset();
    for (const auto & token : tokens) {
      if (token.ignore) lexer.IgnoreToken(token.name, token.regex);
      else lexer.AddToken(token.name, token.regex);
    }
  }

  void WriteLexerCPP(emp::CPPFile & file, const emp::Lexer & lexer) const {
    file.Clear();
    file.SetGuards("EMPALA_LEXER_HPP_INCLUDE_");
    file.SetNamespace(settings.name_space);
    lexer.WriteCPP(file, settings.lexer_name, settings.dfa_name, settings.token_name,
                   settings.use_token_lexemes, settings.use_token_line_num, settings.use_token_column);
  }

  // emp::notify handlers supplied by the caller collect parser diagnostics.
  bool Build(emp::Lexer & lexer, emp::Parser & parser) const try {
    parser.Reset();
    BuildLexer(lexer);
    for (const auto & rule : rules) {
      parser.AddRule(rule.name, rule.pattern, nullptr, rule.function, rule.no_ast, rule.note);
    }
    return parser.Validate();
  } catch (const DiagnosticError &) {
    return false;
  }
};

}  // namespace empala
