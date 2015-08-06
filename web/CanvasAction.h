#ifndef EMP_WEB_CANVAS_ACTION_H
#define EMP_WEB_CANVAS_ACTION_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the CanvasAction widget, which is a base class for all available
//  actions on Canvases.
//
//  Other specific actions defined here include:
//    CanvasStrokeColor
//

#include <string>

#include "Widget.h"

namespace emp {
namespace web {

  // Base class to maintain canvas actions.
  class CanvasAction {
  protected:
    // Helper functions that may be useful to specific actions.
    void Fill(const std::string & style="") {
      if (style != "") {
        EM_ASM_ARGS({
            emp_i.ctx.fillStyle = Pointer_stringify($0);
          }, style.c_str());
      }
      EM_ASM({ emp_i.ctx.fill(); });
    }
    void Stroke(const std::string & style="") {
      if (style != "") {
        EM_ASM_ARGS({
            emp_i.ctx.strokeStyle = Pointer_stringify($0);
          }, style.c_str());
      }
      EM_ASM({ emp_i.ctx.stroke(); });
      
    }

  public:
    CanvasAction() { EMP_TRACK_CONSTRUCT(CanvasAction); }
    CanvasAction(const CanvasAction &) { EMP_TRACK_CONSTRUCT(CanvasAction); }
    virtual ~CanvasAction() { EMP_TRACK_DESTRUCT(CanvasAction); }

    
    virtual void Apply() = 0;            // Apply current action to emp_i.ctx.
    virtual CanvasAction * Clone() = 0;  // Make a copy of the current action.
  };


  class CanvasStrokeColor : public CanvasAction {
    std::string color;
  public:
    CanvasStrokeColor(const std::string & c) : color(c) { ; }

    void Apply() {
      EM_ASM_ARGS({
          var color = Pointer_stringify($0);
          emp_i.ctx.strokeStyle = color;
        }, color.c_str());
    }
    CanvasAction * Clone() { return new CanvasStrokeColor(*this); }
  };


}
}

#endif
