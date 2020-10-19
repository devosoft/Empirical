//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Basic information about an author.
//
//
//  Developer notes:
//  * Prefixs (Dr., Prof., etc) and Suffixes (Jr., Sr., III, etc.) should be allowed.
//  * Most parts of names can be auto-detected.
//  * GetName() (with formatting) needs to be implemented (notes below)
//  * Consider using ce_string for inputs?  And making other aspects const experession?

#ifndef EMP_AUTHOR_H
#define EMP_AUTHOR_H

#include <string>

#include "../compiler/Lexer.hpp"

namespace emp {

  class Author {
  private:
    std::string prefix;
    std::string first_name;
    emp::vector<std::string> middle_names;
    std::string last_name;
    std::string suffix;

    static Lexer & GetFormatLexer() {
      static Lexer lexer;
      if (lexer.GetNumTokens() == 0) {
        lexer.AddToken("type", "[FMLfmlPSx]");
        lexer.AddToken("spacing", "[- ,.:]+");
      }
      return lexer;
    }
  public:
    Author(const std::string & first, const std::string & middle, const std::string & last)
      : first_name(first), last_name(last) { middle_names.push_back(middle); }
    Author(const std::string & first, const std::string & last)
      : first_name(first), last_name(last) { ; }
    Author(const std::string & last)
      : last_name(last) { ; }
    Author(const Author &) = default;
    ~Author() { ; }
    Author & operator=(const Author &) = default;

    bool operator==(const Author & other) const {
      return (prefix == other.prefix) && (first_name == other.first_name) &&
        (middle_names == other.middle_names) &&
        (last_name == other.last_name) && (suffix == other.suffix);
    }
    bool operator!=(const Author & other) const { return !(*this == other); }
    bool operator<(const Author & other) const {
      if (last_name != other.last_name) return (last_name < other.last_name);
      if (first_name != other.first_name) return (first_name < other.first_name);
      for (size_t i = 0; i < middle_names.size(); i++) {
        if (other.middle_names.size() <= i) return false;
        if (middle_names[i] != other.middle_names[i]) {
          return (middle_names[i] < other.middle_names[i]);
        }
      }
      if (middle_names.size() < other.middle_names.size()) return true;
      if (suffix != other.suffix) return (suffix < other.suffix);
      if (prefix != other.prefix) return (prefix < other.prefix);
      return false; // Must be equal!
    }
    bool operator>(const Author & other) const { return other < *this; }
    bool operator>=(const Author & other) const { return !(*this < other); }
    bool operator<=(const Author & other) const { return !(*this > other); }

    bool HasPrefix() const { return prefix.size(); }
    bool HasFirstName() const { return first_name.size(); }
    bool HasMiddleName() const { return middle_names.size(); }
    bool HasLastName() const { return last_name.size(); }
    bool HasSuffix() const { return suffix.size(); }

    const std::string & GetPrefix() const { return prefix; }
    const std::string & GetFirstName() const { return first_name; }
    const std::string & GetMiddleName(size_t id=0) const {
      if (middle_names.size() == 0) return emp::empty_string();
      return middle_names[id];
    }
    const std::string & GetLastName() const { return last_name; }
    const std::string & GetSuffix() const { return suffix; }

    std::string GetFullName() const {
      std::string full_name(prefix);
      if (full_name.size() && HasFirstName()) full_name += " ";
      full_name += first_name;
      for (const auto & middle_name : middle_names) {
        if (full_name.size()) full_name += " ";
        full_name += middle_name;
      }
      if (full_name.size() && HasLastName()) full_name += " ";
      full_name += last_name;
      if (full_name.size() && HasSuffix()) full_name += " ";
      full_name += suffix;
      return full_name;
    }
    std::string GetReverseName() const {
      std::string full_name(last_name);
      if (full_name.size() && HasFirstName()) full_name += ", ";
      full_name += first_name;
      for (const auto & middle_name : middle_names) {
        if (full_name.size()) full_name += " ";
        full_name += middle_name;
      }
      if (full_name.size() && HasSuffix()) full_name += ", ";
      full_name += suffix;
      return full_name;
    }

    std::string GetFirstInitial() const {
      return HasFirstName() ? to_string(first_name[0]) : emp::empty_string();
    }
    std::string GetMiddleInitials() const {
      std::string out;
      for (const auto & m : middle_names) out += m[0];
      return out;
    }
    std::string GetLastInitial() const {
      return HasLastName() ? to_string(last_name[0]) : emp::empty_string();
    }
    std::string GetInitials() const {
      std::string inits;
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
    //    x = an empty breakpoint to ensure certain puctuation exists.
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
    //  Note that without the 'x', the space would be associated with all middle names:
    //
    //    GetName("f.m. L") would return "A.B. C. Davidson"

    std::string GetName(std::string pattern="FML") {
      std::string out_name;
      Lexer & lexer = GetFormatLexer();
      lexer.Process(pattern);
      return out_name;
    }

    Author & Clear() {
      prefix=""; first_name=""; last_name=""; middle_names.resize(0); suffix="";
      return *this;
    }
    Author & SetFirst(const std::string & str) { first_name = str; return *this; }
    Author & SetLast(const std::string & str) { last_name = str; return *this; }
    Author & AddMiddle(const std::string & str) { middle_names.push_back(str); return *this; }
  };

}

#endif
