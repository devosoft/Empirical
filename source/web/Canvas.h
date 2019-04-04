/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file  Canvas.h
 *  @brief Manage an HTML canvas object.
 *
 */


#ifndef EMP_WEB_CANVAS_H
#define EMP_WEB_CANVAS_H

#include <string>

#include "../base/vector.h"
#include "../geometry/Circle2D.h"

#include "CanvasAction.h"
#include "CanvasShape.h"

namespace emp {
namespace web {

  /// Manage an HTML Canvas object.
  class Canvas : public internal::WidgetFacet<Canvas> {
    friend class CanvasInfo;
  protected:

    class CanvasInfo : public internal::WidgetInfo {
      friend Canvas;

    protected:
      double width;   ///< pixel width of the canvas.
      double height;  ///< pixel height of the canvas.

      emp::vector<CanvasAction *> actions;

      CanvasInfo(const std::string & in_id="") : internal::WidgetInfo(in_id) { ; }
      CanvasInfo(const CanvasInfo &) = delete;               // No copies of INFO allowed
      CanvasInfo & operator=(const CanvasInfo &) = delete;   // No copies of INFO allowed
      virtual ~CanvasInfo() { ClearActions(); }

      std::string GetTypeName() const override { return "CanvasInfo"; }

      virtual void GetHTML(std::stringstream & HTML) override {
        HTML.str("");                                           // Clear the current text.
        HTML << "<canvas id=\"" << id
             << "\" width=\"" << width
             << "\" height=\"" << height << "\">";
        // @CAO We can include fallback content here for browsers that don't support canvas.
        HTML << "</canvas>";
      }

      // Setup a canvas to be drawn on.
      void TargetCanvas() {
        EM_ASM_ARGS({
            var cname = UTF8ToString($0);
            var canvas = document.getElementById(cname);
            emp_i.ctx = canvas.getContext('2d');
        }, id.c_str());
      }

      // Trigger any JS code needed on re-draw.
      void TriggerJS() override {
        if (state == Widget::ACTIVE) {            // Only draw on active canvases
          TargetCanvas();                         // Prepare the canvas for drawing
          for (auto & a : actions) a->Apply();    // Run all of the actions
        }
      }

      void AddAction(CanvasAction * new_action) {
        if (state == Widget::ACTIVE) {    // Only draw on active canvases
          TargetCanvas();                 // Prepare the canvas for drawing
          new_action->Apply();            // Draw the current action
        }
        actions.push_back(new_action);    // Store the current action.
      }

      void ClearActions() {
        for (auto * a : actions) delete a;
        actions.resize(0);
      }


    public:
      virtual std::string GetType() override { return "web::CanvasInfo"; }

    };  // End of ButtonInfo definition.


    // Get a properly cast version of indo.
    CanvasInfo * Info() { return (CanvasInfo *) info; }
    const CanvasInfo * Info() const { return (CanvasInfo *) info; }

    Canvas(CanvasInfo * in_info) : WidgetFacet(in_info) { ; }

  public:
    /// Create a new canvas with the specified size and optional HTML identifier.
    Canvas(double w, double h, const std::string & in_id="")
      : WidgetFacet(in_id)
    {
      info = new CanvasInfo(in_id);
      Info()->width = w;
      Info()->height = h;
    }

    /// Link to an existing canvas.
    Canvas(const Canvas & in) : WidgetFacet(in) { ; }
    Canvas(const Widget & in) : WidgetFacet(in) { emp_assert(in.IsCanvas()); }
    Canvas() { ; }
    virtual ~Canvas() { ; }

    using INFO_TYPE = CanvasInfo;

    double GetWidth() const { return Info()->width; }    ///< Get the pixel width of this Canvas.
    double GetHeight() const { return Info()->height; }  ///< Get the pixel height of this Canvas.

    void SetWidth(double w) { Info()->width=w; }         ///< Set a new width for this Canvas.
    void SetHeight(double h) { Info()->height=h; }       ///< Set a new height for this Canvas.

    /// Set Canvas size.
    void SetSize(double w, double h) { Info()->width=w; Info()->height=h; }

    /// Add a Circle to this canvas; provide constructor for the CanvasCircle with a position and radius
    /// as well as optional face color, line color, and line width.
    template <typename... Ts>
    Canvas & Circle(Point center, double _r, Ts &&... vals) {
      Info()->AddAction( new CanvasCircle(emp::Circle(center, _r), std::forward<Ts>(vals)...) );
      return *this;
    }

    template <typename... Ts>
    Canvas & Circle(double _x, double _y, double _r, Ts &&... vals) {
      Info()->AddAction( new CanvasCircle(emp::Circle(_x, _y, _r), std::forward<Ts>(vals)...) );
      return *this;
    }

    /// Add a Rectangle to this canvas at x,y with width w and heigh h.  Optional face color and
    /// line color.
    template <typename... Ts>
    Canvas & Rect(Point corner, double w, double h, Ts &&... vals) {
      Info()->AddAction( new CanvasRect(corner, w, h, std::forward<Ts>(vals)...) );
      return *this;
    }

    template <typename... Ts>
    Canvas & Rect(double x, double y, double w, double h, Ts &&... vals) {
      Info()->AddAction( new CanvasRect(x, y, w, h, std::forward<Ts>(vals)...) );
      return *this;
    }

    /// Add an Image to this canvas at x,y with width w and heigh h.
    template <typename... Ts>
    Canvas & Image(const emp::RawImage & image, Point corner, Ts &&... vals) {
      Info()->AddAction( new CanvasImage(image, corner, std::forward<Ts>(vals)...) );
      return *this;
    }

    template <typename... Ts>
    Canvas & Image(const emp::RawImage & image, double x, double y, Ts &&... vals) {
      Info()->AddAction( new CanvasImage(image, x, y, std::forward<Ts>(vals)...) );
      return *this;
    }


    /// Add a Line from x1,y1 to x2,y2.  Optional face color and line color.
    template <typename... Ts>
    Canvas & Line(double x1, double y1, double x2, double y2, Ts &&... vals) {
      Info()->AddAction( new CanvasLine(x1, y1, x2, y2, std::forward<Ts>(vals)...) );
      return *this;
    }

    template <typename... Ts>
    Canvas & Line(emp::Point p1, emp::Point p2, Ts &&... vals) {
      Info()->AddAction( new CanvasLine(p1, p2, std::forward<Ts>(vals)...) );
      return *this;
    }

    /// Add a Line from x1,y1 to x2,y2.  Optional face color and line color.
    template <typename... Ts>
    Canvas & MultiLine(emp::Point p1, const emp::vector<emp::Point> & points, Ts &&... vals) {
      Info()->AddAction( new CanvasMultiLine(p1, points, std::forward<Ts>(vals)...) );
      return *this;
    }

    /// Add a string to this canvas at x,y with specified text.  Optional face color and
    /// line color.
    template <typename... Ts>
    Canvas & Text(emp::Point p, Ts &&... vals) {
      Info()->AddAction( new CanvasText(p, std::forward<Ts>(vals)...) );
      return *this;
    }

    template <typename... Ts>
    Canvas & Text(double x, double y, Ts &&... vals) {
      Info()->AddAction( new CanvasText(x, y, std::forward<Ts>(vals)...) );
      return *this;
    }

    /// Add a string to this canvas centered at x,y with specified text.  Optional face color and
    /// line color.
    template <typename... Ts>
    Canvas & CenterText(emp::Point p, Ts &&... vals) {
      auto * ctext = new CanvasText(p, std::forward<Ts>(vals)...);
      ctext->Center();
      Info()->AddAction( ctext );
      return *this;
    }

    template <typename... Ts>
    Canvas & CenterText(double x, double y, Ts &&... vals) {
      auto * ctext = new CanvasText({x, y}, std::forward<Ts>(vals)...);
      ctext->Center();
      Info()->AddAction( ctext );
      return *this;
    }

    /// Update the default font for text.
    Canvas & Font(const std::string font) {
      Info()->AddAction( new CanvasFont(font) );
      return *this;
    }

    /// Draw a circle onto this canvas.
    Canvas & Draw(const emp::Circle & circle,
                  const std::string & fc="", const std::string & lc="") {
      Info()->AddAction( new CanvasCircle(circle, fc, lc) );
      return *this;
    }

    /// Draw an arbitrary shape onto this canvas.
    Canvas & Draw(const CanvasShape & shape) {
      Info()->AddAction( shape.Clone() );
      return *this;
    }

    /// Change the default stroke color.
    Canvas & StrokeColor(std::string c) {
      Info()->AddAction( new CanvasStrokeColor(c) );
      return *this;
    }

    /// Rotate the entire canvas.
    Canvas & Rotate(double angle) {
      Info()->AddAction( new CanvasRotate(angle) );
      return *this;
    }

    /// Clear everything off of this canvas.
    Canvas & Clear() {
      Info()->ClearActions();
      Info()->AddAction( new CanvasClearRect({0,0}, GetWidth(), GetHeight()) );
      return *this;
    }

    /// Clear to a specific background color.
    Canvas & Clear(const std::string & bg_color) {
      Info()->ClearActions();
      Info()->AddAction( new CanvasClearRect({0,0}, GetWidth(), GetHeight()) );
      Info()->AddAction( new CanvasRect({0,0}, GetWidth(), GetHeight(), bg_color, "") );
      return *this;
    }

    /// Download a PNG image of a canvas.
    void DownloadPNG() { DownloadPNG(Info()->id + ".png"); }

    /// Download a PNG image of a canvas.
    void DownloadPNG(const std::string & fname) {

      const std::string ext = ".png";
      emscripten_run_script(
        (
          std::string()
          + "emp.download(document.getElementById('"
          + Info()->id
          + "').toDataURL('img/png'), '"
          + fname
          + (fname.rfind(ext, fname.length()) == std::string::npos ? ext : "")
          + "', 'img/png');"
        ).c_str()
      );

    }

  };


}
}

#endif
