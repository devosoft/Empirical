/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  Datum.hpp
 *  @brief A single piece of data, either a value or a string.
 *  @note Status: ALPHA
 * 
 *  DEVELOPER NOTES:
 *  - For now, using unions, but this creates complications with non-trivial constructor/destructor
 *    for strings, so could try shifting over.
 */

#ifndef EMP_DATUM_HPP
#define EMP_DATUM_HPP

#include <string>

#include "../base/assert.hpp"

namespace emp {

  class Datum {
  private:
    union {
      double num;
      std::string str;
    };
    bool is_num = true;

    void InitString() { new (&str) std::string; }
    void InitString(const std::string & in) { new (&str) auto(in); }
    void FreeString() { str.~basic_string(); }
  public:
    Datum() : num(0.0), is_num(true) { }
    Datum(double in) : num(in), is_num(true) { }
    Datum(const std::string & in) : is_num(false) { InitString(in); }
    Datum(const Datum & in) {
      is_num = in.is_num;
      if (is_num) num = in.num;
      else InitString(in.str);
    }
    ~Datum() { if (!is_num) FreeString(); }

    bool IsDouble() const { return is_num; }   ///< Is this natively stored as a double?
    bool IsString() const { return !is_num; }  ///< Is this natively stored as a string?

    double AsDouble() const {
      if (is_num) return num;
      return std::stod(str);
    }

    const std::string & AsString() const {
      if (is_num) return std::to_string(num);
      return str;
    }

    Datum & Set(double in) {
      if (!is_num) FreeString();  // If this were previously a string, clean it up!
      num = in;
      return *this;
    }

    Datum & Set(const std::string & in) {
      if (is_num) InitString(in);  // If this were previously a num, change to string.
      else str = in;               // Already a string; just change its value.
      return *this;
    }

    Datum & Set(const Datum & in) {
      if (in.is_num) return Set(in.num);
      else return Set(in.str);
    }

    Datum & operator=(double in) { return Set(in); }
    Datum & operator=(const std::string & in) { return Set(in); }
    Datum & operator=(const Datum & in) { return Set(in); }

    bool operator==(double in) const { return AsDouble() == in; }
    bool operator!=(double in) const { return AsDouble() != in; }
    bool operator< (double in) const { return AsDouble() <  in; }
    bool operator<=(double in) const { return AsDouble() <= in; }
    bool operator> (double in) const { return AsDouble() >  in; }
    bool operator>=(double in) const { return AsDouble() >= in; }

    bool operator==(const std::string & in) const { return AsString() == in; }
    bool operator!=(const std::string & in) const { return AsString() != in; }
    bool operator< (const std::string & in) const { return AsString() <  in; }
    bool operator<=(const std::string & in) const { return AsString() <= in; }
    bool operator> (const std::string & in) const { return AsString() >  in; }
    bool operator>=(const std::string & in) const { return AsString() >= in; }

    bool operator==(const Datum & in) const {
      if (is_num && in.is_num) return num == in.num;   // Both numbers
      if (!is_num && !in.is_num) return str == in.str; // Both strings
      return AsDouble() == in.AsDouble() && AsString() == in.AsString(); // Mixed - check both!
    }
    bool operator!=(const Datum & in) const { return !(*this == in); }
    bool operator< (const Datum & in) const {
      if (is_num && in.is_num) return num < in.num;    // Both numbers
      return AsString() < in.AsString();               // Otherwise treat as strings.
    }
    bool operator> (const Datum & in) const { return in < *this; }
    bool operator<=(const Datum & in) const { return !(in < *this); }
    bool operator>=(const Datum & in) const { return !(*this < in); }

  };

}

#endif
