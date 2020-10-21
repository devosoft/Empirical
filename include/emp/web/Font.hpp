/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file  Font.hpp
 *  @brief Maintains basic information about a font to be used in HTML.
 */


#ifndef EMP_FONT_H
#define EMP_FONT_H

#include <string>
#include <sstream>

#include "color_map.hpp"
#include "Style.hpp"

namespace emp {
namespace web {

  /// Maintain information about an HTML font.
  class Font {
  protected:
    std::string family;     ///< Font family to use.
    int size;               ///< Font size (in px) to use.
    std::string color;      ///< Font color.
    std::string line_color; ///< Color of lines through the text (underline, linethrough, etc.)

    bool is_bold;           ///< Is this font bold?
    bool is_italic;         ///< Is this font itaic?
    bool is_smallcaps;      ///< Should this test be in small caps?
    bool is_underlined;     ///< Should this text be underlined?
    bool is_overlined;      ///< Should this text have a line above it?
    bool is_linethrough;    ///< Should this text have a line through it?
    bool is_wavy_line;      ///< Should lines be made wavy?

  public:
    Font(const Font &) = default;
    Font(Font &&) = default;
    Font(const std::string & _family="Helvetica", int _size=15,
	       const std::string & _color="black", bool _bold=false, bool _italic=false)
      : family(_family), size(_size), color(_color)
      , is_bold(_bold), is_italic(_italic), is_smallcaps(false)
      , is_underlined(false), is_overlined(false), is_linethrough(false), is_wavy_line(false)  { ; }
    Font(int _size, const std::string & _color="black", bool _bold=false, bool _italic=false)
      : Font("Helvetica", _size, _color, _bold, _italic)  { ; }
    ~Font() { ; }

    Font & operator=(const Font &) = default;
    Font & operator=(Font &&) = default;

    const std::string & GetFamily() const { return family; }
    int GetSize() const { return size; }
    const std::string & GetColor() const { return color; }
    const std::string & GetLineColor() const { return line_color; }
    bool IsBold() const { return is_bold; }
    bool IsItalic() const { return is_italic; }
    bool IsSmallcaps() const { return is_smallcaps; }
    bool IsUnderlined() const { return is_underlined; }
    bool IsOverlined() const { return is_overlined; }
    bool IsStrikethrough() const { return is_linethrough; }
    bool IsWavyLine() const { return is_wavy_line; }
    bool HasLine() const { return is_underlined || is_overlined || is_linethrough; }

    Font & SetFamily(const std::string & _family) { family = _family; return *this; }
    Font & SetSize(int _size) { size = _size; return *this; }
    Font & SetColor(const std::string & _color) { color = _color; return *this; }
    Font & SetLineColor(const std::string & _color) { line_color = _color; return *this; }
    Font & SetBold(bool _in=true) { is_bold = _in; return *this; }
    Font & SetItalic(bool _in=true) { is_italic = _in; return *this; }
    Font & SetSmallcaps(bool _in=true) { is_smallcaps = _in; return *this; }
    Font & SetUnderlined(bool _in = true) { is_underlined = _in; return *this; }
    Font & SetOverlined(bool _in = true) { is_overlined = _in; return *this; }
    Font & SetStrikethrough(bool _in = true) { is_linethrough = _in; return *this; }
    Font & SetWavyLine(bool _in = true) { is_wavy_line = _in; return *this; }

    /// Take a Style object an fill it out based on this font information.
    void ConfigStyle(Style & style) const {
      style.Set("color", color);
      style.Set("font-family", family);
      style.Set("font-size", to_string(size,"px"));
      if (is_bold) style.Set("font-weight", "bold");
      if (is_italic) style.Set("font-style", "italic");
      if (is_smallcaps) style.Set("font-variant", "small-caps");
      if (HasLine()) {
        std::string decoration("");
        if (is_underlined) decoration += " underline";
        if (is_overlined) decoration += " overline";
        if (is_linethrough) decoration += " line-through";
        if (line_color != "") { decoration += " "; decoration += line_color; }
        if (is_wavy_line) decoration += " wavy";
        style.Set("text-decoration", decoration);
      }
    }

    Style AsStyle() const {
      Style style;
      ConfigStyle(style);
      return style;
    }

    std::string GetHTMLStart() {
      std::stringstream ss;
      ss << "<span style=\"color:" << color
         << "; font-family:" << family
         << "; font-size:" << size;
      if (is_bold) ss << "; font-weight:bold";
      if (is_italic) ss << "; font-style:italic";
      if (is_smallcaps) ss << "; font-variant:small-caps";
      if (HasLine()) {
        ss << "; text-decoration:";
        if (is_underlined) ss << " underline";
        if (is_overlined) ss << " overline";
        if (is_linethrough) ss << " line-through";
        if (line_color != "") ss << " " << line_color;
        if (is_wavy_line) ss << " wavy";
      }
      ss << "\">";
      return ss.str();
    }
    std::string GetHTMLEnd() { return "</span>"; }

    bool operator==(const Font & _in) const {
      return (family == _in.family)
        && (size == _in.size)
        && (color == _in.color)
        && (line_color == _in.line_color)
        && (is_bold == _in.is_bold)
        && (is_italic == _in.is_italic)
        && (is_smallcaps == _in.is_smallcaps)
        && (is_underlined == _in.is_underlined)
        && (is_overlined == _in.is_overlined)
        && (is_linethrough == _in.is_linethrough)
        && (is_wavy_line == _in.is_wavy_line)
        ;
    }
    bool operator!=(const Font & _in) const { return !operator==(_in); }
  };

}
}

#endif
