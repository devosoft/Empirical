//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Basic regular expression handler.
//
//  Special chars:
//   '|'          - or
//   '*'          - zero or more of previous
//   '+'          - one or more of previous
//   '?'          - previous is optional
//   '.'          - Match any character except \n
//
//  Pluse the following group contents (and change may translation rules)
//   '(' and ')'  - group contents
//   '"'          - Ignore special characters in contents (quotes still need to be escaped)
//   '[' and ']'  - character set -- choose ONE character
//                  ^ as first char negates contents ; - indicates range UNLESS first or last.
//
//
//  Additional overloads for functions in lexer_utils.h:
//
//    static NFA to_NFA(const RegEx & regex, int stop_id=1);
//    static DFA to_DFA(const RegEx & regex);
//
//
//  Developer Notes:
//   * Need to implement  ^ and $ (beginning and end of line)
//   * Need to implement {n}, {n,} and {n,m} (exactly n, at least n, and n-m copies, respecitvely)

#ifndef EMP_REGEX_H
#define EMP_REGEX_H


#include <ostream>
#include <sstream>
#include <string>

#include "BitSet.h"
#include "lexer_utils.h"
#include "NFA.h"
#include "Ptr.h"
#include "string_utils.h"
#include "vector.h"


namespace emp {

  class RegEx {
  private:
    constexpr static int NUM_SYMBOLS = 128;
    using opts_t = BitSet<NUM_SYMBOLS>;
    std::string regex;                     // Original string to define this RegEx.
    emp::vector<std::string> notes;        // Any warnings or errors would be provided here.
    bool valid;                            // Set to false if regex cannot be processed.
    int pos;                               // Position being read in regex.

    mutable DFA dfa;                       // DFA that this RegEx translates to.
    mutable bool dfa_ready;                // Is the dfa ready? (or does it need to be generated?)

    template <typename... T>
    void Error(T... args) {
      notes.push_back(emp::to_string(std::forward<T>(args)...));
      valid = false;
    }

    // Pre-declarations
    struct re_block;
    struct re_charset;
    struct re_parent;
    struct re_string;

    // Internal representation of regex
    struct re_base {                     // Also used for empty regex
      virtual ~re_base() { ; }
      virtual void Print(std::ostream & os) const { os << "[]"; }
      virtual Ptr<re_block> AsBlock() { return nullptr; }
      virtual Ptr<re_charset> AsCharSet() { return nullptr; }
      virtual Ptr<re_parent> AsParent() { return nullptr; }
      virtual Ptr<re_string> AsString() { return nullptr; }
      virtual int GetSize() const { return 0; }
      virtual bool Simplify() { return false; }
      virtual void AddToNFA(NFA & nfa, int start, int stop) const { nfa.AddFreeTransition(start, stop); }
    };

    struct re_string : public re_base {  // Series of specific chars
      std::string str;
      re_string() { ; }
      re_string(char c) { str.push_back(c); }
      re_string(const std::string & s) : str(s) { ; }
      void Print(std::ostream & os) const override { os << "STR[" << to_escaped_string(str) << "]"; }
      Ptr<re_string> AsString() override { return to_ptr(this); }
      int GetSize() const override { return (int) str.size(); }
      virtual void AddToNFA(NFA & nfa, int start, int stop) const override {
        int prev_id = start;
        for (char x : str) {
          int next_id = nfa.AddNewState();
          nfa.AddTransition(prev_id, next_id, x);
          prev_id = next_id;
        }
        nfa.AddFreeTransition(prev_id, stop);
      }
    };

    struct re_charset : public re_base { // Any char from set.
      opts_t char_set;
      re_charset() { ; }
      re_charset(char x, bool neg=false) { char_set[x]=true; if (neg) char_set.NOT_SELF(); }
      re_charset(const std::string & s, bool neg=false)
        { for (char x : s) char_set[x]=true; if (neg) char_set.NOT_SELF(); }
      void Print(std::ostream & os) const override {
        auto chars = char_set.GetOnes();
        bool use_not = false;
        if (chars.size() > 64) { chars = (~char_set).GetOnes(); use_not = true; }
        os << "SET[";
        if (use_not) os << "NOT ";
        for (int c : chars) os << to_escaped_string((char) c);
        os << "]";
      }
      Ptr<re_charset> AsCharSet() override { return to_ptr(this); }
      int GetSize() const override { return char_set.CountOnes(); }
      char First() const { return (char) char_set.FindBit(); }
      virtual void AddToNFA(NFA & nfa, int start, int stop) const override {
        for (int i = 0; i < NUM_SYMBOLS; i++) if (char_set[i]) nfa.AddTransition(start, stop, i);
      }
    };

    struct re_parent : public re_base {
    protected:
      emp::vector<Ptr<re_base>> nodes;
    public:
      ~re_parent() { for (auto x : nodes) x.Delete(); }
      void Clear() { for (auto x : nodes) x.Delete(); nodes.resize(0); }
      virtual void push(Ptr<re_base> x) { emp_assert(x != nullptr); nodes.push_back(x); }
      Ptr<re_base> pop() { auto out = nodes.back(); nodes.pop_back(); return out; }
      int GetSize() const override { return (int) nodes.size(); }
      Ptr<re_parent> AsParent() override { return to_ptr(this); }
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

    struct re_block : public re_parent {   // Series of re's
      void Print(std::ostream & os) const override {
        os << "BLOCK["; for (auto x : nodes) x->Print(os); os << "]";
      }
      Ptr<re_block> AsBlock() override { return to_ptr(this); }
      bool Simplify() override {
        bool modify = false;
        // Loop through block elements, simplifying when possible.
        for (size_t i = 0; i < nodes.size(); i++) {
          // If node is a charset with one option, replace it with a string.
          if (nodes[i]->AsCharSet() && nodes[i]->GetSize() == 1) {
            auto new_node = new re_string(nodes[i]->AsCharSet()->First());
            nodes[i].Delete();
            nodes[i] = new_node;
            modify = true;
          }
          // If two neighboring nodes are strings, merge them.
          if (i > 0 && nodes[i]->AsString() && nodes[i-1]->AsString()) {
            nodes[i-1]->AsString()->str += nodes[i]->AsString()->str;
            nodes[i].Delete();
            nodes.erase(nodes.begin()+i);
            i--;
            modify = true;
            continue;
          }

          // If blocks are nested, merge them into a single block.
          if (nodes[i]->AsBlock()) {
            auto old_node = nodes[i]->AsBlock();
            nodes.erase(nodes.begin() + i);
            nodes.insert(nodes.begin() + i, old_node->nodes.begin(), old_node->nodes.end());
            old_node->nodes.resize(0);  // Don't recurse delete since nodes were moved!
            old_node.Delete();
            // @CAO do this.
            i--;
            modify = true;
            continue;
          }
        }

        // Also run the default Simplify on nodes.
        modify |= re_parent::Simplify();

        return modify;
      }
      virtual void AddToNFA(NFA & nfa, int start, int stop) const override {
        int prev_id = start;
        for (auto x : nodes) {
          int next_id = nfa.AddNewState();
          x->AddToNFA(nfa, prev_id, next_id);
          prev_id = next_id;
        }
        nfa.AddFreeTransition(prev_id, stop);
      }
    };

    struct re_or : public re_parent {      // lhs -or- rhs
      re_or(Ptr<re_base> l, Ptr<re_base> r) { push(l); push(r); }
      void Print(std::ostream & os) const override {
        os << "|[";
        nodes[0]->Print(os);
        nodes[1]->Print(os);
        os << "]";
      }
      virtual void AddToNFA(NFA & nfa, int start, int stop) const override {
        nodes[0]->AddToNFA(nfa, start, stop);
        nodes[1]->AddToNFA(nfa, start, stop);
      }
    };

    struct re_star : public re_parent {    // zero-or-more
      re_star(Ptr<re_base> c) { push(c); }
      void Print(std::ostream & os) const override { os << "*["; nodes[0]->Print(os); os << "]"; }

      virtual void AddToNFA(NFA & nfa, int start, int stop) const override {
        const int target = nfa.AddNewState();
        nodes[0]->AddToNFA(nfa, start, target);
        nfa.AddFreeTransition(target, start);
        nfa.AddFreeTransition(start, stop);
      }
    };

    struct re_plus : public re_parent {    // one-or-more
      re_plus(Ptr<re_base> c) { push(c); }
      void Print(std::ostream & os) const override { os << "+["; nodes[0]->Print(os); os << "]"; }
      virtual void AddToNFA(NFA & nfa, int start, int stop) const override {
        const int target = nfa.AddNewState();
        nodes[0]->AddToNFA(nfa, start, target);
        // From the target, can either go back to start and repeat, or straight to stop.
        nfa.AddFreeTransition(target, start);
        nfa.AddFreeTransition(target, stop);
      }
    };

    struct re_qm : public re_parent {      // zero-or-one
      re_qm(Ptr<re_base> c) { push(c); }
      void Print(std::ostream & os) const override { os << "?["; nodes[0]->Print(os); os << "]"; }
      virtual void AddToNFA(NFA & nfa, int start, int stop) const override {
        nodes[0]->AddToNFA(nfa, start, stop);
        nfa.AddFreeTransition(start, stop);
      }
    };

    re_block head;

    bool EnsureNext(char x) {
      if (pos >= (int) regex.size()) Error("Expected ", x, " before end.");
      else if (regex[pos] != x) Error("Expected ", x, " at position ", pos,
                                      "; found ", regex[pos], ".");
      ++pos;               // We have what we were expecting!  Move on...
      return valid;
    }

    Ptr<re_charset> ConstructSet() {
      char c = regex[pos++];
      bool neg = false;
      if (c == '^') { neg = true; c = regex[pos++]; }
      auto out = new re_charset;
      char prev_c = -1;
      while (c != ']' && pos < (int) regex.size()) {
        if (c == '-' && prev_c != -1) {
          c = regex[pos++];
          if (c < prev_c) { Error("Invalid character range ", prev_c, '-', c); continue; }
          for (char x = prev_c; x <= c; x++) {
            out->char_set[x] = true;
          }
          prev_c = -1;
          c = regex[pos++];
          continue;
        }
        else if (c == '\\') {
          c = regex[pos++];  // Identify the specific escape char.
          switch(c) {
            case 'n': c = '\n'; break;
            case 'r': c = '\r'; break;
            case 't': c = '\t'; break;
            // Any of these characters should just be themselves!
            case '-':
            case '\\':
            case ']':
            case '[':
            case '^':
              break;
            default:
              Error("Unknown escape char for char sets: '\\", c, "'.");
          }
        }
        out->char_set[c] = true;
        prev_c = c;
        c = regex[pos++];
      }
      if (neg) out->char_set.NOT_SELF();
      if (c == ']') --pos;
      return out;
    }

    Ptr<re_string> ConstructString() {
      char c = regex[pos++];
      auto out = new re_string;
      while (c != '\"' && pos < (int) regex.size()) {
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

    // Should only be called when we know we have a single unit to produce.  Build and return it.
    Ptr<re_base> ConstructSegment() {
      Ptr<re_base> result;
      char c = regex[pos++];  // Grab the current character and move pos to next.
      switch (c) {
        case '.':
          result = new re_charset('\n', true);  // Anything except newline.
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
            case 'n': c = '\n'; break;
            case 'r': c = '\r'; break;
            case 't': c = '\t'; break;
            // Any of these characters should just be themselves!
            case '\\':
            case '\"':
            case '*':
            case '+':
            case '?':
            case '.':
            case '|':
            case '(':
            case ')':
            case '[':
            case ']':
              break;
            default:
              Error("Unknown escape char for regex: '\\", c, "'.");
          }
          result = new re_string(c);
          break;

        // Error cases
        case '|':
        case '*':
        case '+':
        case '?':
        case ')':
          Error("Expected regex segment but got '", c, "' at position ", pos, ".");
          result = new re_string(c);
          break;

        default:
          // Take this char directly.
          result = new re_string(c);
      }

      emp_assert(result != nullptr);
      return result;
    }

    // Process the input regex into a tree representaion.
    Ptr<re_block> Process(Ptr<re_block> cur_block=nullptr) {
      emp_assert(pos >= 0 && pos < (int) regex.size(), pos, regex.size());

      // If caller does not provide current block, create one (and return it.)
      if (cur_block==nullptr) cur_block = new re_block;

      // All blocks need to start with a single token.
      cur_block->push( ConstructSegment() );

      while (pos < (int) regex.size()) {
        const char c = regex[pos++];
        switch (c) {
          // case '|': cur_block->push( new re_or( cur_block->pop(), ConstructSegment() ) ); break;
          case '|': cur_block->push( new re_or( cur_block->pop(), Process() ) ); break;
          case '*': cur_block->push( new re_star{ cur_block->pop() } ); break;
          case '+': cur_block->push( new re_plus{ cur_block->pop() } ); break;
          case '?': cur_block->push( new re_qm{ cur_block->pop() } ); break;
          case ')': pos--; return cur_block;  // Must be ending segment (restore pos to check on return)

          default:     // Must be a regular "segment"
            pos--;     // Restore to previous char to construct the next seqment.
            cur_block->push( ConstructSegment() );
        }
      }

      return cur_block;
    }

  public:
    RegEx() = delete;
    RegEx(const std::string & r) : regex(r), valid(true), pos(0), dfa_ready(false) {
      Process(to_ptr(head));
      while(head.Simplify());
    }
    RegEx(const RegEx & r) : regex(r.regex), valid(true), pos(0), dfa_ready(false) {
      Process(to_ptr(head));
      while(head.Simplify());
    }
    ~RegEx() { ; }

    RegEx & operator=(const RegEx & r) {
      regex = r.regex;
      notes.resize(0);
      valid = true;
      pos = 0;
      head.Clear();
      Process(to_ptr(head));
      while (head.Simplify());
      return *this;
    }

    std::string AsString() const { return to_literal(regex); }

    virtual void AddToNFA(NFA & nfa, int start, int stop) const { head.AddToNFA(nfa, start, stop); }

    void Generate() const;

    bool Test(const std::string & str) const {
      if (!dfa_ready) Generate();
      return dfa.Test(str);
    }

    // For debugging: print the internal representation of the regex.
    void PrintInternal() { head.Print(std::cout); std::cout << std::endl; }
    void PrintNotes() {
      for (const std::string & n : notes) {
        std::cout << n << std::endl;
      }
    }
    void PrintDebug() {
      if (notes.size()) {
        std::cout << "NOTES:" << std::endl;
        PrintNotes();
      }
      std::cout << "RegEx: " << to_escaped_string(regex) << std::endl;
      std::cout << "INTERNAL: ";
      PrintInternal();
    }
  };


  // Simple conversion of RegEx to NFA (mostly implemented in RegEx)
  static NFA to_NFA(const RegEx & regex, int stop_id=1) {
    NFA nfa(2);  // State 0 = start, state 1 = stop.
    nfa.SetStop(1, stop_id);
    regex.AddToNFA(nfa, 0, 1);
    return nfa;
  }

  // Conversion of RegEx to DFA, via NFA intermediate.
  static DFA to_DFA(const RegEx & regex) {
    return to_DFA( to_NFA(regex) );
  }

  void RegEx::Generate() const {
    dfa = to_DFA(*this);
    dfa_ready = true;
  }
}

#endif
