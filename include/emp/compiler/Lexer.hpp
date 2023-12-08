/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2016-2023.
*/
/**
 *  @file
 *  @brief A general-purpose, fast lexer.
 *  @note Status: BETA
 *
 *  Build a lexer that can convert input strings or streams into a series of provided tokens.
 *
 *  Use AddToken(name, regex) to list out the relevant tokens.
 *   'name' is the unique name for this token.
 *   'regex' is the regular expression that describes this token.
 *  It will return a unique ID associated with this lexeme.
 *
 *  IgnoreToken(name, regex) uses the same arguments, but is used for tokens that
 *  should be skipped over.
 *
 *  Names and IDs can be recovered later using GetTokenID(name) and GetTokenName(id).
 *
 *  Tokens can be retrieved either one at a time with Process(string) or Process(stream),
 *  which will return the next (non-ignored) token, removing it from the input.
 *
 *  Alternatively, an entire series of tokens can be processed with Tokenize().
 *
 *  Finally, GetLexeme() can be used to retrieve the lexeme from the most recent token found.
 */

#ifndef EMP_COMPILER_LEXER_HPP_INCLUDE
#define EMP_COMPILER_LEXER_HPP_INCLUDE

#include <cstdint>
#include <iostream>
#include <map>
#include <stddef.h>
#include <string>

#include "../base/map.hpp"
#include "../base/vector.hpp"
#include "../datastructs/map_utils.hpp"

#include "lexer_utils.hpp"
#include "RegEx.hpp"

namespace emp {

  /// Information about an individual TYPE of token to be processed within a Lexer.
  struct TokenInfo {
    std::string name;    ///< Name of this token type.
    std::string desc;    ///< More detailed description of this token type.
    RegEx regex;         ///< Pattern to describe token type.
    int id;              ///< Unique id for token.
    bool save_lexeme;    ///< Preserve the lexeme for this token?
    bool save_token;     ///< Keep token at all? (Whitespace and comments are often discarded).

    // Default constructor produces an error token.
    TokenInfo() : name(""), desc("Unable to parse input!"), regex(""),
                  id(-1), save_lexeme(true), save_token(true) { }
    TokenInfo(const std::string & _name, const std::string & _regex, int _id,
              bool _save_l=true, bool _save_t=true, const std::string & _desc="")
      : name(_name), desc(_desc), regex(_regex), id(_id), save_lexeme(_save_l), save_token(_save_t) { }
    TokenInfo(const TokenInfo &) = default;
    TokenInfo(TokenInfo &&) = default;
    TokenInfo & operator=(const TokenInfo &) = default;
    TokenInfo & operator=(TokenInfo &&) = default;

    /// Print out the status of this token (for debugging)
    void Print(std::ostream & os=std::cout) const {
      os << "Name:" << name
         << "  RegEx:" << regex.AsString()
         << "  ID:" << id
         << "  save_lexeme:" << save_lexeme
         << "  save_token:" << save_token
         << std::endl;
    }
  };

  /// Information about a token instance from an input stream.
  struct Token {
    int id;              ///< Which type of token is this?
    std::string lexeme;  ///< Sequence matched by this token (or empty if not saved)
    size_t line_id;      ///< Which line did this token start on?

    Token(int _id, const std::string & str="", size_t _line=0)
      : id(_id), lexeme(str), line_id(_line) { ; }
    Token(const Token &) = default;
    Token(Token &&) = default;
    Token & operator=(const Token &) = default;
    Token & operator=(Token &&) = default;

    /// Token will automatically convert to its ID if used as an int.
    operator int() const { return id; }

    /// Token will automatically convert to its matched sequence (lexeme) is used as a string.
    operator const std::string &() const { return lexeme; }
  };

  class TokenStream {
  private:
    std::string name = "";
    emp::vector<Token> tokens;

  public:
    TokenStream(const std::string & in_name) : name(in_name) { }
    TokenStream(const TokenStream &) = default;
    TokenStream(TokenStream &&) = default;
    TokenStream(const emp::vector<Token> & in_tokens, const std::string & in_name)
    : name(in_name), tokens(in_tokens) { }

    TokenStream & operator=(const TokenStream &) = default;
    TokenStream & operator=(TokenStream &&) = default;

    class Iterator {
    private:
      emp::Ptr<const TokenStream> ts;
      size_t pos;

    public:
      Iterator(const Iterator &) = default;
      Iterator(const TokenStream & in_ts, size_t in_pos) : ts(&in_ts), pos(in_pos) { }
      Iterator & operator=(const Iterator &) = default;

      const TokenStream & GetTokenStream() const { return *ts; }
      size_t GetIndex() const { return pos; }
      emp::Ptr<const Token> ToPtr() const { return ts->GetPtr(pos); }

      Token operator*() const { return ts->tokens[pos]; }
      const Token * operator->() const { return &(ts->tokens[pos]); }

      bool operator==(const Iterator & in) const { return ToPtr() == in.ToPtr(); }
      bool operator!=(const Iterator & in) const { return ToPtr() != in.ToPtr(); }
      bool operator< (const Iterator & in) const { return ToPtr() <  in.ToPtr(); }
      bool operator<=(const Iterator & in) const { return ToPtr() <= in.ToPtr(); }
      bool operator> (const Iterator & in) const { return ToPtr() >  in.ToPtr(); }
      bool operator>=(const Iterator & in) const { return ToPtr() >= in.ToPtr(); }

      Iterator & operator++() { ++pos; return *this; }
      Iterator operator++(int) { Iterator old(*this); ++pos; return old; }
      Iterator & operator--() { --pos; return *this; }
      Iterator operator--(int) { Iterator old(*this); --pos; return old; }

      bool IsValid() const { return pos < ts->size(); }
      bool AtEnd() const { return pos == ts->size(); }

      operator bool() const { return IsValid(); }
    };

    size_t size() const { return tokens.size(); }
    const Token & Get(size_t pos) const { return tokens[pos]; }
    emp::Ptr<const Token> GetPtr(size_t pos) const { return &(tokens.data()[pos]); }
    const std::string & GetName() const { return name; }
    Iterator begin() const { return Iterator(*this, 0); }
    Iterator end() const { return Iterator(*this, tokens.size()); }
    const Token & back() const { return tokens.back(); }

    void push_back(const Token & in) { tokens.push_back(in); }

    void Print(std::ostream & os=std::cout) const {
      for (auto x : tokens) {
        os << " [" << x.lexeme << "]";
      }
      os << std::endl;
    }
  };


  /// A lexer with a set of token types (and associated regular expressions)
  class Lexer {
  private:
    static constexpr int MAX_ID = 255;      ///< IDs count down so that first ones have priority.
    static constexpr int ERROR_ID = -1;     ///< Code for unknown token ID.

    emp::vector<TokenInfo> token_set;       ///< List of all active tokens types.
    emp::map<std::string, int> token_map;   ///< Map of token names to id.
    int cur_token_id = MAX_ID;              ///< Which ID should the next new token get?
    mutable bool generate_lexer = false;    ///< Do we need to regenerate the lexer?
    mutable DFA lexer_dfa;                  ///< Table driven lexer implementation.
    mutable std::string lexeme;             ///< Current state of lexeme being generated.

    static const TokenInfo & ERROR_TOKEN() {
      static const TokenInfo token;
      return token;
    }

  public:
    Lexer() = default;
    Lexer(const Lexer &) = default;
    Lexer(Lexer &&) = default;
    ~Lexer() = default;

    Lexer & operator=(const Lexer &) = default;
    Lexer & operator=(Lexer &&) = default;

    /// How many types of tokens can be identified in this Lexer?
    size_t GetNumTokens() const { return token_set.size(); }

    void Reset() {
      token_set.resize(0);
      token_map.clear();
      cur_token_id = MAX_ID;
      generate_lexer = false;
    }

    bool TokenOK(int id) const { return id >= 0 && id < cur_token_id; }

    /// Add a new token, specified by a name and the regex used to identify it.
    /// Note that token ids count down with highest IDs having priority.
    int AddToken(const std::string & name,
                 const std::string & regex,
                 bool save_lexeme = true,
                 bool save_token = true,
                 const std::string & desc = "")
    {
      int id = cur_token_id--;                // Grab the next available token id.
      generate_lexer = true;                  // Indicate the the lexer DFA needs to be rebuilt.
      token_set.emplace_back( name, regex, id, save_lexeme, save_token, desc );
      token_map[name] = id;
      return id;
    }

    /// Add a token to ignore, specified by a name and the regex used to identify it.
    int IgnoreToken(const std::string & name, const std::string & regex, const std::string & desc="") {
      int id = cur_token_id--;                // Grab the next available token id.
      generate_lexer = true;                  // Indicate the the lexer DFA needs to be rebuilt.
      token_set.emplace_back( name, regex, id, false, false, desc );
      token_map[name] = id;
      return id;
    }

    /// Get the ID associated with a token type (you provide the token name)
    int GetTokenID(const std::string & name) const {
      int default_id = ERROR_ID;
      if (name.size() == 1) default_id = (int) name[0];
      return emp::Find(token_map, name, default_id);
    }

    /// Get the full information about a token (you provide the id)
    const TokenInfo & GetTokenInfo(int id) const {
      if (id > MAX_ID || id <= cur_token_id) return ERROR_TOKEN();
      return token_set[(size_t)(MAX_ID - id)];
    }

    /// Get the name associated with a token type (you provide the ID)
    std::string GetTokenName(int id) const {
      if (id < 0) return emp::to_string("Error (", id, ")");   // Negative tokens specify errors.
      if (id == 0) return "EOF";                               // Token zero is end-of-file.
      if (id < 128) return emp::to_escaped_string((char) id);  // Individual characters.
      if (id <= cur_token_id) return emp::to_string("Error (", id, ")"); // Invalid token range!
      return GetTokenInfo(id).name;                            // User-defined tokens.
    }

    /// Identify if a token should be saved.
    bool GetSaveToken(int id) const {
      if (id > MAX_ID || id <= cur_token_id) return true;
      return GetTokenInfo(id).save_token;
    }

    /// Create the NFA that will identify the current set of tokens in a sequence.
    void Generate() const {
      NFA lexer_nfa;
      for (const auto & t : token_set) {
        lexer_nfa.Merge( to_NFA(t.regex, (uint8_t) t.id) );
      }
      generate_lexer = false; // We just generated it!  Don't again until another change is made.
      lexer_dfa = to_DFA(lexer_nfa);
    }

    /// Get the next token found in an input stream.  Do so by examining one character at a time.
    /// Keep going as long as there is a chance of a valid lexeme (since we want to choose the
    /// longest one we can find.)  Every time we do hit a valid lexeme, store it as the current
    /// "best" and keep going.  Once we hit a point where no other valid lexemes are possible,
    /// stop and return the best we've found so far.
    Token Process(std::istream & is) const {
      // If we still need to generate the DFA for the lexer, do so.
      if (generate_lexer) Generate();

      size_t cur_pos = 0;   // What position in the file are we actively analyzing?
      size_t best_pos = 0;  // What is the best look-ahead we've found so far?
      int cur_state = 0;    // What is the next state for the DFA analysis?
      int cur_stop = 0;     // What is the current "stop" state (if we can stop here)
      int best_stop = -1;   // What is the best stop state found so far?

      lexeme.resize(0);

      // Keep looking as long as:
      // 1: We may still be able to continue the current lexeme.
      // 2: We have not entered an invalid state.
      // 3: Our input stream has more symbols.
      while (cur_stop >= 0 && cur_state >= 0 && is) {
        const char next_char = (char) is.get();
        cur_pos++;
        cur_state = lexer_dfa.Next(cur_state, (size_t) next_char);
        cur_stop = lexer_dfa.GetStop(cur_state);
        if (cur_stop > 0) { best_pos = cur_pos; best_stop = cur_stop; }
        lexeme.push_back( next_char );
      }

      // If best_pos < cur_pos, we need to rewind the input stream and adjust the lexeme.
      if (best_pos > 0 && best_pos < cur_pos) {
        lexeme.resize(best_pos);
        while (best_pos < cur_pos) { is.unget(); cur_pos--; }
      }

      // If we are at the end of this input stream and still haven't found a token, return 0.
      if (best_stop < 0) {
        if (!is) return { 0, "" };
        return { ERROR_ID, lexeme };
      }

      // Otherwise return the best token we've found so far.
      return { best_stop, lexeme };
    }

    /// Shortcut to process a string rather than a stream, chopping off one token each time.
    Token Process(std::string & in_str) const {
      std::stringstream ss;
      ss << in_str;
      auto out_val = Process(ss);
      in_str = ss.str();
      return out_val;
    }

    /// Shortcut to just get a single token.
    Token ToToken(std::string_view in_str) const {
      std::stringstream ss;
      ss << in_str;
      auto out_val = Process(ss);
      return out_val;
    }

    /// Turn an input stream of text into a vector of tokens.
    TokenStream Tokenize(std::istream & is, const std::string & name="in_stream") const {
      emp::vector<Token> out_tokens;
      size_t cur_line = 1;
      emp::Token token = Process(is);
      while (token > 0) {
        token.line_id = cur_line;
        cur_line += emp::count(token.lexeme, '\n');
        if (GetSaveToken(token)) out_tokens.push_back(token);
        token = Process(is);
      }
      return TokenStream{out_tokens, name};
    }

    /// Turn an input string into a vector of tokens.
    TokenStream Tokenize(std::string_view str, const std::string & name="in_view") const {
      std::stringstream ss;
      ss << str;
      return Tokenize(ss, name);
    }

    /// Turn a vector of strings into a vector of tokens.
    TokenStream Tokenize(const emp::vector<std::string> & str_v,
                         const std::string & name="in_string vector") const
    {
      std::stringstream ss;
      for (auto & str : str_v) {
        ss << str << '\n';
      }

      return Tokenize(ss, name);
    }

    /// Get the lexeme associated with the last token identified.
    const std::string & GetLexeme() const { return lexeme; }

    /// Print the full information about this lexer (for debugging)
    void Print(std::ostream & os=std::cout) const {
      for (const auto & t : token_set) t.Print(os);
      if (generate_lexer) Generate();               // Do we need to regenerate the lexer?
      lexer_dfa.Print(os);                          // Table driven lexer implementation.
    }

    /// Try out the lexer on a string and demonstrate how it's tokenized.
    void DebugString(std::string test_string) const {
      std::stringstream ss;
      ss << test_string;

      emp::Token token = 1;
      while (token > 0) {
        token = Process(ss);
        std::cout << GetTokenName(token) << " : \"" << token.lexeme << "\"" << std::endl;
      }
    }

    void DebugToken(int token_id) const {
      const auto & token = GetTokenInfo(token_id);
      std::cout << "Debugging token " << token.id << ": '" << token.name << "'.\n"
                << "  Desc: " << token.desc << "\n";
      token.regex.PrintDebug();
    }
  };


}

#endif // #ifndef EMP_COMPILER_LEXER_HPP_INCLUDE
