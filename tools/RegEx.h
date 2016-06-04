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

#ifndef EMP_REGEX_H
#define EMP_REGEX_H

#include <string>

#include "BitSet.h"
#include "string_utils.h"
#include "vector.h"

namespace emp {

  class RegEx {
  private:
    constexpr static int NUM_SYMBOLS = 128;
    using opts_t = BitSet<NUM_SYMBOLS>;
    const std::string regex;
    bool valid;                          // Set to false if cannot be processed.
    emp::vector<std::string> notes;      // Any warnings or errors would be provided here.
    int pos;                             // Position being read in regex.

    struct re_base {                     // Also used for empty re
    };
    struct re_block : public re_base {   // Series of re's
      emp::vector<re_base *> nodes;
      void push(re_base * x) { nodes.push_back(x); }
      re_base * pop() { auto * out = nodes.back(); nodes.pop_back(); return out; }
    };
    struct re_char : public re_base {  // Series of specific chars
      char c;
      re_char(char _c) : c(_c) { ; }
    };
    struct re_string : public re_base {  // Series of specific chars
      std::string str;
    };
    struct re_charset : public re_base { // Any char from set.
      opts_t char_set;
      re_charset() { ; }
      re_charset(char x, bool neg=false) { char_set[x]=true; if (neg) char_set.NOT_SELF(); }
      re_charset(const std::string & s, bool neg=false)
        { for (char x : s) char_set[x]=true; if (neg) char_set.NOT_SELF(); }
    };
    struct re_or : public re_base {      // lhs -or- rhs
      re_base * lhs; re_base * rhs;
      re_or(re_base * l, re_base * r) : lhs(l), rhs(r) { ; }
    };
    struct re_star : public re_base {    // zero-or-more
      re_base * child;
      re_star(re_base * c) : child(c) { ; }
    };
    struct re_plus : public re_base {    // one-or-more
      re_base * child;
      re_plus(re_base * c) : child(c) { ; }
    };
    struct re_qm : public re_base {      // zero-or-one
      re_base * child;
      re_qm(re_base * c) : child(c) { ; }
    };

    re_block head;

    bool EnsureNext(char x) {
      if (pos >= (int) regex.size()) {
        notes.push_back(emp::to_string("Expected ", x, " before end."));
        valid = false;
      }
      else if (regex[pos] != x) {
        notes.push_back(emp::to_string("Expected ", x, " at position ", pos, "."));
        valid = false;
      }
      ++pos;
      return valid;
    }

    re_charset * ConstructSet() {
      char c = regex[pos++];
      bool neg = false;
      if (c == '^') { neg = true; c = regex[pos++]; }
      auto * out = new re_charset;
      while (c != ']' && pos < (int) regex.size()) {
        // @CAO need to add range ('-') functionality.
        // @CAO Error if we run out of chars before ']'
        out->char_set[c] = true;
        c = regex[pos++];
      }
      if (neg) out->char_set.NOT_SELF();
      return out;
    }

    re_string * ConstructString() {
      char c = regex[pos++];
      auto * out = new re_string;
      while (c != '"' && pos < (int) regex.size()) {
        // @CAO Add escape ('\') functionality.
        // @CAO Error if we run out of chars before close '"'
        out->str.push_back(c);
        c = regex[pos++];
      }
      return out;
    }

    // Should only be called when we know we have a single unit to produce.  Build and return it.
    re_base * ConstructSegment() {
      re_base * result;
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
          // @CAO Add escape ('\') functionality.

        // Error cases
        case '|':
        case '*':
        case '+':
        case '?':
        case ')':
          notes.push_back(emp::to_string("Expected regex segment but got '", c,
                                         "' at position ", pos, "."));
          valid = false;
          break;

        default:
          // Take this char directly.
          result = new re_char(c);
      }

      return result;
    }

    // Process the input regex into a tree representaion.
    re_block * Process(re_block * cur_block=nullptr) {
      emp_assert(pos >= 0 && pos < (int) regex.size(), pos, regex.size());

      // If caller does not provide current block, create one (and return it.)
      if (cur_block==nullptr) cur_block = new re_block;

      // All blocks need to start with a single token.
      cur_block->nodes.push_back( ConstructSegment() );

      const char c = regex[pos];   // Don't increment pos in case we don't use c here.
      switch (c) {
        case '|': cur_block->push( new re_or{ cur_block->pop(), ConstructSegment() } ); ++pos; break;
        case '*': cur_block->push( new re_star{ cur_block->pop() } ); ++pos; break;
        case '+': cur_block->push( new re_plus{ cur_block->pop() } ); ++pos; break;
        case '?': cur_block->push( new re_qm{ cur_block->pop() } ); ++pos; break;
        case ')': return cur_block;  // Must be ending segment (keep pos to check on return)

        default:     // Must be a regular "segment"
          cur_block->nodes.push_back( ConstructSegment() );
      }

      return cur_block;
    }

  public:
    RegEx() = delete;
    RegEx(const std::string & r) : regex(r), pos(0) { ; }
    RegEx(const RegEx & r) : regex(r.regex), pos(0) { ; }
    ~RegEx() { ; }

    const std::string & AsString() const { return regex; }
  };

}

#endif
