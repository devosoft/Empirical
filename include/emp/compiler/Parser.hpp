/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/compiler/Parser.hpp
 * @brief A general-purpose parser that builds abstract syntax trees from lexer tokens.
 * @note Status: ALPHA
 *
 * Build a parser that converts a series of tokens (from an emp::Lexer) into an abstract
 * syntax tree, using a grammar specified as a set of named parse rules.
 *
 * Use AddRule(name, pattern) to provide each rule.
 *  'name' is the rule being defined (the first rule added is the default start rule).
 *  'pattern' is a whitespace-separated sequence of symbols to match, where each symbol is:
 *     TOKEN_NAME - the name of a token type registered in the associated Lexer
 *     rule_name  - the name of another parse rule (which may be defined later)
 *     'c'        - a literal character token (with normal escapes: '\n', '\t', '\\', '\'')
 *     ( ... )    - a group of symbols, which may contain alternatives separated by '|'
 *  Any symbol or group may be followed (with no intervening space) by a modifier:
 *     ?  - match zero or one time      *  - match zero or more times (greedy)
 *     +  - match one or more times (greedy)
 *  Prefix an element with - to match it but omit its subtree from the AST.
 *  AddRule can mark an alternative no_ast to pass through zero or one retained child.
 *  A no_ast alternative cannot have an action, generated function, or note.
 *  An optional named token may supply default text: INTEGER?:1 or WORD?:"hello".
 *  When absent, it inserts a leaf with that token ID and lexeme without consuming input.
 *  Default text must lex as exactly one visible token of the specified type. Bare text
 *  ends at whitespace or ()|?*+: or a quote; double quotes allow those characters and
 *  the escapes \n, \r, \t, \\, and \". Defaults are not allowed on rules or groups.
 *  Synthetic tokens have line (and generated column, if stored) zero: no source location.
 *  A pattern may also contain top-level alternatives separated by '|'.  Calling AddRule()
 *  multiple times with the same rule name appends additional alternatives to that rule.
 *
 * Alternatives are tested IN THE ORDER PROVIDED, with full backtracking (i.e., PEG-style
 * ordered choice); place longer alternatives before their prefixes.  Grammars must not be
 * left recursive; left recursion (along with undefined rules and repetition of patterns
 * that can match an empty sequence) is detected and reported when the grammar is validated.
 *
 * Two additional arguments to AddRule() associate behavior with a rule's pattern(s):
 *  - an ACTION (std::function) to call when the resulting AST node is Run() dynamically;
 *    the action receives the ASTNode and may call RunChild() to evaluate sub-nodes.
 *  - a FUNCTION NAME (string) to call in generated C++ code; when the parser is output
 *    with WriteCPP(), the generated ASTNode::Run(handler) will call handler.fun_name(node)
 *    for nodes matched by that pattern.
 * Patterns with neither simply Run() each child rule-node in order (returning the last
 * result), making pass-through rules free to write.
 *
 * Parse(...) converts a TokenStream (or raw input, tokenized via the associated Lexer)
 * into an emp::AST, which owns the tree and can be Run().
 *
 * WriteCPP(...) outputs a standalone recursive-descent parser as C++ source code; it is
 * designed to be written into the same emp::CPPFile as the associated Lexer's WriteCPP()
 * so that the generated parser can consume the generated lexer's tokens directly.
 *
 * In an AST, nodes from anonymous groups "(...)" are spliced into their parent, so the
 * final tree contains only named-rule nodes and token leaves.
 */

#pragma once

#ifndef INCLUDE_EMP_COMPILER_PARSER_HPP_GUARD
#define INCLUDE_EMP_COMPILER_PARSER_HPP_GUARD

#include <algorithm>
#include <cctype>
#include <functional>
#include <map>
#include <set>
#include <string_view>
#include <stddef.h>

#include "../base/notify.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../io/CPPFile.hpp"
#include "../tools/String.hpp"

#include "_ASTNode.hpp"
#include "_Token.hpp"
#include "_TokenStream.hpp"
#include "Lexer.hpp"

namespace emp {

  /// A parser that uses a provided lexer and set of parse rules to build abstract syntax
  /// trees.  VALUE_T is the type produced when an AST node is Run(); LEXER_T is the type
  /// of the associated lexer.
  template <typename VALUE_T, typename LEXER_T = Lexer>
  class Parser_Base {
  public:
    using value_t  = VALUE_T;
    using lexer_t  = LEXER_T;
    using node_t   = ASTNode<VALUE_T>;
    using ast_t    = AST<VALUE_T>;
    using action_t = typename node_t::action_t;

    static constexpr size_t npos = static_cast<size_t>(-1);

  private:
    /// How many times can a pattern symbol be matched?
    enum class Mod : uint8_t {
      ONCE,  ///< Exactly one time (default)
      OPT,   ///< Zero or one time ('?')
      ANY,   ///< Zero or more times ('*')
      PLUS   ///< One or more times ('+')
    };

    /// A single symbol in a parse pattern: either a lexer token or a reference to a rule.
    struct PatternSymbol {
      bool is_terminal = false;  ///< Does this symbol match a token (vs. another rule)?
      int token_id     = -1;     ///< Token ID to match (terminals only)
      size_t rule_id   = npos;   ///< Rule ID to match (non-terminals only)
      Mod mod          = Mod::ONCE;
      bool discard = false;  ///< Match this element but omit its entire subtree.
      bool has_default = false;  ///< Insert a leaf if this optional named token is absent.
      String default_lexeme{};   ///< Default token text, validated against the associated lexer.
    };

    /// One alternative for how a parse rule can be matched.
    struct Pattern {
      emp::vector<PatternSymbol> symbols{};  ///< Sequence of symbols to match, in order.
      action_t action{};  ///< Function to call when a resulting AST node is Run().
      String fun_name{};  ///< Function to call in generated C++ code (see WriteCPP).
      bool no_ast = false;  ///< Pass zero or one retained child directly to the parent.
      String note{};        ///< Annotation for this alternative, stored on its AST node.
    };

    /// A named parse rule along with the ordered set of patterns that can match it.
    struct ParseRule {
      String name{};
      bool anonymous = false;  ///< Was this rule auto-generated for a "(...)" group?
      emp::vector<Pattern> patterns{};
    };

    emp::Ptr<const LEXER_T> lexer;         ///< Lexer providing token names and streams.
    emp::vector<ParseRule> rules{};        ///< All parse rules, indexed by rule ID.
    std::map<String, size_t> rule_map{};   ///< Map of rule names to rule IDs.
    size_t start_rule = npos;              ///< Which rule must match the full input?
    mutable bool validated = false;        ///< Has the current grammar passed validation?
    mutable bool last_parse_succeeded = false;
    mutable String last_error{};           ///< Most recent parse error message.

    // -- Internal parse state (passed through the recursive descent) --
    struct ParseState {
      TokenStream::Iterator it;   ///< Current position in the token stream.
      size_t furthest = 0;        ///< Deepest token index reached (for error reporting).
      std::set<int> expected{};   ///< Token types that would have allowed further progress.
    };

    // -- Helper functions (definitions below) --
    size_t RequireRuleID(const String & name);
    size_t MakeAnonymousRule(emp::vector<Pattern> && alts);
    static void SkipWhitespace(const String & pat, size_t & pos);
    bool ParseCharLiteral(const String & pat, size_t & pos, int & out_id) const;
    bool ParseDefaultLiteral(const String & pat, size_t & pos, String & out) const;
    bool ParsePatternSeq(const String & pat, size_t & pos, Pattern & out);
    bool ParsePatternAlts(const String & pat, size_t & pos, emp::vector<Pattern> & out);

    bool TrySymbolOnce(const PatternSymbol & sym, node_t & parent, ParseState & state) const;
    bool TrySequence(const emp::vector<PatternSymbol> & symbols,
                     node_t & parent, ParseState & state) const;
    emp::Ptr<node_t> TryRule(size_t rule_id, ParseState & state) const;
    String MakeParseError(const TokenStream & tokens, const ParseState & state) const;

    void WriteCPP_RunDispatch(emp::CPPFile & file) const;
    void WriteCPP_MatchSymbol(emp::CPPFile & file, const PatternSymbol & sym) const;
    void WriteCPP_ParseRule(emp::CPPFile & file, size_t rule_id) const;

  public:
    Parser_Base(const LEXER_T & in_lexer) : lexer(&in_lexer) {}

    Parser_Base(const Parser_Base &) = default;
    Parser_Base(Parser_Base &&)      = default;
    Parser_Base & operator=(const Parser_Base &) = default;
    Parser_Base & operator=(Parser_Base &&)      = default;

    /// How many parse rules does this parser have (including anonymous group rules)?
    [[nodiscard]] size_t GetNumRules() const { return rules.size(); }

    /// Reset the entire parser to its start state (keeping the associated lexer).
    void Reset() {
      rules.resize(0);
      rule_map.clear();
      start_rule = npos;
      validated  = false;
      last_parse_succeeded = false;
      last_error.clear();
    }

    /// Add a new parse rule (or a new set of alternatives for an existing rule).
    /// @param name     Name of the rule being defined.
    /// @param pattern  Symbol sequence to match (see file comments for full syntax).
    /// @param action   Function to call when a matched AST node is Run() [optional]
    /// @param fun_name Function for generated C++ code to call on Run [optional]
    /// @param no_ast   Omit this alternative's node and promote its zero or one child.
    /// @param note     Annotation for this alternative (requires a retained node).
    /// @return The unique ID associated with this rule.
    size_t AddRule(String name, String pattern, action_t action = nullptr,
                   String fun_name = "", bool no_ast = false, String note = "");

    /// Shortcut to add a rule with only a generated-code function name (no dynamic action).
    size_t AddRule(String name, String pattern, String fun_name) {
      return AddRule(name, pattern, nullptr, fun_name);
    }

    /// Specify which rule must match a full input (default: the first rule added).
    void SetStartRule(String name) {
      start_rule = RequireRuleID(name);
      validated  = false;
      last_parse_succeeded = false;
      last_error.clear();
    }

    /// Get the ID associated with a rule name (or npos if that rule does not exist).
    [[nodiscard]] size_t GetRuleID(const String & name) const {
      auto it = rule_map.find(name);
      return (it == rule_map.end()) ? npos : it->second;
    }

    /// Get the name associated with a rule ID.
    [[nodiscard]] const String & GetRuleName(size_t id) const {
      emp_assert(id < rules.size(), id, rules.size());
      return rules[id].name;
    }

    /// Get the most recent parse error message (empty if the last parse succeeded).
    [[nodiscard]] const String & GetError() const { return last_error; }

    [[nodiscard]] bool LastParseSucceeded() const { return last_parse_succeeded; }
    [[nodiscard]] bool HasNotes() const {
      for (const auto & rule : rules) for (const auto & pattern : rule.patterns) {
        if (!pattern.note.empty()) return true;
      }
      return false;
    }

    /// Check the current grammar for problems: undefined rules, left recursion, and
    /// repetition ('*' or '+') of symbols that can match an empty sequence.  Also warn
    /// about character literals that the lexer would consume as part of a larger token.
    /// Called automatically by Parse() and WriteCPP(); returns success.
    bool Validate() const;

    /// Parse a stream of tokens, producing an abstract syntax tree.  If require_all is
    /// set (the default), the start rule must consume the entire token stream.
    /// On failure, an invalid AST is returned and an error is provided via notify.
    ast_t Parse(const TokenStream & tokens, bool require_all = true) const;

    /// Tokenize an input string with the associated lexer, then parse it.
    /// (Takes a String rather than a string_view to avoid ambiguity with the TokenStream
    ///  overload, since TokenStream is implicitly constructible from a String name.)
    ast_t Parse(const String & in, String name = "in_view", bool require_all = true) const {
      return Parse(lexer->Tokenize(in, name), require_all);
    }

    // -- Debugging tools --

    /// Build a human-readable version of one pattern symbol (expanding anonymous groups).
    [[nodiscard]] String MakeSymbolString(const PatternSymbol & sym) const {
      String out;
      if (sym.is_terminal) {
        out = lexer->GetTokenName(sym.token_id);
      } else {
        const ParseRule & rule = rules[sym.rule_id];
        if (rule.anonymous) {
          out = "(";
          for (size_t i = 0; i < rule.patterns.size(); ++i) {
            if (i) { out += " | "; }
            out += MakePatternString(rule.patterns[i]);
          }
          out += ")";
        } else {
          out = rule.name;
        }
      }
      switch (sym.mod) {
        case Mod::ONCE: break;
        case Mod::OPT:  out += '?'; break;
        case Mod::ANY:  out += '*'; break;
        case Mod::PLUS: out += '+'; break;
      }
      if (sym.discard) { out = "-" + out; }
      if (sym.has_default) { out += ':'; out += sym.default_lexeme.AsLiteral(); }
      return out;
    }

    /// Build a human-readable version of a full pattern.
    [[nodiscard]] String MakePatternString(const Pattern & pattern) const {
      if (pattern.symbols.size() == 0) { return "<empty>"; }
      String out;
      for (size_t i = 0; i < pattern.symbols.size(); ++i) {
        if (i) { out += ' '; }
        out += MakeSymbolString(pattern.symbols[i]);
      }
      return out;
    }

    /// Print the current grammar (for debugging).
    void Print(std::ostream & os = std::cout) const {
      for (size_t id = 0; id < rules.size(); ++id) {
        const ParseRule & rule = rules[id];
        if (rule.anonymous) { continue; }  // Groups are printed inline.
        os << rule.name << " ::= ";
        for (size_t i = 0; i < rule.patterns.size(); ++i) {
          if (i) { os << " | "; }
          os << MakePatternString(rule.patterns[i]);
        }
        if (id == start_rule) { os << "   (start rule)"; }
        os << "\n";
      }
    }

    /// Print an AST using this parser's rule and token names (for debugging).
    void PrintAST(const node_t & node, std::ostream & os = std::cout) const {
      node.Print(os, [this](const node_t & n) -> String {
        if (n.IsLeaf()) { return lexer->GetTokenName(n.GetToken().id); }
        return rules[n.GetRuleID()].name;
      });
    }
    void PrintAST(const ast_t & ast, std::ostream & os = std::cout) const {
      if (!ast.IsValid()) { os << "<invalid AST>\n"; return; }
      if (!ast.HasRoot()) { os << "<successful parse; no AST>\n"; return; }
      PrintAST(ast.GetRoot(), os);
    }

    /// Get a C++-safe identifier for a rule (anonymous groups get generated names).
    [[nodiscard]] String CppRuleName(size_t id) const {
      emp_assert(id < rules.size(), id, rules.size());
      if (rules[id].anonymous) { return MakeString("_grp_", id); }
      return rules[id].name;
    }

    /// Generate a C++ parser file.  Should be written into the same CPPFile as the
    /// associated Lexer's WriteCPP() so that token types (and IDs) match up.
    /// @param file The C++ file to write to.
    /// @param object_name Class name to use for the parser itself. (Default: "Parser")
    /// @param node_name Class name to use for AST nodes. (Default: "ASTNode")
    /// @param token_name Name of the token class produced by the lexer. (Default: "Token")
    /// @param value_type C++ type produced when AST nodes are Run. (Default: "double")
    /// @param token_has_lexeme Does the token type store lexemes? (Default: true)
    /// @param token_has_line Does the token type store line numbers? (Default: true)
    /// @param use_emp Should we use Empirical classes? (Default: false)
    void WriteCPP(emp::CPPFile & file,
                  emp::String object_name = "Parser",
                  emp::String node_name   = "ASTNode",
                  emp::String token_name  = "Token",
                  emp::String value_type  = "double",
                  bool token_has_lexeme   = true,
                  bool token_has_line     = true,
                  bool use_emp            = false) const;
  };

  /// Parser is the most standard Parser_Base setup.
  using Parser = Parser_Base<double>;

  /// --------------------
  /// FUNCTION DEFINITIONS
  /// --------------------

  // -- Grammar construction --

  template <typename VALUE_T, typename LEXER_T>
  size_t Parser_Base<VALUE_T, LEXER_T>::RequireRuleID(const String & name) {
    auto it = rule_map.find(name);
    if (it != rule_map.end()) { return it->second; }
    const size_t id = rules.size();
    rules.emplace_back();
    rules[id].name = name;
    rule_map[name] = id;
    return id;
  }

  template <typename VALUE_T, typename LEXER_T>
  size_t Parser_Base<VALUE_T, LEXER_T>::MakeAnonymousRule(emp::vector<Pattern> && alts) {
    const size_t id = rules.size();
    rules.emplace_back();
    rules[id].name      = MakeString("@", id);  // '@' cannot appear in user identifiers.
    rules[id].anonymous = true;
    rules[id].patterns  = std::move(alts);
    rule_map[rules[id].name] = id;
    return id;
  }

  template <typename VALUE_T, typename LEXER_T>
  void Parser_Base<VALUE_T, LEXER_T>::SkipWhitespace(const String & pat, size_t & pos) {
    while (pos < pat.size() && std::isspace(static_cast<unsigned char>(pat[pos]))) { ++pos; }
  }

  template <typename VALUE_T, typename LEXER_T>
  bool Parser_Base<VALUE_T, LEXER_T>::ParseCharLiteral(const String & pat, size_t & pos,
                                                       int & out_id) const {
    emp_assert(pos < pat.size() && pat[pos] == '\'');
    ++pos;  // Skip the opening quote.
    if (pos >= pat.size()) {
      notify::Error("Unterminated character literal in parse pattern: ", pat);
      return false;
    }
    char c = pat[pos++];
    if (c == '\\') {  // Handle escape characters.
      if (pos >= pat.size()) {
        notify::Error("Unterminated escape sequence in parse pattern: ", pat);
        return false;
      }
      const char esc = pat[pos++];
      switch (esc) {
        case 'n': c = '\n'; break;
        case 'r': c = '\r'; break;
        case 't': c = '\t'; break;
        case '0': c = '\0'; break;
        case '\\': case '\'': case '"': c = esc; break;
        default:
          notify::Error("Unknown escape sequence '\\", esc, "' in parse pattern: ", pat);
          return false;
      }
    }
    if (pos >= pat.size() || pat[pos] != '\'') {
      notify::Error("Unterminated character literal in parse pattern: ", pat);
      return false;
    }
    ++pos;  // Skip the closing quote.
    out_id = static_cast<int>(static_cast<unsigned char>(c));
    return true;
  }

  /// Read a default lexeme immediately after ':'. Quotes delimit text, not a regex.
  template <typename VALUE_T, typename LEXER_T>
  bool Parser_Base<VALUE_T, LEXER_T>::ParseDefaultLiteral(
    const String & pat, size_t & pos, String & out
  ) const {
    if (pos < pat.size() && pat[pos] == '"') {
      ++pos;
      bool closed = false;
      while (pos < pat.size()) {
        char c = pat[pos++];
        if (c == '"') { closed = true; break; }
        if (c == '\\') {
          if (pos == pat.size()) { break; }
          c = pat[pos++];
          switch (c) {
            case 'n': c = '\n'; break;
            case 'r': c = '\r'; break;
            case 't': c = '\t'; break;
            case '\\': case '"': break;
            default:
              notify::Error("Unknown escape in default token text: ", pat);
              return false;
          }
        }
        out += c;
      }
      if (!closed) {
        notify::Error("Unterminated default token text: ", pat);
        return false;
      }
    } else {
      const size_t start = pos;
      while (pos < pat.size() && !std::isspace(static_cast<unsigned char>(pat[pos])) &&
             std::string_view("()|?*+:\"'").find(pat[pos]) == std::string_view::npos) { ++pos; }
      out = pat.substr(start, pos - start);
    }
    if (out.empty()) {
      notify::Error("Missing or empty default token text after '?:' in pattern: ", pat);
      return false;
    }
    return true;
  }

  template <typename VALUE_T, typename LEXER_T>
  bool Parser_Base<VALUE_T, LEXER_T>::ParsePatternSeq(const String & pat, size_t & pos,
                                                      Pattern & out) {
    while (true) {
      SkipWhitespace(pat, pos);
      if (pos >= pat.size() || pat[pos] == '|' || pat[pos] == ')') { return true; }

      PatternSymbol sym;
      bool named_token = false;
      if (pat[pos] == '-') {
        sym.discard = true;
        ++pos;
        if (pos == pat.size() || std::isspace(static_cast<unsigned char>(pat[pos]))) {
          notify::Error("'-' must immediately precede a rule element: ", pat);
          return false;
        }
      }
      const char c = pat[pos];
      if (c == '\'') {  // Character literal terminal.
        sym.is_terminal = true;
        if (!ParseCharLiteral(pat, pos, sym.token_id)) { return false; }
      }
      else if (c == '(') {  // Anonymous group, possibly with alternatives.
        ++pos;
        emp::vector<Pattern> group_alts;
        if (!ParsePatternAlts(pat, pos, group_alts)) { return false; }
        if (pos >= pat.size() || pat[pos] != ')') {
          notify::Error("Missing ')' in parse pattern: ", pat);
          return false;
        }
        ++pos;
        sym.rule_id = MakeAnonymousRule(std::move(group_alts));
      }
      else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {  // Identifier.
        const size_t start = pos;
        while (pos < pat.size() &&
               (std::isalnum(static_cast<unsigned char>(pat[pos])) || pat[pos] == '_')) {
          ++pos;
        }
        const String name = pat.substr(start, pos - start);
        // Multi-character names registered in the lexer are terminals; all other names
        // refer to parse rules (which may be defined later).
        const int token_id = (name.size() > 1) ? lexer->GetTokenID(name) : -1;
        if (token_id > 0) {
          sym.is_terminal = true;
          sym.token_id    = token_id;
          named_token     = true;
        } else {
          sym.rule_id = RequireRuleID(name);
        }
      }
      else {
        notify::Error("Unexpected character '", MakeEscaped(c), "' in parse pattern: ", pat);
        return false;
      }

      // Check for a modifier (must immediately follow the symbol, with no whitespace).
      if (pos < pat.size()) {
        switch (pat[pos]) {
          case '?': sym.mod = Mod::OPT;  ++pos; break;
          case '*': sym.mod = Mod::ANY;  ++pos; break;
          case '+': sym.mod = Mod::PLUS; ++pos; break;
          default: break;
        }
      }

      if (pos < pat.size() && pat[pos] == ':') {
        if (sym.mod != Mod::OPT || !named_token) {
          notify::Error("Defaults require an optional named token (TOKEN?:text): ", pat);
          return false;
        }
        ++pos;
        sym.has_default = true;
        if (!ParseDefaultLiteral(pat, pos, sym.default_lexeme)) { return false; }
      }

      out.symbols.emplace_back(sym);
    }
  }

  template <typename VALUE_T, typename LEXER_T>
  bool Parser_Base<VALUE_T, LEXER_T>::ParsePatternAlts(const String & pat, size_t & pos,
                                                       emp::vector<Pattern> & out) {
    while (true) {
      Pattern alt;
      if (!ParsePatternSeq(pat, pos, alt)) { return false; }
      out.emplace_back(std::move(alt));
      if (pos < pat.size() && pat[pos] == '|') { ++pos; continue; }
      return true;
    }
  }

  template <typename VALUE_T, typename LEXER_T>
  size_t Parser_Base<VALUE_T, LEXER_T>::AddRule(String name, String pattern,
                                                action_t action, String fun_name, bool no_ast, String note) {
    emp_assert(!lexer.IsNull());
    emp_assert(name.size() && (std::isalpha(static_cast<unsigned char>(name[0])) || name[0] == '_'),
               "Parse rule names must be valid identifiers.", name);
    emp_assert(!(name.size() > 1 && lexer->GetTokenID(name) > 0),
               "Parse rule name shadows a lexer token name.", name);

    validated = false;
    const size_t rule_id = RequireRuleID(name);
    if (start_rule == npos) { start_rule = rule_id; }  // Default start = first rule added.

    emp::vector<Pattern> alts;
    size_t pos = 0;
    if (!ParsePatternAlts(pattern, pos, alts)) { return rule_id; }
    if (pos != pattern.size()) {  // ParsePatternAlts stops early only at an unmatched ')'.
      notify::Error("Unexpected ')' in parse pattern: ", pattern);
      return rule_id;
    }

    for (auto & alt : alts) {
      alt.action   = action;
      alt.fun_name = fun_name;
      alt.no_ast = no_ast;
      alt.note = note;
      rules[rule_id].patterns.emplace_back(std::move(alt));
    }
    return rule_id;
  }

  // -- Grammar validation --

  template <typename VALUE_T, typename LEXER_T>
  bool Parser_Base<VALUE_T, LEXER_T>::Validate() const {
    if (validated) { return true; }
    bool ok = true;

    if (start_rule == npos) {
      notify::Error("Parser cannot be used until rules have been added.");
      return false;
    }

    // Make sure that every rule referenced in a pattern has been defined.
    for (const ParseRule & rule : rules) {
      if (rule.patterns.size() == 0) {
        notify::Error("Parse rule '", rule.name, "' is used but never defined.");
        ok = false;
      }
    }
    if (!ok) { return false; }

    // A default must match the full lexeme under the actual lexer's priorities,
    // and must be a visible token. A prefix match or an ignored token is not enough.
    for (const ParseRule & rule : rules) {
      for (const Pattern & pattern : rule.patterns) {
        for (const PatternSymbol & sym : pattern.symbols) {
          if (!sym.has_default) { continue; }
          const Token token = lexer->ToToken(sym.default_lexeme);
          if (token.id != sym.token_id || token.lexeme != sym.default_lexeme ||
              !lexer->GetSaveToken(sym.token_id)) {
            notify::Error("Default ", sym.default_lexeme.AsLiteral(), " for token '",
                          lexer->GetTokenName(sym.token_id), "' in rule '", rule.name,
                          "' must lex as exactly one visible token of that type.");
            ok = false;
          }
        }
      }
    }
    if (!ok) { return false; }

    // Determine which rules can match an empty token sequence (NULLABLE), to fixed point.
    emp::vector<char> nullable(rules.size(), 0);
    auto SymNullable = [&nullable](const PatternSymbol & sym) {
      if (sym.mod == Mod::OPT || sym.mod == Mod::ANY) { return true; }
      return !sym.is_terminal && static_cast<bool>(nullable[sym.rule_id]);
    };
    bool progress = true;
    while (progress) {
      progress = false;
      for (size_t id = 0; id < rules.size(); ++id) {
        if (nullable[id]) { continue; }
        for (const Pattern & pattern : rules[id].patterns) {
          bool all_nullable = true;
          for (const PatternSymbol & sym : pattern.symbols) {
            if (!SymNullable(sym)) { all_nullable = false; break; }
          }
          if (all_nullable) { nullable[id] = 1; progress = true; break; }
        }
      }
    }

    // Repeating a symbol that can match an empty sequence would loop forever.
    for (const ParseRule & rule : rules) {
      for (const Pattern & pattern : rule.patterns) {
        for (const PatternSymbol & sym : pattern.symbols) {
          if ((sym.mod == Mod::ANY || sym.mod == Mod::PLUS) &&
              !sym.is_terminal && nullable[sym.rule_id]) {
            notify::Error("In parse rule '", rule.name, "', pattern \"",
                          MakePatternString(pattern), "\": cannot repeat '",
                          rules[sym.rule_id].name,
                          "', which can match an empty sequence (infinite loop).");
            ok = false;
          }
        }
      }
    }

    // Detect left recursion: build the "can start with" relation between rules (a rule
    // can start with any rule that appears before the first non-nullable symbol of one
    // of its patterns) and search it for cycles.
    emp::vector<char> color(rules.size(), 0);  // 0 = unvisited; 1 = in progress; 2 = done.
    emp::vector<size_t> path;
    auto dfs = [&](auto && self, size_t id) -> void {
      color[id] = 1;
      path.push_back(id);
      for (const Pattern & pattern : rules[id].patterns) {
        for (const PatternSymbol & sym : pattern.symbols) {
          if (!sym.is_terminal) {
            const size_t next = sym.rule_id;
            if (color[next] == 1) {  // Found a cycle; report the path.
              String msg("Left recursion detected in parser grammar: ");
              size_t start = 0;
              while (path[start] != next) { ++start; }
              for (size_t i = start; i < path.size(); ++i) {
                msg += rules[path[i]].name;
                msg += " -> ";
              }
              msg += rules[next].name;
              notify::Error(msg);
              ok = false;
            }
            else if (color[next] == 0) { self(self, next); }
          }
          if (!SymNullable(sym)) { break; }  // Later symbols cannot start the rule.
        }
      }
      path.pop_back();
      color[id] = 2;
    };
    for (size_t id = 0; id < rules.size() && ok; ++id) {
      if (color[id] == 0) { dfs(dfs, id); }
    }

    // Bound the number of nodes each rule can contribute: 0, 1, or 2 (many).
    // Named normal rules contribute one node; groups/No AST alternatives contribute
    // their retained children. Saturation makes recursive analysis terminate.
    emp::vector<size_t> max_nodes(rules.size(), 0);
    auto CountChildren = [&](const Pattern & pattern) {
      size_t count = 0;
      for (const auto & sym : pattern.symbols) {
        if (sym.discard) continue;
        size_t n = sym.is_terminal ? 1 : max_nodes[sym.rule_id];
        if ((sym.mod == Mod::ANY || sym.mod == Mod::PLUS) && n) n = 2;
        count = std::min(size_t{2}, count + n);
      }
      return count;
    };
    bool changed = true;
    while (changed) {
      changed = false;
      for (size_t id = 0; id < rules.size(); ++id) {
        for (const auto & pattern : rules[id].patterns) {
          const size_t n = (rules[id].anonymous || pattern.no_ast) ? CountChildren(pattern) : 1;
          if (n > max_nodes[id]) { max_nodes[id] = n; changed = true; }
        }
      }
    }
    for (const auto & rule : rules) for (const auto & pattern : rule.patterns) {
      if (!pattern.no_ast) continue;
      if (pattern.action || !pattern.fun_name.empty() || !pattern.note.empty()) {
        notify::Error("No AST alternative in rule '", rule.name, "' cannot have a function or note.");
        ok = false;
      }
      if (CountChildren(pattern) > 1) {
        notify::Error("No AST alternative in rule '", rule.name,
                      "' may retain more than one child: ", MakePatternString(pattern));
        ok = false;
      }
    }

    // Warn about character literals that the lexer would fold into a larger token.
    std::set<int> char_terminals;
    for (const ParseRule & rule : rules) {
      for (const Pattern & pattern : rule.patterns) {
        for (const PatternSymbol & sym : pattern.symbols) {
          if (sym.is_terminal && sym.token_id > 0 && sym.token_id < 128) {
            char_terminals.insert(sym.token_id);
          }
        }
      }
    }
    for (int id : char_terminals) {
      const Token test = lexer->ToToken(String(1, static_cast<char>(id)));
      if (test.id != id) {
        notify::Warning("Parse pattern uses ", lexer->GetTokenName(id),
                        ", but the lexer tokenizes that character as ",
                        lexer->GetTokenName(test.id), "; this symbol can never match.");
      }
    }

    validated = ok;
    return ok;
  }

  // -- Dynamic parsing --

  template <typename VALUE_T, typename LEXER_T>
  bool Parser_Base<VALUE_T, LEXER_T>::TrySymbolOnce(const PatternSymbol & sym,
                                                    node_t & parent,
                                                    ParseState & state) const {
    if (sym.is_terminal) {
      if (state.it.Is(sym.token_id)) {
        const auto & token = state.it.Use();
        if (!sym.discard) parent.AdoptChild(emp::NewPtr<node_t>(token));
        return true;
      }
      // Track the deepest failure point (and what was expected there) for error reports.
      const size_t index = state.it.GetIndex();
      if (index >= state.furthest) {
        if (index > state.furthest) { state.furthest = index; state.expected.clear(); }
        state.expected.insert(sym.token_id);
      }
      return false;
    }

    // Otherwise this symbol references another rule.
    emp::Ptr<node_t> child = TryRule(sym.rule_id, state);
    if (child.IsNull()) { return false; }
    if (sym.discard) { child.Delete(); return true; }
    if (rules[sym.rule_id].anonymous ||
        rules[sym.rule_id].patterns[child->GetPatternID()].no_ast) {  // Splice groups and transparent alternatives.
      parent.SpliceChildren(*child);
      child.Delete();
    } else {
      parent.AdoptChild(child);
    }
    return true;
  }

  template <typename VALUE_T, typename LEXER_T>
  bool Parser_Base<VALUE_T, LEXER_T>::TrySequence(const emp::vector<PatternSymbol> & symbols,
                                                  node_t & parent,
                                                  ParseState & state) const {
    for (const PatternSymbol & sym : symbols) {
      switch (sym.mod) {
        case Mod::ONCE:
          if (!TrySymbolOnce(sym, parent, state)) { return false; }
          break;
        case Mod::OPT:
          if (!TrySymbolOnce(sym, parent, state) && sym.has_default && !sym.discard) {
            // Line zero denotes synthetic text. Do not advance or modify the input.
            parent.AdoptChild(emp::NewPtr<node_t>(Token{sym.token_id, sym.default_lexeme, 0}));
          }
          break;
        case Mod::PLUS:
          if (!TrySymbolOnce(sym, parent, state)) { return false; }
          [[fallthrough]];
        case Mod::ANY:
          while (true) {
            const size_t before = state.it.GetIndex();
            if (!TrySymbolOnce(sym, parent, state)) { break; }
            if (state.it.GetIndex() == before) { break; }  // Guard vs. empty matches.
          }
          break;
      }
    }
    return true;
  }

  template <typename VALUE_T, typename LEXER_T>
  emp::Ptr<typename Parser_Base<VALUE_T, LEXER_T>::node_t>
  Parser_Base<VALUE_T, LEXER_T>::TryRule(size_t rule_id, ParseState & state) const {
    const ParseRule & rule = rules[rule_id];
    for (size_t pattern_id = 0; pattern_id < rule.patterns.size(); ++pattern_id) {
      const TokenStream::Iterator backup_it = state.it;
      emp::Ptr<node_t> node = emp::NewPtr<node_t>(rule_id, pattern_id);
      if (TrySequence(rule.patterns[pattern_id].symbols, *node, state)) {
        node->SetAction(&rule.patterns[pattern_id].action);
        node->SetNote(rule.patterns[pattern_id].note);
        return node;
      }
      node.Delete();       // Toss any partially-built node...
      state.it = backup_it;  // ...and back up to try the next pattern.
    }
    return nullptr;
  }

  template <typename VALUE_T, typename LEXER_T>
  String Parser_Base<VALUE_T, LEXER_T>::MakeParseError(const TokenStream & tokens,
                                                       const ParseState & state) const {
    String out("Parse error");
    const bool at_eof = (state.furthest >= tokens.size());
    const Token & bad = at_eof ? TokenStream::GetEOF() : tokens.Get(state.furthest);
    if (!at_eof) { out += MakeString(" on line ", bad.line_id); }
    out += MakeString(": unexpected ", lexer->GetTokenName(bad.id));
    if (bad.lexeme.size()) { out += MakeString(" \"", MakeEscaped(bad.lexeme), "\""); }
    if (state.expected.size()) {
      out += "; expected ";
      bool first = true;
      for (int id : state.expected) {
        if (!first) { out += " or "; }
        out += lexer->GetTokenName(id);
        first = false;
      }
    }
    out += ".";
    return out;
  }

  template <typename VALUE_T, typename LEXER_T>
  typename Parser_Base<VALUE_T, LEXER_T>::ast_t
  Parser_Base<VALUE_T, LEXER_T>::Parse(const TokenStream & tokens, bool require_all) const {
    last_error.resize(0);
    last_parse_succeeded = false;
    if (!Validate()) { return ast_t{}; }

    ParseState state{tokens.begin()};
    emp::Ptr<node_t> root = TryRule(start_rule, state);

    // If the start rule matched only a prefix of the input, the parse has still failed.
    if (!root.IsNull() && require_all && !state.it.AtEnd()) {
      if (state.it.GetIndex() > state.furthest) {
        state.furthest = state.it.GetIndex();
        state.expected.clear();
      }
      root.Delete();
      root = nullptr;
    }

    if (root.IsNull()) {
      last_error = MakeParseError(tokens, state);
      notify::Error(last_error);
      return ast_t{};
    }
    last_parse_succeeded = true;
    if (rules[start_rule].patterns[root->GetPatternID()].no_ast) {
      auto wrapper = root;
      root = wrapper->NumChildren() ? wrapper->ReleaseOnlyChild() : nullptr;
      wrapper.Delete();
    }
    if (root.IsNull()) notify::Warning("Parsing succeeded, but the start alternative produced no AST.");
    return ast_t{root};
  }

  // -- C++ code generation --

  template <typename VALUE_T, typename LEXER_T>
  void Parser_Base<VALUE_T, LEXER_T>::WriteCPP_RunDispatch(emp::CPPFile & file) const {
    file.AddCode("      switch (rule_id) {");
    for (size_t id = 0; id < rules.size(); ++id) {
      const ParseRule & rule = rules[id];
      bool has_fun = false;
      for (const Pattern & pattern : rule.patterns) {
        if (pattern.fun_name.size()) { has_fun = true; break; }
      }
      if (!has_fun) { continue; }
      file.AddCode("      case RULE_", CppRuleName(id), ":");
      file.AddCode("        switch (pattern_id) {");
      for (size_t pattern_id = 0; pattern_id < rule.patterns.size(); ++pattern_id) {
        const Pattern & pattern = rule.patterns[pattern_id];
        if (pattern.fun_name.size() == 0) { continue; }
        file.AddCode("        case ", pattern_id, ": return handler.", pattern.fun_name,
                     "(*this);");
      }
      file.AddCode("        default: break;");
      file.AddCode("        }");
      file.AddCode("        break;");
    }
    file.AddCode("      default: break;");
    file.AddCode("      }");
  }

  template <typename VALUE_T, typename LEXER_T>
  void Parser_Base<VALUE_T, LEXER_T>::WriteCPP_MatchSymbol(emp::CPPFile & file,
                                                           const PatternSymbol & sym) const {
    // Build the code fragments to (1) attempt a single match and (2) absorb the result.
    String attempt, absorb;
    if (sym.is_terminal) {
      attempt = MakeString("MatchToken(", sym.token_id, ", *out, ", !sym.discard, ")");
    } else {
      attempt = MakeString("Parse_", CppRuleName(sym.rule_id), "(child)");
      if (sym.discard) { absorb = " child.reset();"; }
      else if (rules[sym.rule_id].anonymous) {  // Splice group children into the parent.
        absorb = " for (auto & c : child->children) out->children.push_back(std::move(c));";
      } else {
        absorb = " if (child) out->children.push_back(std::move(child));";
      }
    }
    const String comment = MakeString("  // ", MakeSymbolString(sym));

    if (sym.is_terminal) {
      switch (sym.mod) {
        case Mod::ONCE:
          file.AddCode("      if (ok) ok = ", attempt, ";", comment);
          break;
        case Mod::OPT:
          if (sym.has_default && !sym.discard) {
            file.AddCode("      if (ok && !", attempt, ") { AddDefaultToken(", sym.token_id,
                         ", ", sym.default_lexeme.AsLiteral(), ", *out); }", comment);
          } else {
            file.AddCode("      if (ok) { ", attempt, "; }", comment);
          }
          break;
        case Mod::ANY:
          file.AddCode("      if (ok) { while (", attempt, ") { } }", comment);
          break;
        case Mod::PLUS:
          file.AddCode("      if (ok) { ok = ", attempt, "; while (", attempt, ") { } }",
                       comment);
          break;
      }
      return;
    }

    switch (sym.mod) {
      case Mod::ONCE:
        file.AddCode("      if (ok) { node_ptr_t child; ok = ", attempt, ";", comment);
        file.AddCode("        if (ok) {", absorb, " } }");
        break;
      case Mod::OPT:
        file.AddCode("      if (ok) { node_ptr_t child;", comment);
        file.AddCode("        if (", attempt, ") {", absorb, " } }");
        break;
      case Mod::ANY:
        file.AddCode("      if (ok) { node_ptr_t child;", comment);
        file.AddCode("        while (", attempt, ") {", absorb, " } }");
        break;
      case Mod::PLUS:
        file.AddCode("      if (ok) { node_ptr_t child; ok = ", attempt, ";", comment);
        file.AddCode("        if (ok) {", absorb, " }");
        file.AddCode("        while (", attempt, ") {", absorb, " } }");
        break;
    }
  }

  template <typename VALUE_T, typename LEXER_T>
  void Parser_Base<VALUE_T, LEXER_T>::WriteCPP_ParseRule(emp::CPPFile & file,
                                                         size_t rule_id) const {
    const ParseRule & rule = rules[rule_id];
    const String cpp_name  = CppRuleName(rule_id);
    file.AddCode("");
    file.AddCode("  // Rule ", rule.name, (rule.anonymous ? " (anonymous group)" : ""));
    file.AddCode("  bool Parse_", cpp_name, "(node_ptr_t & out) {");
    file.AddCode("    out = std::make_unique<${node_name}>();");
    file.AddCode("    out->rule_id = RULE_", cpp_name, ";");
    for (size_t pattern_id = 0; pattern_id < rule.patterns.size(); ++pattern_id) {
      const Pattern & pattern = rule.patterns[pattern_id];
      file.AddCode("    { // Pattern ", pattern_id, ": ", MakePatternString(pattern));
      file.AddCode("      const size_t start_pos = pos;");
      file.AddCode("      out->pattern_id = ", pattern_id, ";");
      file.AddCode("      bool ok = true;");
      for (const PatternSymbol & sym : pattern.symbols) { WriteCPP_MatchSymbol(file, sym); }
      if (pattern.no_ast) {
        file.AddCodeBlock(
          "      if (ok) {",
          "        node_ptr_t promoted;",
          "        if (!out->children.empty()) promoted = std::move(out->children.front());",
          "        out = std::move(promoted);",
          "        return true;",
          "      }");
      } else {
        if (HasNotes()) file.AddCode("      if (ok) out->note = ", pattern.note.AsLiteral(), ";");
        file.AddCode("      if (ok) return true;");
      }
      file.AddCode("      pos = start_pos;         // This pattern failed; backtrack...");
      file.AddCode("      out->children.clear();   // ...and toss any partial results.");
      file.AddCode("    }");
    }
    file.AddCode("    out.reset();");
    file.AddCode("    return false;");
    file.AddCode("  }");
  }

  template <typename VALUE_T, typename LEXER_T>
  void Parser_Base<VALUE_T, LEXER_T>::WriteCPP(
    emp::CPPFile & file,
    emp::String object_name,
    emp::String node_name,
    emp::String token_name,
    emp::String value_type,
    bool token_has_lexeme,
    bool token_has_line,
    bool use_emp
  ) const {
    if (!Validate()) {
      notify::Error("Unable to write C++ parser code: grammar failed validation.");
      return;
    }

    file.AddVarSetting("parser_name", object_name);
    file.AddVarSetting("node_name", node_name);
    file.AddVarSetting("token_name", token_name);
    file.AddVarSetting("value_type", value_type);
    if (use_emp) {
      file.AddVarSetting("str_type", "emp::String");
      file.AddVarSetting("vec_type", "emp::vector");
    } else {
      file.AddVarSetting("str_type", "std::string");
      file.AddVarSetting("vec_type", "std::vector");
    }

    file.Include("<cstddef>");
    file.Include("<iostream>");
    file.Include("<memory>");
    file.Include("<type_traits>");
    file.Include("<utility>");
    if (use_emp) {
      file.Include("emp/base/vector.hpp");
      file.Include("emp/tools/String.hpp");
    } else {
      file.Include("<string>");
      file.Include("<vector>");
    }

    file.AddCode("");
    file.AddCode("// Recursive-descent parser (generated by emp::Parser).");
    file.AddCodeBlock(
      "class ${parser_name} {",
      "public:",
      "  using value_t = ${value_type};",
      "");

    // -- Rule ID constants --
    file.AddCode("  // -- Parse rule IDs --");
    for (size_t id = 0; id < rules.size(); ++id) {
      file.AddCode("  static constexpr int RULE_", CppRuleName(id), " = ", id, ";")
        .AppendPadding(45)
        .AppendCode("// ", rules[id].name);
      if (!rules[id].anonymous) {
        file.AppendCode(" ::= ");
        for (size_t i = 0; i < rules[id].patterns.size(); ++i) {
          if (i) { file.AppendCode(" | "); }
          file.AppendCode(MakePatternString(rules[id].patterns[i]));
        }
      }
    }
    file.AddCode("  static constexpr int NUM_RULES = ", rules.size(), ";");

    file.AddCodeBlock(
      "",
      "  // Return the name of a parse rule given its ID.",
      "  static ${str_type} RuleName(int id) {",
      "    switch (id) {");
    for (size_t id = 0; id < rules.size(); ++id) {
      file.AddCode("    case RULE_", CppRuleName(id), ": return ", rules[id].name.AsLiteral(),
                   ";");
    }
    file.AddCodeBlock(
      "    default: return \"_unknown_\";",
      "    }",
      "  }");

    // -- The AST node --
    file.AddCodeBlock(
      "",
      "  // -- Abstract syntax tree --",
      "  struct ${node_name};",
      "  using node_ptr_t = std::unique_ptr<${node_name}>;",
      "  struct ${node_name} {",
      "    int rule_id = -1;                    // Parse rule matched (-1 = leaf token).",
      "    int pattern_id = 0;                  // Which alternative pattern was matched?",
      "    ${token_name} token{};                       // Token matched (leaf nodes only).",
      "    ${vec_type}<node_ptr_t> children{};  // Sub-nodes, in matched order.");
    if (HasNotes()) {
      file.AddCode("    ${str_type} note{};                  // Annotation for the matched alternative.");g
    }
    file.AddCodeBlock(
      "",
      "    bool IsLeaf() const { return rule_id < 0; }",
      "    bool IsRule() const { return rule_id >= 0; }",
      "    size_t NumChildren() const { return children.size(); }",
      "    ${node_name} & GetChild(size_t id) { return *children[id]; }",
      "    const ${node_name} & GetChild(size_t id) const { return *children[id]; }");
    if (token_has_lexeme) {
      file.AddCodeBlock(
        "    const ${str_type} & GetLexeme() const { return token.lexeme; }",
        "    const ${str_type} & GetChildLexeme(size_t id) const { return children[id]->token.lexeme; }");
    }
    file.AddCodeBlock(
      "",
      "    // Run this node, using `handler` to provide the action for each parse rule.",
      "    // Patterns without an associated action run each rule-node child, in order,",
      "    // and return the final result.",
      "    template <typename HANDLER>",
      "    value_t Run(HANDLER & handler) {");
    WriteCPP_RunDispatch(file);
    file.AddCodeBlock(
      "      // Default behavior (no action associated with the matched pattern):",
      "      if constexpr (std::is_void_v<value_t>) {",
      "        for (auto & child : children) if (child->IsRule()) child->Run(handler);",
      "      } else {",
      "        value_t out{};",
      "        for (auto & child : children) if (child->IsRule()) out = child->Run(handler);",
      "        return out;",
      "      }",
      "    }",
      "",
      "    // Run the specified child node and return its result.",
      "    template <typename HANDLER>",
      "    value_t RunChild(size_t id, HANDLER & handler) { return children[id]->Run(handler); }",
      "  };");

    // -- Parse machinery --
    file.AddCodeBlock(
      "",
      "private:",
      "  const ${vec_type}<${token_name}> * tokens = nullptr; // Tokens being parsed.",
      "  size_t pos = 0;       // Current position in the token sequence.",
      "  size_t furthest = 0;  // Deepest position reached (used for error reporting).",
      "  bool last_parse_succeeded = false;",
      "",
      "  // Try to match a specific token type; on success add a leaf node and advance.",
      "  bool MatchToken(int id, ${node_name} & parent, bool retain = true) {",
      "    if (pos < tokens->size() && (*tokens)[pos].id == id) {",
      "      if (retain) {",
      "        node_ptr_t leaf = std::make_unique<${node_name}>();",
      "        leaf->token = (*tokens)[pos];",
      "        parent.children.push_back(std::move(leaf));",
      "      }",
      "      ++pos;",
      "      return true;",
      "    }",
      "    if (pos > furthest) furthest = pos;",
      "    return false;",
      "  }");

    file.AddCodeBlock(
      "",
      "  // Insert a synthetic leaf without consuming input. Source coordinates stay zero.",
      "  void AddDefaultToken(int id, [[maybe_unused]] const ${str_type} & lexeme, ${node_name} & parent) {",
      "    node_ptr_t leaf = std::make_unique<${node_name}>();",
      "    leaf->token.id = id;");
    if (token_has_lexeme) { file.AddCode("    leaf->token.lexeme = lexeme;"); }
    file.AddCodeBlock(
      "    parent.children.push_back(std::move(leaf));",
      "  }");

    for (size_t id = 0; id < rules.size(); ++id) { WriteCPP_ParseRule(file, id); }

    // -- Public interface --
    file.AddCodeBlock(
      "",
      "public:",
      "  // Parse a token sequence. Return its root (possibly nullptr for No AST).",
      "  // Check LastParseSucceeded() to distinguish no root from a parse failure.",
      "  // If require_all is set, the full token sequence must be consumed.",
      "  node_ptr_t Parse(const ${vec_type}<${token_name}> & in_tokens, bool require_all = true) {",
      "    last_parse_succeeded = false;",
      "    tokens = &in_tokens;",
      "    pos = 0;",
      "    furthest = 0;",
      "    node_ptr_t root;");
    file.AddCode("    bool ok = Parse_", CppRuleName(start_rule), "(root);");
    file.AddCodeBlock(
      "    if (ok && require_all && pos < in_tokens.size()) {",
      "      if (pos > furthest) furthest = pos;",
      "      ok = false;  // Only a prefix of the input was parsed.",
      "    }",
      "    last_parse_succeeded = ok;",
      "    if (!ok) { root.reset(); return nullptr; }",
      "    return root;",
      "  }",
      "",
      "  // A successful No AST start alternative may return nullptr.",
      "  bool LastParseSucceeded() const { return last_parse_succeeded; }",
      "",
      "  // Position of the deepest token reached in the last failed parse.",
      "  size_t GetErrorPos() const { return furthest; }",
      "",
      "  // Print an error message describing the last failed parse.",
      "  void PrintError(std::ostream & os = std::cerr) const {",
      "    os << \"Parse error\";",
      "    if (tokens != nullptr && furthest < tokens->size()) {");
    if (token_has_line) {
      file.AddCode("      os << \" (line \" << (*tokens)[furthest].line_id << \")\";");
    }
    file.AddCode("      os << \": unexpected input\";");
    if (token_has_lexeme) {
      file.AddCode("      os << \" '\" << (*tokens)[furthest].lexeme << \"'\";");
    }
    file.AddCodeBlock(
      "    } else {",
      "      os << \": unexpected end of input\";",
      "    }",
      "    os << \".\" << std::endl;",
      "  }",
      "};");
  }

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_COMPILER_PARSER_HPP_GUARD
