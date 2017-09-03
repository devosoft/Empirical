/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
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

  ///  To build a Canvas object, a width and height (in pixels, by default) need to be
  ///  supplied, along with an optional HTML id.
  ///
  ///  As of now, only circles, rectangles lines, and text are supported, but more soon.

  class Canvas : public internal::WidgetFacet<Canvas> {
    friend class CanvasInfo;
  protected:

    class CanvasInfo : public internal::WidgetInfo {
      friend Canvas;

    protected:
      size_t width;   ///< pixel width of the canvas.
      size_t height;  ///< pixel height of the canvas.

      emp::vector<CanvasAction *> actions;

      CanvasInfo(const std::string & in_id="") : internal::WidgetInfo(in_id) { ; }
      CanvasInfo(const CanvasInfo &) = delete;               // No copies of INFO allowed
      CanvasInfo & operator=(const CanvasInfo &) = delete;   // No copies of INFO allowed
      virtual ~CanvasInfo() { ClearActions(); }

      std::string TypeName() const override { return "CanvasInfo"; }

      virtual bool IsCanvasInfo() const override { return true; }

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
            var cname = Pointer_stringify($0);
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
    /// Create a new canvas with the specified size and option HTML identifier.
    Canvas(size_t w, size_t h, const std::string & in_id="")
      : WidgetFacet(in_id)
    {
      info = new CanvasInfo(in_id);
      Info()->width = w;
      Info()->height = h;
    }

    /// Link to an existing canvas.
    Canvas(const Canvas & in) : WidgetFacet(in) { ; }
    Canvas(const Widget & in) : WidgetFacet(in) { emp_assert(info->IsCanvasInfo()); }
    Canvas() { ; }
    virtual ~Canvas() { ; }

    using INFO_TYPE = CanvasInfo;

    size_t GetWidth() const { return Info()->width; }    ///< Get the pixel width of this Canvas.
    size_t GetHeight() const { return Info()->height; }  ///< Get the pixel height of this Canvas.

    void SetWidth(size_t w) { Info()->width=w; }         ///< Set a new width for this Canvas.
    void SetHeight(size_t h) { Info()->height=h; }       ///< Set a new height for this Canvas.

    /// Set Canvas size.
    void SetSize(size_t w, size_t h) { Info()->width=w; Info()->height=h; }

    /// Add a Circle to this canvas centered at x,y with radius r.  Optional face color and
    /// line color.
    Canvas & Circle(double x, double y, double r,
                    const std::string & fc="", const std::string & lc="") {
      Info()->AddAction( new CanvasCircle(x, y, r, fc, lc) );
      return *this;
    }

    /// Add a Circle specified with a Circle object.  Optional face color and line color.
    Canvas & Circle(const emp::Circle & circle,
                    const std::string & fc="", const std::string & lc="") {
      Info()->AddAction( new CanvasCircle(circle, fc, lc) );
      return *this;
    }

    /// Add a Rectangle to this canvas at x,y with width w and heigh h.  Optional face color and
    /// line color.
    Canvas & Rect(double x, double y, double w, double h,
                  const std::string & fc="", const std::string & lc="") {
      Info()->AddAction( new CanvasRect(x, y, w, h, fc, lc) );
      return *this;
    }

    /// Add a Line from x1,y1 to x2,y2.  Optional face color and line color.
    Canvas & Line(double x1, double y1, double x2, double y2, const std::string & lc="") {
      Info()->AddAction( new CanvasLine(x1, y1, x2, y2, lc) );
      return *this;
    }

    /// Add a string to this canvas at x,y with specified text.  Optional face color and
    /// line color.
    Canvas & Text(double x, double y, const std::string text,
                  const std::string & fc="", const std::string & lc="") {
      Info()->AddAction( new CanvasText(x, y, text, fc, lc) );
      return *this;
    }

    /// Add a string to this canvas centered at x,y with specified text.  Optional face color and
    /// line color.
    Canvas & CenterText(double x, double y, const std::string text,
                  const std::string & fc="", const std::string & lc="") {
      auto * ctext = new CanvasText(x, y, text, fc, lc);
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
      Info()->AddAction( new CanvasClearRect(0, 0, GetWidth(), GetHeight()) );
      return *this;
    }

    /// Clear to a specific background color.
    Canvas & Clear(const std::string & bg_color) {
      Info()->ClearActions();
      Info()->AddAction( new CanvasClearRect(0, 0, GetWidth(), GetHeight()) );
      Info()->AddAction( new CanvasRect(0, 0, GetWidth(), GetHeight(), bg_color, "") );
      return *this;
    }

  };


}
}

#endif
