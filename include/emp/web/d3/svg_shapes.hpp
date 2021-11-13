/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file svg_shapes.hpp
 *  @brief Tools to build common SVG shapes.
 */

#ifndef EMP_WEB_D3_SVG_SHAPES_HPP_INCLUDE
#define EMP_WEB_D3_SVG_SHAPES_HPP_INCLUDE

#include "../../base/assert.hpp"
#include "../js_utils.hpp"

#include "d3_init.hpp"
// #include "dataset.hpp"
#include "scales.hpp"
#include "selection.hpp"

namespace D3 {

  /// A few particularly common shapes (circles, rectangles, and ellipses) have corresponding SVG
  /// elements that you can create directly. All other shapes (including lines) must be created by
  /// specifying a "path" describing their outline. Paths are defined with
  /// [a mini-language] (https://www.w3.org/TR/SVG/paths.html#PathData) that
  /// describes how you would draw the shape with a pen. You could write them by hand, but that's
  /// rarely desirable (especially when you're trying to systematically represent data). So d3
  /// provides functions for generating functions that will convert data to paths.
  /// This is a base clase for all objects that manage such functions to inherit from.
  /// You probably want to instantiate derived versions, rather than this class directly.
  class SvgShapeGenerator : public D3_Base {
  protected:
    SvgShapeGenerator(){;};
  public:

    /// Generate the string describing the path associated with [data]
    /// Assumes [data] is an array of 2-element arrays describing (x,y) coordinates and makes
    /// the line that connects them
    template <typename T, size_t SIZE>
    std::string Generate(emp::array<emp::array<T, 2>, SIZE> & data){
      emp::pass_array_to_javascript(data);

      char * buffer = (char *)EM_ASM_INT({
        var result = emp_d3.objects[$0](emp_i.__incoming_array);
        var buffer = Module._malloc(result.length+1);
        Module.stringToUTF8(result, buffer, lengthBytesUTF8(result)+1);
        return buffer;
      }, this->id);

      std::string result = std::string(buffer);
      free(buffer);
      return result;
    }

    /// Draws the path associated with [data] onto the [s] selection (must contain a single SVG)
    /// element).
    template <typename T, std::size_t SIZE>
    Selection DrawShape(emp::array<emp::array<T, 2>, SIZE> & data, Selection & s) {
      Selection path = s.Append("path");
      path.SetAttr("d", Generate(data));
      return path;
    }

    /// DrawShape will also accept a D3::Dataset
    Selection DrawShape(Dataset data, Selection s){

      s = s.Append("path");

      MAIN_THREAD_EM_ASM({
        var sel = emp_d3.objects[$0].attr("d", emp_d3.objects[$1](emp_d3.objects[$2]));
      }, s.GetID(), this->id, data.GetID());
      return s;
    }

   /// If you pass a triple-nested array, it will be treated as an array of paths
    template <typename T, std::size_t SIZE, std::size_t SIZE2>
    Selection DrawShape(emp::array<emp::array<emp::array<T, 2>, SIZE>,\
                        SIZE2> & data) {
      Selection group = Select("svg").Append("g");
      for (auto arr: data) {
        Selection path = group.Append("path");
        path.SetAttr("d", Generate(arr).c_str());
      }
      return group;
    }
  };

  /// Generate symbols ("circle", "cross" "diamond", "square", "triangle-down", "triangle-up").
  /// Often useful for making scatter plots.
  class SymbolGenerator : public SvgShapeGenerator {
  public:
    SymbolGenerator() {
      MAIN_THREAD_EM_ASM({
        var new_line = d3.symbol();
        emp_d3.objects[$0] = new_line;
      }, this->id);
    }

    /// Set the type of symbol generated. Must be a C++ function, a string containing the name of
    /// a Javascript function (in the current window, d3, or emp namespaces), or a string
    /// specifying a type ("circle", "cross" "diamond", "square", "triangle-down", "triangle-up").
    void SetType(std::string type){
      emp_assert(MAIN_THREAD_EM_ASM_INT({
        var func_string = UTF8ToString($0);
        return emp_d3.is_function(func_string);
      }, type.c_str()) && "String passed to SetType is not a Javascript function", type);

      MAIN_THREAD_EM_ASM({
        const id = $0;
        const type_str = UTF8ToString($1);
        var sel = emp_d3.find_function(type_str);
        emp_d3.objects[id].type(sel);
      }, this->id,  type.c_str());
    }

    /// @cond TEMPLATES

    template <typename T>
    emp::sfinae_decoy<void, decltype(&T::operator())>
    SetType(T type){
      const uint32_t func_id = emp::JSWrap(type);

      MAIN_THREAD_EM_ASM({
        const id = $0;
        const func_id = $1;
        emp_d3.objects[id].type(function(d, i, j) {
                                  return emp.Callback(func_id, d, i, j);
                                });
      }, this->id, func_id);

      emp::JSDelete(func_id);
    }

    /// @endcond

    /// Set size in pixels to [size] - can be an int, a C++ function, or string naming a Javascript
    /// function in the current window, the emp namespace, or the d3 namespace.
    //If size is a constant, it's in pixels, so an int is reasonable
    void SetSize(int size) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].size($1);
      }, this->id, size);
    }

    /// @cond TEMPLATES
    //Otherwise it's a function
    void SetSize(std::string size){
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const size_str = UTF8ToString($1);
        var sel = emp_d3.find_function(size_str);
        emp_d3.objects[id].size(sel);
      }, this->id,  size.c_str());
    }

    template <typename T>
    emp::sfinae_decoy<void, decltype(&T::operator())>
    SetSize(T type){
      const uint32_t func_id = emp::JSWrap(type);

      MAIN_THREAD_EM_ASM({
        const id = $0;
        const func_id = $1;
        emp_d3.objects[id].size(function(d, i, j) {
                                  return emp.Callback(func_id, d, i, j);
                                });
      }, this->id, func_id);

      emp::JSDelete(func_id);
    }
    /// @endcond


  };

  /// Base class for generating both cartesian and radial lines
  /// You don't normally want to instantiate this - use LineGenerator or RadialLineGenerator
  /// instead.
  class BaseLineGenerator : public SvgShapeGenerator {
  public:
    BaseLineGenerator() {;}

    /// Set the method used to interpolate a curve between points in the line.
    /// For allowed options, see the
    /// [d3 documntation](https://github.com/d3/d3-3.x-api-reference/blob/master/SVG-Shapes.md#line_interpolate)
    void SetCurve(std::string curve){
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const curve_str = UTF8ToString($1);
        var sel = emp_d3.find_function(curve_str);
        emp_d3.objects[id].curve(sel);
      }, this->id,  curve.c_str());
    }

    /// If interpolation is "bundle", "cardinal", "cardinal-open", or "cardinal-closed", a tension
    /// parameter is used.
    void SetTension(float tension){
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].tension($1);
      }, this->id, tension);
    }

    /// Set a function indicating where the line is defined (i.e. valid)
    /// Can be a C++ function or a string indicating a Javascript function
    void SetDefined(std::string defined){
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const defined_str = UTF8ToString($1);
        var sel = emp_d3.find_function(defined_str);
        emp_d3.objects[id].defined(sel);
      }, this->id,  defined.c_str());
    }

    /// @cond TEMPLATES
    template <typename T>
    emp::sfinae_decoy<void, decltype(&T::operator())>
    SetDefined(T defined){
      const uint32_t func_id = emp::JSWrap(defined);

      MAIN_THREAD_EM_ASM({
        const id = $0;
        const func_id = $1;
        emp_d3.objects[id].defined(function(d, i, j) {
                                  return emp.Callback(func_id, d, i, j);
                                });
      }, this->id, func_id);

      emp::JSDelete(func_id);
    }
    /// @endcond
  };

  /// Generator for regular old (cartesian) lines.
  class LineGenerator : public BaseLineGenerator {
  public:
    LineGenerator() {
      MAIN_THREAD_EM_ASM({
        var new_line = d3.line();
        emp_d3.objects[$0] = new_line;
      }, this->id);
    }

    /// Often, when you're drawing cartesion lines, you want to use a scale to transform numbers
    /// from range of your data to the range of pixels on your screen. Adding an X scale will
    /// cause the x-coordinates of all points on the line to be passed through that scale function.
    /// This stacks on top of whatever the current function for accessing x is (which means scales
    /// will also stack).
    template <typename X_SCALE_TYPE>
    void AddXScale(X_SCALE_TYPE & scale){
      MAIN_THREAD_EM_ASM({
        var scale = emp_d3.objects[$1];
        var curr_x = emp_d3.objects[$0].x();

        //Apply scale to whatever the current x axis function is
        emp_d3.objects[$0].x(function(d, i){return scale(curr_x(d, i));});
      }, this->id, scale.GetID());
    }

    /// Often, when you're drawing cartesion lines, you want to use a scale to transform numbers
    /// from range of your data to the range of pixels on your screen. Adding a Y scale will
    /// cause the y-coordinates of all points on the line to be passed through that scale function.
    /// This stacks on top of whatever the current function for accessing y is (which means scales
    /// will also stack).
    template <typename Y_SCALE_TYPE>
    void AddYScale(Y_SCALE_TYPE & scale){
      MAIN_THREAD_EM_ASM({
        var scale = emp_d3.objects[$1];
        var curr_y = emp_d3.objects[$0].y();

        //Apply scale on top of whatever the current y axis function is
        emp_d3.objects[$0].y(function(d, i){return scale(curr_y(d, i));});
      }, this->id, scale.GetID());
    }

    /// If the data that you are generating lines from is anything more complicated than a sequence
    /// of pairs of numbers, representing x and y (in that order), you need to tell the line
    /// generator how it should figure out what the x coordinate of a point in the line is. The
    /// parameter you pass to SetX should be instructions for doing so. It can either be a function
    /// (as a string indicating a Javascript function or as a literal C++ function) that accepts
    /// an element of the data sequence you are generating the line from, or it can be a constant,
    /// in which case the x coordinate of every point will be that constant.
    ///
    /// Note: This function will re-set any scales that you've added to the X coordinate
    ///
    /// As an example, the default function expects data like this (array of arrays):
    /// [[0,0], [1,1], [2,2]]
    /// And has (a Javascript equivalent of) this accessor:
    /// int x(emp::array<int, 2> d) {return d[0];}
    ///
    /// If your data instead looked like this (array of Javascript objects with x and y values):
    /// [{x:0, y:0}, {x:1, y:1}, {x:2, y:2}]
    /// You might want to use an accessor like this:
    /// int x(JSONObject d) {return d.x();}
    /// Where JSONObject is a struct designed to hold necessary data from the Javascript object:
    ///  struct JSONObject {
    ///    EMP_BUILD_INTROSPECTIVE_TUPLE( int, x,
    ///                                   int, y
    ///                                  )
    ///  };
    // Handles setting x accessor to a function or string
    void SetX(std::string x) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const x_str = UTF8ToString($1);
        var sel = emp_d3.find_function(x_str);
        emp_d3.objects[id].x(sel);
      }, this->id,  x.c_str());
    }

    /// @cond TEMPLATES
    // Handles setting x to a constant
    template <typename T>
    typename std::enable_if<std::is_fundamental<T>::value, void>::type
    SetX(T x) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].x($1);}, this->id, x);
    }

    // handles C++ functions
    template <typename T>
    emp::sfinae_decoy<void, decltype(&T::operator())>
    SetX(T x) {
      const uint32_t func_id = emp::JSWrap(x);

      MAIN_THREAD_EM_ASM({
        const id = $0;
        const func_id = $1;
        emp_d3.objects[id].x(function(d, i, j) {
                                  return emp.Callback(func_id, d, i, j);
                                });
      }, this->id, func_id);

      emp::JSDelete(func_id);
    }

    /// @endcond

    /// If the data that you are generating lines from is anything more complicated than a sequence
    /// of pairs of numbers, representing x and y (in that order), you need to tell the line
    /// generator how it should figure out what the y coordinate of a point in the line is. The
    /// parameter you pass to SetY should be instructions for doing so. It can either be a function
    /// (as a string indicating a Javascript function or as a literal C++ function) that accepts
    /// an element of the data sequence you are generating the line from, or it can be a constant,
    /// in which case the y coordinate of every point will be that constant.
    ///
    /// Note: This function will re-set any scales that you've added to the Y coordinate
    ///
    /// As an example, the default function expects data like this (array of arrays):
    /// [[0,0], [1,1], [2,2]]
    /// And has (a Javascript equivalent of) this accessor:
    /// int x(emp::array<int, 2> d) {return d[1];}
    ///
    /// If your data instead looked like this (array of Javascript objects with x and y values):
    /// [{x:0, y:0}, {x:1, y:1}, {x:2, y:2}]
    /// You might want to use an accessor like this:
    /// int x(JSONObject d) {return d.y();}
    /// Where JSONObject is a struct designed to hold necessary data from the Javascript object:
    ///  struct JSONObject {
    ///    EMP_BUILD_INTROSPECTIVE_TUPLE( int, x,
    ///                                   int, y
    ///                                  )
    ///  };
    //Handles setting x accessor to a function or string
    void SetY(std::string y) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const y_str = UTF8ToString($1);
        var sel = emp_d3.find_function(y_str);
        emp_d3.objects[id].y(sel);
      }, this->id,  y.c_str());
    }

    /// @cond TEMPLATES

    // Handles constants
    template <typename T>
    typename std::enable_if<std::is_fundamental<T>::value, void>::type
    SetY(T y) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].y($1);}, this->id, y);
    }

    // handles C++ functions
    template <typename T>
    emp::sfinae_decoy<void, decltype(&T::operator())>
    SetY(T y) {
      const uint32_t func_id = emp::JSWrap(y);

      MAIN_THREAD_EM_ASM({
        const id = $0;
        const func_id = $1;
        emp_d3.objects[id].y(function(d, i, j) {
                                  return emp.Callback(func_id, d, i, j);
                                });
      }, this->id, func_id);

      emp::JSDelete(func_id);
    }

    /// @endcond

  };

  class LinkGenerator : public LineGenerator {
  public:
    LinkGenerator(std::string type) {
        if (type == "vertical") {
            MAIN_THREAD_EM_ASM({emp_d3.objects[$0] = d3.linkVertical();}, this->id);
        } else if (type == "horizontal") {
            MAIN_THREAD_EM_ASM({emp_d3.objects[$0] = d3.linkHorizontal();}, this->id);
        } else {
            std::cout << "WARNING: Invalid link type" << std::endl;
            MAIN_THREAD_EM_ASM({emp_d3.objects[$0] = d3.linkVertical();}, this->id);
        }
    }

    // Handles setting source accessor to a function or string
    void SetSource(std::string source) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const source_str = UTF8ToString($1);
        var sel = emp_d3.find_function(source_str);
        emp_d3.objects[id].source(sel);
      }, this->id,  source.c_str());      
    }

    /// @cond TEMPLATES
    // Handles setting source to a constant
    template <typename T>
    typename std::enable_if<std::is_fundamental<T>::value, void>::type
    SetSource(T source) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].source($1);}, this->id, source);
    }

    // handles C++ functions
    template <typename T>
    emp::sfinae_decoy<void, decltype(&T::operator())>
    SetSource(T source) {
      const uint32_t func_id = emp::JSWrap(source);

      MAIN_THREAD_EM_ASM({
        const id = $0;
        const func_id = $1;
        emp_d3.objects[id].source(function(d, i, j) {
                                  return emp.Callback(func_id, d, i, j);
                                });
      }, this->id, func_id);

      emp::JSDelete(func_id);
    }

    /// @endcond

    //Handles setting target accessor to a function or string
    void SetTarget(std::string target) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const target_str = UTF8ToString($1);
        var sel = emp_d3.find_function(target_str);
        emp_d3.objects[id].target(sel);
      }, this->id, target.c_str());
    }

    /// @cond TEMPLATES
    // Handles setting source to a constant
    template <typename T>
    typename std::enable_if<std::is_fundamental<T>::value, void>::type
    SetTarget(T target) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].target($1);}, this->id, target);
    }

    // handles C++ functions
    template <typename T>
    emp::sfinae_decoy<void, decltype(&T::operator())>
    SetTarget(T target) {
      const uint32_t func_id = emp::JSWrap(target);

      MAIN_THREAD_EM_ASM({
        const id = $0;
        const func_id = $1;
        emp_d3.objects[id].target(function(d, i, j) {
                                  return emp.Callback(func_id, d, i, j);
                                });
      }, this->id, func_id);

      emp::JSDelete(func_id);
    }

    /// @endcond


    /// @cond TEMPLATES

    // Handles constants
    template <typename T>
    typename std::enable_if<std::is_fundamental<T>::value, void>::type
    SetY(T target) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].target($1);}, this->id, target);
    }

    // handles C++ functions
    template <typename T>
    emp::sfinae_decoy<void, decltype(&T::operator())>
    SetY(T target) {
      const uint32_t func_id = emp::JSWrap(target);

      MAIN_THREAD_EM_ASM({
        const id = $0;
        const func_id = $1;
        emp_d3.objects[id].target(function(d, i, j) {
                                  return emp.Callback(func_id, d, i, j);
                                });
      }, this->id, func_id);

      emp::JSDelete(func_id);
    }

    /// @endcond
  };

  /// An area is defined by two lines, with the area in between shaded
  class AreaGenerator : public LineGenerator {
  public:
    AreaGenerator() {
      MAIN_THREAD_EM_ASM({
        var new_line = d3.area();
        emp_d3.objects[$0] = new_line;
      }, this->id);
    }

    //Handles setting x0 accessor to a constant
    template <typename T>
    void SetX0(T x) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].x0($1);}, this->id, x);
    }

    //Handles setting y0 accessor to a constant
    template <typename T>
    void SetY0(T y) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].y0($1);}, this->id, y);
    }

    //Handles setting x0 accessor to a function or string
    void SetX0(std::string x) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const x_str = UTF8ToString($1);
        var sel = emp_d3.find_function(x_str);
        emp_d3.objects[id].x0(sel);
      }, this->id,  x.c_str());
    }

    //Handles setting y0 accessor to a function or string
    void SetY0(std::string y) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const y_str = UTF8ToString($1);
        var sel = emp_d3.find_function(y_str);
        emp_d3.objects[id].y0(sel);
      }, this->id,  y.c_str());
    }

    //Handles setting x1 accessor to a constant
    template <typename T>
    void SetX1(T x) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].x1($1);}, this->id, x);
    }

    //Handles setting y1 accessor to a constant
    template <typename T>
    void SetY1(T y) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].y1($1);}, this->id, y);
    }

    //Handles setting x1 accessor to a function or string
    void SetX1(std::string x) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const x_str = UTF8ToString($1);
        var sel = emp_d3.find_function(x_str);
        emp_d3.objects[id].x1(sel);
      }, this->id,  x.c_str());
    }

    //Handles setting y1 accessor to a function or string
    void SetY1(std::string y) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const y_str = UTF8ToString($1);
        var sel = emp_d3.find_function(y_str);
        emp_d3.objects[id].y1(sel);
      }, this->id,  y.c_str());
    }
  };

  class RadialLineGenerator : public BaseLineGenerator {
  public:
    RadialLineGenerator(){
      MAIN_THREAD_EM_ASM({
        var new_line = d3.radialLine();
        emp_d3.objects[$0] = new_line;
      }, this->id);
    }

    void SetRadius(float radius) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].radius($1);}, this->id, radius);
    }

    void SetRadius(std::string radius) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const radius_str = UTF8ToString($1);
        var sel = emp_d3.find_function(radius_str);
        emp_d3.objects[id].radius(sel);
      }, this->id,  radius.c_str());
    }

    void SetAngle(float angle) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].angle($1);}, this->id, angle);
    }

    void SetAngle(std::string angle) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const angle_str = UTF8ToString($1);
        var sel = emp_d3.find_function(angle_str);
        emp_d3.objects[id].angle(sel);
      }, this->id,  angle.c_str());
    }
  };

  class RadialAreaGenerator : public RadialLineGenerator {
  public:
    RadialAreaGenerator() {
      MAIN_THREAD_EM_ASM({
        var new_line = d3.radialArea();
        emp_d3.objects[$0] = new_line;
      }, this->id);
    }

    void SetInnerRadius(float radius) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].innerRadius($1);}, this->id, radius);
    }

    void SetInnerRadius(std::string radius) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const radius_str = UTF8ToString($1);
        var sel = emp_d3.find_function(radius_str);
        emp_d3.objects[id].innerradius(sel);
      }, this->id,  radius.c_str());
    }

    void SetOuterRadius(float radius) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].outerRadius($1);}, this->id, radius);
    }

    void SetOuterRadius(std::string radius) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const radius_str = UTF8ToString($1);
        var sel = emp_d3.find_function(radius_str);
        emp_d3.objects[id].outerradius(sel);
      }, this->id,  radius.c_str());
    }

    void SetStartAngle(float angle) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].startAngle($1);}, this->id, angle);
    }

    void SetStartAngle(std::string angle) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const angle_str = UTF8ToString($1);
        var sel = emp_d3.find_function(angle_str);
        emp_d3.objects[id].startAngle(sel);
      }, this->id,  angle.c_str());
    }

    void SetEndAngle(float angle) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].endAngle($1);}, this->id, angle);
    }

    void SetEndAngle(std::string angle) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const angle_str = UTF8ToString($1);
        var sel = emp_d3.find_function(angle_str);
        emp_d3.objects[id].endAngle(sel);
      }, this->id,  angle.c_str());
    }
  };

  class ChordGenerator : public RadialAreaGenerator {
  public:
    ChordGenerator()  {
      MAIN_THREAD_EM_ASM({
        var new_line = d3.ribbon();
        emp_d3.objects[$0] = new_line;
      }, this->id);
    }

    template <typename T>
    void SetSource(T source) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].source($1);}, this->id, source);
    }

    void SetSource(std::string source) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const source_str = UTF8ToString($1);
        var sel = emp_d3.find_function(source_str);
        emp_d3.objects[id].source(sel);
      }, this->id,  source.c_str());
    }

    template <typename T>
    void SetTarget(T target) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].target($1);}, this->id, target);
    }

    void SetTarget(std::string target) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const target_str = UTF8ToString($1);
        var sel = emp_d3.find_function(target_str);
        emp_d3.objects[id].target(sel);
      }, this->id,  target.c_str());
    }
  };

  class ArcGenerator : public RadialAreaGenerator {
  public:
    ArcGenerator()  {
      MAIN_THREAD_EM_ASM({
        var new_line = d3.arc();
        emp_d3.objects[$0] = new_line;
      }, this->id);
    }

    void SetCornerRadius(float radius) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].cornerRadius($1);}, this->id, radius);
    }

    void SetCornerRadius(std::string radius) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const radius_str = UTF8ToString($1);
        var sel = emp_d3.find_function(radius_str);
        emp_d3.objects[id].cornerRadius(sel);
      }, this->id,  radius.c_str());
    }

    void SetPadRadius(float radius) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].padRadius($1);}, this->id, radius);
    }

    void SetPadRadius(std::string radius) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const radius_str = UTF8ToString($1);
        var sel = emp_d3.find_function(radius_str);
        emp_d3.objects[id].padRadius(sel);
      }, this->id,  radius.c_str());
    }

    void SetPadAngle(float angle) {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0].padAngle($1);}, this->id, angle);
    }

    void SetPadAngle(std::string angle) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const angle_str = UTF8ToString($1);
        var sel = emp_d3.find_function(angle_str);
        emp_d3.objects[id].padAngle(sel);
      }, this->id,  angle.c_str());
    }

    //TODO: Centroid (requires passing array back)

  };

}

#endif // #ifndef EMP_WEB_D3_SVG_SHAPES_HPP_INCLUDE
