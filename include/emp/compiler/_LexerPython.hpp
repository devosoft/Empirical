/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/compiler/_LexerPython.hpp
 * @brief Python output implementation for Lexer.hpp.
 */

#pragma once

#ifndef INCLUDE_EMP_COMPILER_LEXER_PYTHON_HPP_impl_GUARD
#define INCLUDE_EMP_COMPILER_LEXER_PYTHON_HPP_impl_GUARD

#include <unordered_map>

#include "../io/PythonFile.hpp"
#include "../tools/String.hpp"

#include "_CodeGen.hpp"

namespace emp {

  template <int MAX_ID>
  void Lexer_Base<MAX_ID>::WritePython(emp::PythonFile & file,
                                       emp::String object_name,
                                       emp::String DFA_name,
                                       emp::String token_name,
                                       bool save_lexeme,
                                       bool save_line_id,
                                       bool save_col_id) const {
    if (generate_lexer) { Generate(); }

    file.ImportFrom("dataclasses", "dataclass");
    file.AddRawCode("@dataclass").AddRawCode("class ", token_name, ":");
    file.AddRawCode("    id: int");
    if (save_lexeme) { file.AddRawCode("    lexeme: str = \"\""); }
    if (save_line_id) { file.AddRawCode("    line_id: int = 0"); }
    if (save_col_id) { file.AddRawCode("    col_id: int = 0"); }
    file.AddRawCode("    def __int__(self): return self.id");
    file.AddRawCode("    def __bool__(self): return self.id != 0").AddRawCode("");
    lexer_dfa.WritePython(file, DFA_name);
    file.AddRawCode("class ", object_name, ":");
    file.AddRawCode("    NUM_TOKENS = ", token_set.size());
    file.AddRawCode("    ID__EOF_ = 0");
    for (const auto & token : token_set) {
      file.AddRawCode("    ID_", token.name, " = ", token.id);
    }
    file.AddRawCode("    _token_names = {-1: \"_ERROR_\", 0: \"_EOF_\",");
    for (const auto & token : token_set) {
      file.AddRawCode("        ", token.id, ": ", internal::QuoteCodeString(token.name), ",");
    }
    file.AddRawCode("    }").AddRawCode("    _ignored = {0");
    for (const auto & token : token_set) {
      if (!token.save_token) { file.AppendRawCode(", ", token.id); }
    }
    file.AppendRawCode("}");
    String args = "id";
    if (save_lexeme) { args += ", lexeme"; }
    if (save_line_id) { args += ", line"; }
    if (save_col_id) { args += ", col"; }
    String location = "\"\"";
    if (save_line_id) {
      location = save_col_id ? "f\" (line {self.Peek().line_id}:{self.Peek().col_id})\""
                             : "f\" (line {self.Peek().line_id})\"";
    }
    const std::unordered_map<String, String> vars{{"lexer", object_name},
                                                  {"dfa", DFA_name},
                                                  {"token", token_name},
                                                  {"token_args", args},
                                                  {"location", location}};
    internal::WriteCodeTemplate(file,
                                R"CODE(
    def __init__(self):
        self.cur_line = 1
        self.cur_col = 0
        self.start_pos = 0
        self.lexeme = ""
        self.tokens = []
        self.token_id = 0
        self.eof_token = self._MakeToken(0, "_EOF_", 0, 0)

    @staticmethod
    def _MakeToken(id, lexeme, line, col):
        return @{token}(@{token_args})

    @classmethod
    def TokenName(cls, id):
        if isinstance(id, @{token}):
            id = id.id
        if id in cls._token_names:
            return cls._token_names[id]
        escaped = {10: "'\\n'", 13: "'\\r'", 9: "'\\t'", 92: "'\\\\'"}
        if id in escaped:
            return escaped[id]
        return "'" + chr(id) + "'" if 32 <= id < 127 else "Token ID: " + str(id)

    @classmethod
    def IgnoreToken(cls, id):
        return id in cls._ignored

    @classmethod
    def GetNumTokens(cls):
        return cls.NUM_TOKENS

    def NextToken(self, text):
        if self.start_pos >= len(text):
            return self._MakeToken(0, "", self.cur_line, self.cur_col)
        start = self.start_pos
        cur_pos = best_pos = tc_boundary_pos = start
        cur_state = cur_stop = 0
        best_stop = -1
        if start == 0 or text[start - 1] == "\n":
            cur_state = @{dfa}.GetNext(0, @{dfa}.SYMBOL_START)
        while cur_stop >= 0 and cur_state >= 0 and cur_pos < len(text):
            symbol = ord(text[cur_pos])
            cur_pos += 1
            cur_state = @{dfa}.GetNext(cur_state, symbol)
            cur_stop = @{dfa}.GetStop(cur_state)
            if @{dfa}.GetTCStop(cur_state) > 0:
                tc_boundary_pos = cur_pos
            if cur_stop > 0:
                use_tc = @{dfa}.GetTCFinal(cur_state) == cur_stop and tc_boundary_pos > start
                best_pos = tc_boundary_pos if use_tc else cur_pos
                best_stop = cur_stop
            if cur_pos == len(text) or text[cur_pos] == "\n":
                eol_state = @{dfa}.GetNext(cur_state, @{dfa}.SYMBOL_STOP)
                eol_stop = @{dfa}.GetStop(eol_state)
                if eol_stop > 0:
                    use_tc = @{dfa}.GetTCFinal(eol_state) == eol_stop and tc_boundary_pos > start
                    best_pos = tc_boundary_pos if use_tc else cur_pos
                    best_stop = eol_stop
        if best_pos == start:
            best_stop = ord(text[start])
            if best_stop >= 128:
                best_stop = -1
            best_pos += 1
        self.lexeme = text[start:best_pos]
        self.start_pos = best_pos
        out_line, out_col = self.cur_line, self.cur_col
        last_newline = self.lexeme.rfind("\n")
        if last_newline == -1:
            self.cur_col += len(self.lexeme)
        else:
            self.cur_col = len(self.lexeme) - last_newline - 1
            self.cur_line += self.lexeme.count("\n")
        return self._MakeToken(best_stop, self.lexeme, out_line, out_col)

    def Tokenize(self, text):
        if hasattr(text, "read"):
            text = text.read()
        if not isinstance(text, str):
            raise TypeError("Tokenize expects a string or a text stream")
        self.start_pos = self.cur_col = self.token_id = 0
        self.cur_line = 1
        self.tokens = []
        while self.start_pos < len(text):
            token = self.NextToken(text)
            if not self.IgnoreToken(token.id):
                self.tokens.append(token)
        self.eof_token = self._MakeToken(0, "_EOF_", self.cur_line, self.cur_col)
        return self.tokens

    def Any(self):
        return self.token_id < len(self.tokens)

    def None_(self):
        # None is a Python keyword; this is the counterpart of C++ None().
        return not self.Any()

    def Is(self, id):
        return self.Any() and self.Peek().id == id

    def Peek(self, skip_count=0):
        if skip_count < 0:
            raise ValueError("skip_count must be nonnegative")
        pos = self.token_id + skip_count
        return self.tokens[pos] if pos < len(self.tokens) else self.eof_token

    def Error(self, *message):
        location = @{location}
        raise RuntimeError("ERROR" + location + ": " + "".join(str(part) for part in message))

    def Use(self, id=None, *message):
        if id is not None and not self.Is(id):
            if not message:
                message = ("Expected token of type ", self.TokenName(id),
                           ", but found type ", self.TokenName(self.Peek().id))
            self.Error(*message)
        token = self.Peek()
        if self.Any():
            self.token_id += 1
        return token

    def UseIf(self, *ids):
        for id in ids:
            if self.Is(id):
                return self.Use().id
        return 0

    def Rewind(self, steps=1):
        if steps < 0:
            raise ValueError("steps must be nonnegative")
        self.token_id = max(0, self.token_id - steps)
)CODE",
                                vars);
  }

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_COMPILER_LEXER_PYTHON_HPP_impl_GUARD

// Local settings for Empecable file checker.
// empecable_words: chr classmethod cls dataclass dataclasses hasattr isinstance nonnegative ord staticmethod
