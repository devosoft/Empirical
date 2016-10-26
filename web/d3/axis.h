#ifndef __AXIS_H__
#define __AXIS_H__

#include "../../web/js_utils.h"
#include "../../tools/string_utils.h"

#include "d3_init.h"
#include "selection.h"
#include "scales.h"

namespace D3 {

  /// Axis objects are in charge of drawing graphical axes onto svg canvases. An axis depicts a
  /// scale, so every axis has a scale, and is templated off of the type of that scale.
  template <typename SCALE_TYPE = LinearScale>
  class Axis : public D3_Base {
  private:
    SCALE_TYPE scale;
    std::string label;
    std::string dom_id = "";
    std::string label_offset = "";

  public:

    /// There are a lot of graphical elements associated with an axis, so it's best to group them
    /// all together into an html group element. This selection holds a pointer to the group for
    /// this axis
    Selection group;

    /// Consruct an axis - this doesn't draw anything yet, but sets up the necessary infrastructure
    /// to draw it when you call the Draw method. Optionally takes a label to label the axis with.
    /// This label will also be used to create an id for the axis, to make it easier to select it
    /// later. The id will be the same as [label], but with all whitespace removed and "_axis"
    /// appended to the end.
    ///
    /// For example, if your label was "Per capita mortality", you could select the axis with:
    /// `D3::Select("#Percapitamortality_axis");`.
    Axis(std::string label = "") {
      //The scale got added to th the list of objects before this one
      this->label = label;
      EM_ASM_ARGS({js.objects[$0] = d3.svg.axis();}, this->id);
    }

    /// Draw axis on [selection] (must contain a single SVG element) with intelligent default
    /// positioning
    void Draw(Selection selection){
      //this->SetTickFormat("g");

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

        if (Pointer_stringify($5) != "") {
          dy = Pointer_stringify($5);
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
      }, this->id, selection.GetID(), dom_id.c_str(), group.GetID(), label.c_str(),
      label_offset.c_str());
    }

    template <typename T>
    void ApplyAxis(SelectionOrTransition<T> selection) {
      EM_ASM_ARGS({
	    js.objects[$1].call(js.objects[$0]);
	  }, this->id, selection.GetID());
    }

    /// An axis must have a scale. By default, a scale of SCALE_TYPE will be constructed, but
    /// usually you want an axis to depict a specific scale. This method points this object's
    /// scale member variable at [scale].
    void SetScale(SCALE_TYPE & scale) {
      this->scale = scale;

      EM_ASM_ARGS({
	    js.objects[$0].scale(js.objects[$1]);
	  }, this->id, scale.GetID());
    }

    /// Get a reference to this object's scale.
    SCALE_TYPE& GetScale(){
      return this->scale;
    }

    /// Adjust the location of the label text relative to the axis
    /// (helpful if numbers are overlapping it). Can be negative.
    /// Use "em" (e.g. "2em") to specify distance relative to font size.
    void AdjustLabelOffset(std::string offset) {
      label_offset = offset;
      if (dom_id != "") { //We've already drawn stuff
        group.Select("#axis_label").SetAttr("dy", label_offset);
      }
    }

    /// Draw tries to make a good guess about where to place the axis, but sometimes you want to
    /// scoot it over. This method will move the axis to the x,y location specified.
    void Move(int x, int y) {
      group.Move(x,y);
    }

    /// Set orientation of this axis to [orientation] (must be "bottom", "top", "left", or "right")
    /// Controls default placement on SVG, whether main line is vertical or horizontal, and which
    /// side the ticks and label show up on.
    //Needs to be called before Draw
    void SetOrientation(std::string orientation) {
      EM_ASM_ARGS({
	    js.objects[$0].orient(Pointer_stringify($1));
      }, this->id, orientation.c_str(), group.GetID());
    }

    template <typename T, std::size_t SIZE>
    void SetTickValues(std::array<T, SIZE> values) {
      emp::pass_array_to_javascript(values);

      EM_ASM_ARGS({
  	    js.objects[$0].tickValues(emp_i.__incoming_array);
	  }, this->id);
    }

    void SetTickSize(float size) {
      EM_ASM_ARGS({
	    js.objects[$0].tickSize($1);
      }, this->id, size);
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

    /// Set the number of ticks along the axis
    void SetTicks(int count){
      EM_ASM_ARGS({
	    js.objects[$0].ticks($1);
	  }, this->id, count);
    }

    /// Set the format for displaying numbers assoiated with ticks. [format] should be a format
    /// following
    /// [the rules for d3.format()](https://github.com/d3/d3-3.x-api-reference/blob/master/Formatting.md#d3_format)
    void SetTickFormat(std::string format) {
      EM_ASM_ARGS({
        js.objects[$0].tickFormat(d3.format(Pointer_stringify($1)));
      }, this->id, format.c_str());
    }

    /// Adjust scale and axis to accomodate the new range of data specified by [new_min],
    /// and [new_max]. [svg] is a Selection or Transition containing the current axis. If it's a
    /// transition, then the rescaling will be animated.
    template <typename T>
    void Rescale(double new_min, double new_max, D3::SelectionOrTransition<T> & svg){
      this->scale.SetDomain(std::array<double, 2>({new_min, new_max}));
      ApplyAxis(svg.Select("#"+dom_id));
    }

    //TODO:  ticks

  };

  /// Helper function to draw a standard set of x and y axes
  /// Takes the desired x axis, y axis, and the selection on which to draw them
  template <typename SCALE_X_TYPE = D3::LinearScale, typename SCALE_Y_TYPE = D3::LinearScale>
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
