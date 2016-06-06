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
#include "vector.h"

namespace emp {

  template <int NUM_SYMBOLS=128>
  class RegEx {
  private:
    using opts_t = BitSet<NUM_SYMBOLS>;
    const std::string regex;
    bool valid;                          // Set to false if cannot be processed.
    emp::vector<std::string> notes;      // Any warnings or errors would be provided here.
    int pos;                             // Position being read in regex.

    struct re_base {                     // Also used for empty re
    };
    struct re_block : public base_re {   // Series of re's
      emp::vector<base_re *> nodes;
      void push(base_re * x) { nodes.push_back(x); }
      base_re * pop() { auto * out = nodes.back(); nodes.pop(); return out; }
    };
    struct re_string : public base_re {  // Series of specific chars
      std::string str;
      re_string() { ; }
      re_string(const std::string s) : str(s) { ; }
    }
    struct re_charset : public base_re { // Any char from set.
      opts_t char_set;
      re_charset() { ; }
      re_charset(char x, bool neg=false) { char_set[x]=true; if (neg) char_Set.NOT_SELF(); }
      re_charset(const std::string & s, bool neg=false)
        { for (char x : s) char_set[x]=true; if (neg) char_Set.NOT_SELF(); }
    };
    struct re_or : public base_re {      // lhs -or- rhs
      base_re * lhs; base_re * rhs;
    };
    struct re_star : public base_re {    // zero-or-more
      base_re * child;
    }
    struct re_plus : public base_re {    // one-or-more
      base_re * child;
    }
    struct re_qm : public base_re {      // zero-or-one
      base_re * child;
    }

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

    // Should only be called when we know we have a single unit to produce.  Build and return it.
    re_base * ConstructSingle() {
      re_base * result;
      char c = regex[pos++];  // Grab the current character and move pos to next.
      switch (c) {
        case '.':
          return new re_charset('\n', true);  // Anything except newline.
        case '(':
          re_base * out = Process();   // Process the internal contents of parens.
          EnsureNext(')');             // Make sure last char is a paren and advance.
          return out;
        case '[':
          c = regex[pos++];
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
          pos++;
          return out;
        case '"':
          c = regex[pos++];
          auto * out = new re_string;
          while (c != '"' && pos < (int) regex.size()) {
            // @CAO Add escape ('\') functionality.
            // @CAO Error if we run out of chars before close '"'
            out->str.push_back(c);
            c = regex[pos++];
          }
          pos++;
          return out;
        case '\\':
          // @CAO Add escape ('\') functionality.

        // Error cases
        case '|':
        case '*':
        case '+':
        case '?':
        case ')':
          // @CAO These should all be error cases...

        default:
      }
    }

    // Process the input regex into a tree representaion.
    re_block * Process(re_block * cur_block=nullptr) {
      emp_assert(pos >= 0 && pos < (int) regex.size(), pos, regex.size());

      // If caller does not provide current block, create one (and return it.)
      if (cur_block==nullptr) cur_block = new re_block;

      // All blocks need to start with a single token.
      cur_block->nodes.push_back( ConstructSingle() );

      const char c = regex(pos);   // Don't increment pos in case we don't use c here.
      switch (c) {
        case '|': cur_block->push( new re_or{ cur_block->pop(), ConstructSingle() } ); break;
        case '*':
        case '+':
        case '?':
        case ')': return cur_block;  // Must be ending segment.

        default:     // Must be a regular "segment"
          cur_block->nodes.push_back( ConstructSingle() );
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
