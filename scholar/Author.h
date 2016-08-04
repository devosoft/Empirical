//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Basic information about an author.
//
//
//  Developer notes:
//  * Any number of middle names should be allowed
//  * Prefixs (Dr., Prof., etc) and Suffixes (Jr., Sr., III, etc.) should be allowed.
//  * Most parts of names can be auto-detected.
//  * GetName() (with formatting) needs to be implemented (notes below)
//  * Set functiions need to be added for precise configuration.

#ifndef EMP_AUTHOR_H
#define EMP_AUTHOR_H

#include <string>

namespace emp {

  class Author {
  private:
    std::string first_name;
    std::string middle_name;
    std::string last_name;
  public:
    Author(const std::string & first, const std::string & middle, const std::string & last)
      : first_name(first), middle_name(middle), last_name(last) { ; }
    Author(const std::string & first, const std::string & last)
      : first_name(first), last_name(last) { ; }
    Author(const std::string & last)
      : last_name(last) { ; }
    Author(const Author &) = default;
    ~Author() { ; }
    Author & operator=(const Author &) = default;

    bool operator==(const Author & other) const {
      return (first_name == other.first_name) &&
        (middle_name == other.middle_name) &&
        (last_name == other.last_name);
    }
    bool operator!=(const Author & other) const { return !(*this == other); }
    bool operator<(const Author & other) const {
      if (last_name != other.last_name) return (last_name < other.last_name);
      if (first_name != other.first_name) return (first_name < other.first_name);
      return (middle_name < other.middle_name);
    }
    bool operator>(const Author & other) const { return other < *this; }
    bool operator>=(const Author & other) const { return !(*this < other); }
    bool operator<=(const Author & other) const { return !(*this > other); }

    bool HasFirstName() const { return first_name.size(); }
    bool HasMiddleName() const { return middle_name.size(); }
    bool HasLastName() const { return last_name.size(); }

    const std::string & GetFirstName() const { return first_name; }
    const std::string & GetMiddleName() const { return middle_name; }
    const std::string & GetLastName() const { return last_name; }
    std::string GetFullName() const {
      std::string full_name(first_name);
      if (full_name.size() && HasMiddleName()) full_name += " ";
      full_name += middle_name;
      if (full_name.size() && HasLastName()) full_name += " ";
      full_name += last_name;
      return full_name;
    }
    std::string GetReverseName() const {
      std::string full_name(last_name);
      if (full_name.size() && HasFirstName()) full_name += ", ";
      full_name += first_name;
      if (full_name.size() && HasMiddleName()) full_name += " ";
      full_name += middle_name;
      return full_name;
    }

    char GetFirstInitial() const { return HasFirstName() ? first_name[0] : 0; }
    char GetMiddleInitial() const { return HasMiddleName() ? middle_name[0] : 0; }
    char GetLastInitial() const { return HasLastName() ? last_name[0] : 0; }
    std::string GetInitials() const {
      std::string inits;
      if (HasFirstName()) inits.push_back(first_name[0]);
      if (HasMiddleName()) inits.push_back(middle_name[0]);
      if (HasLastName()) inits.push_back(last_name[0]);
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
  };

};

#endif
