/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2016-2024.
*/
/**
 *  @file
 *  @brief Basic regular expression handler.
 *  @note Status: BETA
 *
 *  A fully (well, mostly) functional regular expression processor.
 *
 *  Special chars:
 *   '|'          - or
 *   '*'          - zero or more of previous
 *   '+'          - one or more of previous
 *   '?'          - previous is optional
 *   '.'          - Match any character except \n
 *
 *  Plus the following group contents (and change may translation rules)
 *   '(' and ')'  - group contents
 *   '"'          - Ignore special characters in contents (quotes still need to be escaped)
 *   '[' and ']'  - character set -- choose ONE character
 *                  '^' as first char negates contents
 *                  '-' indicates range UNLESS first or last.
 *
 *  Additional overloads for functions in lexer_utils.h:
 *
 *    static NFA to_NFA(const RegEx & regex, int stop_id=1);
 *    static DFA to_DFA(const RegEx & regex);
 *
 *
 *  @todo Implement  ^ and $ (beginning and end of line)
 *  @todo Implement {n}, {n,} and {n,m} (exactly n, at least n, and n-m copies, respectively)
 *  @todo Consider a separator (maybe backtick?) to divide up a regex expression;
 *        the result can be returned by each section as a vector of strings.
 */

#ifndef EMP_COMPILER_REGEX_HPP_INCLUDE
#define EMP_COMPILER_REGEX_HPP_INCLUDE


#include <ostream>
#include <sstream>
#include <stddef.h>
#include <string>

#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../bits/BitSet.hpp"
#include "../tools/String.hpp"

#include "lexer_utils.hpp"
#include "NFA.hpp"

namespace emp {

  /// A basic regular expression handler.
  class RegEx {
  private:
    constexpr static size_t NUM_SYMBOLS = 128; ///< Maximum number of symbol the RegEx can handle.
    using opts_t = BitSet<NUM_SYMBOLS>;
    emp::String regex;                         ///< Original string to define this RegEx.
    emp::vector<emp::String> notes;            ///< Any warnings or errors would be provided here.
    bool valid = true;                         ///< Set to false if regex cannot be processed.
    size_t pos = 0;                            ///< Position being read in regex.

    mutable DFA dfa;                           ///< DFA that this RegEx translates to.
    mutable bool dfa_ready = false;            ///< Is the dfa ready? (or does it need to be generated?)

    struct RepeatInfo {
      int min_repeat = 1;
      int max_repeat = 1;
    };

    template <typename... T>
    void Error(T &&... args) {
      notes.push_back(emp::MakeString(std::forward<T>(args)...));
      valid = false;
    }

    // Pre-declarations
    struct re_block;
    struct re_charset;
    struct re_parent;
    struct re_string;

    /// Internal base representation of a portion of a regex
    struct re_base {                     // Also used for empty regex
      virtual ~re_base() { ; }
      virtual void Print(std::ostream & os) const { os << "[]"; }
      virtual Ptr<re_block> AsBlock() { return nullptr; }
      virtual Ptr<re_charset> AsCharSet() { return nullptr; }
      virtual Ptr<re_parent> AsParent() { return nullptr; }
      virtual Ptr<re_string> AsString() { return nullptr; }
      virtual size_t GetSize() const { return 0; }
      virtual bool Simplify() { return false; }
      virtual void AddToNFA(NFA & nfa, size_t start, size_t stop) const { nfa.AddFreeTransition(start, stop); }
    };

    /// Representation of strings stored in a RegEx.
    struct re_string : public re_base {  // Series of specific chars
      emp::String str{};
      re_string() = default;
      re_string(char c) { str.push_back(c); }
      re_string(const emp::String & s) : str(s) { ; }
      void Print(std::ostream & os) const override { os << "STR[" << str.AsEscaped() << "]"; }
      Ptr<re_string> AsString() override { return ToPtr(this); }
      size_t GetSize() const override { return str.size(); }
      virtual void AddToNFA(NFA & nfa, size_t start, size_t stop) const override {
        size_t prev_id = start;
        for (char x : str) {
          size_t next_id = nfa.AddNewState();
          nfa.AddTransition(prev_id, next_id, (size_t) x);
          prev_id = next_id;
        }
        nfa.AddFreeTransition(prev_id, stop);
      }
    };

    /// Representation of a character set e.g., [abc]
    struct re_charset : public re_base { // Any char from set.
      opts_t char_set;
      re_charset() : char_set() { ; }
      re_charset(char x, bool neg=false) : char_set() {
        char_set[(size_t)x]=true;
        if (neg) char_set.NOT_SELF();
      }
      re_charset(const emp::String & s, bool neg=false) : char_set() {
        for (char x : s) char_set[(size_t)x]=true;
        if (neg) char_set.NOT_SELF();
      }
      void Print(std::ostream & os) const override {
        auto chars = char_set.GetOnes();
        bool use_not = false;
        if (chars.size() > 64) { chars = (~char_set).GetOnes(); use_not = true; }
        os << "SET[";
        if (use_not) os << "NOT ";
        for (auto c : chars) os << MakeEscaped((char) c);
        os << "]";
      }
      Ptr<re_charset> AsCharSet() override { return ToPtr(this); }
      size_t GetSize() const override { return char_set.CountOnes(); }
      char First() const { return (char) char_set.FindOne(); }
      virtual void AddToNFA(NFA & nfa, size_t start, size_t stop) const override {
        for (size_t i = 0; i < NUM_SYMBOLS; i++) if (char_set[i]) nfa.AddTransition(start, stop, i);
      }
    };

    /// Intermediate base class for RegEx components that have children (such as "and" and "or")
    struct re_parent : public re_base {
    protected:
      emp::vector<Ptr<re_base>> nodes;
    public:
      re_parent() = default;
      ~re_parent() { for (auto x : nodes) x.Delete(); }
      void Clear() { for (auto x : nodes) x.Delete(); nodes.resize(0); }
      virtual void push(Ptr<re_base> x) { emp_assert(x != nullptr); nodes.push_back(x); }
      Ptr<re_base> pop() { auto out = nodes.back(); nodes.pop_back(); return out; }
      size_t GetSize() const override { return nodes.size(); }
      Ptr<re_parent> AsParent() override { return ToPtr(this); }

      bool Simplify() override {
        bool m=false;
        for (auto & x : nodes) {
          // Recursively simplify children.
          m |= x->Simplify();
          // A block with one child can be replaced by child.
          if (x->AsBlock() && x->GetSize() == 1) {
            auto child = x->AsParent()->nodes[0];
            x->AsParent()->nodes.resize(0);
            x.Delete();
            x = child;
            m = true;
          }
        }
        return m;
      }
    };

    /// Representation of a series of components...
    struct re_block : public re_parent {   // Series of re's
      void Print(std::ostream & os) const override {
        os << "BLOCK[";
        for (size_t i = 0; i < nodes.size(); i++) {
          if (i > 0) os << " ";
          nodes[i]->Print(os);
        }
        os << "]";
      }
      Ptr<re_block> AsBlock() override { return ToPtr(this); }
      bool Simplify() override {
        bool modify = false;
        // Loop through block elements, simplifying when possible.
        for (size_t i = 0; i < nodes.size(); i++) {
          // If node is a charset with one option, replace it with a string.
          if (nodes[i]->AsCharSet() && nodes[i]->GetSize() == 1) {
            auto new_node = NewPtr<re_string>(nodes[i]->AsCharSet()->First());
            nodes[i].Delete();
            nodes[i] = new_node;
            modify = true;
          }

          // If two neighboring nodes are strings, merge them.
          if (i > 0 && nodes[i]->AsString() && nodes[i-1]->AsString()) {
            nodes[i-1]->AsString()->str += nodes[i]->AsString()->str;
            nodes[i].Delete();
            nodes.erase(nodes.begin() + (int) i);
            i--;
            modify = true;
            continue;
          }

          // If blocks are nested, merge them into a single block.
          if (nodes[i]->AsBlock()) {
            auto old_node = nodes[i]->AsBlock();    // Save the old node for merging.
            nodes.erase(nodes.begin() + (int) i);   // Remove block from nodes.
            if (old_node->nodes.size()) {
              nodes.insert(nodes.begin() + (int) i, old_node->nodes.begin(), old_node->nodes.end());
              old_node->nodes.resize(0);  // Don't recurse delete since nodes were moved!
            }
            old_node.Delete();
            i--;
            modify = true;
            continue;
          }
        }

        // Also run the default Simplify on nodes.
        modify |= re_parent::Simplify();

        return modify;
      }
      virtual void AddToNFA(NFA & nfa, size_t start, size_t stop) const override {
        size_t prev_id = start;
        for (auto x : nodes) {
          size_t next_id = nfa.AddNewState();
          x->AddToNFA(nfa, prev_id, next_id);
          prev_id = next_id;
        }
        nfa.AddFreeTransition(prev_id, stop);
      }
    };

    /// Representation of two options in a regex, e.g., a|b
    struct re_or : public re_parent {      // lhs -or- rhs
      re_or(Ptr<re_base> l, Ptr<re_base> r) { push(l); push(r); }
      void Print(std::ostream & os) const override {
        os << "|[";
        nodes[0]->Print(os);
        os << ",";
        nodes[1]->Print(os);
        os << "]";
      }
      virtual void AddToNFA(NFA & nfa, size_t start, size_t stop) const override {
        nodes[0]->AddToNFA(nfa, start, stop);
        nodes[1]->AddToNFA(nfa, start, stop);
      }
    };

    /// Representations of zero-or-more instances of a component.  e.g., a*
    struct re_star : public re_parent {    // zero-or-more
      re_star(Ptr<re_base> c) { push(c); }
      void Print(std::ostream & os) const override { os << "*["; nodes[0]->Print(os); os << "]"; }

      virtual void AddToNFA(NFA & nfa, size_t start, size_t stop) const override {
        const size_t origin = nfa.AddNewState();
        const size_t target = nfa.AddNewState();
        nodes[0]->AddToNFA(nfa, origin, target);
        nfa.AddFreeTransition(start, origin);
        nfa.AddFreeTransition(target, origin);
        nfa.AddFreeTransition(origin, stop);
      }
    };

    /// Representations of one-or-more instances of a component.  e.g., a+
    struct re_plus : public re_parent {    // one-or-more
      re_plus(Ptr<re_base> c) { push(c); }
      void Print(std::ostream & os) const override { os << "+["; nodes[0]->Print(os); os << "]"; }
      virtual void AddToNFA(NFA & nfa, size_t start, size_t stop) const override {
        const size_t origin = nfa.AddNewState();
        const size_t target = nfa.AddNewState();
        nodes[0]->AddToNFA(nfa, origin, target);
        // From the target, can either go back to origin and repeat, or straight to stop.
        nfa.AddFreeTransition(start, origin);
        nfa.AddFreeTransition(target, origin);
        nfa.AddFreeTransition(target, stop);
      }
    };

    /// Representations of zero-or-one instances of a component.  e.g., a?
    struct re_qm : public re_parent {      // zero-or-one
      re_qm(Ptr<re_base> c) { push(c); }
      void Print(std::ostream & os) const override { os << "?["; nodes[0]->Print(os); os << "]"; }
      virtual void AddToNFA(NFA & nfa, size_t start, size_t stop) const override {
        nodes[0]->AddToNFA(nfa, start, stop);
        nfa.AddFreeTransition(start, stop);
      }
    };

    /// Representations of specified number of instances of a component.  e.g., a{m,n}
    struct re_repeat : public re_parent {      // From m to n times.
      RepeatInfo repeat;
      re_repeat(Ptr<re_base> c, RepeatInfo in_repeat) : repeat(in_repeat) { push(c); }
      void Print(std::ostream & os) const override {
        os << "{" << repeat.min_repeat << "," << repeat.max_repeat << "}[";
        nodes[0]->Print(os);
        os << "]"; }
      virtual void AddToNFA(NFA & nfa, size_t start, size_t stop) const override {
        size_t state1 = nfa.AddNewState();
        nfa.AddFreeTransition(start, state1);

        // Start by making REQUIRED transitions.
        for (int i = 0; i < repeat.min_repeat; ++i) {
          size_t state2 = nfa.AddNewState();
          nodes[0]->AddToNFA(nfa, state1, state2);
          state1 = state2;
        }

        // If we are allowed to have any number of additional transitions, do so.
        if (repeat.max_repeat == -1) {
          size_t state2 = nfa.AddNewState();
          nodes[0]->AddToNFA(nfa, state1, state2);
          nfa.AddFreeTransition(state1, state2);    // Allow skipping over.
          nfa.AddFreeTransition(state2, state1);    // Allow repeating.
          state1 = state2;
        }

        // Otherwise allow for specific count of additional transitions.
        else {
          int opt_count = repeat.max_repeat - repeat.min_repeat;
          for (int i = 0; i < opt_count; ++i) {
            size_t state2 = nfa.AddNewState();
            nodes[0]->AddToNFA(nfa, state1, state2);
            nfa.AddFreeTransition(state1, state2);    // Allow skipping over.
            state1 = state2;
          }
        }

        nfa.AddFreeTransition(state1, stop);
      }
    };

    emp::Ptr<re_parent> head_ptr = nullptr;

    /// Make sure that there is another element in the RegEx (e.g., that '[' is followed by ']') or else
    /// trigger an error to report the problem.
    bool EnsureNext(char x) {
      if (pos >= regex.size()) Error("Expected ", x, " before end.");
      else if (regex[pos] != x) Error("Expected ", x, " at position ", pos,
                                      "; found ", regex[pos], ".");
      ++pos;               // We have what we were expecting!  Move on...
      return valid;
    }

    /// Construct a character range.
    Ptr<re_charset> ConstructSet() {
      char c = regex[pos++];
      bool neg = false;
      if (c == '^') { neg = true; c = regex[pos++]; }
      auto out = NewPtr<re_charset>();
      char prev_c = -1;
      while (c != ']' && pos < regex.size()) {
        // Hyphens indicate a range UNLESS they are the first character in the set.
        if (c == '-' && prev_c != -1) {
          c = regex[pos++];
          if (c < prev_c) { Error("Invalid character range ", prev_c, '-', c); continue; }
          for (char x = prev_c; x <= c; x++) {
            out->char_set[(size_t)x] = true;
          }
          prev_c = -1;
          c = regex[pos++];
          continue;
        }
        // Sets need to have certain escape characters identified.
        else if (c == '\\') {
          c = regex[pos++];  // Identify the specific escape char.
          char c2, c3;       // In case they are needed.
          switch(c) {
            // Escape sequences
            case 'f': c = '\f'; break;
            case 'n': c = '\n'; break;
            case 'r': c = '\r'; break;
            case 't': c = '\t'; break;
            case 'v': c = '\v'; break;

            // A backslash followed by a digit indicates we should expect an ascii code.
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
              if (pos+3 >= regex.size()) { Error("Escaped ascii codes must have three digits!"); }
              c2 = regex[pos+1];
              if (!is_digit(c2)) { Error("Escaped ascii codes must have three digits!"); }
              c3 = regex[pos+2];
              if (!is_digit(c3)) { Error("Escaped ascii codes must have three digits!"); }
              c -= '0';  c2 -= '0'; c3 -= '0';   // Find actual digit values.
              if (c > 1) { Error("Escaped ascii codes must be in range 0-127!"); }
              if (c == 1 && c2 > 2) { Error("Escaped ascii codes must be in range 0-127!"); }
              if (c == 1 && c2 == 2 && c3 > 7) { Error("Escaped ascii codes must be in range 0-127!"); }
              c = c*100 + c2*10 + c3;
              pos += 3;
              break;
            // Any of these characters should just be themselves; escaping may be only way to get them.
            case '-':
            case '\\':
            case ']':
            case '[':  // Technically not needed. 
            case '^':

            // These technically don't need to be escaped, but any symbol should be allowed to be escaped.
            case '!':  case '\"':  case '#':  case '$':  case '%':  case '&':  case '\'':
            case '(':  case ')':   case '*':  case '+':  case ',':  case '.':  case '/':
            case ':':  case ';':   case '<':  case '=':  case '>':  case '?':  case '@':
            case '_':  case '`':   case '{':  case '|':  case '}':  case '~':
              break;
            default:  // Give error for other characters, but use them directly.
              Error("Unknown escape char for char set: '\\", c, "'; using directly.");
          }
        }
        out->char_set[(size_t)c] = true;
        prev_c = c;
        c = regex[pos++];
      }
      if (neg) out->char_set.NOT_SELF();
      if (c == ']') --pos;  // SHOULD be the case, but is checked after return.
      return out;
    }

    /// Construct a string, loading everything needed.
    Ptr<re_string> ConstructString() {
      char c = regex[pos++];
      auto out = NewPtr<re_string>();
      while (c != '\"' && pos < regex.size()) {
        // @CAO Error if we run out of chars before close '"'
        if (c == '\\') {
          c = regex[pos++];  // Identify the specific escape char.
          switch(c) {
            case 'n': c = '\n'; break;
            case 'r': c = '\r'; break;
            case 't': c = '\t'; break;
            // Any of these characters should just be themselves!
            case '\"':
            case '\\':
              break;
            default:
              Error("Unknown escape char for literal string: '\\", c, "'.");
          }
        }
        out->str.push_back(c);
        c = regex[pos++];
      }
      if (c == '\"') --pos;

      return out;
    }

    /// Should only be called when we know we have a single unit to produce.  Build and return it.
    Ptr<re_base> ConstructSegment() {
      Ptr<re_base> result;
      char c = regex[pos++];  // Grab the current character and move pos to next.
      switch (c) {
        case '.':
          result = NewPtr<re_charset>('\n', true);  // Anything except newline.
          break;
        case '(':
          result = Process();         // Process the internal contents of parens.
          EnsureNext(')');            // Make sure last char is a paren and advance.
          break;
        case '[':
          result = ConstructSet();    // Build the inside of the set.
          EnsureNext(']');            // Make sure last char is a close-bracket and advance.
          break;
        case '"':
          result = ConstructString(); // Build the inside of the string.
          EnsureNext('"');            // Make sure last char is a quote and advance.
          break;
        case '\\':
          c = regex[pos++];  // Identify the specific escape char.
          switch(c) {
            // Shortcuts for character sets.
            case 'd': result = NewPtr<re_charset>("0123456789"); break;
            case 'D': result = NewPtr<re_charset>("0123456789", true); break;
            case 'l': result = NewPtr<re_charset>("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"); break;
            case 'L': result = NewPtr<re_charset>("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", true); break;
            case 's': result = NewPtr<re_charset>(" \f\n\r\t\v"); break;
            case 'S': result = NewPtr<re_charset>(" \f\n\r\t\v", true); break;
            case 'w': result = NewPtr<re_charset>("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"); break;
            case 'W': result = NewPtr<re_charset>("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_", true); break;
            
            // Special escape sequences
            case 'f': result = NewPtr<re_string>('\f'); break;
            case 'n': result = NewPtr<re_string>('\n'); break;
            case 'r': result = NewPtr<re_string>('\r'); break;
            case 't': result = NewPtr<re_string>('\t'); break;
            case 'v': result = NewPtr<re_string>('\v'); break;

            // Any of these characters should just be themselves!
            case '!':  case '\"':  case '#':   case '$':  case '%':  case '&':  case '\'':
            case '(':  case ')':   case '*':   case '+':  case ',':  case '-':  case '.':
            case '/':  case ':':   case ';':   case '<':  case '=':  case '>':  case '?':
            case '@':  case '[':   case '\\':  case ']':  case '^':  case '_':  case '`':
            case '{':  case '|':   case '}':   case '~':
              result = NewPtr<re_string>(c);
              break;
            default:
              Error("Unknown escape char: '\\", c, "'; ignoring backslash.");
              result = NewPtr<re_string>(c);
          }
          break;

        // Error cases
        case '|':
        case '*':
        case '+':
        case '?':
        case ')':
          Error("Expected regex segment but got '", c, "' at position ", pos, ".");
          result = NewPtr<re_string>(c);
          break;

        default:
          // Take this character directly.
          result = NewPtr<re_string>(c);
      }

      emp_assert(result != nullptr);
      return result;
    }

    // Read the body of an {m,n} style repeat, advancing pos past it.
    RepeatInfo ReadRepeat() {
      int min_repeat = regex.ScanAsInt(pos);
      int max_repeat = min_repeat;
      if (pos < regex.size() && regex[pos] == ',') {
        ++pos;
        if (pos < regex.size() && regex[pos] == '}') max_repeat = -1;
        else max_repeat = regex.ScanAsInt(pos);
      }
      if (pos >= regex.size()) {
        Error("Expected close brace ('}') at end of repeat.");
      } else if (regex[pos] != '}') {
        Error("Unexpected '", regex[pos], "' in repeat specifier.");
      } else if (max_repeat != -1 && max_repeat < min_repeat) {
        Error("In repeat block {m,n}, m must be <= n, but ", min_repeat, " > ", max_repeat, ".");
      }
      pos++; // Skip past '}'
      return RepeatInfo{min_repeat,max_repeat};
    }

    /// Process the input regex into a tree representation.
    Ptr<re_parent> Process() {
      Ptr<re_parent> cur_parent = NewPtr<re_block>();

      // Make sure the input stream is good to load from.
      if (pos >= regex.size()) {
        if (regex.size() == 0) Error("Cannot process an empty RegEx");
        else if (regex.back() == '|') Error("Another option must follow OR ('|'); use '?' to make a segment optional.");
        else Error("Cannot end a RegEx with '", regex.back(), "'.");
        return cur_parent;
      }

      // All blocks need to start with a single token.
      cur_parent->push( ConstructSegment() );

      while (pos < regex.size()) {
        const char c = regex[pos++];
        switch (c) {
          // case '|': cur_parent->push( NewPtr<re_or>( cur_parent->pop(), Process() ) ); break;
          case '|': cur_parent = NewPtr<re_or>( cur_parent, Process() ); break;
          case '*': cur_parent->push( NewPtr<re_star>( cur_parent->pop() ) ); break;
          case '+': cur_parent->push( NewPtr<re_plus>( cur_parent->pop() ) ); break;
          case '?': cur_parent->push( NewPtr<re_qm>( cur_parent->pop() ) ); break;
          case ')': pos--; return cur_parent;  // Must be ending segment (restore pos to check on return)
          case '{': cur_parent->push( NewPtr<re_repeat>( cur_parent->pop(), ReadRepeat() ) ); break;

          default:     // Must be a regular "segment"
            pos--;     // Restore to previous char to construct the next segment.
            cur_parent->push( ConstructSegment() );
        }
      }

      return cur_parent;
    }

  public:
    RegEx() = delete;
    RegEx(const emp::String & r) : regex(r) {
      if (regex.size()) head_ptr = Process();
      while(head_ptr->Simplify());
    }
    RegEx(const RegEx & r) : regex(r.regex) {
      if (regex.size()) head_ptr = Process();
      while(head_ptr->Simplify());
    }
    ~RegEx() = default;

    /// Set this RegEx equal to another.
    RegEx & operator=(const RegEx & r) {
      regex = r.regex;
      notes.resize(0);
      valid = true;
      pos = 0;
      head_ptr.Delete();
      head_ptr = Process();
      while (head_ptr->Simplify());
      return *this;
    }

    /// Convert the RegEx to an standard string, readable from outside this class.
    emp::String AsString() const { return regex.AsLiteral(); }

    /// Add this regex to an NFA being built.
    void AddToNFA(NFA & nfa, size_t start, size_t stop) const {
      emp_assert(head_ptr);
      head_ptr->AddToNFA(nfa, start, stop);
    }

    /// Assume the RegEx is ready and setup processing for it.
    void Generate() const;

    /// Test if a string satisfies this regex.
    bool Test(const emp::String & str) const {
      if (!dfa_ready) Generate();
      return dfa.Test(str);
    }

    const emp::vector<emp::String> & GetNotes() const { return notes; }

    /// For debugging: print the internal representation of the regex.
    void PrintInternal() const {
      emp_assert(head_ptr);
      head_ptr->Print(std::cout); std::cout << std::endl;
    }

    /// For debugging: print any internal notes generated about this regex.
    void PrintNotes() const {
      for (const emp::String & n : notes) {
        std::cout << n << std::endl;
      }
    }

    /// Print general debugging information about this regex.
    void PrintDebug() const {
      if (notes.size()) {
        std::cout << "NOTES:" << std::endl;
        PrintNotes();
      }
      std::cout << "RegEx: " << regex.AsEscaped() << std::endl;
      std::cout << "INTERNAL: ";
      PrintInternal();
    }
  };


  /// Simple conversion of RegEx to NFA (mostly implemented in RegEx)
  static NFA to_NFA(const RegEx & regex, size_t stop_id=1) {
    NFA nfa(2);  // State 0 = start, state 1 = stop.
    nfa.SetStop(1, stop_id);
    regex.AddToNFA(nfa, 0, 1);
    return nfa;
  }

  /// Conversion of RegEx to DFA, via NFA intermediate.
  static DFA to_DFA(const RegEx & regex) {
    return to_DFA( to_NFA(regex) );
  }

  void RegEx::Generate() const {
    dfa = to_DFA(*this);
    dfa_ready = true;
  }
}

#endif // #ifndef EMP_COMPILER_REGEX_HPP_INCLUDE
