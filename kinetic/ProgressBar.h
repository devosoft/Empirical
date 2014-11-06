#ifndef EMK_PROGRESS_BAR_H
#define EMK_PROGRESS_BAR_H

#include "../libs/Kinetic.h"
#include "canvas_tools.h"

namespace emk {

  class ProgressBar : public CustomShape {
  private:
    Color stroke_color;
    Color panel_color;
    Color bar_color_full;
    Color bar_color_empty;
    Color text_color;

    int max_count;
    int cur_count;
    
    std::string msg;
  public:
    ProgressBar(int x=0, int y=0, int w=100, int h=30)
      : CustomShape(x, y, w, h, this, &ProgressBar::Draw)
      , stroke_color("black"), panel_color(200, 200, 200), bar_color_full("white"), bar_color_empty(50,50,80), text_color("black")
      , max_count(0), cur_count(0)
    { ; }
    ~ProgressBar() { ; }

    int GetMaxCount() const { return max_count; }
    int GetCurCount() const { return cur_count; }

    ProgressBar & SetMaxCount(int _in) { max_count = _in; return *this; }
    ProgressBar & SetCurCount(int _in) { cur_count = _in; return *this; }
    ProgressBar & IncCurCount() { cur_count++; return *this; }
    ProgressBar & SetMessage(const std::string & _msg) { msg = _msg; return *this; }

    void Draw(Canvas & canvas) {
      const int width = GetWidth();
      const int height = GetHeight();
      const int bar_x = width/10;
      const int bar_y = height/4;
      const int inner_width = width - 2*bar_x;
      const int inner_height = height/4;
      const int msg_y = 2*bar_y + inner_height;
      const int bar_min = 3;
      const double frac_done = (((double) cur_count) / (double) max_count);

      canvas.SetStroke(stroke_color);
      canvas.SetFill(panel_color);

      DrawRoundedRect(canvas, 0, 0, width, height, 8, true, true);

      // canvas.Rect(0, 0, width, height, true);
      // canvas.Rect(0, 0, width, height, false);

      canvas.SetFill(bar_color_empty);
      canvas.Rect(bar_x, bar_y, inner_width, inner_height, true);
      canvas.SetFill(bar_color_full);
      canvas.Rect(bar_x, bar_y, bar_min + (inner_width-bar_min) * frac_done, inner_height, true);
      canvas.Rect(bar_x, bar_y, inner_width, inner_height, false);

      canvas.SetFont(Font(height/8));
      canvas.SetFill(text_color);
      std::string out_msg = msg + std::to_string(cur_count) + std::string("/") + std::to_string(max_count);
      canvas.Text(out_msg, bar_x, msg_y);
    }
  };

};

#endif
