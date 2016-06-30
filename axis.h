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
    std::string label;
    std::string dom_id;

  public:
    Selection group;

    Axis(std::string label = "") {
      //The scale got added to th the list of objects before this one
      this->label = label;
      EM_ASM_ARGS({js.objects[$0] = d3.svg.axis();}, this->id);
    }

    //Draw axis on specified selection with intelligent default positioning
    void Draw(Selection selection){
      this->SetTickFormat("g");

      //Dom ids can't contain whitespace
      dom_id = label;
      emp::remove_whitespace(dom_id);
      dom_id += "_axis";

      EM_ASM_ARGS({
        var axis_range = js.objects[$0].scale().range();
	    js.objects[$3] = js.objects[$1].append("g")
                                  .attr("id", Pointer_stringify($2))
                                  .call(js.objects[$0]);

        var canvas_width = js.objects[$1].attr("width");
        var canvas_height = js.objects[$1].attr("height");

        var orient = js.objects[$0].orient();
        var dy = "2em";
        var x_divisor = 2.0;
        var text_orient = 0;
        js.objects[$3].attr("transform", "translate(0,"+(canvas_height-60)+")");
        if (orient == "top") {
          dy = "-2em";
          x_divisor = 2.0;
          js.objects[$3].attr("transform", "translate(0,60)");
        } else if (orient == "left") {
          dy = "-2em";
          x_divisor = -2.0;
          text_orient = -90;
          js.objects[$3].attr("transform", "translate(60,0)");
        } else if(orient == "right") {
          dy = "2em";
          text_orient = -90;
          js.objects[$3].attr("transform", "translate("+(canvas_width-60)+",0)");
        }

        js.objects[$3].selectAll("line, .domain")
             .attr("stroke-width", 1)
             .attr("fill", "none")
             .attr("stroke", "black");
        js.objects[$3].append("text")
             .attr("id", "axis_label")
             .attr("transform", "rotate("+text_orient+")")
             .attr("x", axis_range[0]+(axis_range[1]-axis_range[0])/x_divisor)
             .attr("dy", dy).style("text-anchor", "middle")
             .text(Pointer_stringify($4));
      }, this->id, selection.GetID(), dom_id.c_str(), group.GetID(), label.c_str());
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
      EM_ASM_ARGS({
	    js.objects[$0].orient(Pointer_stringify($1));
      }, this->id, orientation.c_str(), group.GetID());
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

    void Rescale(double new_min, double new_max, D3::Selection & svg){
      this->scale.SetDomain(std::array<double, 2>({new_min, new_max}));
      ApplyAxis(svg.Select("#"+dom_id));
    }

    //TODO:  ticks

  };

  //Helper function to draw a standard set of x and y axes
  //Takes the desired x axis, y axis, and the selection on which to draw them
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

}

#endif
