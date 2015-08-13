#ifndef EMP_WEB_CANVAS_H
#define EMP_WEB_CANVAS_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Canvas widget.
//

#include <string>

#include "../geometry/Circle2D.h"
#include "../tools/vector.h"

#include "CanvasAction.h"
#include "CanvasShape.h"

namespace emp {
namespace web {

  class Canvas : public internal::WidgetFacet<Canvas> {
    friend class CanvasInfo;
  protected:

    class CanvasInfo : public internal::WidgetInfo {
      friend Canvas;

    protected:
      int width;
      int height;
      
      emp::vector<CanvasAction *> actions;

      CanvasInfo(const std::string & in_id="") : internal::WidgetInfo(in_id) { ; }
      CanvasInfo(const CanvasInfo &) = delete;               // No copies of INFO allowed
      CanvasInfo & operator=(const CanvasInfo &) = delete;   // No copies of INFO allowed
      virtual ~CanvasInfo() { ClearActions(); }
      
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
    Canvas(int w, int h, const std::string & in_id="")
      : WidgetFacet(in_id)
    {
      info = new CanvasInfo(in_id);
      Info()->width = w;
      Info()->height = h;
    }
    Canvas(const Canvas & in) : WidgetFacet(in) { ; }
    Canvas(const Widget & in) : WidgetFacet(in) { emp_assert(info->IsCanvasInfo()); }
    virtual ~Canvas() { ; }

    using INFO_TYPE = CanvasInfo;

    int GetWidth() const { return Info()->width; }
    int GetHeight() const { return Info()->height; }

    // Setup Canvas Actions
    Canvas & Circle(double x, double y, double r,
                    const std::string & fc="", const std::string & lc="") {
      Info()->AddAction( new CanvasCircle(x, y, r, fc, lc) );
      return *this;
    }
    Canvas & Circle(const emp::Circle<> & circle,
                    const std::string & fc="", const std::string & lc="") {
      Info()->AddAction( new CanvasCircle(circle, fc, lc) );
      return *this;
    }
    Canvas & Rect(double x, double y, double w, double h,
                  const std::string & fc="", const std::string & lc="") {
      Info()->AddAction( new CanvasRect(x, y, w, h, fc, lc) );
      return *this;
    }
    Canvas & StrokeColor(std::string c) {
      Info()->AddAction( new CanvasStrokeColor(c) );
      return *this;
    }

    Canvas & Clear() {
      Info()->ClearActions();
      Info()->AddAction( new CanvasClearRect(0, 0, GetWidth(), GetHeight()) );
      return *this;
    }

  };


}
}

#endif
