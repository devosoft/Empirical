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
    Color(const std::string & _name) : colorname(_name) { ; }
    Color(const Color & _in) : colorname(_in.colorname) { ; }
    Color(const char * _name) : colorname(_name) { ; }
    Color(int r, int g, int b) { // @CAO This is technically shorter than "rgb(##,##,##)", but more processing up front.
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

    const Color & operator=(const Color & _in) { colorname = _in.colorname; return *this; }
    bool operator==(const Color & _in) const { return colorname == colorname; }
    bool operator!=(const Color & _in) const { return colorname != colorname; }

    const std::string & AsString() const { return colorname; }

    void Set(const Color & _in) { colorname = _in.colorname; }
  };


  class ColorMap {
  private:
    std::vector<Color> color_map;
  public:
    ColorMap(int size, bool autocolor=false) : color_map(size) {
      if (autocolor) {
        //const double cap = 240.0;
        const double cap = 300.0;
        
        const double step = cap / (double) size;
        for (int i = 0; i < size; i++) {
          const double hue = 330 + step * (double) i;
          std::stringstream stream;
          stream << "hsl(" << hue << ",100%,50%)";
          color_map[i].Set(stream.str());
        }
      }
    }
    ~ColorMap() { ; }

    int GetSize() const { return (int) color_map.size(); }

    Color & operator[](int id) { return color_map[id]; }
    const Color & operator[](int id) const { return color_map[id]; }
  };
}

#endif

