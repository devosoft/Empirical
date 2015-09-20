#ifndef __SVG_SHAPES_H__
#define __SVG_SHAPES_H__

#include "../Empirical/emtools/js_utils.h"

namespace D3 {
  class Scale;

  
  class SvgShapeGenerator {
  protected:
    int id;
    SvgShapeGenerator();
  public:
  };

  SvgShapeGenerator::SvgShapeGenerator() {}
  

  class LineGenerator : public SvgShapeGenerator {
  public:
    LineGenerator();
    
    void SetInterpolate(std::string interpolate){
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(interpolate,interpolate.c_str())
    }

    void SetTension(float tension){
      EM_ASM_ARGS({
	  js.objects[$0].tension($1);
	}, this->id, tension);
    }

    void SetDefined(std::string defined){
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(defined, defined.c_str())
    }

    //This function creates a string specifying a path
    template <typename T, size_t SIZE>
    std::string Path(std::array<std::array<T, 2>, SIZE> data){
      emp::pass_array_to_javascript(data);
    
      int buffer = EM_ASM_INT({
	  var result = js.objects[$0](emp.__incoming_array);
	  var buffer = Module._malloc(result.length+1);
	  Module.writeStringToMemory(result, buffer);
	  return buffer;
	}, this->id);

      return (char *)buffer;
    }

    //This function actually handles the binding of the path string to the dom
    template <typename T, std::size_t SIZE>
    Selection DrawLine(std::array<std::array<T, 2>, SIZE> data) {
      Selection path = Select("svg").Append("path");
      path.SetAttr("d", Path(data).c_str());
      return path;
    }

   //This function draws an array of lines
    template <typename T, std::size_t SIZE, std::size_t SIZE2>
    Selection DrawLines(std::array<std::array<std::array<T, 2>, SIZE>, SIZE2>  data) {
      Selection group = Select("svg").Append("g");
      for (auto arr: data) {
	Selection path = group.Append("path");
	path.SetAttr("d", Path(arr).c_str());
      }
      return group;
    }

  };

  LineGenerator::LineGenerator() {
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({
	var new_line = d3.svg.line();
	js.objects.push(new_line);
      });
  }

  class CartesianLineGenerator : public LineGenerator {

  private:
    Scale xscale;
    Scale yscale;
  public:
    CartesianLineGenerator();

    void SetXScale(Scale scale){
      this->xscale = scale;
      EM_ASM_ARGS({
	  var scale = js.objects[$1];
	  var curr_x = js.objects[$0].x;

	  //Apply scale to whatever the current x axis function is
	  js.objects[$0].x(function(d, i){return scale(curr_x)});
	}, this->id, scale.GetID());
    }

    void SetYScale(Scale scale){
      this->yscale = scale;
      EM_ASM_ARGS({
	  var scale = js.objects[$1];
	  var curr_y = js.objects[$0].y;

	  //Apply scale on top of whatever the current y axis function is
	  js.objects[$0].y(function(d, i){return scale(curr_y)});
	}, this->id, scale.GetID());
    }

    Scale GetXScale(){
      return this->xscale;
    }

    Scale GetYScale(){
      return this->yscale;
    }

    //Handles setting x accessor to a constant
    template <typename T>
    void SetX(T x) {
      EM_ASM_ARGS({js.objects[$0].x($1);}, this->id, x);
    }

    //Handles setting y accessor to a constant
    template <typename T>
    void SetY(T y) {
      EM_ASM_ARGS({js.objects[$0].y($1);}, this->id, y);
    }

    //Handles setting x accessor to a function or string
    void SetX(std::string x) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(x, x.c_str())
    }

    //Handles setting x accessor to a function or string
    void SetY(std::string y) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(y, y.c_str())
    }

  };
  
  CartesianLineGenerator::CartesianLineGenerator() {
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({
	var new_line = d3.svg.line();
	js.objects.push(new_line);
      });
  }
 
  class AreaGenerator : public CartesianLineGenerator {
  public:
    AreaGenerator();

    //Handles setting x0 accessor to a constant
    template <typename T>
    void SetX0(T x) {
      EM_ASM_ARGS({js.objects[$0].x0($1);}, this->id, x);
    }

    //Handles setting y0 accessor to a constant
    template <typename T>
    void SetY0(T y) {
      EM_ASM_ARGS({js.objects[$0].y0($1);}, this->id, y);
    }

    //Handles setting x0 accessor to a function or string
    void SetX0(std::string x) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(x0, x.c_str())
    }

    //Handles setting y0 accessor to a function or string
    void SetY0(std::string y) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(y0, y.c_str())
    }

    //Handles setting x1 accessor to a constant
    template <typename T>
    void SetX1(T x) {
      EM_ASM_ARGS({js.objects[$0].x1($1);}, this->id, x);
    }

    //Handles setting y1 accessor to a constant
    template <typename T>
    void SetY1(T y) {
      EM_ASM_ARGS({js.objects[$0].y1($1);}, this->id, y);
    }

    //Handles setting x1 accessor to a function or string
    void SetX1(std::string x) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(x1, x.c_str())
    }

    //Handles setting y1 accessor to a function or string
    void SetY1(std::string y) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(y1, y.c_str())
    }    
  };

  AreaGenerator::AreaGenerator() {
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({
	var new_line = d3.svg.area();
	js.objects.push(new_line);
      });
  }

  class RadialLineGenerator : public LineGenerator {
  public:
    RadialLineGenerator();

    void SetRadius(float radius) {
      EM_ASM_ARGS({js.objects[$0].radius($1);}, this->id, radius);
    }

    void SetRadius(std::string radius) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(radius, radius.c_str())
    }

    void SetAngle(float angle) {
      EM_ASM_ARGS({js.objects[$0].angle($1);}, this->id, angle);
    }

    void SetAngle(std::string angle) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(angle, angle.c_str())
    }
  };

  RadialLineGenerator::RadialLineGenerator() {
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({
	var new_line = d3.svg.line.radial();
	js.objects.push(new_line);
      });
  }

  class RadialAreaGenerator : public RadialLineGenerator {
  public:
    RadialAreaGenerator();

    void SetInnerRadius(float radius) {
      EM_ASM_ARGS({js.objects[$0].innerRadius($1);}, this->id, radius);
    }

    void SetInnerRadius(std::string radius) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(innerRadius, radius.c_str())
    }

    void SetOuterRadius(float radius) {
      EM_ASM_ARGS({js.objects[$0].outerRadius($1);}, this->id, radius);
    }

    void SetOuterRadius(std::string radius) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(outerRadius, radius.c_str())
    }

    void SetStartAngle(float angle) {
      EM_ASM_ARGS({js.objects[$0].startAngle($1);}, this->id, angle);
    }

    void SetStartAngle(std::string angle) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(startAngle, angle.c_str())
    }

    void SetEndAngle(float angle) {
      EM_ASM_ARGS({js.objects[$0].endAngle($1);}, this->id, angle);
    }

    void SetEndAngle(std::string angle) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(endAngle, angle.c_str())
    }
  };

  RadialAreaGenerator::RadialAreaGenerator() {
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({
	var new_line = d3.svg.area.radial();
	js.objects.push(new_line);
      });
  }
 
  class ArcGenerator : public RadialAreaGenerator {
  public:
    ArcGenerator();
    
    void SetCornerRadius(float radius) {
      EM_ASM_ARGS({js.objects[$0].cornerRadius($1);}, this->id, radius);
    }

    void SetCornerRadius(std::string radius) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(cornerRadius, radius.c_str())
    }

    void SetPadRadius(float radius) {
      EM_ASM_ARGS({js.objects[$0].padRadius($1);}, this->id, radius);
    }

    void SetPadRadius(std::string radius) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(padRadius, radius.c_str())
    }

    void SetPadAngle(float angle) {
      EM_ASM_ARGS({js.objects[$0].padAngle($1);}, this->id, angle);
    }

    void SetPadAngle(std::string angle) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(padAngle, angle.c_str())
    }

    //TODO: Centroid (requires passing array back)

  };

  ArcGenerator::ArcGenerator() {
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({
	var new_line = d3.svg.arc();
	js.objects.push(new_line);
      });
  }

  //TODO: symbol, chord, diagonal

}

#endif
