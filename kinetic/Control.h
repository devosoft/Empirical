#ifndef EMK_CONTROL_H
#define EMK_CONTROL_H

#include <map>
#include <unordered_set>

#include "emtools/Font.h"
#include "geometry/Point2D.h"
#include "Kinetic.h"
#include "Button.h"
#include "ButtonGrid.h"
#include "Events.h"
#include "Grid.h"
#include "Panel.h"

namespace emp {

  class Control {
  protected:
    // What aspect ratio should the screen have?
    // aspect_ratio = width/height; a 16:9 ratio would be 1.778 (use values <= 0 to adjust values freely)
    double aspect_ratio;
    int base_width;
    int base_height;

    std::map<std::string, Stage *> stage_map;
    std::map<std::string, Layer *> layer_map;

    std::map<std::string, Color *> color_map;
    std::map<std::string, Font *> font_map;
    std::map<std::string, Point<int> *> point_map;

    std::map<std::string, Image *> image_map;
    std::map<std::string, Rect *> rect_map;
    std::map<std::string, TextBox *> text_map;

    std::map<std::string, Button *> button_map;
    std::map<std::string, ButtonGrid *> buttonset_map;
    std::map<std::string, Grid *> grid_map;
    std::map<std::string, Panel *> panel_map;

    std::map<std::string, Callback *> animation_map;  // Animations are templates so using base class in map.
    std::map<std::string, EventChain *> eventchain_map;
    std::map<std::string, Tween *> tween_map;

    std::map<std::string, Shape *> shape_map; // Fill map of all objects, by name.

    // Setup storage for temporary objects that don't have a name.
    class TempObj_Base {
    public:
      TempObj_Base() { ; }
      virtual ~TempObj_Base() { ; }
    };

    template <class T> class TempObj : public TempObj_Base {
    private:
      T * obj_ptr;
    public:
      TempObj(T * _obj) : obj_ptr(_obj) { ; }
      ~TempObj() { delete obj_ptr; }
    };
    std::unordered_set<TempObj_Base*> temp_objs;
    template <class T> void ManageTemp(T * obj) { temp_objs.insert(new TempObj<T>(obj)); }
    
    // Current object being worked with.
    Stage * cur_stage;
    Layer * cur_layer;
    Color * cur_color;
    Font * cur_font;
    Point<int> * cur_point;
    Image * cur_image;
    Rect * cur_rect;
    TextBox * cur_text;
    Button * cur_button;
    ButtonGrid * cur_buttonset;
    Grid * cur_grid;
    Panel * cur_panel;
    // cur_animation ??
    EventChain * cur_eventchain;
    Tween * cur_tween;

    // Some default use values
    Color default_color;
    Font default_font;
    Point<int> default_point;

  public:
    Control(int width=1600, int height=900, const std::string & name="container")
      : aspect_ratio(0.0)
      , base_width(width), base_height(height)
      , cur_layer(NULL)
      , default_color("black")
      , default_font("Helvetica", 30, "black")
      , default_point(10, height/2)
    {
      BuildStage(name, width, height); // Build the default stage.
      Stage().ResizeMax();
    }
    ~Control() {
      // Delete all objects created as part of this contoller.
      for (auto it = stage_map.begin(); it != stage_map.end(); it++) delete (*it).second;
      for (auto it = layer_map.begin(); it != layer_map.end(); it++) delete (*it).second;
      for (auto it = color_map.begin(); it != color_map.end(); it++) delete (*it).second;
      for (auto it = font_map.begin(); it != font_map.end(); it++) delete (*it).second;
      for (auto it = point_map.begin(); it != point_map.end(); it++) delete (*it).second;
      for (auto it = image_map.begin(); it != image_map.end(); it++) delete (*it).second;
      for (auto it = rect_map.begin(); it != rect_map.end(); it++) delete (*it).second;
      for (auto it = text_map.begin(); it != text_map.end(); it++) delete (*it).second;
      for (auto it = button_map.begin(); it != button_map.end(); it++) delete (*it).second;
      for (auto it = buttonset_map.begin(); it != buttonset_map.end(); it++) delete (*it).second;
      for (auto it = grid_map.begin(); it != grid_map.end(); it++) delete (*it).second;
      for (auto it = panel_map.begin(); it != panel_map.end(); it++) delete (*it).second;
      for (auto it = animation_map.begin(); it != animation_map.end(); it++) delete (*it).second;
      for (auto it = eventchain_map.begin(); it != eventchain_map.end(); it++) delete (*it).second;
      for (auto it = tween_map.begin(); it != tween_map.end(); it++) delete (*it).second;
      
      for (auto it = temp_objs.begin(); it != temp_objs.end(); it++) delete (*it);
    }

    inline double ScaleX(double x_frac) const {
      assert(cur_stage != NULL);
      assert(x_frac >= 0.0 && x_frac <= 1.0);
      return cur_stage->ScaleX(x_frac);
    }
    inline double ScaleY(double y_frac) const {
      assert(cur_stage != NULL);
      assert(y_frac >= 0.0 && y_frac <= 1.0);
      return cur_stage->ScaleY(y_frac);
    }
    inline emp::Point<int> ScaleXY(double x_frac, double y_frac) { return emp::Point<int>( ScaleX(x_frac), ScaleY(y_frac) ); }
    
    // Setup basic parentheses usage for scaling to new positions...
    inline double operator()(double x_frac) { return ScaleX(x_frac); }
    inline emp::Point<int> operator()(double x_frac, double y_frac) { return emp::Point<int>( ScaleX(x_frac), ScaleY(y_frac) ); }

    Control & SetAspect(double aspect_ratio) { Stage().SetAspect(aspect_ratio); return *this; }

    Stage & BuildStage(const std::string & name, int width, int height) {
      cur_stage = new emp::Stage(width, height, name);
      if (stage_map.find(name) != stage_map.end()) delete stage_map[name]; // If we a replacing a stage, delete the old one.
      stage_map[name] = cur_stage;
      return *cur_stage;
    }

    Layer & BuildLayer(const std::string & name) {
      cur_layer = new emp::Layer();
      if (layer_map.find(name) != layer_map.end()) delete layer_map[name]; // If we a replacing a layer, delete the old one.
      layer_map[name] = cur_layer;
      return *cur_layer;
    }

    Color & BuildColor(const std::string & name, const std::string & color_name) {
      cur_color = new emp::Color(color_name);
      default_color = *cur_color;
      if (color_map.find(name) != color_map.end()) delete color_map[name]; // If we a replacing a color, delete the old one.
      color_map[name] = cur_color;
      return *cur_color;
    }

    Font & BuildFont(const std::string & name, const emp::Font & font) {
      cur_font = new emp::Font(font);
      default_font = *cur_font;
      if (font_map.find(name) != font_map.end()) delete font_map[name]; // If we a replacing a font, delete the old one.
      font_map[name] = cur_font;
      return *cur_font;
    }

    Point<int> & BuildPoint(const std::string & name, int x=0, int y=0) {
      cur_point = new emp::Point<int>(x, y);
      default_point = *cur_point;
      if (point_map.find(name) != point_map.end()) delete point_map[name]; // If we a replacing a point, delete the old one.
      point_map[name] = cur_point;
      return *cur_point;
    }

    Image & BuildImage(const std::string & name, const std::string & filename, const Point<int> & point=emp::Point<int>(0,0), int width=0, int height=0) {
      cur_image = new emp::Image(filename, point, width, height);
      if (image_map.find(name) != image_map.end()) delete image_map[name]; // If we a replacing a image, delete the old one.
      image_map[name] = cur_image;
      return *cur_image;
    }

    Rect & BuildRect(const std::string & name, const Point<int> & point, int w=10, int h=10,
                     std::string fill="white", std::string stroke="black", int stroke_width=1, int draggable=0) {
      cur_rect = new emp::Rect(point, w, h, fill, stroke, stroke_width, draggable);
      if (rect_map.find(name) != rect_map.end()) delete rect_map[name]; // If we a replacing a rect, delete the old one.
      rect_map[name] = cur_rect;
      shape_map[name] = cur_rect;
      return *cur_rect;
    }

    TextBox & BuildTextBox(const std::string & name, const Point<int> & point, std::string text, const emp::Font & font) {
      cur_text = new emp::TextBox(point, text, font);
      if (text_map.find(name) != text_map.end()) delete text_map[name]; // If we a replacing a text, delete the old one.
      text_map[name] = cur_text;
      shape_map[name] = cur_text;
      return *cur_text;
    }

    // If no explicit font is provided for the text use the cur_font (plus other default values).
    TextBox & BuildTextBox(const std::string & name, const Point<int> & point, std::string text="") { return BuildTextBox(name, point, text, default_font); }


    template<class T> Button & BuildButton(const std::string & name, T * target, void (T::*method_ptr)()) {
      cur_button = new emp::Button(target, method_ptr, name);
      if (button_map.find(name) != button_map.end()) delete button_map[name]; // If we a replacing a button, delete the old one.
      button_map[name] = cur_button;
      shape_map[name] = cur_button;
      return *cur_button;
    }

    ButtonGrid & BuildButtonGrid(const std::string & name, int cols, int rows, const Point<int> & point, int width, int height, int spacing=0) {
      cur_buttonset = new emp::ButtonGrid(cols, rows, point, width, height, spacing);
      if (buttonset_map.find(name) != buttonset_map.end()) delete buttonset_map[name]; // If we a replacing a buttonset, delete the old one.
      buttonset_map[name] = cur_buttonset;
      return *cur_buttonset;
    }

    Grid & BuildGrid(const std::string & name, const Point<int> & point, int width, int height, int cols, int rows, int num_colors=12, int border_width=1) {
      cur_grid = new emp::Grid(point, width, height, cols, rows, num_colors, border_width);
      if (grid_map.find(name) != grid_map.end()) delete grid_map[name]; // If we a replacing a grid, delete the old one.
      grid_map[name] = cur_grid;
      shape_map[name] = cur_grid;
      return *cur_grid;
    }

    EventChain & BuildEventChain(const std::string & name) {
      cur_eventchain = new emp::EventChain();
      if (eventchain_map.find(name) != eventchain_map.end()) delete eventchain_map[name]; // If we a replacing a eventchain, delete the old one.
      eventchain_map[name] = cur_eventchain;
      return *cur_eventchain;
    }

    Tween & BuildTween(const std::string & name, Object & target, double seconds) {
      cur_tween = new emp::Tween(target, seconds);
      if (tween_map.find(name) != tween_map.end()) delete tween_map[name]; // If we a replacing a tween, delete the old one.
      tween_map[name] = cur_tween;
      return *cur_tween;
    }



    emp::Stage & Stage(const std::string & name="") {
      if (name != "") cur_stage = stage_map[name];
      assert(cur_stage != NULL);
      return *cur_stage;
    }
    emp::Layer & Layer(const std::string & name="") {
      if (name != "") cur_layer = layer_map[name];
      assert(cur_layer != NULL);
      return *cur_layer;
    }
    emp::Color & Color(const std::string & name="") {
      if (name != "") cur_color = color_map[name];
      assert(cur_color != NULL);
      return *cur_color;
    }
    emp::Font & Font(const std::string & name="") {
      if (name != "") cur_font = font_map[name];
      assert(cur_font != NULL);
      return *cur_font;
    }
    emp::Point<int> & Point(const std::string & name="") {
      if (name != "") cur_point = point_map[name];
      assert(cur_point != NULL);
      return *cur_point;
    }
    emp::Image & Image(const std::string & name="") {
      if (name != "") cur_image = image_map[name];
      assert(cur_image != NULL);
      return *cur_image;
    }
    emp::Rect & Rect(const std::string & name="") {
      if (name != "") cur_rect = rect_map[name];
      assert(cur_rect != NULL);
      return *cur_rect;
    }
    emp::TextBox & TextBox(const std::string & name="") {
      if (name != "") cur_text = text_map[name];
      assert(cur_text != NULL);
      return *cur_text;
    }
    emp::Button & Button(const std::string & name="") {
      if (name != "") cur_button = button_map[name];
      assert(cur_button != NULL);
      return *cur_button;
    }
    emp::ButtonGrid & ButtonGrid(const std::string & name="") {
      if (name != "") cur_buttonset = buttonset_map[name];
      assert(cur_buttonset != NULL);
      return *cur_buttonset;
    }
    emp::Grid & Grid(const std::string & name="") {
      if (name != "") cur_grid = grid_map[name];
      assert(cur_grid != NULL);
      return *cur_grid;
    }
    emp::EventChain & EventChain(const std::string & name="") {
      if (name != "") cur_eventchain = eventchain_map[name];
      assert(cur_eventchain != NULL);
      return *cur_eventchain;
    }
    emp::Tween & Tween(const std::string & name="") {
      if (name != "") cur_tween = tween_map[name];
      assert(cur_tween != NULL);
      return *cur_tween;
    }

  };

};

#endif
