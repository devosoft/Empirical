/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018
 *
 *  @file  axis.hpp
 *  @brief Handle drawing of axes on D3 graphts.
 */

#ifndef EMP_D3_AXIS_H
#define EMP_D3_AXIS_H

#include "../js_utils.hpp"
#include "../../tools/string_utils.hpp"
#include "d3_init.hpp"
#include "selection.hpp"
#include "scales.hpp"

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
    std::string orientation;

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
    Axis(std::string type, std::string label = "") {
      //The scale got added to th the list of objects before this one
      this->label = label;
      this->orientation = type;
      if (type == "left") {
          MAIN_THREAD_EMP_ASM({
              js.objects[$0] = d3.axisLeft(js.objects[$1]);
          }, this->id, scale.GetID());
      } else if (type == "right") {
          MAIN_THREAD_EMP_ASM({
              js.objects[$0] = d3.axisRight(js.objects[$1]);
          }, this->id, scale.GetID());
      } else if (type == "bottom") {
          MAIN_THREAD_EMP_ASM({
              js.objects[$0] = d3.axisBottom(js.objects[$1]);
          }, this->id, scale.GetID());
      } else if (type == "top") {
        MAIN_THREAD_EMP_ASM({
            js.objects[$0] = d3.axisTop(js.objects[$1]);
        }, this->id, scale.GetID());
      } else {
          std::cout << "WARNING: Invalid type given to axis constructor" << std::endl;
          MAIN_THREAD_EMP_ASM({
              js.objects[$0] = d3.axisBottom(js.objects[$1]);
          }, this->id, scale.GetID());
      }

    }

    /// Draw axis on [selection] (must contain a single SVG element) with intelligent default
    /// positioning
    Axis& Draw(Selection & selection){
      //this->SetTickFormat("g");

      //Dom ids can't contain whitespace
      dom_id = label;
      emp::remove_whitespace(dom_id);
      dom_id += "_axis";

      MAIN_THREAD_EMP_ASM({
        var axis_range = js.objects[$0].scale().range();
	    js.objects[$3] = js.objects[$1].append("g");

        js.objects[$3].append("g").attr("id", UTF8ToString($2))
                                  .call(js.objects[$0]);

        var canvas_width = js.objects[$1].attr("width");
        var canvas_height = js.objects[$1].attr("height");

        var orient = UTF8ToString($6);
        var dy = "2.5em";
        var x_divisor = 2.0;
        var text_orient = 0;
        js.objects[$3].attr("transform", "translate(0,"+(canvas_height-60)+")");
        if (orient == "top") {
          dy = "-2.5em";
          x_divisor = 2.0;
          js.objects[$3].attr("transform", "translate(0,60)");
        } else if (orient == "left") {
          dy = "-2.5em";
          x_divisor = -2.0;
          text_orient = -90;
          js.objects[$3].attr("transform", "translate(60,0)");
        } else if(orient == "right") {
          dy = "2.5em";
          text_orient = -90;
          js.objects[$3].attr("transform", "translate("+(canvas_width-60)+",0)");
        }

        if (UTF8ToString($5) != "") {
          dy = UTF8ToString($5);
        }

        var label_x = axis_range[0]+(axis_range[1]-axis_range[0])/x_divisor;
        if (axis_range[0] > axis_range[1]) {
            label_x = axis_range[1]+(axis_range[0]-axis_range[1])/x_divisor;
        }

        js.objects[$3].append("text")
             .attr("id", UTF8ToString($2)+"_label")
             .attr("transform", "rotate("+text_orient+")")
             .attr("x", label_x)
             .attr("dy", dy).style("text-anchor", "middle")
             .text(UTF8ToString($4));
      }, this->id, selection.GetID(), dom_id.c_str(), group.GetID(), label.c_str(),
      label_offset.c_str(), orientation.c_str());
      return *this;
    }

    // selection needs to be const for this to compile, but it feels wrong since
    // technically the contents of the selection are changed
    template <typename T>
    Axis& ApplyAxis(const SelectionOrTransition<T> & selection) {
      MAIN_THREAD_EMP_ASM({
	    js.objects[$1].call(js.objects[$0]);
	  }, this->id, selection.GetID());
      return *this;
    }

    /// An axis must have a scale. By default, a scale of SCALE_TYPE will be constructed, but
    /// usually you want an axis to depict a specific scale. This method points this object's
    /// scale member variable at [scale].
    Axis& SetScale(SCALE_TYPE & scale) {
      this->scale = scale;

      MAIN_THREAD_EMP_ASM({
	    js.objects[$0].scale(js.objects[$1]);
	  }, this->id, scale.GetID());
      return *this;
    }

    /// Get a reference to this object's scale.
    SCALE_TYPE& GetScale(){
      return this->scale;
    }

    /// Adjust the location of the label text relative to the axis
    /// (helpful if numbers are overlapping it). Can be negative.
    /// Use "em" (e.g. "2em") to specify distance relative to font size.
    Axis& AdjustLabelOffset(std::string offset) {
      label_offset = offset;
      if (dom_id != "") { //We've already drawn stuff
        group.Select("#"+dom_id+"_label").SetAttr("dy", label_offset);
      }
      return *this;
    }

    /// Draw tries to make a good guess about where to place the axis, but sometimes you want to
    /// scoot it over. This method will move the axis to the x,y location specified.
    Axis& Move(int x, int y) {
      group.Move(x,y);
      return *this;
    }

    template <typename T, std::size_t SIZE>
    Axis& SetTickValues(emp::array<T, SIZE> values) {
      emp::pass_array_to_javascript(values);

      MAIN_THREAD_EMP_ASM({
  	    js.objects[$0].tickValues(emp_i.__incoming_array);
	  }, this->id);
      return *this;
    }

    Axis& SetTickSize(float size) {
      MAIN_THREAD_EMP_ASM({
	    js.objects[$0].tickSize($1);
      }, this->id, size);
      return *this;
    }

    Axis& SetTickSizeInner(float size) {
      MAIN_THREAD_EMP_ASM({
	    js.objects[$0].tickSizeInner($1);
	  }, this->id, size);
      return *this;
    }

    Axis& SetTickSizeOuter(float size) {
      MAIN_THREAD_EMP_ASM({
	    js.objects[$0].tickSizeOuter($1);
  	  }, this->id, size);
      return *this;
    }

    Axis& SetTickPadding(int padding) {
      MAIN_THREAD_EMP_ASM({
	    js.objects[$0].tickPadding($1);
	  }, this->id, padding);
      return *this;
    }

    /// Set the number of ticks along the axis
    Axis& SetTicks(int count){
      MAIN_THREAD_EMP_ASM({
	    js.objects[$0].ticks($1);
	  }, this->id, count);
      return *this;
    }

    /// Set the format for displaying numbers assoiated with ticks. [format] should be a format
    /// following
    /// [the rules for d3.format()](https://github.com/d3/d3-3.x-api-reference/blob/master/Formatting.md#d3_format)
    Axis& SetTickFormat(std::string format) {
      MAIN_THREAD_EMP_ASM({
        js.objects[$0].tickFormat(d3.format(UTF8ToString($1)));
      }, this->id, format.c_str());
      return *this;
    }

    /// Adjust scale and axis to accomodate the new range of data specified by [new_min],
    /// and [new_max]. [svg] is a Selection or Transition containing the current axis. If it's a
    /// transition, then the rescaling will be animated.
    template <typename T>
    Axis& Rescale(double new_min, double new_max, const D3::SelectionOrTransition<T> & svg){
      this->scale.SetDomain(emp::array<double, 2>{{new_min, new_max}});
      ApplyAxis(svg.Select("#"+dom_id));
      return *this;
    }

    //TODO:  ticks

  };

  /// Helper function to draw a standard set of x and y axes
  /// Takes the desired x axis, y axis, and the selection on which to draw them
  template <typename SCALE_X_TYPE = D3::LinearScale, typename SCALE_Y_TYPE = D3::LinearScale>
  void DrawAxes(Axis<SCALE_X_TYPE> & x_axis, Axis<SCALE_Y_TYPE> & y_axis, Selection & selection){
    x_axis.Draw(selection);
    y_axis.Draw(selection);

    MAIN_THREAD_EMP_ASM({
      x_range = js.objects[$0].scale().range();
      y_range = js.objects[$1].scale().range();

      js.objects[$2].attr("transform", "translate(0,"+d3.max(y_range)+")");
      js.objects[$3].attr("transform", "translate("+x_range[0]+",0)");
    }, x_axis.GetID(), y_axis.GetID(), x_axis.group.GetID(), y_axis.group.GetID());
  }

}

#endif
