#ifndef EMP_BUTTON_H
#define EMP_BUTTON_H

#include <string>

#include "../tools/const.h"
#include "../emtools/Color.h"
#include "canvas_tools.h"
#include "Kinetic.h"

namespace emp {
  
  class Button : public CustomShape {
  private:
    bool is_active;  // Is this button currently functional?
    bool mouse_down;
    bool mouse_over;

    std::string name;
    std::string tooltip;

    Callback * trigger_cb;
    Callback * draw_icon_cb;

    // By default, this will be a square button with optionally rounded corners, but the draw function can be replaced.
    bool ul_round;
    bool ur_round;
    bool ll_round;
    bool lr_round;

    Color color_bg;
    Color color_bg_toggled;
    Color color_bg_mouseover;
    Color color_bg_toggled_mouseover;
    Color color_bg_mousedown;

  public:
    Button(const std::string & _name="")
      : CustomShape(this, &Button::Default_Draw)
      , is_active(true), mouse_down(false), mouse_over(false), name(_name)
      , trigger_cb(NULL), draw_icon_cb(NULL)
      , ul_round(true), ur_round(true), ll_round(true), lr_round(true)
      , color_bg("rgb(255,250,245)"), color_bg_toggled("rgb(255,255,100)"), color_bg_mouseover("rgb(240,240,255)")
      , color_bg_toggled_mouseover("rgb(250,250,200)"), color_bg_mousedown("blue")
    {
      On("mousedown", this, &Button::Default_OnMouseDown);
      On("mouseup", this, &Button::Default_OnMouseUp);
      On("mouseenter", this, &Button::Default_OnMouseEnter);
      On("mouseleave", this, &Button::Default_OnMouseLeave);
    }
    template<class T> Button(T * _target, void (T::*_method_ptr)(), const std::string & _name="")
      : CustomShape(this, &Button::Default_Draw)
      , is_active(true), mouse_down(false), mouse_over(false), name(_name)
      , trigger_cb(NULL), draw_icon_cb(NULL)
      , ul_round(true), ur_round(true), ll_round(true), lr_round(true)
      , color_bg("rgb(255,250,245)"), color_bg_toggled("rgb(255,255,100)"), color_bg_mouseover("rgb(240,240,255)")
      , color_bg_toggled_mouseover("rgb(250,250,200)"), color_bg_mousedown("blue")
    {
      assert(_target && _method_ptr);
      trigger_cb = new emp::MethodCallback<T>(_target, _method_ptr);
      On("mousedown", this, &Button::Default_OnMouseDown);
      On("mouseup", this, &Button::Default_OnMouseUp);
      On("mouseenter", this, &Button::Default_OnMouseEnter);
      On("mouseleave", this, &Button::Default_OnMouseLeave);
    }
    virtual ~Button() { ; }

    Button & SetActive(bool _in=true) { is_active = _in; return *this; }
    Button & SetToolTip(const std::string & msg) { tooltip = msg; return *this; }
    Button & SetRoundCorners(bool _ul=true, bool _ur=true, bool _lr=true, bool _ll=true) {
      ul_round = _ul;      ur_round = _ur;
      ll_round = _ll;      lr_round = _lr;
      return *this;
    }
    Button & SetRoundCornerUL(bool round=true) { ul_round = round; return *this; }
    Button & SetRoundCornerUR(bool round=true) { ur_round = round; return *this; }
    Button & SetRoundCornerLR(bool round=true) { lr_round = round; return *this; }
    Button & SetRoundCornerLL(bool round=true) { ll_round = round; return *this; }

    Shape & SetFillPatternImage(const Image & _image) {
      // Don't automatically draw the image here, just record it.
      image = &_image;
      return *this;
    }

    Button & SetBGColor(const Color & _color) { color_bg = _color; return *this; }
    Button & SetBGColorToggled(const Color & _color) { color_bg_toggled = _color; return *this; }
    Button & SetBGColorMouseover(const Color & _color) { color_bg_mouseover = _color; return *this; }
    Button & SetBGColorToggledMouseover(const Color & _color) { color_bg_toggled_mouseover = _color; return *this; }
    Button & SetBGColorMousedown(const Color & _color) { color_bg_mousedown = _color; return *this; }

    template<class T> Button & SetTrigger(T * _target, void (T::*_method_ptr)()) {
      if (trigger_cb != NULL) delete trigger_cb;
      trigger_cb = new emp::MethodCallback<T>(_target, _method_ptr);
      return *this;
    }

    template<class T> Button & SetDrawIcon(T * _target, void (T::*_method_ptr)(Canvas &)) {
      if (draw_icon_cb != NULL) delete draw_icon_cb;
      draw_icon_cb = new Callback_Canvas<T>(_target, _method_ptr);
      return *this;
    }

    virtual void Toggle() { ; }  // If this is a toggle button, switch its state.
    virtual bool ToggleOn() const { return false; }
    inline void Trigger() { trigger_cb->DoCallback(); }
    void UpdateHover() { ; } // @CAO -- figure out what to do here.

    // A few default behaviors...
    void Default_Draw(Canvas & canvas)
    {
      const int width = GetWidth();
      const int height = GetHeight();

      // Set the button color
      if (mouse_down) canvas.SetFill(color_bg_mousedown);
      else if (mouse_over) {
        if (ToggleOn()) canvas.SetFill(color_bg_toggled_mouseover);
        else canvas.SetFill(color_bg_mouseover);
      }
      else {
        if (ToggleOn()) canvas.SetFill(color_bg_toggled);
        else canvas.SetFill(color_bg);
      }

      canvas.SetLineWidth(ToggleOn() ? 4:2);
      DrawRoundedRect(canvas, 0, 0, width, height, 8, true, true, ul_round, ur_round, lr_round, ll_round);

      // Draw the appropriate icon.
      canvas.Save();
      if (draw_icon_cb) {
        // Shift the icon to be on a 100-height grid, and shift back afterward.
        canvas.Translate(5, 5);
        canvas.Scale( ((double) height-10.0) / 100.0 );
        draw_icon_cb->DoCallback();
      }
      else if (image) { // otherwise, if there's an image, use it.
        canvas.DrawImage(*image, 0, 0, width, height);
      }
      canvas.Restore();
  
      // Make the button clickable (or grayed out!)
      canvas.SetLineWidth(2);
      if (is_active == false) canvas.SetFill("rgba(200,200,200,0.5)");
      DrawRoundedRect(canvas, 0, 0, width, height, 8, !is_active, false, ul_round, ur_round, lr_round, ll_round);

      canvas.SetupTarget(*this);
    }

    void Default_OnMouseDown(const emp::EventInfo &) {
      if (is_active == false) return;
      mouse_down = true;
      DrawLayer();
    }

    void Default_OnMouseUp(const emp::EventInfo &) {
      if (is_active == false) return;
      mouse_down = false;
      Toggle();                         // If this is a toggle button...
      Trigger(); // @CAO Make sure to update board if relevant!
      UpdateHover();
      DrawLayer();
      // UpdateTooltip(this.x, this.y, this.tooltip); // ?????
    }
    

    void Default_OnMouseEnter(const emp::EventInfo &) {
      mouse_over = true;
      UpdateHover();
      DrawLayer();
      // UpdateTooltip(this.x, this.y, this.tooltip); // ?????
    }
  
    void Default_OnMouseLeave(const emp::EventInfo &) {
      mouse_over = false;
      mouse_down = false;
      // highlight_id = -1;  // Need to unhighlight and redraw in sudoku... @CAO
      DrawLayer();
      // HideTooltip();
    }
  
  };

  class ToggleButton : public Button {
  private:
    bool is_pressed;

  public:
    template<class T> ToggleButton(T * _target, void (T::*_method_ptr)(), const std::string & _name="")
      : Button(_target, _method_ptr, _name), is_pressed(false) { ; }
    ~ToggleButton() { ; }

    void Toggle() { is_pressed = !is_pressed; }
    bool ToggleOn() const { return is_pressed; }
    
    bool IsPressed() const { return is_pressed; }
  };
  
};

#endif
