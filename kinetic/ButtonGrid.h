#ifndef EMP_BUTTON_GRID_H
#define EMP_BUTTON_GRID_H

#include "Button.h"

#include <vector>

namespace emp {
  
  class ButtonGrid : public ObjectGrid<Button> {
  private:
    int button_width;
    int button_height;

  public:
    ButtonGrid(int _cols, int _rows, int _x, int _y, int _width, int _height, int _spacing=0)
      : ObjectGrid(_cols, _rows, _x, _y, spacing)
      , button_width(_width), button_height(_height)
    {
      const int extra_width = button_width + spacing;  // Gives room for all buttons to have spacing on right.
      const int extra_height = button_height + spacing;  // Gives room for all buttons to have a spacing below.

      for (int i = 0; i < cols; i++) {
        const int cur_x = x + extra_width * i;
        for (int j = 0; j < rows; j++) {
          const int cur_y = y + extra_height * j;
          Button * new_button = new Button();
          new_button->SetLayout(cur_x, cur_y, button_width, button_height);
          object_set[i + j * cols] = new_button;
        }
      }

    }
    ButtonGrid(int _cols, int _rows, const Point<int> & point, int _width, int _height, int _spacing=0)
      : ButtonGrid(_cols, _rows, point.GetX(), point.GetY(), _width, _height, _spacing) { ; }

    ~ButtonGrid() {
      for (int i = 0; i < (int) object_set.size(); i++) {
        if (object_set[i]) delete object_set[i];
      }
    }

    int GetWidth() const { return cols * (button_width + spacing) - spacing; }
    int GetHeight() const { return rows * (button_height + spacing) - spacing; }

    ButtonGrid & SetActive(bool active=true) { for (int i = 0; i < set_size; i++) { object_set[i]->SetActive(active); } return *this; }
    ButtonGrid & SetRoundCorners(bool ul=true, bool ur=true, bool lr=true, bool ll=true) {
      for (int i = 0; i < set_size; i++) { object_set[i]->SetRoundCorners(ul, ur, lr, ll); }
      return *this;
    }
    void AutoRoundCorners() {
      for (int i = 0; i < set_size; i++) object_set[i]->SetRoundCorners(false, false, false, false);
      object_set[0]->SetRoundCornerUL();
      object_set[cols-1]->SetRoundCornerUR();
      object_set[(rows-1) * cols + 1]->SetRoundCornerLR();
      object_set[set_size-1]->SetRoundCornerLL();
    }
    ButtonGrid & SetFillPatternImage(const Image & image) {
      for (int i = 0; i < set_size; i++) { object_set[i]->SetFillPatternImage(image); }
      return *this;
    }

    ButtonGrid & SetBGColor(const Color & color) { 
      for (int i = 0; i < set_size; i++) { object_set[i]->SetBGColor(color); }
      return *this;
    }
    ButtonGrid & SetBGColorToggled(const Color & color) {
      for (int i = 0; i < set_size; i++) { object_set[i]->SetBGColorToggled(color); }
      return *this;
    }
    ButtonGrid & SetBGColorMouseover(const Color & color) {
      for (int i = 0; i < set_size; i++) { object_set[i]->SetBGColorMouseover(color); }
      return *this;
    }
    ButtonGrid & SetBGColorToggledMouseover(const Color & color) {
      for (int i = 0; i < set_size; i++) { object_set[i]->SetBGColorToggledMouseover(color); }
      return *this;
    }
    ButtonGrid & SetBGColorMousedown(const Color & color) {
      for (int i = 0; i < set_size; i++) { object_set[i]->SetBGColorMousedown(color); }
      return *this;
    }

    Point<int> GetUL(int x_offset=0, int y_offset=0) const { return Point<int>(x+x_offset,               y+y_offset); }
    Point<int> GetUM(int x_offset=0, int y_offset=0) const { return Point<int>(x+GetWidth()/2+x_offset,  y+y_offset); }
    Point<int> GetUR(int x_offset=0, int y_offset=0) const { return Point<int>(x+GetWidth()+x_offset,    y+y_offset); }
    Point<int> GetML(int x_offset=0, int y_offset=0) const { return Point<int>(x+x_offset,               y+GetHeight()/2+y_offset); }
    Point<int> GetMM(int x_offset=0, int y_offset=0) const { return Point<int>(x+GetWidth()/2+x_offset,  y+GetHeight()/2+y_offset); }
    Point<int> GetMR(int x_offset=0, int y_offset=0) const { return Point<int>(x+GetWidth()+x_offset,    y+GetHeight()/2+y_offset); }
    Point<int> GetLL(int x_offset=0, int y_offset=0) const { return Point<int>(x+x_offset,               y+GetHeight()+y_offset); }
    Point<int> GetLM(int x_offset=0, int y_offset=0) const { return Point<int>(x+GetWidth()/2+x_offset,  y+GetHeight()+y_offset); }
    Point<int> GetLR(int x_offset=0, int y_offset=0) const { return Point<int>(x+GetWidth()+x_offset,    y+GetHeight()+y_offset); }
    Point<int> GetCenter(int x_offset=0, int y_offset=0) const { return GetMM(x_offset, y_offset); }
  };
  
};

#endif
