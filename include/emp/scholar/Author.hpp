/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2016-2017 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/scholar/Author.hpp
 * @brief Basic information about an author.
 *
 * Developer notes:
 *   * Prefixes (Dr., Prof., etc) and Suffixes (Jr., Sr., III, etc.) should be allowed.
 *   * Most parts of names can be auto-detected.
 *   * GetName() (with formatting) needs to be implemented (notes below)
 *   * Consider using ce_string for inputs?  And making other aspects const expression?
 */

#pragma once

#ifndef INCLUDE_EMP_SCHOLAR_AUTHOR_HPP_GUARD
#define INCLUDE_EMP_SCHOLAR_AUTHOR_HPP_GUARD

#include <stddef.h>

#include "../compiler/Lexer.hpp"
#include "../tools/String.hpp"

namespace emp {

  class Author {
  private:
    String prefix;
    String first_name;
    emp::vector<String> middle_names;
    String last_name;
    String suffix;

    static size_t ID_type;
    static size_t ID_spacing;

    static Lexer & GetFormatLexer() {
      static Lexer lexer;
      if (lexer.GetNumTokens() == 0) {
        ID_type    = lexer.AddToken("type", "[FMLfmlPSx]");
        ID_spacing = lexer.AddToken("spacing", "[- ,.:]+");
      }
      return lexer;
    }
  public:
    Author(const String & first, const String & middle, const String & last)
      : first_name(first), last_name(last) {
      middle_names.push_back(middle);
    }

    Author(const String & first, const String & last) : first_name(first), last_name(last) { ; }

    Author(const String & last) : last_name(last) { ; }

    Author(const Author &) = default;

    ~Author() { ; }

    Author & operator=(const Author &) = default;

    bool operator==(const Author & other) const {
      return (prefix == other.prefix) && (first_name == other.first_name) &&
             (middle_names == other.middle_names) && (last_name == other.last_name) &&
             (suffix == other.suffix);
    }

    bool operator!=(const Author & other) const { return !(*this == other); }

    bool operator<(const Author & other) const {
      if (last_name != other.last_name) { return (last_name < other.last_name); }
      if (first_name != other.first_name) { return (first_name < other.first_name); }
      for (size_t i = 0; i < middle_names.size(); i++) {
        if (other.middle_names.size() <= i) { return false; }
        if (middle_names[i] != other.middle_names[i]) {
          return (middle_names[i] < other.middle_names[i]);
        }
      }
      if (middle_names.size() < other.middle_names.size()) { return true; }
      if (suffix != other.suffix) { return (suffix < other.suffix); }
      if (prefix != other.prefix) { return (prefix < other.prefix); }
      return false;  // Must be equal!
    }

    bool operator>(const Author & other) const { return other < *this; }

    bool operator>=(const Author & other) const { return !(*this < other); }

    bool operator<=(const Author & other) const { return !(*this > other); }

    bool HasPrefix() const { return prefix.size(); }

    bool HasFirstName() const { return first_name.size(); }

    bool HasMiddleName() const { return middle_names.size(); }

    bool HasLastName() const { return last_name.size(); }

    bool HasSuffix() const { return suffix.size(); }

    const String & GetPrefix() const { return prefix; }

    const String & GetFirstName() const { return first_name; }

    const String & GetMiddleName(size_t id = 0) const {
      if (middle_names.size() == 0) { return String::StaticEmpty(); }
      return middle_names[id];
    }

    const String & GetLastName() const { return last_name; }

    const String & GetSuffix() const { return suffix; }

    String GetFullName() const {
      String full_name(prefix);
      if (full_name.size() && HasFirstName()) { full_name += " "; }
      full_name += first_name;
      for (const auto & middle_name : middle_names) {
        if (full_name.size()) { full_name += " "; }
        full_name += middle_name;
      }
      if (full_name.size() && HasLastName()) { full_name += " "; }
      full_name += last_name;
      if (full_name.size() && HasSuffix()) { full_name += " "; }
      full_name += suffix;
      return full_name;
    }

    String GetReverseName() const {
      String full_name(last_name);
      if (full_name.size() && HasFirstName()) { full_name += ", "; }
      full_name += first_name;
      for (const auto & middle_name : middle_names) {
        if (full_name.size()) { full_name += " "; }
        full_name += middle_name;
      }
      if (full_name.size() && HasSuffix()) { full_name += ", "; }
      full_name += suffix;
      return full_name;
    }

    String GetFirstInitial() const { return HasFirstName() ? first_name.substr(0, 1) : ""; }

    String GetMiddleInitials() const {
      String out;
      for (const auto & m : middle_names) { out += m[0]; }
      return out;
    }

    String GetLastInitial() const { return HasLastName() ? last_name.substr(0, 1) : ""; }

    String GetInitials() const {
      String inits;
      inits += GetFirstInitial();
      inits += GetMiddleInitials();
      inits += GetLastInitial();
      return inits;
    }

    //  A generic GetName() function that takes a string to produce the final format.
    //    F = first name     f = first initial
    //    M = middle names   m = middle initials
    //    L = last name      l = last initial
    //    P = prefix         S = suffix
    //    x = an empty breakpoint to ensure certain punctuation exists.
    //
    //  Allowable punctuation = [ ,.-:] and is associated with the prior name key, so it will
    //  appear only if the name does (and in the case of the middle name, will appear with each).
    //
    //  For example, if the person's name is "Abraham Bartholomew Carmine Davidson" then...
    //    GetName("FML") would return "Abraham Bartholomew Carmine Davidson"
    //    GetName("fml") would return "ABCD"
    //    GetName("L, fm") would return "Davidson, ABC"
    //    GetName("f.m.x L") would return "A.B.C. Davidson"
    //
    //  Note that without the 'x', the space would be linked to each middle name:
    //
    //    GetName("f.m. L") would return "A.B. C. Davidson"

    String GetName(String pattern = "FML") {
      String out_name;
      Lexer & lexer = GetFormatLexer();
      auto tokens   = lexer.Tokenize(pattern);
      (void) tokens;
      return out_name;
    }

    Author & Clear() {
      prefix     = "";
      first_name = "";
      last_name  = "";
      middle_names.resize(0);
      suffix = "";
      return *this;
    }

    Author & SetFirst(const String & str) {
      first_name = str;
      return *this;
    }

    Author & SetLast(const String & str) {
      last_name = str;
      return *this;
    }

    Author & AddMiddle(const String & str) {
      middle_names.push_back(str);
      return *this;
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_SCHOLAR_AUTHOR_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: fml inits lfml
