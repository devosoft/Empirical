#ifndef __AXIS_H__
#define __AXIS_H__

#include "d3_init.h"
#include "../Empirical/emtools/js_utils.h"
#include "../Empirical/tools/string_utils.h"
#include "selection.h"
#include "scales.h"

namespace D3 {

  template <typename SCALE_TYPE>
  class Axis : public D3_Base {
  private:
    SCALE_TYPE scale;
    int orientation = 0;
    std::string label;

  public:
    Selection group;

    Axis(std::string label = "") {
      //The scale got added to th the list of objects before this one
      this->label = label;
      this->id++;
      this->id++;
      EM_ASM({js.objects.push(d3.svg.axis())});
    }

    void Draw(Selection selection){
      this->SetTickFormat("g");

      //Dom ids can't contain whitespace
      std::string dom_id = label;
      emp::remove_whitespace(dom_id);

      EM_ASM_ARGS({
        var axis_range = js.objects[$0].scale().range();
	    js.objects[$3] = js.objects[$1].append("g")
                                  .attr("id", Pointer_stringify($2)+"_axis")
                                  .call(js.objects[$0]);

        js.objects[$3].selectAll("line, .domain")
             .attr("stroke-width", 1)
             .attr("fill", "none")
             .attr("stroke", "black");
        js.objects[$3].append("text")
             .attr("id", "axis_label")
             .attr("transform", "rotate("+$4+")")
             .attr("x", (axis_range[1]-axis_range[0])/(($4<0) ? -2.0 : 2.0))
             .attr("dy", ($4<0) ? "-2em" : "3em").style("text-anchor", "middle")
             .text(Pointer_stringify($2));
      }, this->id, selection.GetID(), dom_id.c_str(), group.GetID(), orientation);
    }

    void ApplyAxis(Selection selection) {
      EM_ASM_ARGS({
	    js.objects[$1].call(js.objects[$0]);
	  }, this->id, selection.GetID());
    }

    void SetScale(SCALE_TYPE & scale) {
      this->scale = scale;

      EM_ASM_ARGS({
	    js.objects[$0].scale(js.objects[$1]);
	  }, this->id, scale.GetID());
    }

    SCALE_TYPE GetScale(){
      return this->scale;
    }

    //Needs to be called before Draw
    void SetOrientation(std::string orientation) {

      if (orientation == "top" || orientation == "bottom") {
        this->orientation = 0;
      } else {
        this->orientation = -90;
      }

      EM_ASM_ARGS({
	    js.objects[$0].orient(Pointer_stringify($1));
      }, this->id, orientation.c_str(), group.GetID(), this->orientation);
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

    void SetTicks(int count){
      EM_ASM_ARGS({
	    js.objects[$0].ticks($1);
	  }, this->id, count);
    }

    void SetTickFormat(std::string format) {
      EM_ASM_ARGS({
        js.objects[$0].tickFormat(d3.format(Pointer_stringify([$1])));
      }, this->id, format.c_str());
    }

    //TODO:  ticks

  };

  template <typename SCALE_X_TYPE, typename SCALE_Y_TYPE>
  void DrawAxes(Axis<SCALE_X_TYPE> & x_axis, Axis<SCALE_Y_TYPE> & y_axis, Selection & selection){
    x_axis.Draw(selection);
    y_axis.SetOrientation("left");
    y_axis.Draw(selection);

    EM_ASM_ARGS({
      x_range = js.objects[$0].scale().range();
      y_range = js.objects[$1].scale().range();

      js.objects[$2].attr("transform", "translate(0,"+y_range[1]+")");
      js.objects[$3].attr("transform", "translate("+x_range[0]+",0)");
    }, x_axis.GetID(), y_axis.GetID(), x_axis.group.GetID(), y_axis.group.GetID());
  }

  //  template <typename SCALE_TYPE>
  //Axis::Axis()

}

#endif
