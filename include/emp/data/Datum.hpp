/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file Datum.hpp
 *  @brief A single piece of data, either a value or a string.
 *  @note Status: ALPHA
 *
 *  DEVELOPER NOTES:
 *  - For now, using unions, but this creates complications with non-trivial constructor/destructor
 *    for strings, so could try shifting over.
 */

#ifndef EMP_DATA_DATUM_HPP_INCLUDE
#define EMP_DATA_DATUM_HPP_INCLUDE

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
    Datum(const char * in) : is_num(false) { InitString(in); }
    Datum(const Datum & in) {
      is_num = in.is_num;
      if (is_num) num = in.num;
      else InitString(in.str);
    }
    ~Datum() { if (!is_num) FreeString(); }

    bool IsDouble() const { return is_num; }   ///< Is this natively stored as a double?
    bool IsString() const { return !is_num; }  ///< Is this natively stored as a string?

    /// If we know Datum is a Double, we can request its native form.
    double & NativeDouble() { emp_assert(is_num); return num; }
    double NativeDouble() const { emp_assert(is_num); return num; }

    /// If we know Datum is a String, we can request its native form.
    std::string & NativeString() { emp_assert(!is_num); return str; }
    const std::string & NativeString() const { emp_assert(!is_num); return str; }

    double AsDouble() const {
      if (is_num) return num;
      return std::stod(str);
    }

    std::string AsString() const {
      if (is_num) return std::to_string(num);
      return str;
    }

    operator double() const { return AsDouble(); }
    operator std::string() const { return AsString(); }

    Datum & SetDouble(double in) {  // If this were previously a string, clean it up!
      if (!is_num) {
        FreeString();
        is_num = true;
      }
      num = in;
      return *this;
    }

    Datum & SetString(const std::string & in) {
      if (is_num) {        // If this were previously a num, change to string.
        InitString(in);
        is_num = false;
      }
      else str = in;       // Already a string; just change its value.
      return *this;
    }

    Datum & Set(const Datum & in) {
      if (in.is_num) return SetDouble(in.num);
      else return SetString(in.str);
    }

    Datum & operator=(double in) { return SetDouble(in); }
    Datum & operator=(const std::string & in) { return SetString(in); }
    Datum & operator=(const char * in) { return SetString(in); }
    Datum & operator=(const Datum & in) { return Set(in); }

    int CompareNumber(double rhs) const {
      const double val = AsDouble();
      return (val == rhs) ? 0 : ((val < rhs) ? -1 : 1);
    }

    int CompareString(const std::string & rhs) const {
      if (is_num) {
        const std::string val = std::to_string(num);
        return (val == rhs) ? 0 : ((val < rhs) ? -1 : 1);
      }
      return (str == rhs) ? 0 : ((str < rhs) ? -1 : 1);
    }

    int Compare(double rhs) const { return CompareNumber(rhs); }
    int Compare(const std::string & rhs) const { return CompareString(rhs); }
    int Compare(const char * rhs) const { return CompareString(rhs); }
    int Compare(const Datum & rhs) const { return (rhs.is_num) ? CompareNumber(rhs) : CompareString(rhs); }

    template<typename T> bool operator==(T && rhs) const { return Compare(std::forward<T>(rhs)) == 0; }
    template<typename T> bool operator!=(T && rhs) const { return Compare(std::forward<T>(rhs)) != 0; }
    template<typename T> bool operator< (T && rhs) const { return Compare(std::forward<T>(rhs)) == -1; }
    template<typename T> bool operator>=(T && rhs) const { return Compare(std::forward<T>(rhs)) != -1; }
    template<typename T> bool operator> (T && rhs) const { return Compare(std::forward<T>(rhs)) == 1; }
    template<typename T> bool operator<=(T && rhs) const { return Compare(std::forward<T>(rhs)) != 1; }

  };

  std::ostream & operator<<(std::ostream & out, const emp::Datum & d) {
    out << d.AsString();
    return out;
  }

}

#endif // #ifndef EMP_DATA_DATUM_HPP_INCLUDE
