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

namespace emp {

  class RegEx {
    struct re_base { ; };

    struct re_block : public base_re { emp::vector<base_re *> nodes; };  // Series of re's
    struct re_charset : public base_re { std::string chars; };           // Any char from set.
    struct re_or : public base_re { base_re * lhs; base_re * rhs; };     // lhs -or- rhs
  };

}

#endif
