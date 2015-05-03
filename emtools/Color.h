#ifndef EMP_COLOR_H
#define EMP_COLOR_H

#include <iomanip>
#include <string>
#include <sstream>

namespace emp {

  class Color {
  private:
    std::string colorname;

  public:
    Color() { ; }
    Color(const Color & _in) = default;
    Color(const std::string & _name) : colorname(_name) { ; }
    Color(const char * _name) : colorname(_name) { ; }
    Color(int r, int g, int b) {
      // @CAO Technically shorter than "rgb(##,##,##)", but more processing up front.
      std::stringstream stream;
      stream << '#' << std::setw(2) << std::setfill('0') << std::hex << r
             << std::setw(2) << std::setfill('0') << std::hex << g
             << std::setw(2) << std::setfill('0') << std::hex << b;
      colorname = stream.str();
    }
    Color(int r, int g, int b, double a) {
      std::stringstream stream;
      stream << "rgba(" << r << ',' << g << ',' << b << ',' << a << ')';
      colorname = stream.str();
    }

    Color & operator=(const Color & _in) = default;
    bool operator==(const Color & _in) const { return colorname == colorname; }
    bool operator!=(const Color & _in) const { return colorname != colorname; }

    const std::string & AsString() const { return colorname; }

    void Set(const Color & _in) { colorname = _in.colorname; }
  };

}

#endif

