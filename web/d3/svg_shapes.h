#ifndef __SVG_SHAPES_H__
#define __SVG_SHAPES_H__

#include "d3_init.h"
#include "selection.h"
#include "scales.h"
#include "dataset.h"

#include "../../emtools/js_utils.h"

namespace D3 {

  class SvgShapeGenerator : public D3_Base {
  protected:
    SvgShapeGenerator();
  public:

    //This function creates a string specifying a path
    template <typename T, size_t SIZE>
    std::string Generate(std::array<std::array<T, 2>, SIZE> data){
      emp::pass_array_to_javascript(data);

      int buffer = EM_ASM_INT({
	  var result = js.objects[$0](emp_i.__incoming_array);
	  var buffer = Module._malloc(result.length+1);
	  Module.writeStringToMemory(result, buffer);
	  return buffer;
  }, this->id);

      return (char *)buffer;
    }

    //This function actually handles the binding of the path string to the dom
    template <typename T, std::size_t SIZE>
    Selection DrawShape(std::array<std::array<T, 2>, SIZE> data, Selection s) {
      Selection path = s.Append("path");
      path.SetAttr("d", Generate(data).c_str());
      return path;
    }

    //This function actually handles the binding of the path string to the dom
    //without passing the data through C++
    Selection DrawShape(Dataset data, Selection s){//, Selection appendto=Select("svg"),
      //std::string type = "path") {
      Selection path = s.Append("path");
      //Selection path = appendto.SelectAll("path");//type.c_str());
      //Selection path = Select("svg").SelectAll("path");//type.c_str());

      EM_ASM_ARGS({
	    var accessor = function(data){
	      for (i=0; i<data.length; i++) {
	        for (j=0; j<data[i].length; j++) {
		      data[i][j] = +data[i][j];
	        }
	      }
	      return(data);
	    };
	    emp.__incoming_data = accessor(emp.__incoming_data);
	    js.objects[$0].data(emp.__incoming_data).enter().append("path").attr("d", js.objects[$1]);
	    console.log(d3.selectAll("path"));
	  }, path.GetID(), this->id, data.GetID());
      return path;
    }

   //This function draws an array of lines
    template <typename T, std::size_t SIZE, std::size_t SIZE2>
    Selection DrawShapes(std::array<std::array<std::array<T, 2>, SIZE>,\
			 SIZE2>  data) {
      Selection group = Select("svg").Append("g");
      for (auto arr: data) {
	    Selection path = group.Append("path");
	    path.SetAttr("d", Generate(arr).c_str());
      }
      return group;
    }
  };

  SvgShapeGenerator::SvgShapeGenerator() {}

  class SymbolGenerator : public SvgShapeGenerator {
  public:
    SymbolGenerator();

    void SetType(std::string type){
      //TODO: Should we check that type is in d3.svg.symbolTypes?
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(type, type.c_str())
    }

    //If size is a constant, it's in pixels, so an int is reasonable
    void SetSize(int size) {
      EM_ASM_ARGS({
	    js.objects[$0].size($1);
	  }, this->id, size);
    }

    //Otherwise it's a function
    void SetSize(std::string size){
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(size, size.c_str())
    }
  };

  SymbolGenerator::SymbolGenerator() {

    EM_ASM_ARGS({
	  var new_line = d3.svg.symbol();
	  js.objects[$0] = new_line;
    }, this->id);
  }


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
  };

  LineGenerator::LineGenerator() {
    EM_ASM_ARGS({
	  var new_line = d3.svg.line();
	  js.objects[$0] = new_line;
    }, this->id);
  }

  template <typename X_SCALE_TYPE, typename Y_SCALE_TYPE>
  class CartesianLineGenerator : public LineGenerator {

  private:
    X_SCALE_TYPE xscale;
    Y_SCALE_TYPE yscale;
  public:
    CartesianLineGenerator() {
      EM_ASM_ARGS({
	    var new_line = d3.svg.line();
	    js.objects[$0] = new_line;
      }, this->id);
    }

    void SetXScale(X_SCALE_TYPE scale){
      this->xscale = scale;
      EM_ASM_ARGS({
	    var scale = js.objects[$1];
	    var curr_x = js.objects[$0].x();

	    //Apply scale to whatever the current x axis function is
	    js.objects[$0].x(function(d, i){return scale(curr_x(d, i))});
	  }, this->id, scale.GetID());
    }

    void SetYScale(Y_SCALE_TYPE scale){
      this->yscale = scale;
      EM_ASM_ARGS({
	    var scale = js.objects[$1];
	    var curr_y = js.objects[$0].y();

	    //Apply scale on top of whatever the current y axis function is
	    js.objects[$0].y(function(d, i){return scale(curr_y(d, i))});
	  }, this->id, scale.GetID());
    }

    X_SCALE_TYPE GetXScale(){
      return this->xscale;
    }

    Y_SCALE_TYPE GetYScale(){
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

  template <typename X_SCALE_TYPE, typename Y_SCALE_TYPE>
  class AreaGenerator : public CartesianLineGenerator<X_SCALE_TYPE, Y_SCALE_TYPE> {
  public:
    AreaGenerator() {
      EM_ASM_ARGS({
	    var new_line = d3.svg.area();
	    js.objects[$0] = new_line;
      }, this->id);
    }

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
    EM_ASM_ARGS({
	  var new_line = d3.svg.line.radial();
	  js.objects[$0] = new_line;
  }, this->id);
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
    EM_ASM_ARGS({
   	  var new_line = d3.svg.area.radial();
	  js.objects[$0] = new_line;
    }, this->id);
  }

  class ChordGenerator : public RadialAreaGenerator {
  public:
    ChordGenerator();

    template <typename T>
    void SetSource(T source) {
      EM_ASM_ARGS({js.objects[$0].source($1);}, this->id, source);
    }

    void SetSource(std::string source) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(source, source.c_str())
    }

    template <typename T>
    void SetTarget(T target) {
      EM_ASM_ARGS({js.objects[$0].target($1);}, this->id, target);
    }

    void SetTarget(std::string target) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(target, target.c_str())
    }
  };

  ChordGenerator::ChordGenerator() {
    EM_ASM_ARGS({
  	  var new_line = d3.svg.chord();
	  js.objects[$0] = new_line;
    }, this->id);
  }

  class DiagonalGenerator : public ChordGenerator {
  public:
    DiagonalGenerator();

   void SetProjection(std::string projection) {
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(projection, projection.c_str())
    }
  };

  DiagonalGenerator::DiagonalGenerator() {
    EM_ASM_ARGS({
	  var new_line = d3.svg.diagonal();
	  js.objects[$0] = new_line;
    }, this->id);
  }

  //There is no documentation on this class in D3 other than that it exists
  //so I'm just making it exist here too.
  class DiagonalRadialGenerator : public ChordGenerator {
  public:
    DiagonalRadialGenerator();
  };

  DiagonalRadialGenerator::DiagonalRadialGenerator() {
    EM_ASM_ARGS({
	  var new_line = d3.svg.area.diagonal.radil();
	  js.objects[$0] = new_line;
    }, this->id);
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
    EM_ASM_ARGS({
	  var new_line = d3.svg.arc();
	  js.objects[$0] = new_line;
    }, this->id);
  }

}

#endif
