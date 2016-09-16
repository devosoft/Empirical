// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_KINETIC_H
#define EMP_KINETIC_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This file contains information for using KineticJS from C++
//
//
//  Development Notes:
//  * Need to move from callbacks.h to JSWrap.h
//    - Setup JSWrap to be able to convert AnimationFrame object (or original versions?)
//

#include <emscripten.h>

#include <list>
#include <string>
#include <vector>

#include "../tools/assert.h"
#include "callbacks.h"
#include "../tools/const.h"
#include "../geometry/Circle2D.h"
#include "../geometry/Point2D.h"

#include "../emtools/color_map.h"
#include "../emtools/emfunctions.h"
#include "../emtools/Font.h"
#include "../emtools/RawImage.h"

extern "C" {
  extern void EMP_SETUP(); // @CAO -- This function should be more generic!
  extern int EMP_Tween_Build(int target_id, double seconds);
  extern int EMP_Tween_Clone(int orig_settings_id);

  extern int EMP_Rect_Build(int _x, int _y, int _w, int _h, const char * _fill, const char * _stroke, int _stroke_width, int _draggable);
  extern int EMP_RegularPolygon_Build(int _x, int _y, int _sides, int _radius,
                                      const char * _fill, const char * _stroke, int _stroke_width, int _draggable);

  extern int EMP_Animation_Build(int callback_ptr, int layer_id);
  extern int EMP_Animation_Build_NoFrame(int callback_ptr, int layer_id);

  extern int EMP_Custom_Shape_Build(int x, int y, int w, int h, int draw_callback);
}


namespace emp {
namespace Kinetic {

  // Pre-declarations of some classes...
  class Layer;
  class Stage;


  // All emscripten-wrapped Kinetic objects should be derived from this base class.
  class Node {
  protected:
    int obj_id;
    Layer * layer;
    std::string name;

    // Helpers...
    static int GetNumObjs() { return EM_ASM_INT_V({ return emp_kinetic.objs.length; }); }

    Node(const std::string & _name="") : obj_id(-1), layer(NULL), name(_name) {;}  // Protected so that you can't make a direct Node.
  private:
    Node(const Node &); // Never allowed.
    Node & operator=(Node & _in);  // Never allowed.

  public:
    ~Node() {
      EM_ASM_ARGS({
          if ($0 >= 0 && emp_kinetic.objs[$0] != 0) emp_kinetic.objs[$0].destroy();
        }, obj_id);
    }
    int GetID() const { emp_assert(obj_id >= 0 && obj_id < GetNumObjs()); return obj_id; }
    const std::string & GetName() const { return name; }

    // Retrieve info from JS Kinetic::Node objects
    int GetX() const {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      return EM_ASM_INT({return emp_kinetic.objs[$0].x();}, obj_id);
    }
    int GetY() const {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      return EM_ASM_INT({return emp_kinetic.objs[$0].y();}, obj_id);
    }
    Point<int> GetPos() const { return Point<int>(GetX(), GetY()); }
    int GetWidth() const {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      return EM_ASM_INT({return emp_kinetic.objs[$0].width();}, obj_id);
    }
    int GetHeight() const {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      return EM_ASM_INT({return emp_kinetic.objs[$0].height();}, obj_id);
    }
    Point<int> GetUL(int x_offset=0, int y_offset=0) const { return Point<int>(GetX()+x_offset,              GetY()+y_offset); }
    Point<int> GetUM(int x_offset=0, int y_offset=0) const { return Point<int>(GetX()+GetWidth()/2+x_offset, GetY()+y_offset); }
    Point<int> GetUR(int x_offset=0, int y_offset=0) const { return Point<int>(GetX()+GetWidth()+x_offset,   GetY()+y_offset); }
    Point<int> GetML(int x_offset=0, int y_offset=0) const { return Point<int>(GetX()+x_offset,              GetY()+GetHeight()/2+y_offset); }
    Point<int> GetMM(int x_offset=0, int y_offset=0) const { return Point<int>(GetX()+GetWidth()/2+x_offset, GetY()+GetHeight()/2+y_offset); }
    Point<int> GetMR(int x_offset=0, int y_offset=0) const { return Point<int>(GetX()+GetWidth()+x_offset,   GetY()+GetHeight()/2+y_offset); }
    Point<int> GetLL(int x_offset=0, int y_offset=0) const { return Point<int>(GetX()+x_offset,              GetY()+GetHeight()+y_offset); }
    Point<int> GetLM(int x_offset=0, int y_offset=0) const { return Point<int>(GetX()+GetWidth()/2+x_offset, GetY()+GetHeight()+y_offset); }
    Point<int> GetLR(int x_offset=0, int y_offset=0) const { return Point<int>(GetX()+GetWidth()+x_offset,   GetY()+GetHeight()+y_offset); }
    Point<int> GetCenter(int x_offset=0, int y_offset=0) const { return GetMM(x_offset, y_offset); }
    bool GetVisible() const {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      return EM_ASM_INT({return emp_kinetic.objs[$0].visible();}, obj_id);
    }
    double GetOpacity() const {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      return EM_ASM_DOUBLE({return emp_kinetic.objs[$0].opacity();}, obj_id);
    }
    bool GetListening() const {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      return EM_ASM_INT({return emp_kinetic.objs[$0].listening();}, obj_id);
    }
    double GetScaleX() const {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      return EM_ASM_DOUBLE({return emp_kinetic.objs[$0].scaleX();}, obj_id);
    }
    double GetScaleY() const {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      return EM_ASM_DOUBLE({return emp_kinetic.objs[$0].scaleY();}, obj_id);
    }
    int GetOffsetX() const {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      return EM_ASM_INT({return emp_kinetic.objs[$0].offsetX();}, obj_id);
    }
    int GetOffsetY() const {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      return EM_ASM_INT({return emp_kinetic.objs[$0].offsetY();}, obj_id);
    }
    int GetRotation() const {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      return EM_ASM_INT({return emp_kinetic.objs[$0].rotation();}, obj_id);
    }
    int GetDraggable() const {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      return EM_ASM_INT({return emp_kinetic.objs[$0].draggable();}, obj_id);
    }
    int GetZIndex() const {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      return EM_ASM_INT({return emp_kinetic.objs[$0].getZIndex();}, obj_id);
    }

    Node & SetName(const std::string & _name) { name = _name; return *this; }
    Node & SetX(int _in) {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].x($1);}, obj_id, _in); return *this;
    }
    Node & SetY(int _in) {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].y($1);}, obj_id, _in); return *this;
    }
    Node & SetWidth(int _in) {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].width($1);}, obj_id, _in); return *this;
    }
    Node & SetHeight(int _in) {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].height($1);}, obj_id, _in); return *this;
    }
    Node & SetVisible(int _in) {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].visible($1);}, obj_id, _in); return *this;
    }
    Node & SetOpacity(double _in) {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].opacity($1);}, obj_id, _in); return *this;
    }
    Node & SetListening(int _in) {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].listening($1);}, obj_id, _in); return *this;
    }
    Node & SetScaleX(double _in) {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].scaleX($1);}, obj_id, _in); return *this;
    }
    Node & SetScaleY(double _in) {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].scaleY($1);}, obj_id, _in); return *this;
    }
    Node & SetOffsetX(int _in) {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].offsetX($1);}, obj_id, _in); return *this;
    }
    Node & SetOffsetY(int _in) {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].offsetY($1);}, obj_id, _in); return *this;
    }
    Node & SetRotation(int _in) {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].rotation($1);}, obj_id, _in); return *this;
    }
    Node & SetDraggable(int _in) {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].draggable($1);}, obj_id, _in); return *this;
    }
    Node & SetZIndex(int _in) {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].setZIndex($1);}, obj_id, _in); return *this;
    }
    Node & SetZBottom() {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].moveToBottom();}, obj_id); return *this;
    }
    Node & SetZTop() {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].moveToTop();}, obj_id); return *this;
    }

    inline Node & SetXY(int x, int y) { SetX(x); SetY(y); return *this; }
    inline Node & SetXY(const Point<int> & point) { SetX(point.GetX()); SetY(point.GetY()); return *this; }
    inline Node & SetSize(int w, int h) { SetWidth(w); SetHeight(h); return *this; }
    inline Node & SetLayout(int x, int y, int w, int h) { SetX(x); SetY(y); SetWidth(w); SetHeight(h); return *this; }
    inline Node & SetLayout(const Point<int> & point, int w, int h) { SetXY(point); SetWidth(w); SetHeight(h); return *this; }
    inline Node & SetScale(double _x, double _y) { SetScaleX(_x); SetScaleY(_y); return *this; }
    inline Node & SetScale(double _in) { SetScaleX(_in); SetScaleY(_in); return *this; }
    inline Node & SetOffset(int _x, int _y) { SetOffsetX(_x); SetOffsetY(_y); return *this; }
    inline Node & SetOffset(const Point<int> & point) { SetOffsetX(point.GetX()); SetOffsetY(point.GetY()); return *this; }

    inline Node & SetUL(const Point<int> & point) { return SetXY(point); }
    inline Node & SetUM(const Point<int> & point) { return SetXY(point.GetX()-GetWidth()/2, point.GetY()); }
    inline Node & SetUR(const Point<int> & point) { return SetXY(point.GetX()-GetWidth(),   point.GetY()); }
    inline Node & SetML(const Point<int> & point) { return SetXY(point.GetX(),              point.GetY()-GetHeight()/2); }
    inline Node & SetMM(const Point<int> & point) { return SetXY(point.GetX()-GetWidth()/2, point.GetY()-GetHeight()/2); }
    inline Node & SetMR(const Point<int> & point) { return SetXY(point.GetX()-GetWidth(),   point.GetY()-GetHeight()/2); }
    inline Node & SetLL(const Point<int> & point) { return SetXY(point.GetX(),              point.GetY()-GetHeight()); }
    inline Node & SetLM(const Point<int> & point) { return SetXY(point.GetX()-GetWidth()/2, point.GetY()-GetHeight()); }
    inline Node & SetLR(const Point<int> & point) { return SetXY(point.GetX()-GetWidth(),   point.GetY()-GetHeight()); }

    inline Node & SetCenter(const Point<int> & point) { return SetXY(point.GetX()-GetWidth()/2, point.GetY()-GetHeight()/2); }

    Layer * GetLayer() { return layer; }
    void SetLayer(Layer * _layer) { layer = _layer; }

    // Draw either this object or objects in contains.
    void Draw() {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].draw();}, obj_id);
    }

    // Draw all objects in this layer.
    void DrawLayer(); // Defined *after* Layer.

    // Move this object to the top of the current layer.
    void MoveToTop() {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].moveToTop();}, obj_id);
    }

    template<class T> void On(const std::string & _trigger, T * _target, void (T::*_method_ptr)());
    template<class T> void On(const std::string & _trigger, T * _target, void (T::*_method_ptr)(const EventInfo &));
  };


  // This is a base class for other classes that contain a whole set of objects
  // @CAO Should this be an Node itself?
  template <class T> class NodeGrid { // : public Node {
  protected:
    int cols;
    int rows;
    int x;
    int y;
    int spacing;

    int set_size;
    std::vector<T *> object_set;

    // Protected to prevent NodeGrids from being directly created or destroyed.
    NodeGrid(int _cols, int _rows, int _x, int _y, int _spacing) 
      : cols(_cols), rows(_rows), x(_x), y(_y), spacing(_spacing), set_size(cols * rows), object_set(set_size)
    { ; }     
    ~NodeGrid() { ; }
  public:
    int GetCols() const { return cols; }
    int GetRows() const { return rows; }
    int GetX() const { return x; }
    int GetY() const { return y; }
    int GetSpacing() const { return spacing; }
    int GetSetSize() const { return set_size; }  // @CAO Move to NodeSet base class when we build it.

    T & Get(int pos) { return *(object_set[pos]); }
    T & Get(int col, int row) { return *(object_set[col + row*cols]); }
    T & operator[](int pos) { return *(object_set[pos]); }
  };

  class Tween : public Node {
  private:
    Node * target;   // What object is this tween associated with?

    int settings_id;   // JS memory position where the tween settings should go.
    bool needs_config; // Does the Tween need to be reconfigured?

    void ConfigureTween() {
      EM_ASM_ARGS({
          emp_kinetic.objs[$0].node = emp_kinetic.objs[$1];
          emp_kinetic.objs[$2] = new Kinetic.Tween( emp_kinetic.objs[$0] );
      }, settings_id, target->GetID(), obj_id);
      needs_config = false;
    }
  public:
    Tween(Node & _target, double _seconds) : target(&_target), needs_config(true) {
      settings_id = EMP_Tween_Build(target->GetID(), _seconds);
      obj_id = settings_id + 1;
    }
    Tween(const Tween & _in) : target(_in.target), needs_config(true) {
      settings_id = EMP_Tween_Clone(_in.settings_id);
      obj_id = settings_id + 1;
    }
    virtual ~Tween() {
      EM_ASM_ARGS({
          if ($0 >= 0) delete emp_kinetic.objs[$0];  // Remove the tween settings from memory.
      }, settings_id);
    }

    enum easing { 
      Linear,
      EaseIn, EaseOut, EaseInOut,
      BackEaseIn, BackEaseOut, BackEaseInOut,
      ElasticEaseIn, ElasticEaseOut, ElasticEaseInOut,
      BounceEaseIn, BounceEaseOut, BounceEaseInOut,
      StrongEaseIn, StrongEaseOut, StrongEaseInOut
    };

    Tween & SetTarget(Node & _target) { target = &_target; needs_config=true; return *this; }
    Tween & SetTime(double _in) { EM_ASM_ARGS({ emp_kinetic.objs[$0].duration = $1; }, settings_id, _in); needs_config=true; return *this; }
    Tween & SetX(int _in) { EM_ASM_ARGS({ emp_kinetic.objs[$0].x = $1; }, settings_id, _in); needs_config=true; return *this; }
    Tween & SetY(int _in) { EM_ASM_ARGS({ emp_kinetic.objs[$0].y = $1; }, settings_id, _in); needs_config=true; return *this; }
    Tween & SetScaleX(double _in) { EM_ASM_ARGS({ emp_kinetic.objs[$0].scaleX = $1; }, settings_id, _in); needs_config=true; return *this; }
    Tween & SetScaleY(double _in) { EM_ASM_ARGS({ emp_kinetic.objs[$0].scaleY = $1; }, settings_id, _in); needs_config=true; return *this; }
    Tween & SetRotation(double _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].rotation = $1; }, settings_id, _in); needs_config=true; return *this; }
    Tween & SetOpacity(double _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].opacity = $1; }, settings_id, _in); needs_config=true; return *this; }
    Tween & SetStrokeWidth(double _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].strokeWidth = $1; }, settings_id, _in); needs_config=true; return *this; }
    Tween & SetEasing(easing in_ease) {
      switch (in_ease) {
      case Linear: EM_ASM_ARGS({emp_kinetic.objs[$0].easing = Kinetic.Easings.Linear; }, settings_id); break;
      case EaseIn: EM_ASM_ARGS({emp_kinetic.objs[$0].easing = Kinetic.Easings.EaseIn; }, settings_id); break;
      case EaseOut: EM_ASM_ARGS({emp_kinetic.objs[$0].easing = Kinetic.Easings.EaseOut; }, settings_id); break;
      case EaseInOut: EM_ASM_ARGS({emp_kinetic.objs[$0].easing = Kinetic.Easings.EaseInOut; }, settings_id); break;
      case BackEaseIn: EM_ASM_ARGS({emp_kinetic.objs[$0].easing = Kinetic.Easings.BackEaseIn; }, settings_id); break;
      case BackEaseOut: EM_ASM_ARGS({emp_kinetic.objs[$0].easing = Kinetic.Easings.BackEaseOut; }, settings_id); break;
      case BackEaseInOut: EM_ASM_ARGS({emp_kinetic.objs[$0].easing = Kinetic.Easings.BackEaseInOut; }, settings_id); break;
      case ElasticEaseIn: EM_ASM_ARGS({emp_kinetic.objs[$0].easing = Kinetic.Easings.ElasticEaseIn; }, settings_id); break;
      case ElasticEaseOut: EM_ASM_ARGS({emp_kinetic.objs[$0].easing = Kinetic.Easings.ElasticEaseOut; }, settings_id); break;
      case ElasticEaseInOut: EM_ASM_ARGS({emp_kinetic.objs[$0].easing = Kinetic.Easings.ElasticEaseInOut; }, settings_id); break;
      case BounceEaseIn: EM_ASM_ARGS({emp_kinetic.objs[$0].easing = Kinetic.Easings.BounceEaseIn; }, settings_id); break;
      case BounceEaseOut: EM_ASM_ARGS({emp_kinetic.objs[$0].easing = Kinetic.Easings.BounceEaseOut; }, settings_id); break;
      case BounceEaseInOut: EM_ASM_ARGS({emp_kinetic.objs[$0].easing = Kinetic.Easings.BounceEaseInOut; }, settings_id); break;
      case StrongEaseIn: EM_ASM_ARGS({emp_kinetic.objs[$0].easing = Kinetic.Easings.StrongEaseIn; }, settings_id); break;
      case StrongEaseOut: EM_ASM_ARGS({emp_kinetic.objs[$0].easing = Kinetic.Easings.StrongEaseOut; }, settings_id); break;
      case StrongEaseInOut: EM_ASM_ARGS({emp_kinetic.objs[$0].easing = Kinetic.Easings.StrongEaseInOut; }, settings_id); break;
      default: Alert("Unknown Easing..."); break;
      };
      
      needs_config=true;
      return *this;
    }

    Tween & SetXY(int _x, int _y) { SetX(_x); SetY(_y); return *this; }
    Tween & SetXY(const Point<int> & point) { SetX(point.GetX()); SetY(point.GetY()); return *this; }
    Tween & SetScaleXY(double _x, double _y) { SetScaleX(_x); SetScaleY(_y); return *this; }
    Tween & SetScale(double scale) { SetScaleX(scale); SetScaleY(scale); return *this; }

    Tween & SetFinishedCallback(Callback * callback, int * info_ptr) {
      EM_ASM_ARGS({
          emp_kinetic.objs[$0].onFinish = function() { empJSDoCallback($1, $2); };
        }, settings_id, (int) (callback), (int) info_ptr);
      return *this;
    }

    void Play() {
      if (needs_config) ConfigureTween();
      EM_ASM_ARGS({ emp_kinetic.objs[$0].play(); }, obj_id);
    }
    void Reverse() {
      if (needs_config) ConfigureTween();
      EM_ASM_ARGS({ emp_kinetic.objs[$0].reverse(); }, obj_id);
    }
  };


  class Image : public Node {
  private:
    RawImage & raw_image;
    mutable std::list<Layer *> layers_waiting;

    // Kinetic details to store until image is loaded.
    int x;
    int y;
    int width;
    int height;
    double scale_x;
    double scale_y;
  public:
    Image(const std::string & _filename, int _x=0, int _y=0, int _w=-1, int _h=-1)
      : raw_image(LoadRawImage(_filename))
      , x(_x), y(_y), width(_w), height(_h)
      , scale_x(1.0), scale_y(1.0)
    {
      raw_image.AddLoadCallback(std::bind(&Image::ImageLoaded, this));
    }

    Image(const std::string & _filename, const Point<int> & point, int _w=-1, int _h=-1) 
      : Image(_filename, point.GetX(), point.GetY(), _w, _h) { ; }

    Image(const Image & _image)
      : Image(_image.raw_image.GetFilename(), _image.x, _image.y, _image.width, _image.height)
    {
      raw_image.AddLoadCallback(std::bind(&Image::ImageLoaded, this));
    }

    virtual ~Image() { ; }

    int GetX() const { return x; }
    int GetY() const { return y; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    double GetScaleX() const { return scale_x; }
    double GetScaleY() const { return scale_y; }
    Point<int> GetUL(int x_offset=0, int y_offset=0) const { return Point<int>(GetX()+x_offset, GetY()+y_offset); }
    Point<int> GetUR(int x_offset=0, int y_offset=0) const { return Point<int>(GetX()+GetWidth()+x_offset, GetY()+y_offset); }
    Point<int> GetLR(int x_offset=0, int y_offset=0) const { return Point<int>(GetX()+GetWidth()+x_offset, GetY()+GetHeight()+y_offset); }
    Point<int> GetLL(int x_offset=0, int y_offset=0) const { return Point<int>(GetX()+x_offset, GetY()+GetHeight()+y_offset); }
    const RawImage & GetRawImage() const { return raw_image; }

    bool HasLoaded() const { return raw_image.HasLoaded(); }
    bool HasError() const { return raw_image.HasError(); }

    Image & SetWidth(int _in) { width = _in; if (HasLoaded()) Node::SetWidth(_in); return *this; }
    Image & SetHeight(int _in) { height = _in; if (HasLoaded()) Node::SetHeight(_in); return *this; }
    Image & SetLayout(int _w, int _h) { SetWidth(_w); return SetHeight(_h); }
    Image & SetScaleX(double _in) { scale_x = _in; if (HasLoaded()) Node::SetScaleX(_in); return *this; }
    Image & SetScaleY(double _in) { scale_y = _in; if (HasLoaded()) Node::SetScaleY(_in); return *this; }
    Image & SetScale(double _x, double _y) { SetScaleX(_x); return SetScaleY(_y); }
    Image & SetScale(double _in) { return SetScale(_in, _in); }

    void DrawOnLoad(Layer * _layer) const { layers_waiting.push_back(_layer); }

    void ImageLoaded(); // Called back when image is loaded

    // @CAO Move these to RawImage?
    static int NumImages() { return EM_ASM_INT_V({return emp_kinetic.images.length;}); }
    static int NumLoaded() { return EM_ASM_INT_V({return emp_kinetic.image_load_count;}); }
    static bool AllLoaded() { return EM_ASM_INT_V({return (emp_kinetic.images.length == emp_kinetic.image_load_count);}); }
  };


  // Manual control over the canvas...  For the moment, we assume that a context has been placed in emp_kinetic.ctx
  class Canvas {
  public:
    // Setting values
    inline static void SetFill(const std::string & color) { 
      EM_ASM_ARGS({var fs = Pointer_stringify($0); emp_kinetic.ctx.fillStyle = fs;}, color.c_str());
    }

    inline static void SetStroke(const std::string & color) {
      EM_ASM_ARGS({var stroke = Pointer_stringify($0); emp_kinetic.ctx.strokeStyle = stroke;}, color.c_str());
    }

    inline static void SetLineWidth(double width) { EM_ASM_ARGS({emp_kinetic.ctx.lineWidth = $0;}, width); }

    inline static void SetLineJoin(const std::string & lj) {
      EM_ASM_ARGS({var lj = Pointer_stringify($0); emp_kinetic.ctx.lineJoin = lj;}, lj.c_str());
    }

    inline static void SetFont(const std::string & font) {
      EM_ASM_ARGS({var font = Pointer_stringify($0); emp_kinetic.ctx.font = font;}, font.c_str());
    }

    inline static void SetFont(const Font & font) {
      std::string font_str = std::to_string(font.GetSize()) + std::string("pt ") + font.GetFamily();
      // @CAO Also set font color?
      EM_ASM_ARGS({var font = Pointer_stringify($0); emp_kinetic.ctx.font = font;}, font_str.c_str());
    }

    inline static void SetTextAlign(const std::string & align) {
      EM_ASM_ARGS({var align = Pointer_stringify($0); emp_kinetic.ctx.textAlign = align;}, align.c_str());
    }

    inline static void SetShadowColor(const std::string & color) {
      EM_ASM_ARGS({var color = Pointer_stringify($0); emp_kinetic.ctx.shadowColor = color;}, color.c_str());
    }
    inline static void SetShadowBlur(int _in) { EM_ASM_ARGS({emp_kinetic.ctx.shadowBlur = $0;}, _in); }
    inline static void SetShadowOffsetX(int _in) { EM_ASM_ARGS({emp_kinetic.ctx.shadowOffsetX = $0;}, _in); }
    inline static void SetShadowOffsetY(int _in) { EM_ASM_ARGS({emp_kinetic.ctx.shadowOffsetY = $0;}, _in); }

    // Shapes and Text
    inline static void Text(const std::string & msg, int x, int y, bool fill=true) {
      if (fill) EM_ASM_ARGS({var msg = Pointer_stringify($0); emp_kinetic.ctx.fillText(msg, $1, $2);}, msg.c_str(), x, y);
      else EM_ASM_ARGS({var msg = Pointer_stringify($0); emp_kinetic.ctx.strokeText(msg, $1, $2);}, msg.c_str(), x, y);
    }
    inline static void Text(const std::string & msg, const Point<int> & point, bool fill=true) {
      Text(msg, point.GetX(), point.GetY(), fill);
    }

    inline static void Rect(int x, int y, int width, int height, bool fill=false) {
      if (fill) EM_ASM_ARGS({emp_kinetic.ctx.fillRect($0, $1, $2, $3);}, x, y, width, height);
      else EM_ASM_ARGS({emp_kinetic.ctx.strokeRect($0, $1, $2, $3);}, x, y, width, height);
    }
    inline static void Rect(const Point<int> & point, int width, int height, bool fill=false) {
      Rect(point.GetX(), point.GetY(), width, height, fill);
    }

    template <typename BASE_TYPE>
    inline static void Arc(BASE_TYPE x, BASE_TYPE y, BASE_TYPE radius, double start, double end, bool cclockwise=false) {
      EM_ASM_ARGS({emp_kinetic.ctx.arc($0, $1, $2, $3, $4, $5);},
                  x, y, radius, start, end, cclockwise);
    }
    template <typename BASE_TYPE>
    inline static void Arc(const Point<BASE_TYPE> & point, BASE_TYPE radius, double start, double end, bool cclockwise=false) {
      Arc(point.GetX(), point.GetY(), radius, start, end, cclockwise);
    }
    template <typename BASE_TYPE>
    inline static void Circle(const Point<BASE_TYPE> & point, BASE_TYPE radius) {
      Arc(point.GetX(), point.GetY(), radius, 0.0, 2.0 * emp::PI);
    }
    template <typename BASE_TYPE>
    inline static void Circle(const emp::Circle<BASE_TYPE> & circle) {
      Arc(circle.GetCenter(), circle.GetRadius(), 0.0, 2.0 * emp::PI);
    }

    // Functions to draw shapes where the input type determines the specific shape.
    // (These are useful with templates so the right draw shape is picked).
    inline static void DrawShape(const emp::Circle<int> & circle) { Circle(circle); }

    inline static void DrawImage(const Image & image, int x, int y) {
      // @CAO Do something different if the image hasn't loaded yet?  Maybe draw a placeholder rectangle?
      EM_ASM_ARGS({emp_kinetic.ctx.drawImage(emp_kinetic.images[$0], $1, $2);}, image.GetRawImage().GetImgID(), x, y);
    }
    inline static void DrawImage(const Image & image, const Point<int> & point) {
      DrawImage(image, point.GetX(), point.GetY());
    }

    inline static void DrawImage(const Image & image, int x, int y, int w, int h) {
      EM_ASM_ARGS({emp_kinetic.ctx.drawImage(emp_kinetic.images[$0], $1, $2, $3, $4);}, image.GetRawImage().GetImgID(), x, y, w, h);
    }
    inline static void DrawImage(const Image & image, const Point<int> & point, int w, int h) {
      DrawImage(image, point.GetX(), point.GetY(), w, h);
    }

    // Paths
    inline static void BeginPath() { EM_ASM( emp_kinetic.ctx.beginPath() ); }
    inline static void ClosePath() { EM_ASM( emp_kinetic.ctx.closePath() ); }
    inline static void Fill() { EM_ASM( emp_kinetic.ctx.fill() ); }
    inline static void LineTo(int x, int y) { EM_ASM_ARGS({ emp_kinetic.ctx.lineTo($0, $1); }, x, y); }
    template <typename T> static void LineTo(const Point<T> & point) { LineTo(point.GetX(), point.GetY()); }
    inline static void MoveTo(int x, int y) { EM_ASM_ARGS({ emp_kinetic.ctx.moveTo($0, $1); }, x, y); }
    template <typename T> static void MoveTo(const Point<T> & point) { MoveTo(point.GetX(), point.GetY()); }

    // Transformations
    inline static void Restore() { EM_ASM( emp_kinetic.ctx.restore() ); }
    inline static void Save() { EM_ASM( emp_kinetic.ctx.save() ); }
    inline static void Scale(double x, double y) { EM_ASM_ARGS({ emp_kinetic.ctx.scale($0, $1); }, x, y); }
    inline static void Scale(double new_scale) { EM_ASM_ARGS({ emp_kinetic.ctx.scale($0, $0); }, new_scale); }
    inline static void Translate(int x, int y) { EM_ASM_ARGS({ emp_kinetic.ctx.translate($0, $1); }, x, y); }
    inline static void Translate(const Point<int> & point) { Translate(point.GetX(), point.GetY()); }
    inline static void Rotate(double angle) { EM_ASM_ARGS({ emp_kinetic.ctx.rotate($0); }, angle); }
 

    // Finsihing
    inline static void Stroke() { EM_ASM( emp_kinetic.ctx.stroke() ); }
    inline static void SetupTarget(const Node & obj) {
      EM_ASM_ARGS({emp_kinetic.canvas.fillStrokeShape(emp_kinetic.objs[$0])}, obj.GetID());
    }
  };


  template <class T> class Callback_Canvas : public Callback {
  private:
    T * target;
    void (T::*method_ptr)(Canvas &);
  public:
    Callback_Canvas(T * _target, void (T::*_method_ptr)(Canvas &))
      : target(_target)
      , method_ptr(_method_ptr)
    { ; }

    ~Callback_Canvas() { ; }

    void DoCallback(int * arg_ptr) {
      Canvas canvas; // @CAO For now, all canvas objects are alike; we should allow them to coexist.
      (target->*(method_ptr))(canvas); }
  };


  template<class T> void Node::On(const std::string & trigger, T * target,
                                       void (T::*method_ptr)())
  {
    // @CAO Technically we should track these callbacks to make sure we delete them properly.
    MethodCallback<T> * new_callback = new MethodCallback<T>(target, method_ptr);
    EM_ASM_ARGS({
        var trigger = Pointer_stringify($1);
        emp_kinetic.objs[$0].on(trigger, function() {
            empJSDoCallback($2);
        });
    }, obj_id, trigger.c_str(), (int) new_callback);
  }

  template<class T> void Node::On(const std::string & trigger, T * target,
                                       void (T::*method_ptr)(const EventInfo &))
  {
    // @CAO Technically we should track these callbacks to make sure we delete them properly.
    MethodCallback_Event<T> * new_callback = new MethodCallback_Event<T>(target, method_ptr);

    // emp::Alert("Setting up an On for obj ", obj_id, " with target id ", target->GetID());

    EM_ASM_ARGS({
        var trigger = Pointer_stringify($1);
        emp_kinetic.objs[$0].on(trigger, function(event) {
            var evt = event.evt;
            var ptr = Module._malloc(32); // 8 ints @ 4 bytes each...
            setValue(ptr,    evt.layerX,   'i32');
            setValue(ptr+4,  evt.layerY,   'i32');
            setValue(ptr+8,  evt.button,   'i32');
            setValue(ptr+12, evt.keyCode,  'i32');
            setValue(ptr+16, evt.altKey,   'i32');
            setValue(ptr+20, evt.ctrlKey,  'i32');
            setValue(ptr+24, evt.metaKey,  'i32');
            setValue(ptr+28, evt.shiftKey, 'i32');

            empJSDoCallback($2, ptr);
            Module._free(ptr);
        });
    },  obj_id, trigger.c_str(), (int) new_callback);
  }


  // The subclass of object that may be placed in a layer.
  class Shape : public Node {
  protected:
    const Image * image;  // If we are drawing an image, keep track of it to make sure it loads.
    Callback * draw_callback; // If we override the draw callback, keep track of it here.

    Shape() : image(NULL), draw_callback(NULL) { obj_id = -3; } // The default Shape constructor should only be called from derived classes.
  public:
    virtual ~Shape() { ; }

    virtual Shape & SetFillPatternImage(const Image & _image) {
      image = &_image;
      EM_ASM_ARGS({
        emp_kinetic.objs[$0].setFillPriority('pattern');
        emp_kinetic.objs[$0].setFillPatternImage(emp_kinetic.images[$1]);
      }, obj_id, image->GetRawImage().GetImgID());
      return *this;
    }


    Shape & SetFill(const std::string & color) {
      EM_ASM_ARGS({var fill = Pointer_stringify($1); emp_kinetic.objs[$0].fill(fill);}, obj_id, color.c_str());
      return *this;
    }

    int GetFillPatternX() const { return EM_ASM_INT({return emp_kinetic.objs[$0].fillPatternX();}, obj_id); }
    Shape & SetFillPatternX(int _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].fillPatternX($1);}, obj_id, _in); return *this; }

    int GetFillPatternY() const { return EM_ASM_INT({return emp_kinetic.objs[$0].fillPatternY();}, obj_id); }
    Shape & SetFillPatternY(int _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].fillPatternY($1);}, obj_id, _in); return *this; }

    Shape & SetFillPatternXY(int _x, int _y) { SetFillPatternX(_x); SetFillPatternY(_y); return *this; }


    int GetFillPatternOffsetX() const { return EM_ASM_INT({return emp_kinetic.objs[$0].fillPatternOffsetX();}, obj_id); }
    Shape & SetFillPatternOffsetX(int _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].fillPatternOffsetX($1);}, obj_id, _in); return *this; }

    int GetFillPatternOffsetY() const { return EM_ASM_INT({return emp_kinetic.objs[$0].fillPatternOffsetY();}, obj_id); }
    Shape & SetFillPatternOffsetY(int _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].fillPatternOffsetY($1);}, obj_id, _in); return *this; }

    Shape & SetFillPatternOffsetXY(int _x, int _y) { SetFillPatternOffsetX(_x); SetFillPatternOffsetY(_y); return *this; }


    double GetFillPatternScaleX() const { return EM_ASM_DOUBLE({return emp_kinetic.objs[$0].fillPatternScaleX();}, obj_id); }
    Shape & SetFillPatternScaleX(double _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].fillPatternScaleX($1);}, obj_id, _in); return *this; }

    double GetFillPatternScaleY() const { return EM_ASM_DOUBLE({return emp_kinetic.objs[$0].fillPatternScaleY();}, obj_id); }
    Shape & SetFillPatternScaleY(double _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].fillPatternScaleY($1);}, obj_id, _in); return *this; }

    Shape & SetFillPatternScale(double _x, double _y) { SetFillPatternScaleX(_x); SetFillPatternScaleY(_y); return *this; }
    Shape & SetFillPatternScale(double scale) { SetFillPatternScaleX(scale); SetFillPatternScaleY(scale); return *this; }


    double GetFillPatternRotation() const { return EM_ASM_DOUBLE({return emp_kinetic.objs[$0].fillPatternRotation();}, obj_id); }
    Shape & SetFillPatternRotation(double _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].fillPatternRotation($1);}, obj_id, _in); return *this; }


    Shape & SetFillPatternRepeat(int _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].fillPatternRepeat($1);}, obj_id, _in); return *this; }


    bool GetFillEnabled() const { return EM_ASM_INT({return emp_kinetic.objs[$0].fillEnabled();}, obj_id); }
    Shape & SetFillEnabled(bool _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].fillEnabled($1);}, obj_id, (int) _in); return *this; }


    Shape & SetFillPriority(const std::string & _in) {
      EM_ASM_ARGS({var priority = Pointer_stringify($1); emp_kinetic.objs[$0].fillPriority(priority);}, obj_id, (int) _in.c_str());
      return *this;
    }


    Shape & SetStroke(const std::string & _in) {
      EM_ASM_ARGS({var stroke = Pointer_stringify($1); emp_kinetic.objs[$0].stroke(stroke);}, obj_id, _in.c_str());
      return *this;
    }

    int GetStrokeWidth() const { return EM_ASM_INT({return emp_kinetic.objs[$0].strokeWidth();}, obj_id); }
    Shape & SetStrokeWidth(int _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].strokeWidth($1);}, obj_id, _in); return *this; }

    bool GetStrokeScaleEnabled() const { return EM_ASM_INT({return emp_kinetic.objs[$0].strokeScaleEnabled();}, obj_id); }
    Shape & SetStrokeScaleEnabled(bool _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].strokeScaleEnabled($1);}, obj_id, (int) _in); return *this; }

    bool GetStrokeEnabled() const { return EM_ASM_INT({return emp_kinetic.objs[$0].strokeEnabled();}, obj_id); }
    Shape & SetStrokeEnabled(bool _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].strokeEnabled($1);}, obj_id, (int) _in); return *this; }


    Shape & SetLineJoin(const std::string & _in) {
      EM_ASM_ARGS({var lj = Pointer_stringify($1); emp_kinetic.objs[$0].lineJoin(lj);}, obj_id, _in.c_str());
      return *this;
    }
    Shape & SetLineCap(const std::string & _in) {
      EM_ASM_ARGS({var lc = Pointer_stringify($1); emp_kinetic.objs[$0].lineCap(lc);}, obj_id, _in.c_str());
      return *this;
    }

    Shape & SetShadowColor(const std::string & _in) {
      EM_ASM_ARGS({var sc = Pointer_stringify($1); emp_kinetic.objs[$0].shadowColor($1);}, obj_id, _in.c_str());
      return *this;
    }

    double GetShadowBlur() const { return EM_ASM_DOUBLE({return emp_kinetic.objs[$0].shadowBlur();}, obj_id); }
    Shape & SetShadowBlur(double _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].shadowBlur($1);}, obj_id, _in); return *this; }


    int GetShadowOffsetX() const { return EM_ASM_INT({return emp_kinetic.objs[$0].shadowOffsetX();}, obj_id); }
    Shape & SetShadowOffsetX(int _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].shadowOffsetX($1);}, obj_id, _in); return *this; }

    int GetShadowOffsetY() const { return EM_ASM_INT({return emp_kinetic.objs[$0].shadowOffsetY();}, obj_id); }
    Shape & SetShadowOffsetY(int _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].shadowOffsetY($1);}, obj_id, _in); return *this; }

    Shape & SetShadowOffset(int _x, int _y) { SetShadowOffsetX(_x); SetShadowOffsetY(_y); return *this; }


    double GetShadowOpacity() const { return EM_ASM_DOUBLE({return emp_kinetic.objs[$0].shadowOpacity();}, obj_id); }
    Shape & SetShadowOpacity(double _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].shadowOpacity($1);}, obj_id, _in); return *this; }

    bool GetShadowEnabled() const { return EM_ASM_INT({return emp_kinetic.objs[$0].shadowEnabled();}, obj_id); }
    Shape & SetShadowEnabled(bool _in) { EM_ASM_ARGS({emp_kinetic.objs[$0].shadowEnabled($1);}, obj_id, (int) _in); return *this; }


    // @CAO -- not the proper place for this??
    inline Shape & SetCornerRadius(int radius) { EM_ASM_ARGS({emp_kinetic.objs[$0].cornerRadius($1);}, obj_id, radius); return *this; }


    // Override the drawing of this shape.
    template<class T> Shape & SetDrawFunction(T * target, void (T::*draw_ptr)(Canvas &) ) {
      if (draw_callback != NULL) delete draw_callback;
      draw_callback = new Callback_Canvas<T>(target, draw_ptr);

      EM_ASM_ARGS({
        emp_kinetic.objs[$0].setDrawFunc( function(_ctx) {
            emp_kinetic.ctx = _ctx._context;
            empJSDoCallback($1, 0);
            emp_kinetic.ctx = null;
        } );
      }, obj_id, (int) draw_callback);

      return *this;
    }

    Shape & DoRotate(double rot) { EM_ASM_ARGS({emp_kinetic.objs[$0].rotate($1);}, obj_id, rot); return *this; }

    const Image * GetImage() { return image; }
  };


  // Build your own shape!
  class CustomShape : public Shape {
  public:
    template <class T> CustomShape(T * target, void (T::*draw_ptr)(Canvas &))
    {
      draw_callback = new Callback_Canvas<T>(target, draw_ptr);
      obj_id = EMP_Custom_Shape_Build(0, 0, 0, 0, (int) draw_callback);
    }
    template <class T> CustomShape(int _x, int _y, int _w, int _h, T * target, void (T::*draw_ptr)(Canvas &))
    {
      draw_callback = new Callback_Canvas<T>(target, draw_ptr);
      obj_id = EMP_Custom_Shape_Build(_x, _y, _w, _h, (int) draw_callback);
    }
    virtual ~CustomShape() { ; }
  };



  // Manager for stage layers
  class Layer : public Node {
  public:
    Layer() {
      obj_id = EM_ASM_INT_V({
          var obj_id = emp_kinetic.objs.length;            // Determine next free id for a Kinetic object.
          emp_kinetic.objs[obj_id] = new Kinetic.Layer();  // Build new layer and save as a Kinetic object.
          return obj_id;                                // Return the Kinetic object id.
      });
    }
    virtual ~Layer() { ; }
    
    Layer & Add(Shape & _obj) {
      _obj.SetLayer(this);    // Track what layer this shape is in.

      // If the object we are adding has an image that hasn't been loaded, setup a callback.
      const Image * image = _obj.GetImage();
      if (image && image->HasLoaded() == false) {
        image->DrawOnLoad(this);
      }
      EM_ASM_ARGS({emp_kinetic.objs[$0].add(emp_kinetic.objs[$1]);}, obj_id, _obj.GetID());
      return *this;
    }

    Layer & Add(Image & _obj) {
      _obj.SetLayer(this);   // Track what layer this image is in.
      EM_ASM_ARGS({emp_kinetic.objs[$0].add(emp_kinetic.objs[$1]);}, obj_id, _obj.GetID());
      return *this;
    }

    template <class T> Layer & Add(NodeGrid<T> & obj_set) {
      for (int i = 0; i < obj_set.GetSetSize(); i++) {  Add(obj_set[i]);  }
      return *this;
    }

    Layer & Remove(Node & _obj) {
      EM_ASM_ARGS({emp_kinetic.objs[$0].remove();}, _obj.GetID());
      _obj.SetLayer(NULL);
      return *this;
    }

    void Draw() {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].draw();}, obj_id);
    }
    void BatchDraw() { EM_ASM_ARGS({emp_kinetic.objs[$0].batchDraw();}, obj_id); }
  };

  // Functions that rely on Layers, declared above.
  void Node::DrawLayer() { emp_assert(layer != NULL); layer->Draw(); }


  // The main Stage object from Kinetic
  class Stage : public Node {
  private:
    std::string m_container;      // Name of this stage.

    int scaled_width;             // Width stage should appear internally (-1 means use real width)
    int scaled_height;            // Height stage should appear internally (-1 means use real height)
    double aspect_ratio;          // Aspect ratio to keep stage at (0 means do not lock aspect)
  public:
    Stage(int _w, int _h, std::string name="container", bool lock_aspect=true) 
      : m_container(name), scaled_width(_w), scaled_height(_h), aspect_ratio(0.0)
    {
      obj_id = EM_ASM_INT({
        name = Pointer_stringify($2);
        var obj_id = emp_kinetic.objs.length;
        emp_kinetic.objs[obj_id] = new Kinetic.Stage({
                container: name,
                width: $0,
                height: $1
            });

        return obj_id;
      }, _w, _h, m_container.c_str());

      // Determine if we are lockig the aspect ratio (to keep it constant is size changes).
      if (lock_aspect) aspect_ratio = ((double) scaled_width) / (double) scaled_height;
    }
    virtual ~Stage() { ; }

    // Sizing
    int GetScaledWidth() const { return scaled_width; }
    int GetScaledHeight() const { return scaled_height; }
    double GetAspectRatio() const { return aspect_ratio; }

    Stage & SetScaledSize(int _w, int _h, bool lock_aspect=true) {
      scaled_width = _w;
      scaled_height = _h;
      if (lock_aspect) aspect_ratio = ((double) scaled_width) / (double) scaled_height;
      Rescale();
      return *this;
    }
    void Rescale() {
      double x_scale = 1.0, y_scale = 1.0;
      if (scaled_width > 0 && GetWidth() != scaled_width) x_scale = ((double) scaled_width) / (double) GetWidth();
      if (scaled_height > 0 && GetHeight() != scaled_height) y_scale = ((double) scaled_height) / (double) GetHeight();
      // AlertVar(GetWidth());
      SetScale(1.0/x_scale, 1.0/y_scale);
    }

    inline Node & SetSize(int w, int h) {
      // If the aspect ratio is locked, adjust the new size to fit inside the dimensions provided.
      if (aspect_ratio > 0) {
        if (h * aspect_ratio < w) w = h * aspect_ratio;
        else h = w / aspect_ratio;
      }      

      SetWidth(w);    // Set the new (absolute) size of the stage
      SetHeight(h);
      Rescale();      // Adjust anything already on the stage
      return *this;
    }
    void ResizeMax(int min_width, int min_height) {
      int new_width = std::max( emp::GetWindowInnerWidth() - 10, min_width );
      int new_height = std::max( emp::GetWindowInnerHeight() - 10, min_height );
      SetSize(new_width, new_height);
    }
    void ResizeMax() { ResizeMax(0,0); }
    int ScaleX(double x_frac) const { return x_frac * scaled_width; }
    int ScaleY(double y_frac) const { return y_frac * scaled_height; }

    // @CAO Need to adjust into new aspect ratio system.
    Stage & SetAspect(double aspect_ratio) {
      const double width = GetWidth();
      const double height = GetHeight(); 
      if (height * aspect_ratio < width) SetSize(height * aspect_ratio, height);
      else SetSize(width, width / aspect_ratio);
     return *this;
    }

    // Add a layer and return this stage itself (so adding can be chained...)
    Stage & Add(Layer & _layer) {
      EM_ASM_ARGS({emp_kinetic.objs[$0].add(emp_kinetic.objs[$1]);}, obj_id, _layer.GetID());
      return *this;
    }
  };


  // The text object from Kinetic...
  class TextBox : public Shape {
  public:
    TextBox(int x=0, int y=0, const std::string & text="", int font_size=30,
            const std::string & font_family="Helvetica", const std::string & fill="black")
    {
      obj_id = EM_ASM_INT( {
          var obj_id = emp_kinetic.objs.length;         // Determine the next free id for a Kinetic object.
          _text = Pointer_stringify($2);             // Make sure string values are properly converted (text)
          _font_size = Pointer_stringify($3);        // Make sure string values are properly converted (size)
          _font_family = Pointer_stringify($4);      // Make sure string values are properly converted (font)
          _fill = Pointer_stringify($5);             // Make sure string values are properly converted (color)
        
          emp_kinetic.objs[obj_id] = new Kinetic.Text({           // Build the new text object!
              x: $0,
              y: $1,
              text: _text,
              fontSize: _font_size,
              fontFamily: _font_family,
              fill: _fill
          });
          return obj_id;                                       // Return the Kinetic object id.
      }, x, y, text.c_str(), std::to_string(font_size).c_str(), font_family.c_str(), fill.c_str());
    }
    TextBox(int x, int y, std::string text, const Font & font) : TextBox(x, y, text, font.GetSize(), font.GetFamily(), font.GetColor()) { ; }
    TextBox(const Point<int> & point, std::string text, const Font & font)
      : TextBox(point.GetX(), point.GetY(), text, font.GetSize(), font.GetFamily(), font.GetColor()) { ; }
    ~TextBox() { ; }

    TextBox & SetText(const std::string & _text) {
      EM_ASM_ARGS({var _text = Pointer_stringify($1); emp_kinetic.objs[$0].text(_text);}, obj_id, _text.c_str());
      return *this;
    }
  };

  // The rectangle object from Kinetic...
  class Rect : public Shape {
  public:
    Rect(int x=0, int y=0, int w=10, int h=10, std::string fill="white", std::string stroke="black", int stroke_width=1, int draggable=0)
    {
      obj_id = EMP_Rect_Build(x, y, w, h, fill.c_str(), stroke.c_str(), stroke_width, draggable);
    }
    Rect(const Point<int> & point, int w=10, int h=10, std::string fill="white", std::string stroke="black", int stroke_width=1, int draggable=0)
    {
      obj_id = EMP_Rect_Build(point.GetX(), point.GetY(), w, h, fill.c_str(), stroke.c_str(), stroke_width, draggable);
    }

  };

  // The regular polygon object from Kinetic...
  class RegularPolygon : public Shape {
  public:
    RegularPolygon(int x=0, int y=0, int sides=4, int radius=10, std::string fill="white", std::string stroke="black", int stroke_width=1, int draggable=0)
    {
      obj_id = EMP_RegularPolygon_Build(x, y, sides, radius, fill.c_str(), stroke.c_str(), stroke_width, draggable);
    }

    RegularPolygon(const Point<int> & point, int sides=4, int radius=10, std::string fill="white", std::string stroke="black", int stroke_width=1, int draggable=0)
    {
      obj_id = EMP_RegularPolygon_Build(point.GetX(), point.GetY(), sides, radius, fill.c_str(), stroke.c_str(), stroke_width, draggable);
    }

  };


  // Frame information for animations...
  class AnimationFrame {
  public:
    int time_diff;  // Miliseconds since last frame. 
    int last_time;  // Miliseconds from start to last frame.
    int time;       // Miliseconds from start to current frame.
    int frame_rate; // Current frames per second.
  public:
    AnimationFrame(int _td, int _lt, int _t, int _fr)
      : time_diff(_td), last_time(_lt), time(_t), frame_rate(_fr) { ; }
    ~AnimationFrame() { ; }
  };


  template <class T> class Animation : public Callback, public Node {
  private:
    T * target;
    void (T::*method_ptr)(const AnimationFrame &);
    void (T::*method_ptr_nf)();
    bool is_running;
  public:
    Animation(T * _target, void (T::*_method_ptr)(), Layer & layer) : target(_target), method_ptr(NULL), method_ptr_nf(_method_ptr) {
      obj_id = EMP_Animation_Build_NoFrame((int) this, layer.GetID());
    }
    Animation() : target(NULL), method_ptr(NULL), method_ptr_nf(NULL), is_running(false) { ; }
    ~Animation() { ; }

    bool IsRunning() const { return is_running; }

    // Setup this animation object to know what class it will be working with, which update method it should use,
    // and what Stage layer it is in.  The method pointed to may, optionally, take a frame object.
    void Setup(T * _target, void (T::*_method_ptr)(const AnimationFrame &), Layer & layer) {
      target = _target;
      method_ptr = _method_ptr;
      obj_id = EMP_Animation_Build((int) this, layer.GetID());
    }

    void Setup(T * _target, void (T::*_method_ptr)(), Layer & layer) {
      target = _target;
      method_ptr_nf = _method_ptr;
      obj_id = EMP_Animation_Build_NoFrame((int) this, layer.GetID());
    }

    void DoCallback(int * arg_ptr) {
      if (arg_ptr > 0) {
        AnimationFrame frame(arg_ptr[0], arg_ptr[1], arg_ptr[2], arg_ptr[3]);
        (target->*(method_ptr))(frame);
      } else {
        AnimationFrame frame(arg_ptr[0], arg_ptr[1], arg_ptr[2], arg_ptr[3]);
        (target->*(method_ptr_nf))();
      }
    }
    
    void Start() {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].start();}, obj_id);
      is_running = true;
    }
    void Stop() {
      emp_assert(obj_id >= 0 && obj_id < GetNumObjs());
      EM_ASM_ARGS({emp_kinetic.objs[$0].stop();}, obj_id);
      is_running = false;
    }
  };


  //////////////////////////////////////////////////////////
  // Methods previously declared

  void Image::ImageLoaded() // Called back when image is loaded
  {
    if (width == -1) width = EM_ASM_INT({return emp_kinetic.images[$0].width;}, raw_image.GetImgID());
    if (height == -1) height = EM_ASM_INT({return emp_kinetic.images[$0].height;}, raw_image.GetImgID());

    // Build the kinetic image object now that the raw image has loaded.
    obj_id = EM_ASM_INT({
        var obj_id = emp_kinetic.objs.length;       // Determine the next free id for a Kinetic object.
        emp_kinetic.objs[obj_id] = new Kinetic.Image({
          x: $1,
          y: $2,
          image: emp_kinetic.images[$0],
          width: $3,
          height: $4,
          scaleX: $5,
          scaleY: $6
        });
        return obj_id;                           // Pass back the object ID for long-term storage.
      }, raw_image.GetImgID(), x, y, width, height, scale_x, scale_y);
    
    // Loop through any layers this image is in to make sure to redraw them.
    while (layers_waiting.size()) {
      Layer * cur_layer = layers_waiting.front();
      layers_waiting.pop_front();
      cur_layer->Draw();
    }
  }

}; // namespace Kinetic
}; // namespace emp

#endif
