#ifndef EMP_UI_CANVAS_H
#define EMP_UI_CANVAS_H

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
namespace UI {

  ///////////////  Main Canvas Class ////////////////

  class Canvas : public internal::Widget<Canvas> {
  protected:
    int width;
    int height;
    bool needs_first_update;

    emp::vector<CanvasAction *> actions;
    std::size_t next_action;

    void WriteHTML(std::ostream & os) {
      os << "<canvas id=\"" << div_id << obj_ext
         << "\" width=\"" << width
         << "\" height=\"" << height << "\">";
      // @CAO We can include fallback content here for browsers that don't support canvas.
      os << "</canvas>";
      needs_first_update = false;
    }    

    void ClearActions() {
      for (auto * a : actions) delete a;
      actions.resize(0);
      next_action = 0;
    }

    void TriggerJS() {      // Process Actions...
      // Setup the canvas
      EM_ASM_ARGS({
          var cname = Pointer_stringify($0);
          var canvas = document.getElementById(cname);
          emp.ctx = canvas.getContext('2d');
        }, GetFullID().c_str());

      next_action = 0; // @CAO Move when we go to a Redraw() and Refresh system...
      while (next_action < actions.size()) {
        actions[next_action]->Apply();
        ++next_action;
      };

      // Trigger any JS actions from the base class.
      Widget::TriggerJS();
    }

    Canvas & AddAction(CanvasAction * new_action) {
      actions.push_back(new_action);
      return *this;
    }


  public:
    Canvas(int w, int h, const std::string & in_name="")
      : Widget(in_name), width(w), height(h), needs_first_update(true), next_action(0) 
    { obj_ext = "__c"; }
    Canvas(const Canvas & in)
      : internal::Widget<Canvas>(in), width(in.width), height(in.height)
      , needs_first_update(true), next_action(0)
    {
      actions.resize(in.actions.size());
      for (int i = 0; i < (int) actions.size(); i++) {
        actions[i] = in.actions[i]->Clone();
      }
    }
    Canvas(Canvas && in)
      : internal::Widget<Canvas>(in), width(in.width), height(in.height), needs_first_update(true)
      , actions(in.actions), next_action(0)
    {
      in.actions.resize(0);
      in.next_action = 0;
    }

    Canvas & operator=(const Canvas &) = delete;
    ~Canvas() { ClearActions(); }

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

    // Setup Canvas Actions
    Canvas & Circle(double x, double y, double r,
                    const std::string & fc="", const std::string & lc="") {
      return AddAction( new CanvasCircle(x, y, r, fc, lc) );
    }
    Canvas & Circle(const emp::Circle<> & circle,
                    const std::string & fc="", const std::string & lc="") {
      return AddAction( new CanvasCircle(circle, fc, lc) );
    }
    Canvas & Rect(double x, double y, double w, double h,
                  const std::string & fc="", const std::string & lc="") {
      return AddAction( new CanvasRect(x, y, w, h, fc, lc) );
    }
    Canvas & StrokeColor(std::string c) { return AddAction( new CanvasStrokeColor(c) ); }

    Canvas & Clear() {
      ClearActions();
      AddAction( new CanvasClearRect(0, 0, GetWidth(), GetHeight()) );
      return *this;
    }


    static std::string TypeName() { return "Canvas"; }


    // Refresh() will apply new actions to the screen.  Return value is whether a change was made.

    bool Refresh() {
      emp_assert(IsElement());                          // Can only refresh if we are an element.
      
      // Stop if nothing to refresh or the document isn't ready.
      if (next_action == actions.size() || needs_first_update) return false;

      // Setup the canvas info to act upon.
      EM_ASM_ARGS({
          var cname = Pointer_stringify($0);
          var canvas = document.getElementById(cname);
          emp.ctx = canvas.getContext('2d');
        }, GetFullID().c_str());

      while (next_action < actions.size()) {
        actions[next_action]->Apply();
        ++next_action;
      };

      return true;
    }


    bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
      bool ok = true;

      return ok;
    }
  };

};
};

#endif

