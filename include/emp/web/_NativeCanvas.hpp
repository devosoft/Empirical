/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file  _NativeCanvas.hpp
 *  @brief Manage an HTML canvas object.
 *
 */


#ifndef EMP_WEB_NATIVECANVAS_H
#define EMP_WEB_NATIVECANVAS_H

#include <string>
#include <memory>

#include <SFML/Graphics.hpp>

#include "../base/vector.hpp"
#include "../geometry/Circle2D.hpp"
#include "../tools/string_utils.hpp"
#include "../base/errors.hpp"

#include "_LineShape.hpp"
#include "CanvasShape.hpp"
#include "Color.hpp"
#include "Widget.hpp"
#include "LiberationSansRegular.hpp"

namespace emp {
namespace web {
  /// Manage a SFML Canvas object.
  class Canvas {

    double width{};
    double height{};
    std::string id;

    std::shared_ptr<sf::RenderTexture> window{
      std::make_shared<sf::RenderTexture>()
    };
    // sf::RenderWindow window;

  public:
    /// Create a new canvas with the specified size and optional HTML identifier.
    Canvas(const double w, const double h, const std::string & in_id="")
    : width( w )
    , height( h )
    , id( in_id ) {
    // , window( sf::VideoMode(w, h), id ) {
      window->clear();
      [[maybe_unused]] const auto res = window->create( width, height);
      emp_assert( res );
     }

     /// Create a new canvas that takes a widget and ignores it.
    Canvas(Widget& widget){}

    Canvas() = default;

    /// Get the pixel width of this Canvas.
    double GetWidth() const { return width; }

    /// Get the pixel height of this Canvas.
    double GetHeight() const { return height; }

    /// Set a new width for this Canvas.
    void SetWidth(const double w) {
         LibraryWarning("No support for changing width.");
    }

    /// Set a new height for this Canvas.
    void SetHeight(const double h) {
        LibraryWarning("No support for changing height.");
    }

    /// Set Canvas size.
    void SetSize(const double w, const double h) {
      this->SetWidth(w);
      this->SetHeight(h);
    }

    /// Add a Circle to this canvas; provide constructor for the CanvasCircle with a position and radius
    /// as well as optional face color, line color, and line width.
    Canvas & Circle(Point center, double _r, const std::string& fc, const std::string& lc, const float thickness=2) {
      double x = center.GetX() - _r;
      double y = center.GetY() - _r;
      return this->Circle(x, y, _r, emp::web::Color(fc), emp::web::Color(lc), thickness);
    }

    Canvas & Circle(double _x, double _y, double _r, const std::string& fc, const std::string& lc, const float thickness=2) {
      return this->Circle(_x, _y, _r, emp::web::Color(fc), emp::web::Color(lc), thickness);
    }

    Canvas & Circle(Point center, double _r, const emp::web::Color fc=emp::web::Color("white"),
        const emp::web::Color lc=emp::web::Color("black"), const float thickness=2) {
      double x = center.GetX() - _r;
      double y = center.GetY() - _r;
      return this->Circle(x, y, _r, fc, lc, thickness);
    }

    Canvas & Circle(double _x, double _y, double _r, const emp::web::Color fc=emp::web::Color("white"),
        const emp::web::Color lc=emp::web::Color("black"), const float thickness=2) {
      sf::CircleShape circle(_r);
      circle.setPosition(_x, _y);
      circle.setFillColor((sf::Color)fc);
      circle.setOutlineColor((sf::Color)lc);
      circle.setOutlineThickness(thickness);
      window->draw( circle );
      return *this;
    }

    /// Add a Rectangle to this canvas at x,y with width w and heigh h.  Optional face color and
    /// line color.
    Canvas & Rect(
      Point corner, const double w, const double h,
      const std::string& fc, const std::string& lc="black", const double lw=1.0
    ) {
      return this->Rect(corner.GetX(),corner.GetY(), w, h, emp::web::Color(fc), emp::web::Color(lc), lw);
    }

    Canvas & Rect(
      const double x, const double y, const double w, const double h,
      const std::string& fc, const std::string& lc="black", const double lw=1.0
    ) {
      return this->Rect(x, y, w, h, emp::web::Color(fc), emp::web::Color(lc), lw);
    }
    Canvas & Rect(
      Point corner, const double w, const double h,
      const emp::web::Color fc=emp::web::Color("white"),
      const emp::web::Color lc=emp::web::Color("black"), const double lw=1.0
    ) {
      return this->Rect(corner.GetX(),corner.GetY(), w, h, fc, lc, lw);
    }

    Canvas & Rect(
      const double x, const double y, const double w, const double h,
      const emp::web::Color fc=emp::web::Color("white"),
      const emp::web::Color lc=emp::web::Color("black"), const double lw=1.0
    ) {
      sf::RectangleShape shape( sf::Vector2f(w, h) );
      shape.setPosition(x, y);
      shape.setFillColor( (sf::Color)fc );
      shape.setOutlineColor( (sf::Color)lc );
      shape.setOutlineThickness( lw );

      window->draw( shape );
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
    Canvas & Line(
      const double x1, const double y1, const double x2, const double y2,
      const std::string& fc, const std::string& lc, const double lw=1.0
    ) {
      return this->Line(x1, y1, x2, y2, emp::web::Color(fc), emp::web::Color(lc), lw);
    }

    Canvas & Line(
      emp::Point p1, emp::Point p2, const std::string& fc,
      const std::string& lc, const double lw=1.0
      ) {
      return this->Line(p1.GetX(), p1.GetY(), p2.GetX(), p2.GetY(), emp::web::Color(fc), emp::web::Color(lc), lw);
    }

    Canvas & Line(
      const double x1, const double y1, const double x2, const double y2,
      const emp::web::Color fc=emp::web::Color("black"),
      const emp::web::Color lc=emp::web::Color("black"), const double lw=1.0
    ) {

      sf::LineShape shape( sf::Vector2f(x1, y1), sf::Vector2f(x2, y2) );
      shape.setFillColor( (sf::Color)fc );
      shape.setOutlineColor( (sf::Color)lc );
      shape.setOutlineThickness( lw );

      window->draw( shape );
      return *this;
    }

    Canvas & Line(
      emp::Point p1, emp::Point p2, const emp::web::Color fc=emp::web::Color("black"),
      const emp::web::Color lc=emp::web::Color("black"), const double lw=1.0
      ) {
      return this->Line(p1.GetX(), p1.GetY(), p2.GetX(), p2.GetY(), fc, lc, lw);
    }

    /// Add a Line from x1,y1 to x2,y2.  Optional face color and line color.
    template <typename... Ts>
    Canvas & MultiLine(emp::Point p1, const emp::vector<emp::Point> & points, const std::string& fc="",
      const std::string& lc="", const double lw=1.0) {
      return this->MultiLine(p1, points, emp::web::Color(fc), emp::web::Color(lc), lw);
    }

    template <typename... Ts>
    Canvas & MultiLine(emp::Point p1, const emp::vector<emp::Point> & points,
      const emp::web::Color fc=emp::web::Color("black"),
      const emp::web::Color lc=emp::web::Color("black"), const double lw=1.0) {
          for (auto p2 : points) {
              this->Line(p1.GetX(), p1.GetY(), p2.GetX(), p2.GetY(), fc, lc, lw);
          }
      return *this;
    }

    /// Add a string to this canvas at x,y with specified text.  Optional face color,
    /// line color, size, and thickness.
    Canvas & Text(emp::Point p, const std::string& words, const std::string& fc,
      const std::string& lc, const float size = 15, const float thickness=0.5) {
      return this->Text(p.GetX(), p.GetY(), words, emp::web::Color(fc), emp::web::Color(lc), size, thickness);
    }

    Canvas & Text(double x, double y, const std::string& words, const std::string& fc,
      const std::string& lc, const float size = 15, const float thickness=0.5) {
        return this->Text(x, y, words, emp::web::Color(fc), emp::web::Color(lc), size, thickness);

    }

    Canvas & Text(emp::Point p, const std::string& words="", const emp::web::Color fc=emp::web::Color("black"),
      const emp::web::Color lc=emp::web::Color("black"), const float size = 15, const float thickness=0.5) {
      return this->Text(p.GetX(), p.GetY(), words, fc, lc, size, thickness);
    }

    Canvas & Text(double x, double y, const std::string& words="", const emp::web::Color fc=emp::web::Color("black"),
      const emp::web::Color lc=emp::web::Color("black"), const float size = 15, const float thickness=0.5) {

        // Declare and load a font
        sf::Font font;
        font.loadFromMemory(LiberationSans_Regular_ttf, LiberationSans_Regular_ttf_len);

        // Create a text object and set attributes
        sf::Text message(words, font);
        message.setPosition(x, y);
        message.setCharacterSize(size);
        message.setFillColor((sf::Color)fc);
        message.setOutlineColor((sf::Color)lc);
        message.setOutlineThickness(thickness);

        // draw the message on the canvas
        window->draw( message );
        return *this;

    }

    /// Add a string to this canvas centered at x,y with specified text.  Optional face color and
    /// line color.
    Canvas & CenterText(emp::Point p, const std::string& words, const std::string& fc,
      const std::string& lc, const float size = 15, const float thickness=0.5) {
      this->CenterText(p.GetX(), p.GetY(), words, emp::web::Color(fc), emp::web::Color(lc), size, thickness);
      return *this;
    }

    Canvas & CenterText(double x, double y, const std::string& words, const std::string& fc,
      const std::string& lc, const float size = 15, const float thickness=0.5) {
        return this->CenterText(x, y, words, emp::web::Color(fc), emp::web::Color(lc), size, thickness);
    }

    Canvas & CenterText(emp::Point p, const std::string& words="", const emp::web::Color fc=emp::web::Color("black"),
      const emp::web::Color lc=emp::web::Color("black"), const float size = 15, const float thickness=0.5) {
      this->CenterText(p.GetX(), p.GetY(), words, fc, lc, size, thickness);
      return *this;
    }

    Canvas & CenterText(double x, double y, const std::string& words="", const emp::web::Color fc=emp::web::Color("black"),
      const emp::web::Color lc=emp::web::Color("black"), const float size = 15, const float thickness=0.5) {
      // Declare and load a font
        sf::Font font;
        font.loadFromMemory(LiberationSans_Regular_ttf, LiberationSans_Regular_ttf_len);

        sf::Text message(words, font);
        //message.setPosition(x, y);
        x += message.getLocalBounds().width/2;
        y += message.getLocalBounds().height/2;
        message.setPosition(x, y);
        message.setCharacterSize(size);
        message.setFillColor((sf::Color)fc );
        message.setOutlineColor((sf::Color)lc);
        message.setOutlineThickness(thickness);
        window->draw( message );
        return *this;
    }

    /// Update the default font for text.
    Canvas & Font(const std::string font) {
       LibraryWarning("No support for selecting fonts.");
      return *this;
    }

    /// Draw a circle onto this canvas.
    Canvas & Draw(const emp::Circle & circle,
                  const std::string & fc, const std::string & lc) {
                    return this->Draw(circle, emp::web::Color(fc), emp::web::Color(lc));
    }
    Canvas & Draw(const emp::Circle & circle,
                  const emp::web::Color fc=emp::web::Color("white"),
                  const emp::web::Color lc=emp::web::Color("black")) {
                    this->Circle(width/2, height/2, 5, fc, lc);
      return *this;
    }

    /// Draw an arbitrary shape onto this canvas.
    Canvas & Draw(const CanvasShape & shape) {
      shape.Apply( *window );
      return *this;
    }

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
      window->clear();
      return *this;
    }

    /// Clear to a specific background color.
    Canvas & Clear(const std::string & bg_color) {
      window->clear((sf::Color) emp::web::Color( bg_color ));
      return *this;
    }

    /// Download a PNG image of a canvas.
    void DownloadPNG() { DownloadPNG(id + ".png"); }

    /// Download a PNG image of a canvas.
    void DownloadPNG(const std::string & fname) { SavePNG( fname ); }

    /// Save a PNG image of a canvas.
    void SavePNG(const std::string& fname) {
      window->display();
      window->getTexture().copyToImage().saveToFile( fname );
      // window->capture().saveToFile( fname );
    }

  };

} // namespace web
} // namespace emp


#endif
