/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/compiler/_LexerJS.hpp
 * @brief JS output implementation for Lexer.hpp.
 */

#pragma once

#ifndef INCLUDE_EMP_COMPILER_LEXER_JS_HPP_impl_GUARD
#define INCLUDE_EMP_COMPILER_LEXER_JS_HPP_impl_GUARD

#include <unordered_map>

#include "../io/JSFile.hpp"
#include "../tools/String.hpp"

#include "_CodeGen.hpp"

namespace emp {

  template <int MAX_ID>
  void Lexer_Base<MAX_ID>::WriteJS(emp::JSFile & file,
                                   emp::String object_name,
                                   emp::String DFA_name,
                                   emp::String token_name,
                                   bool save_lexeme,
                                   bool save_line_id,
                                   bool save_col_id) const {
    if (generate_lexer) { Generate(); }

    String params = "id";
    if (save_lexeme) { params += ", lexeme = \"\""; }
    if (save_line_id) { params += ", line_id = 0"; }
    if (save_col_id) { params += ", col_id = 0"; }
    file.AddRawCode("class ", token_name, " {");
    file.AddRawCode("  constructor(", params, ") {").AddRawCode("    this.id = id;");
    if (save_lexeme) { file.AddRawCode("    this.lexeme = lexeme;"); }
    if (save_line_id) { file.AddRawCode("    this.line_id = line_id;"); }
    if (save_col_id) { file.AddRawCode("    this.col_id = col_id;"); }
    file.AddRawCode("  }").AddRawCode("  valueOf() { return this.id; }");
    file.AddRawCode("}").AddRawCode("");
    lexer_dfa.WriteJS(file, DFA_name);
    file.AddRawCode("class ", object_name, " {");
    file.AddRawCode("  static NUM_TOKENS = ", token_set.size(), ";");
    file.AddRawCode("  static ID__EOF_ = 0;");
    for (const auto & token : token_set) {
      file.AddRawCode("  static ID_", token.name, " = ", token.id, ";");
    }
    file.AddRawCode("  static _token_names = {\"-1\": \"_ERROR_\", 0: \"_EOF_\",");
    for (const auto & token : token_set) {
      file.AddRawCode("    ", token.id, ": ", internal::QuoteCodeString(token.name), ",");
    }
    file.AddRawCode("  };").AddRawCode("  static _ignored = new Set([0");
    for (const auto & token : token_set) {
      if (!token.save_token) { file.AppendRawCode(", ", token.id); }
    }
    file.AppendRawCode("]);");
    String args = "id";
    if (save_lexeme) { args += ", lexeme"; }
    if (save_line_id) { args += ", line"; }
    if (save_col_id) { args += ", col"; }
    String location = "\"\"";
    if (save_line_id) {
      location = save_col_id ? "` (line ${this.Peek().line_id}:${this.Peek().col_id})`"
                             : "` (line ${this.Peek().line_id})`";
    }
    const std::unordered_map<String, String> vars{{"lexer", object_name},
                                                  {"dfa", DFA_name},
                                                  {"token", token_name},
                                                  {"token_args", args},
                                                  {"location", location}};
    internal::WriteCodeTemplate(file,
                                R"CODE(
  constructor() {
    this.cur_line = 1;
    this.cur_col = 0;
    this.start_pos = 0;
    this.lexeme = "";
    this.tokens = [];
    this.token_id = 0;
    this.eof_token = this._MakeToken(0, "_EOF_", 0, 0);
  }
  _MakeToken(id, lexeme, line, col) { return new @{token}(@{token_args}); }
  static TokenName(id) {
    if (id instanceof @{token}) id = id.id;
    if (Object.hasOwn(this._token_names, id)) return this._token_names[id];
    const escaped = {10: "'\\n'", 13: "'\\r'", 9: "'\\t'", 92: "'\\\\'"};
    if (Object.hasOwn(escaped, id)) return escaped[id];
    return id >= 32 && id < 127 ? "'" + String.fromCharCode(id) + "'" : "Token ID: " + id;
  }
  static IgnoreToken(id) { return this._ignored.has(id); }
  static GetNumTokens() { return this.NUM_TOKENS; }
  TokenName(id) { return @{lexer}.TokenName(id); }
  IgnoreToken(id) { return @{lexer}.IgnoreToken(id); }
  GetNumTokens() { return @{lexer}.GetNumTokens(); }

  NextToken(text) {
    if (this.start_pos >= text.length) return this._MakeToken(0, "", this.cur_line, this.cur_col);
    const start = this.start_pos;
    let cur_pos = start, best_pos = start, tc_boundary_pos = start;
    let cur_state = 0, cur_stop = 0, best_stop = -1;
    if (start === 0 || text[start - 1] === "\n") cur_state = @{dfa}.GetNext(0, @{dfa}.SYMBOL_START);
    while (cur_stop >= 0 && cur_state >= 0 && cur_pos < text.length) {
      const symbol = text.charCodeAt(cur_pos++);
      cur_state = @{dfa}.GetNext(cur_state, symbol);
      cur_stop = @{dfa}.GetStop(cur_state);
      if (@{dfa}.GetTCStop(cur_state) > 0) tc_boundary_pos = cur_pos;
      if (cur_stop > 0) {
        const use_tc = @{dfa}.GetTCFinal(cur_state) === cur_stop && tc_boundary_pos > start;
        best_pos = use_tc ? tc_boundary_pos : cur_pos;
        best_stop = cur_stop;
      }
      if (cur_pos === text.length || text[cur_pos] === "\n") {
        const eol_state = @{dfa}.GetNext(cur_state, @{dfa}.SYMBOL_STOP);
        const eol_stop = @{dfa}.GetStop(eol_state);
        if (eol_stop > 0) {
          const use_tc = @{dfa}.GetTCFinal(eol_state) === eol_stop && tc_boundary_pos > start;
          best_pos = use_tc ? tc_boundary_pos : cur_pos;
          best_stop = eol_stop;
        }
      }
    }
    if (best_pos === start) {
      best_stop = text.charCodeAt(start);
      if (best_stop >= 128) best_stop = -1;
      ++best_pos;
    }
    this.lexeme = text.slice(start, best_pos);
    this.start_pos = best_pos;
    const out_line = this.cur_line, out_col = this.cur_col;
    const last_newline = this.lexeme.lastIndexOf("\n");
    if (last_newline === -1) this.cur_col += this.lexeme.length;
    else {
      this.cur_col = this.lexeme.length - last_newline - 1;
      this.cur_line += this.lexeme.split("\n").length - 1;
    }
    return this._MakeToken(best_stop, this.lexeme, out_line, out_col);
  }
  Tokenize(text) {
    if (typeof text !== "string") throw new TypeError("Tokenize expects a string");
    this.start_pos = this.cur_col = this.token_id = 0;
    this.cur_line = 1;
    this.tokens = [];
    while (this.start_pos < text.length) {
      const token = this.NextToken(text);
      if (!this.IgnoreToken(token.id)) this.tokens.push(token);
    }
    this.eof_token = this._MakeToken(0, "_EOF_", this.cur_line, this.cur_col);
    return this.tokens;
  }
  Any() { return this.token_id < this.tokens.length; }
  None() { return !this.Any(); }
  Is(id) { return this.Any() && this.Peek().id === id; }
  Peek(skip_count = 0) {
    if (!Number.isInteger(skip_count) || skip_count < 0) throw new RangeError("Invalid skip_count");
    return this.tokens[this.token_id + skip_count] ?? this.eof_token;
  }
  Error(...message) {
    const location = @{location};
    throw new Error("ERROR" + location + ": " + message.join(""));
  }
  Use(id = undefined, ...message) {
    if (id !== undefined && !this.Is(id)) {
      if (!message.length) message = ["Expected token of type ", this.TokenName(id),
                                     ", but found type ", this.TokenName(this.Peek().id)];
      this.Error(...message);
    }
    const token = this.Peek();
    if (this.Any()) ++this.token_id;
    return token;
  }
  UseIf(...ids) {
    for (const id of ids) if (this.Is(id)) return this.Use().id;
    return 0;
  }
  Rewind(steps = 1) {
    if (!Number.isInteger(steps) || steps < 0) throw new RangeError("Invalid steps");
    this.token_id = Math.max(0, this.token_id - steps);
  }
}
export { @{lexer}, @{dfa}, @{token} };
)CODE",
                                vars);
  }

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_COMPILER_LEXER_JS_HPP_impl_GUARD
