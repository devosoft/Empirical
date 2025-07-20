/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2015-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/Canvas.hpp
 * @brief Manage an HTML canvas object.
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_CANVAS_HPP_GUARD
#define INCLUDE_EMP_WEB_CANVAS_HPP_GUARD

#include <functional>
#include <string>

#include "../base/vector.hpp"
#include "../debug/debug.hpp"
#include "../geometry/Box2D.hpp"
#include "../geometry/Circle2D.hpp"
#include "../geometry/Line2D.hpp"
#include "../geometry/Polygon.hpp"
#include "../tools/string_utils.hpp"

#include "Color.hpp"
#include "init.hpp"
#include "RawImage.hpp"
#include "Widget.hpp"

namespace emp::web {

  namespace internal {
    EM_JS(void, DownloadPNG_impl, (const char* id, const char* fname), {
      let canvas = document.getElementById(UTF8ToString(id));
      let url = canvas.toDataURL("image/png");
      emp.download(url, UTF8ToString(fname), "image/png");
    });

    EM_JS(void, SavePNG_impl, (const char* id, const char* fname), {
      const canvas = document.getElementById(UTF8ToString(id));
      if (!canvas) {
        console.error("SavePNG failed: canvas not found");
        return;
      }

      canvas.toBlob(function(blob) {
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = UTF8ToString(fname);
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
      }, "image/png");
    });
  }

  /// Manage an HTML Canvas object.
  class Canvas : public internal::WidgetFacet<Canvas> {
    friend class CanvasInfo;
  protected:
    // Track if we have established context (in debug)
    static emp::String & DebugContext() {
      static emp::String context="";
      return context;
    }

    class CanvasInfo : public internal::WidgetInfo {
      friend Canvas;

    protected:
      Size2D size; // Canvas size in pixels

      using fun_t = std::function<void()>;
      emp::vector<fun_t> draw_funs;

      CanvasInfo(const std::string & in_id = "") : internal::WidgetInfo(in_id) {}

      CanvasInfo(const CanvasInfo &)             = delete;  // No copies of INFO allowed
      CanvasInfo & operator=(const CanvasInfo &) = delete;  // No copies of INFO allowed

      virtual ~CanvasInfo() { ClearActions(); }

      std::string GetTypeName() const override { return "CanvasInfo"; }

      virtual void GetHTML(std::stringstream & HTML) override {
        DEBUG_STACK();
        HTML.str("");  // Clear any current text.
        HTML << "<canvas id=\"" << id << "\" width=\"" << size.Width()
             << "\" height=\"" << size.Height() << "\">";
        HTML << "</canvas>";

        // create an offscreen canvas
        if constexpr (emp::compile::EMSCRIPTEN_PTHREADS) {
          // clang-format off
          EM_ASM({
            var cname = UTF8ToString($0);
            emp_i.offscreen_canvases[cname] = new OffscreenCanvas($1, $2);
          }, id.c_str(), size.Width(), size.Height());
          // clang-format on
        }
      }

      // Set up THIS canvas as active context.
      void TargetCanvas() {
        DEBUG_STACK();
        if constexpr (emp::compile::EMSCRIPTEN_PTHREADS) {
          // clang-format off
          EM_ASM({
            var cname  = UTF8ToString($0);
            var canvas = emp_i.offscreen_canvases[cname];
            emp_i.pending_offscreen_canvas_ids.add(cname);
            emp_i.ctx = canvas.getContext('2d');
          }, id.c_str());
          // clang-format on
        } else {
          // clang-format off
          EM_ASM({
            var cname  = UTF8ToString($0);
            var canvas = document.getElementById(cname);
            if (canvas) { emp_i.ctx = canvas.getContext('2d'); }
          }, id.c_str());
          // clang-format on
        }
        EMP_DEBUG(DebugContext() = id;)
      }

      // Trigger any JS code needed on re-draw.
      void TriggerJS() override {
        DEBUG_STACK();
        if (state == Widget::ACTIVE) {                   // Only draw on active canvases
          TargetCanvas();                                // Prepare the canvas for drawing
          for (const auto & fun : draw_funs) { fun(); }  // Apply all draw actions.
        }
      }

      template <typename FUN_T>
      void AddDrawFun(FUN_T && fun) {
        DEBUG_STACK();
        if (state == Widget::ACTIVE) {  // Only draw on active canvases
          TargetCanvas();               // Prepare the canvas for drawing
          fun();                        // Apply draw function.
        }
        draw_funs.push_back(std::forward<FUN_T>(fun));  // Store the current draw function.
      }

      void ClearActions() { draw_funs.resize(0); }

    public:
      virtual std::string GetType() override { return "web::CanvasInfo"; }

    };  // End of CanvasInfo definition.

    // Get a properly cast version of info.
    CanvasInfo * Info() { return (CanvasInfo *) info; }

    const CanvasInfo * Info() const { return (CanvasInfo *) info; }

    Canvas(CanvasInfo * in_info) : WidgetFacet(in_info) {}

    // === Helper functions to encapsulate JS code ===

    static void JS_BeginPath() {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.beginPath(); });
    }

    static void JS_ClosePath() {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.closePath(); });
    }

    static void JS_Translate(const Point & offset) {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.translate($0, $1); }, offset.GetX(), offset.GetY());
    }

    static void JS_Rotate(double angle) {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.rotate($0); }, angle);
    }

    static void JS_MoveTo(const Point & p) {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.moveTo($0, $1); }, p.GetX(), p.GetY());
    }

    static void JS_LineTo(const Point & p) {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.lineTo($0, $1); }, p.GetX(), p.GetY());
    }

    static void JS_Fill() {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.fill(); });
    }

    static void JS_Stroke() {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.stroke(); });
    }

    static void JS_SetStrokeColor(const Color & color) {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.strokeStyle = UTF8ToString($0); }, color.c_str());
    }

    static void JS_SetFillColor(const Color & color) {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.fillStyle = UTF8ToString($0); }, color.c_str());
    }

    static void JS_LineWidth(double line_width = 1.0) {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.lineWidth = $0; }, line_width);
    }
 
    static void JS_Circle(const Point & center, double radius) {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.arc($0, $1, $2, 0, Math.PI * 2); }, center.X(), center.Y(), radius);
    }

    static void JS_Rect(const Point & p, const Size2D size) {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.rect($0, $1, $2, $3); }, p.X(), p.Y(), size.Width(), size.Height());
    }

    static void JS_ClearRect(const Point & p, const Size2D size) {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.clearRect($0, $1, $2, $3); }, p.X(), p.Y(), size.Width(), size.Height());
    }

    static void JS_SetFont(const emp::String & font) {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.font = UTF8ToString($0); }, font.c_str());
    }

    static void JS_TextAlign_Center() {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.textAlign = "center"; });
    }

    static void JS_TextBaseline_Middle() {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.textBaseline = "middle"; });
    }

    static void JS_FillText(const Point & p, const emp::String & text) {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.fillText(UTF8ToString($0), $1, $2); }, text.c_str(), p.X(), p.Y());
    }

    static void JS_DrawImage(const Point & p, const RawImage & image) {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.drawImage(emp_i.images[$0], $1, $2); }, image.GetID(), p.X(), p.Y());      
    }

    static void JS_DrawImage(const Point & p, const RawImage & image, const Size2D & size) {
      DEBUG_STACK();
      emp_assert(DebugContext().size() > 0, DebugContext(), emp::DebugStackToString());
      EM_ASM({ emp_i.ctx.drawImage(emp_i.images[$0], $1, $2, $3, $4); },
        image.GetID(), p.X(), p.Y(), size.X(), size.Y());
    }

    template <typename T>
    Canvas & AddDrawFun(T && fun) {
      DEBUG_STACK();
      Info()->AddDrawFun(std::forward<T>(fun));
      return *this;
    }

    /// Add a Circle to this canvas.
    Canvas & AddCircle(Circle circle) {
      DEBUG_STACK();
      auto fun = [circle]() {
        JS_BeginPath();
        JS_Circle(circle.GetCenter(), circle.GetRadius());
      };      
      return AddDrawFun(fun);
    }

    /// Add a Rectangle to this canvas.
    Canvas & AddBox(Box2D box) {
      DEBUG_STACK();
      auto fun = [box]() {
        JS_BeginPath();
        JS_Rect(box.GetUL(), box.GetSize());
      };      
      return AddDrawFun(fun);
    }

    /// Draw a line between two points.
    Canvas & AddLine(Point start, Point end) {
      DEBUG_STACK();
      auto fun = [start, end]() {
        JS_BeginPath();
        JS_MoveTo(start);
        JS_LineTo(end);
      };      
      return AddDrawFun(fun);
    }

    /// Add a series of lines.
    Canvas & AddLines(Point start, const emp::vector<Point> & points, bool close=false) {
      DEBUG_STACK();
      emp_assert(points.size() >= 3, points.size());
      auto fun = [start, points, close]() {
        JS_BeginPath();
        JS_MoveTo(start);
        for (const Point & point : points) {
          JS_LineTo(point);
        }
        if (close) JS_ClosePath();
      };
      return AddDrawFun(fun);
    }

    /// Add a Rectangle to this canvas.
    Canvas & AddRect(Point ul_corner, Size2D size) {
      DEBUG_STACK();
      auto fun = [ul_corner, size]() {
        JS_BeginPath();
        JS_Rect(ul_corner, size);
      };      
      return AddDrawFun(fun);
    }

    /// Clear the area of a Rectangle from this canvas.
    Canvas & AddClearRect(Point ul_corner, Size2D size) {
      DEBUG_STACK();
      auto fun = [ul_corner, size]() {
        JS_BeginPath(); // @CAO Is this BeginPath needed?
        JS_ClearRect(ul_corner, size);
      };      
      return AddDrawFun(fun);
    }

    /// Draw text directly on the canvas.
    Canvas & AddText(Point p, emp::String text, bool center=false) {
      DEBUG_STACK();
      auto fun = [p, text, center]() {
        if (center) {
          JS_TextAlign_Center();
          JS_TextBaseline_Middle();
        }
        JS_FillText(p, text);
      };      
      return AddDrawFun(fun);
    }

    Canvas & AddImage(Point p, const RawImage & image) {
      DEBUG_STACK();
      auto fun = [p, image]() {
        if (image.HasLoaded()) JS_DrawImage(p, image);
      };
      return AddDrawFun(fun);
    }

    Canvas & AddImage(Point p, const RawImage & image, Size2D size) {
      DEBUG_STACK();
      auto fun = [p, image, size]() {
        if (image.HasLoaded()) JS_DrawImage(p, image, size);
      };
      return AddDrawFun(fun);
    }

    Canvas & AddImageWhenReady(Point p, RawImage & image) {
      DEBUG_STACK();
      auto fun = [p, &image]() {
        image.OnLoad([p, image](){ JS_DrawImage(p, image); });
      };
      return AddDrawFun(fun);
    }

    Canvas & AddImageWhenReady(Point p, RawImage & image, Size2D size) {
      DEBUG_STACK();
      auto fun = [p, &image, size]() {
        image.OnLoad([p, image, size](){ JS_DrawImage(p, image, size); });
      };
      return AddDrawFun(fun);
    }

  public:
    /// Create a new canvas with the specified size and optional HTML identifier.
    Canvas(Size2D size, const std::string & in_id = "") : WidgetFacet(in_id) {
      info         = new CanvasInfo(in_id);
      Info()->size = size;
    }

    /// Create a new canvas with the specified size and optional HTML identifier.
    Canvas(double w, double h, const std::string & in_id = "") : WidgetFacet(in_id) {
      info           = new CanvasInfo(in_id);
      Info()->size.Set(w, h);
    }

    /// Link to an existing canvas.
    Canvas(const Canvas & in) : WidgetFacet(in) {}

    Canvas(const Widget & in) : WidgetFacet(in) { emp_assert(in.IsCanvas()); }

    // Canvas() {}

    virtual ~Canvas() {}

    using INFO_TYPE = CanvasInfo;

    double GetWidth() const { return Info()->size.Width(); }  ///< Get the pixel width of this Canvas.

    double GetHeight() const { return Info()->size.Height(); }  ///< Get the pixel height of this Canvas.

    Size2D GetSize() const { return Info()->size; }

    /// Set Canvas size.
    void SetSize(double w, double h) { Info()->size.Set(w, h); }
    void SetSize(Size2D size) { Info()->size = size; }

    Canvas & LineWidth(double line_width = 1.0) {
      auto fun = [line_width]() { JS_LineWidth(line_width); };
      return AddDrawFun(fun);
    }

    struct ShapeFormat {
      Color fg_color{};
      Color bg_color{};
      double line_width=1.0;
    };

    Canvas & SetLineColor(const Color & color) {
      DEBUG_STACK();
      auto fun = [color]() { JS_SetStrokeColor(color); };
      return AddDrawFun(fun);
    }

    Canvas & SetFillColor(const Color & color) {
      DEBUG_STACK();
      auto fun = [color]() { JS_SetFillColor(color); };
      return AddDrawFun(fun);
    }

    Canvas & SetLineWidth(double line_width) {
      DEBUG_STACK();
      auto fun = [line_width]() { JS_LineWidth(line_width); };
      return AddDrawFun(fun);
    }

    Canvas & SetFormat(const ShapeFormat & format) {
      DEBUG_STACK();
      auto fun = [format]() {
        JS_SetStrokeColor(format.fg_color);
        JS_SetFillColor(format.bg_color);
        JS_LineWidth(format.line_width);
      };
      return AddDrawFun(fun);
    }

    Canvas & SetFont(const emp::String & font) {
      DEBUG_STACK();
      auto fun = [font]() { JS_SetFont(font); };
      return AddDrawFun(fun);
    }

    Canvas & SetTranslate(Point offset) {
      DEBUG_STACK();
      auto fun = [offset] { JS_Translate(offset); };
      return AddDrawFun(fun);
    }

    Canvas & SetRotate(double angle) {
      DEBUG_STACK();
      auto fun = [angle] { JS_Rotate(angle); };
      return AddDrawFun(fun);
    }

    Canvas & FinalizeLine() {
      DEBUG_STACK();
      auto fun = [] { JS_Stroke(); };
      return AddDrawFun(fun);
    }

    Canvas & FinalizeFill() {
      DEBUG_STACK();
      auto fun = [] { JS_Fill(); };
      return AddDrawFun(fun);
    }

    // === Some Draw() shortcuts ===

    // DRAW LINE: Two points indicates a line should be drawn between them.
    Canvas & Draw(Point start, Point end, const Color & line_color=Color{},
                  double line_width=0.0) {
      DEBUG_STACK();
      AddLine(start, end);
      if (line_color) SetLineColor(line_color);
      if (line_width > 0) SetLineWidth(line_width);
      FinalizeLine();
      return *this;
    }

    // DRAW A LINE OBJECT
    Canvas & Draw(Line2D line, const Color & line_color=Color{},
                  double line_width=0.0) {
      DEBUG_STACK();
      AddLine(line.GetStartPos(), line.GetEndPos());
      if (line_color) SetLineColor(line_color);
      if (line_width > 0) SetLineWidth(line_width);
      FinalizeLine();
      return *this;
    }

    // DRAW MULTIPLE LINES: A series of points indicates lines should be drawn connecting them.
    Canvas & Draw(Point start, const emp::vector<Point> & other,
                  const Color & line_color=Color{}, double line_width=0.0) {
      DEBUG_STACK();
      AddLines(start, other);
      if (line_color) SetLineColor(line_color);
      if (line_width > 0) SetLineWidth(line_width);
      FinalizeLine();
      return *this;
    }

    // DRAW A CIRCLE
    Canvas & Draw(const Circle2D & circle, const Color & fill_color=Color{},
                  const Color & line_color=Color{}, double line_width=0.0) {
      DEBUG_STACK();
      // emp::Alert("Circle! Fill=", fill_color.ToString(), "; Line=", line_color.ToString(),
      //            "; Width=", line_width);

      AddCircle(circle);
      if (line_color) SetLineColor(line_color);
      if (line_width > 0) SetLineWidth(line_width);
      FinalizeLine();
      if (fill_color) {
        SetFillColor(fill_color);
        FinalizeFill();
      }
      return *this;
    }

    // DRAW A RECTANGLE
    Canvas & Draw(const Box2D & box, const Color & fill_color=Color{},
                  const Color & line_color=Color{}, double line_width=0.0) {
      DEBUG_STACK();
      AddBox(box);
      if (line_color) SetLineColor(line_color);
      if (line_width > 0) SetLineWidth(line_width);
      FinalizeLine();
      if (fill_color) {
        SetFillColor(fill_color);
        FinalizeFill();
      }
      return *this;
    }

    // DRAW A GENERIC POLYGON
    Canvas & Draw(const Polygon & polygon, const Color & fill_color=Color{},
                  const Color & line_color=Color{}, double line_width=0.0) {
      DEBUG_STACK();
      SetTranslate(polygon.GetAnchor());
      AddLines(Point{0.0, 0.0}, polygon.GetOther(), true);
      SetTranslate(-polygon.GetAnchor());
      if (line_color) SetLineColor(line_color);
      if (line_width > 0) SetLineWidth(line_width);
      FinalizeLine();
      if (fill_color) {
        SetFillColor(fill_color);
        FinalizeFill();
      }
      return *this;
    }

    // DRAW PROVIDED TEXT
    Canvas & Draw(Point pos, emp::String text, bool center=true,
                  const Color & line_color=Color{}, double line_width=0.0) {
      DEBUG_STACK();
      AddText(pos, text, center);
      if (line_color) SetLineColor(line_color);
      if (line_width > 0) SetLineWidth(line_width);
      FinalizeLine();
      return *this;
    }

    // DRAW IMAGE
    Canvas & Draw(emp::RawImage & image, emp::Point position, emp::Size2D size) {
      DEBUG_STACK();
      AddImageWhenReady(position, image, size);
      return *this;
    }

    /// Clear everything off of this canvas.
    Canvas & Clear(Color bg_color = Color{}) {
      DEBUG_STACK();
      Info()->ClearActions();                           // Remove canvas history.
      if (!bg_color) bg_color = Palette::WHITE;         // Default background is white.
      return Draw(Box2D{{0, 0}, GetSize()}, bg_color);  // Draw a rectangle coverring the screen.
    }

    /// Download a PNG image of a canvas.
    void DownloadPNG(emp::String fname) const {
      if (!fname.ends_with(".png")) fname += ".png";
      internal::DownloadPNG_impl(Info()->id.c_str(), fname.c_str());
    }

    /// Download a PNG image of a canvas using the default filename.
    void DownloadPNG() const { DownloadPNG(Info()->id + ".png"); }

    /// Save a PNG image of a canvas with node.js.
    void SavePNG(emp::String fname) const {
      if (!fname.ends_with(".png")) fname += ".png";
      internal::SavePNG_impl(Info()->id.c_str(), fname.c_str());
    }
  };

}  // namespace emp::web

#endif  // #ifndef INCLUDE_EMP_WEB_CANVAS_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: ext ctext fname ctx cname
