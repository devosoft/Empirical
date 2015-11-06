// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_AUTHOR_H
#define EMP_AUTHOR_H

///////////////////////////////////////////////////////////////////////////////////////////
//
//  Basic information about an author
//

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
    Author(const Author &) = delete;
    ~Author() { ; }
    Author & operator=(const Author &) = delete;

    bool operator==(const Author & other) const {
      return (first_name == other.first_name) &&
        (midd_name == other.middle_name) &&
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

    char GetFirstInitial() const { return HasFirstName() ? first_name[0] : 0; }
    char GetMiddleInitial() const { return HasMiddleName() ? middle_name[0] : 0; }
    char GetLastInitial() const { return HasLastName() ? last_name[0] : 0; }
    std::string GetInitials() const {
      std::string inits;
      if (HasFirstName()) inits.push_back(first_name[0]);
      if (HasMiddleName()) inits.push_back(middle_name[0]);
      if (HasLastName()) inits.push_back(last_name[0]);
    }
  };

};

#endif
