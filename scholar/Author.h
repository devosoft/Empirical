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

    const std::string GetFirstName() const { return first_name; }
    const std::string GetMiddleName() const { return middle_name; }
    const std::string GetLastName() const { return last_name; }

    char GetFirstInitial() const { return first_name.size() ? first_name[0] : 0; }
    char GetMiddleInitial() const { return middle_name.size() ? middle_name[0] : 0; }
    char GetLastInitial() const { return last_name.size() ? last_name[0] : 0; }
  };

};

#endif
