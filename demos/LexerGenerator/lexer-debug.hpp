#ifndef EMPLEX_LEXER_HPP_INCLUDE_
#define EMPLEX_LEXER_HPP_INCLUDE_

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace emplex {
  // Struct to store information about a found Token
  struct Token {
    int id;                             // Type ID for token
    std::string lexeme;                 // Sequence matched by token
    size_t line_id;                     // Line token started on
    size_t col_id;                      // Column token started on
    operator int() const { return id; } // Auto-convert tokens to IDs
  };

  // Deterministic Finite Automaton (DFA) for token recognition.
  class DFA {
  private:
    // DFA transition table
    static constexpr int NUM_STATES=20;
    static constexpr int NUM_SYMBOLS=128;
    using row_t = std::array<int, 128>;
    static constexpr std::array<row_t, NUM_STATES> table = {{
      /* State 0 */ {-1,-1,1,2,-1,-1,-1,-1,-1,2,-1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,2,3,2,3,4,4,4,4,4,4,4,4,4,4,2,2,2,2,2,2,2,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6,5,5,7,5,5,5,5,5,5,5,2,2,2,2,5,2,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,2,2,2,2,2},
      /* State 1 */ {-1,-1,1,2,-1,-1,-1,-1,-1,2,-1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,2,3,2,3,4,4,4,4,4,4,4,4,4,4,2,2,2,2,2,2,2,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6,5,5,7,5,5,5,5,5,5,5,2,2,2,2,5,2,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,2,2,2,2,2},
      /* State 2 */ {-1,-1,-1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      /* State 3 */ {-1,-1,-1,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      /* State 4 */ {-1,-1,-1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,17,-1,18,18,18,18,18,18,18,18,18,18,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      /* State 5 */ {-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,9,-1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1},
      /* State 6 */ {-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,14,9,9,9,9,9,9,9,9,-1,-1,-1,-1,9,-1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1},
      /* State 7 */ {-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,10,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,9,-1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1},
      /* State 8 */ {-1,-1,-1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      /* State 9 */ {-1,-1,-1,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,9,-1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1},
      /* State 10 */ {-1,-1,-1,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,12,9,9,9,9,9,9,-1,-1,-1,-1,9,-1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1},
      /* State 11 */ {-1,-1,-1,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      /* State 12 */ {-1,-1,-1,13,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,9,-1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1},
      /* State 13 */ {-1,-1,-1,13,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      /* State 14 */ {-1,-1,-1,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,9,9,9,9,15,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,9,-1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1},
      /* State 15 */ {-1,-1,-1,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,9,9,9,9,9,9,9,9,9,16,9,9,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,9,-1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1},
      /* State 16 */ {-1,-1,-1,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1,-1,-1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,12,9,9,9,9,9,9,-1,-1,-1,-1,9,-1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,-1,-1,-1,-1,-1},
      /* State 17 */ {-1,-1,-1,19,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,17,17,17,17,17,17,17,17,17,17,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      /* State 18 */ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,17,-1,18,18,18,18,18,18,18,18,18,18,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      /* State 19 */ {-1,-1,-1,19,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
    }};
    // DFA stop states (0 indicates NOT a stop)
    static constexpr std::array<int, NUM_STATES> stop_id = {0,0,251,254,251,253,253,253,253,253,253,253,255,255,253,253,253,252,0,252};
    // TC-boundary stop values (non-zero = r1 boundary for a trailing-context token)
    static constexpr std::array<int, NUM_STATES> tc_stop_id = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    // TC-final values (non-zero = r2 accept state; backtrack lexeme to r1 boundary)
    static constexpr std::array<int, NUM_STATES> tc_final_id = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  public:
    constexpr static int SYMBOL_START = 2;     ///< Symbol to indicate a start of line.
    constexpr static int SYMBOL_STOP = 3;      ///< Symbol to indicate an end of line.
    constexpr static int SYMBOL_MIN_INPUT = 9; ///< Symbols below this are control symbols.

    static constexpr size_t size() { return 20; }
    static constexpr int GetStop(int state) {
      return (state >= 0) ? stop_id[static_cast<size_t>(state)] : 0;
    }
    static constexpr int GetTCStop(int state) {
      return (state >= 0) ? tc_stop_id[static_cast<size_t>(state)] : 0;
    }
    static constexpr int GetTCFinal(int state) {
      return (state >= 0) ? tc_final_id[static_cast<size_t>(state)] : 0;
    }
    static constexpr int GetNext(int state, int sym) {
      if (state < 0 || sym < 0 || sym >= NUM_SYMBOLS) return -1; // Invalid state or symbol.
      int next_state = table[static_cast<size_t>(state)][static_cast<size_t>(sym)];
      // If sym is an unused control symbol (line begin/end), keep old state.
      return (sym < SYMBOL_MIN_INPUT && next_state == -1) ? state : next_state;
    }
    static int GetNext(int state, const std::string & syms) {
      for (char x : syms) state = GetNext(state, x);
      return state;
    }
    static int Test(const std::string & str) {
      int state = GetNext(0, SYMBOL_START);
      state = GetNext(state, str);
      int eol_state = GetNext(state, SYMBOL_STOP);
      return std::max(GetStop(state), GetStop(eol_state));
    }
  };

  class Lexer {
  private:
    static constexpr int NUM_TOKENS=5;

    // -- Load State --
    size_t cur_line = 1;   // Track LINE we are reading in the input.
    size_t cur_col = 0;    // Track COLUMN we are reading in the input.
    int start_pos = 0;     // Track INDEX for the start of current lexeme.
    std::string lexeme{};  // Lexeme found for the current token

    // -- Process State --
    std::vector<Token> tokens{}; // Set of tokens loaded so far.
    size_t token_id = 0;                 // Next token to process.
    Token eof_token{0, "_EOF_", 0, 0};

  public:
    static constexpr int ID__EOF_ = 0;
    static constexpr int ID_OTHER = 251;            // Regex: .
    static constexpr int ID_VALUE = 252;            // Regex: [0-9]+(\.)[0-9]*
    static constexpr int ID_IDENTIFIER = 253;       // Regex: [a-zA-Z_][a-zA-Z0-9_]*
    static constexpr int ID_OPERATOR = 254;         // Regex: [+\-*/]
    static constexpr int ID_KEYWORD = 255;          // Regex: (SET)|(PRINT)

    // Return the name of a token given its ID.
    static std::string TokenName(int id) {
      switch (id) {
      case -1: return "_ERROR_";
      case 0: return "_EOF_";
      case ID_OTHER: return "OTHER";
      case ID_VALUE: return "VALUE";
      case ID_IDENTIFIER: return "IDENTIFIER";
      case ID_OPERATOR: return "OPERATOR";
      case ID_KEYWORD: return "KEYWORD";
      default:
        // Special characters get escaped versions of themselves.
        if (id == '\n') return "'\\n'";
        if (id == '\r') return "'\\r'";
        if (id == '\t') return "'\\t'";
        if (id == '\\') return "'\\\\'";
        // If ID is a visible character print it, otherwise provide ID.
        if (id > 0 && id < 128 && std::isprint(id)) {
          return std::string("'") + static_cast<char>(id) + "'";
        }
        return std::string("Token ID: ") + std::to_string(id);
      }; // End switch.
    }

    // Identify if a token should be skipped during tokenizing.
    static constexpr bool IgnoreToken(int id) {
      switch (id) {
      case 0:
        return true;
      default: return false;
      };
    }

    // Return the number of token types the lexer recognizes.
    static constexpr int GetNumTokens() { return NUM_TOKENS; }

    // Generate and return the next token from the input stream.
    Token NextToken(std::string_view in) {
      // If we cannot read in, return an "EOF" token.
      if (start_pos >= std::ssize(in)) return { 0, "", cur_line, cur_col };

      int cur_pos = start_pos;         // Position in the input that we are actively analyzing
      int best_pos = start_pos;        // Best look-ahead we've found so far
      int tc_boundary_pos = start_pos; // Position at end of r1 for trailing-context tokens
      int cur_state = 0;               // Next state for the DFA analysis
      int cur_stop = 0;                // Current "stop" state (or 0 if we can't stop here)
      int best_stop = -1;              // Best stop state found so far

      // If we are at the START OF A LINE, send a DFA::SYMBOL_START
      if (start_pos == 0 || in[start_pos-1] == '\n') {
        cur_state = DFA::GetNext(0, DFA::SYMBOL_START);
      }
      // Keep looking as long as:
      // 1: We may be able to continue the current lexeme, and
      // 2: We have not entered an invalid state, and
      // 3: Our input string has more symbols to provide
      while (cur_stop >= 0 && cur_state >= 0 && cur_pos < std::ssize(in)) {
        const char next_char = in[cur_pos++];
        if (next_char < 0) break; // Ignore invalid chars.
        cur_state = DFA::GetNext(cur_state, next_char);
        cur_stop = DFA::GetStop(cur_state);
        // Update trailing-context r1 boundary if we crossed one.
        if (DFA::GetTCStop(cur_state) > 0) { tc_boundary_pos = cur_pos; }
        // If we found a valid stopping point, account for trailing context.
        if (cur_stop > 0) {
          int tc_final = DFA::GetTCFinal(cur_state);
          bool use_tc = (tc_final == cur_stop) && (tc_boundary_pos > start_pos);
          best_pos = use_tc ? tc_boundary_pos : cur_pos;
          best_stop = cur_stop;
        }
        // Look ahead to see if we are at the END OF A LINE that can finish a token.
        if (cur_pos == std::ssize(in) || in[cur_pos] == '\n') {
          int eol_state = DFA::GetNext(cur_state, DFA::SYMBOL_STOP);
          int eol_stop = DFA::GetStop(eol_state);
          if (eol_stop > 0) {
            int eol_tc_final = DFA::GetTCFinal(eol_state);
            bool use_tc = (eol_tc_final == eol_stop) && (tc_boundary_pos > start_pos);
            best_pos = use_tc ? tc_boundary_pos : cur_pos;
            best_stop = eol_stop;
          }
        }
      }

      // If we did not find any options, peel off just one character and use it as id.
      if (best_pos == start_pos) {
        best_stop = static_cast<unsigned char>(in[start_pos]);
        if (best_stop >= 128) best_stop = -1;  // Out-of-range byte (e.g., UTF-8)
        ++best_pos;
      }

      lexeme = in.substr(start_pos, best_pos-start_pos);
      start_pos += std::ssize(lexeme);

      // Update the line number we are on.
      const size_t out_line = cur_line;
      const size_t out_col = cur_col;
      if ((cur_col = lexeme.rfind('\n')) == std::string::npos) { // No newlines
        cur_col = out_col + lexeme.size();
      } else {
        cur_col = lexeme.size() - cur_col - 1;
        cur_line += static_cast<size_t>(std::count(lexeme.begin(),lexeme.end(),'\n'));
      }

      // Return the token we found.
      return { best_stop, lexeme, out_line, out_col };
    }

    // Convert an input string into a vector of tokens.
    const std::vector<Token> & Tokenize(std::string_view in) {
      start_pos = 0; // Start processing at beginning of string.
      cur_line = 1;  // Start processing at the first line of the input.
      cur_col = 0;   // Start processing at the first position of the input.
      tokens.resize(0);
      token_id = 0;
      while (Token token = NextToken(in)) {
        if (!IgnoreToken(token.id)) tokens.push_back(token);
      }
      eof_token.line_id = cur_line;
      return tokens;
    }

    // Convert an input stream to a string, then tokenize.
    const std::vector<Token> & Tokenize(std::istream & is) {
      return Tokenize(
        std::string(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>())
      );
    }

    // === Functions for Using Tokens ===

    // Write an error message for invalid tokens and terminate the program.
    template <typename... Ts>
    void Error(Ts... message) {
      std::cerr << "ERROR (line " << Peek().line_id << ":" << Peek().col_id <<  "): ";
      (std::cerr << ... << std::forward<Ts>(message)) << std::endl;
      exit(1);
    }

    // Test if there are ANY tokens remaining.
    bool Any() const { return token_id < tokens.size(); }

    // Test if there are NO tokens remaining.
    bool None() const { return token_id >= tokens.size(); }

    // Test if the current token is a specific type.
    bool Is(int id) const { return Any() && Peek() == id; }

    // Get the current (or upcoming) token, but don't remove it from the queue.
    const Token & Peek(size_t skip_count=0) const {
      if (token_id + skip_count >= tokens.size()) return eof_token;
      return tokens[token_id + skip_count];
    }

    // Get the current token, removing it from the queue.
    const Token & Use() {
      if (None()) return eof_token;
      return tokens[token_id++];
    }

    // Use the current token if it is the expected type; otherwise error.
    // (Use provided error if available, otherwise use default error)
    template <typename... Ts>
    const Token & Use(int id, Ts &&... message) {
      if (!Is(id)) {
        if constexpr (sizeof...(Ts) == 0) {
          Error( "Expected token of type ", TokenName(id),
                 ", but found type ", TokenName(Peek()));
        }
        else Error( std::forward<Ts>(message)...);
      }
      return Use();
    }

    // If the current token is one of the provided ids, use it an return the ID used.
    // Otherwise, don't use it and return 0.
    template <typename... Ts>
    int UseIf(int id, Ts... args) {
      if (Is(id)) { return Use(); }
      return UseIf(args...);
    }

    // Base case for UseIf
    int UseIf() { return 0; }

    // Rewind one or more tokens.
    void Rewind(size_t steps=1) {
      if (token_id >= steps) token_id -= steps;
      else token_id = 0;
    }
  };
} // End of namespace emplex
#endif // #ifndef EMPLEX_LEXER_HPP_INCLUDE_
