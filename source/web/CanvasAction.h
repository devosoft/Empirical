/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file  CanvasAction.h
 *  @brief Define a base class for all actions that can be done to widgets, plus simple actions.
 *
 *  CanvasAction objects modify the appearance of a canvas and can be tracked to reconstruct the
 *  state of the canvas from scratch.
 *
 *  Other, more specific actions defined here are:
 *    CanvasStrokeColor
 *    CanvasRotate
 *    CanvasFont
 *
 *  See also CanvasShape.h for more actions.
 */


#ifndef EMP_WEB_CANVAS_ACTION_H
#define EMP_WEB_CANVAS_ACTION_H

#include <string>

#include "Widget.h"

namespace emp {
namespace web {

  /// Base class to maintain canvas actions.
  class CanvasAction {
  protected:
    /// Helper function to set the fill status.
    void Fill(const std::string & style="") {
      if (style != "") {
        EM_ASM_ARGS({
            emp_i.ctx.fillStyle = Pointer_stringify($0);
          }, style.c_str());
      }
      EM_ASM({ emp_i.ctx.fill(); });
    }

    /// Helper function to set the stroke status.
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


    virtual void Apply() = 0;                  ///< Apply current action to emp_i.ctx.
    virtual CanvasAction * Clone() const = 0;  ///< Make a copy of the current action.
  };


  /// Set the line color on subsequent draw-related actions.
  class CanvasStrokeColor : public CanvasAction {
  protected:
    std::string color;  ///< Color to use.
  public:
    CanvasStrokeColor(const std::string & c) : color(c) { ; }

    void Apply() {
      EM_ASM_ARGS({
        var color = Pointer_stringify($0);
        emp_i.ctx.strokeStyle = color;
      }, color.c_str());
    }
    CanvasAction * Clone() const { return new CanvasStrokeColor(*this); }
  };

  /// Rotate the entire canvas for subsequent drawings.
  class CanvasRotate : public CanvasAction {
  protected:
    double angle;
  public:
    CanvasRotate(double a) : angle(a) { ; }

    void Apply() {
      EM_ASM_ARGS({
        emp_i.ctx.rotate($0);
      }, angle);
    }
    CanvasAction * Clone() const { return new CanvasRotate(*this); }
  };


  /// Change the default font to be used.
  class CanvasFont : public CanvasAction {
  protected:
    std::string font;
  public:
    CanvasFont(const std::string & f) : font(f) { ; }

    void Apply() {
      EM_ASM_ARGS({
        emp_i.ctx.font = Pointer_stringify($0);
      }, font.c_str());
    }
    CanvasAction * Clone() const { return new CanvasFont(*this); }
  };

}
}

#endif
