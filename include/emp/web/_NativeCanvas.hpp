/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file  Canvas.hpp
 *  @brief Manage an HTML canvas object.
 *
 */


#ifndef EMP_WEB_NATIVECANVAS_H
#define EMP_WEB_NATIVECANVAS_H

#include <string>

#include <SFML/Graphics.hpp>

#include "../base/vector.hpp"
#include "../geometry/Circle2D.hpp"
#include "../tools/string_utils.hpp"

#include "_LineShape.hpp"
#include "Color.hpp"

namespace emp {
namespace web {

  /// Manage a SFML Canvas object.
  class Canvas {

    double width{};
    double height{};
    std::string id;

    sf::RenderTexture window;
    // sf::RenderWindow window;

  public:
    /// Create a new canvas with the specified size and optional HTML identifier.
    Canvas(const double w, const double h, const std::string & in_id="")
    : width( w )
    , height( h )
    , id( in_id ) {
    // , window( sf::VideoMode(w, h), id ) {
      window.clear();
      [[maybe_unused]] const auto res = window.create( width, height);
      emp_assert( res );
     }

    Canvas() = default;

    /// Get the pixel width of this Canvas.
    double GetWidth() const { return width; }

    /// Get the pixel height of this Canvas.
    double GetHeight() const { return height; }

    /// Set a new width for this Canvas.
    void SetWidth(const double w) { emp_assert(false, "unimplemented"); }

    /// Set a new height for this Canvas.
    void SetHeight(const double h) { emp_assert(false, "unimplemented"); }

    /// Set Canvas size.
    void SetSize(const double w, const double h) {
      emp_assert(false, "unimplemented");
    }

    /// Add a Circle to this canvas; provide constructor for the CanvasCircle with a position and radius
    /// as well as optional face color, line color, and line width.
    template <typename... Ts>
    Canvas & Circle(Point center, double _r, Ts &&... vals) {
      emp_assert(false, "unimplemented");
      return *this;
    }

    template <typename... Ts>
    Canvas & Circle(double _x, double _y, double _r, Ts &&... vals) {
      emp_assert(false, "unimplemented");
      return *this;
    }

    /// Add a Rectangle to this canvas at x,y with width w and heigh h.  Optional face color and
    /// line color.
    template <typename... Ts>
    Canvas & Rect(Point corner, double w, double h, Ts &&... vals) {
      emp_assert(false, "unimplemented");
      return *this;
    }

    template <typename... Ts>
    Canvas & Rect(
      const double x, const double y, const double w, const double h,
      const std::string& fc="", const std::string& lc="", const double lw=1.0
    ) {
      sf::RectangleShape shape( sf::Vector2f(w, h) );
      shape.setPosition(x, y);
      shape.setFillColor( (sf::Color) emp::web::Color( fc ) );
      shape.setOutlineColor( (sf::Color) emp::web::Color( lc ) );
      shape.setOutlineThickness( lw );

      window.draw( shape );
      return *this;
    }

    /// Add an Image to this canvas at x,y with width w and heigh h.
    // template <typename... Ts>
    // Canvas & Image(const emp::RawImage & image, Point corner, Ts &&... vals) {
    //   emp_assert(false, "unimplemented");
    //   return *this;
    // }

    // template <typename... Ts>
    // Canvas & Image(const emp::RawImage & image, double x, double y, Ts &&... vals) {
    //   emp_assert(false, "unimplemented");
    //   return *this;
    // }


    /// Add a Line from x1,y1 to x2,y2.  Optional face color and line color.
    template <typename... Ts>
    Canvas & Line(
      const double x1, const double y1, const double x2, const double y2,
      const std::string& fc="", const std::string& lc="", const double lw=1.0
    ) {

      sf::LineShape shape( sf::Vector2f(x1, y1), sf::Vector2f(x2, y2) );
      shape.setFillColor( (sf::Color) emp::web::Color( fc ) );
      shape.setOutlineColor( (sf::Color) emp::web::Color( lc ) );
      shape.setOutlineThickness( lw );

      window.draw( shape );
      return *this;
    }

    template <typename... Ts>
    Canvas & Line(emp::Point p1, emp::Point p2, Ts &&... vals) {
      emp_assert(false, "unimplemented");
      return *this;
    }

    /// Add a Line from x1,y1 to x2,y2.  Optional face color and line color.
    template <typename... Ts>
    Canvas & MultiLine(emp::Point p1, const emp::vector<emp::Point> & points, Ts &&... vals) {
      emp_assert(false, "unimplemented");
      return *this;
    }

    /// Add a string to this canvas at x,y with specified text.  Optional face color and
    /// line color.
    template <typename... Ts>
    Canvas & Text(emp::Point p, Ts &&... vals) {
      emp_assert(false, "unimplemented");
      return *this;
    }

    template <typename... Ts>
    Canvas & Text(double x, double y, Ts &&... vals) {
      emp_assert(false, "unimplemented");
      return *this;
    }

    /// Add a string to this canvas centered at x,y with specified text.  Optional face color and
    /// line color.
    template <typename... Ts>
    Canvas & CenterText(emp::Point p, Ts &&... vals) {
      emp_assert(false, "unimplemented");
      return *this;
    }

    template <typename... Ts>
    Canvas & CenterText(double x, double y, Ts &&... vals) {
      emp_assert(false, "unimplemented");
      return *this;
    }

    /// Update the default font for text.
    Canvas & Font(const std::string font) {
      emp_assert(false, "unimplemented");
      return *this;
    }

    /// Draw a circle onto this canvas.
    Canvas & Draw(const emp::Circle & circle,
                  const std::string & fc="", const std::string & lc="") {
                    emp_assert(false, "unimplemented");
      return *this;
    }

    /// Draw an arbitrary shape onto this canvas.
    // Canvas & Draw(const CanvasShape & shape) {
    //   emp_assert(false, "unimplemented");
    //   return *this;
    // }

    /// Change the default stroke color.
    Canvas & StrokeColor(std::string c) {
      emp_assert(false, "unimplemented");
      return *this;
    }

    /// Rotate the entire canvas.
    Canvas & Rotate(double angle) {
      emp_assert(false, "unimplemented");
      return *this;
    }

    /// Clear everything off of this canvas.
    Canvas & Clear() {
      window.clear();
      return *this;
    }

    /// Clear to a specific background color.
    Canvas & Clear(const std::string & bg_color) {
      emp_assert(false, "unimplemented");
      return *this;
    }

    /// Download a PNG image of a canvas.
    void DownloadPNG() { DownloadPNG(id + ".png"); }

    /// Download a PNG image of a canvas.
    void DownloadPNG(const std::string & fname) { SavePNG( fname ); }

    /// Save a PNG image of a canvas.
    void SavePNG(const std::string& fname) {
      window.display();
      window.getTexture().copyToImage().saveToFile( fname );
      // window.capture().saveToFile( fname );
    }

  };

} // namespace web
} // namespace emp

#endif
