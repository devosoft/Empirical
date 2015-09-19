#ifndef __SVG_SHAPES_H__
#define __SVG_SHAPES_H__

namespace D3 {

  class Scale;

  class LineGenerator {
  private:
    int id;
    Scale xscale;
    Scale yscale;
  public:
    LineGenerator();

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

    void SetInterpolate(std::string interpolate){
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(interpolate, interpolate.c_str())
    }

    void SetTension(float tension){
      EM_ASM_ARGS({
	  js.objects[$0].tension($1);
	}, this->id, tension);
    }

    void SetDefined(std::string defined){
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(defined, defined.c_str())
    }

    template <typename T, size_t SIZE>
    std::string MakeLine(std::array<std::array<T, 2>, SIZE> data){
      PassArrayToJavascript(data);
    
      int buffer = EM_ASM_INT({
	  var result = js.objects[$0](emp.__incoming_array);
	  var buffer = Module._malloc(result.length+1);
	  Module.writeStringToMemory(result, buffer);
	  return buffer;
	}, this->id);

      return (char *)buffer;
    }

  };
  
  LineGenerator::LineGenerator(){
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({
	var new_line = d3.svg.line();
	js.objects.push(new_line);
      });
  }

}

#endif
