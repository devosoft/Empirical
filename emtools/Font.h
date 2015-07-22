#ifndef EMP_FONT_H
#define EMP_FONT_H

#include <string>
#include <sstream>

#include "color_map.h"

namespace emp {
  class Font {
  private:
    std::string family;
    int size;
    std::string color;
    
    bool is_bold;
    bool is_italic;

  public:
    Font(const Font & _in) : family(_in.family), size(_in.size), color(_in.color), is_bold(_in.is_bold), is_italic(_in.is_italic) { ; }
    Font(const std::string & _family="Helvetica", int _size=30, const std::string & _color="black", bool _bold=false, bool _italic=false)
      : family(_family), size(_size), color(_color), is_bold(_bold), is_italic(_italic)  { ; }
    Font(int _size, const std::string & _color="black", bool _bold=false, bool _italic=false)
      : family("Helvetica"), size(_size), color(_color), is_bold(_bold), is_italic(_italic)  { ; }
    ~Font() { ; }

    const Font & operator=(const Font & _in) { 
      family = _in.family;   size = _in.size;   color = _in.color;
      is_bold = _in.is_bold;   is_italic = _in.is_italic;
      return *this;
    }

    const std::string & GetFamily() const { return family; }
    int GetSize() const { return size; }
    const std::string & GetColor() const { return color; }
    bool IsBold() const { return is_bold; }
    bool IsItalic() const { return is_italic; }

    Font & SetFamily(const std::string & _family) { family = _family; return *this; }
    Font & SetSize(int _size) { size = _size; return *this; }
    Font & SetColor(const std::string & _color) { color = _color; return *this; }
    Font & SetBold(bool _in=true) { is_bold = _in; return *this; }
    Font & SetItalic(bool _in=true) { is_italic = _in; return *this; }

    std::string GetHTMLStart() {
      std::stringstream ss;
      ss << "<span style=\"color:" << color
         << "; font-family:" << family
         << "; font-size:" << size;
      if (is_bold) ss << "; font-weight:bold";
      if (is_italic) ss << "; font-style:italic";
      ss << "\">";
      return ss.str();
    }
    std::string GetHTMLEnd() { return "</span>"; }

    bool operator==(const Font & _in) const {
      return (family == _in.family) && (size == _in.size) && (color == _in.color) && (is_bold == _in.is_bold) && (is_italic == _in.is_italic);
    }
    bool operator!=(const Font & _in) const { return !operator==(_in); }
  };
};

#endif
