#ifndef __AXIS_H__
#define __AXIS_H__

#include <emscripten.h>
#include "../Empirical/emtools/js_utils.h"

class Selection;
class Scale;

namespace D3 {

  class Axis {
  private:
    int id;
    Scale scale;

  public:
    Axis();

    void ApplyAxis(Selection selection) {
      EM_ASM_ARGS({
	  js.objects[$1].call(js.objects[$0]);
	}, this->id, selection.GetID());
    }

    void SetScale(Scale scale) {
      this->scale = scale;

      EM_ASM_ARGS({
	  js.objects[$0].scale(js.objects[$1]);
	}, this->id, scale.GetID());
    }

    Scale GetScale(){
      return this->scale;
    }

    void SetOrientation(std::string orientation) {
      EM_ASM_ARGS({
	  js.objects[$0].orient(Pointer_stringify($1));
	}, this->id, orientation.c_str());
    }  

    template <typename T, std::size_t SIZE>
    void SetTickValues(std::array<T, SIZE> values) {
      pass_array_to_javascript(values);

      EM_ASM_ARGS({
	  js.objects[$0].tickValues(emp.__incoming_array);
	}, this->id);
    }

    void SetTickSize(float inner, float outer) {
      EM_ASM_ARGS({
	  js.objects[$0].innerTickSize($1, $2);
	}, this->id, inner, outer);
    }

    void SetInnerTickSize(float size) {
      EM_ASM_ARGS({
	  js.objects[$0].innerTickSize($1);
	}, this->id, size);
    }

    void SetOuterTickSize(float size) {
      EM_ASM_ARGS({
	  js.objects[$0].outerTickSize($1);
	}, this->id, size);
    }
    
    void SetTickPadding(int padding) {
      EM_ASM_ARGS({
	  js.objects[$0].tickPadding($1);
	}, this->id, padding);
    }

    //TODO: Format, ticks

  };

  Axis::Axis() {
    this->id = EM_ASM_INT_V({return js.objects.length});

    EM_ASM({js.objects.push(d3.svg.axis())});
  }

}

#endif
