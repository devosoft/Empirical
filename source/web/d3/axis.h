/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2020
 *
 *  @file  axis.h
 *  @brief Handle drawing of axes on D3 graphs.
 */

#ifndef EMP_D3_AXIS_H
#define EMP_D3_AXIS_H

#include <regex>
#include "../js_utils.h"
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
    std::string orientation;

    double margin_top = 0.0;
    double margin_left = 0.0;

  public:

    /// There are a lot of graphical elements associated with an axis, so it's best to group them all 
    /// together into an HTML group element. This selection holds a pointer to the group for this axis.
    Selection group;

    /// Consruct an axis - this doesn't draw anything yet, but sets up the necessary infrastructure
    /// to draw it when you call the Draw method. Optionally takes a label to label the axis with.
    /// This label will also be used to create an id for the axis, to make it easier to select it
    /// later. The id will be the same as [label], but with all whitespace removed and "_axis"
    /// appended to the end.
    ///
    /// For example, if your label was "Per capita mortality", you could select the axis with:
    /// `D3::Select("#Percapitamortality_axis");`.
    ///
    /// By default, this constructor will create an axisBottom with no label,
    /// and will set the axis's id to "axis_[d3_index]." 
    ///
    /// It will also set a margin of 60px between the axis and the side of the svg that it corresponds with. 
    /// For example, an axisLeft will have a 60px margin between the axis line and the left side of its svg.
    /// To set top and left margins yourself, use the other constructor below that takes margin arguments.
    Axis(const std::string & type = "bottom", const std::string & label = "") {
      // The scale got added to the list of objects before this one
      this->label = label;
      this->orientation = type;

      // if invalid type, sets to axisBottom and prints warning in debug mode
      emp_assert(std::regex_match(type, std::regex("left|right|bottom|top")) 
        && "WARNING: Invalid type given to axis constructor");

      EM_ASM({
        const id = $0;
        const scale = emp_d3.objects[$1];
        const type = UTF8ToString($2);
        emp_d3.objects[id] = (type == "left") ? (d3.axisLeft(scale))
                           : (type == "right") ? (d3.axisRight(scale))
                           : (type == "top") ? (d3.axisTop(scale))
                           : (d3.axisBottom(scale)); 
      }, this->id, scale.GetID(), type.c_str());
    }

    /// Consruct an axis and specify its top and left margins in px.
    /// The order of margin arguments is: top, right, bottom, left.
    ///
    /// This doesn't draw anything yet, but sets up the necessary infrastructure
    /// to draw it when you call the Draw method. Optionally takes a label to label the axis with.
    /// This label will also be used to create an id for the axis, to make it easier to select it
    /// later. The id will be the same as [label], but with all whitespace removed and "_axis"
    /// appended to the end.
    ///
    /// For example, if your label was "Per capita mortality", you could select the axis with:
    /// `D3::Select("#Percapitamortality_axis");`.
    ///
    /// By default, this constructor will create an axisBottom with no label,
    /// and will set the axis's id to "axis_[d3_index]."
    Axis(const double & margin_top, const double & margin_left,
         const std::string & type = "bottom", const std::string & label = "") {
      // The scale got added to the list of objects before this one
      this->label = label;
      this->orientation = type;
      this->margin_top = margin_top;
      this->margin_left = margin_left;

      // if invalid type, sets to axisBottom and prints warning in debug mode
      emp_assert(std::regex_match(type, std::regex("left|right|bottom|top")) 
        && "WARNING: Invalid type given to axis constructor");

      EM_ASM({
        const id = $0;
        const scale = emp_d3.objects[$1];
        const type = UTF8ToString($2);
        emp_d3.objects[id] = (type == "left") ? (d3.axisLeft(scale))
                           : (type == "right") ? (d3.axisRight(scale))
                           : (type == "top") ? (d3.axisTop(scale))
                           : (d3.axisBottom(scale)); 
      }, this->id, scale.GetID(), type.c_str());
    }

    /// Draw axis on [selection] with intelligent default positioning or
    /// positioned with specified margins (selection must contain a single SVG element).
    /// Returns a reference to this object.
    Axis& Draw(Selection & selection){

      // if no label given, dom_id is set to <id>_axis
      std::string nospace_label = label;
      emp::remove_whitespace(nospace_label); // DOM ids can't contain whitespace
      dom_id = (label != "") ? nospace_label + "_axis" 
             //: emp::to_string(scale.GetID()) + "_axis";
             : "axis_" + emp::to_string(this->id);

      EM_ASM({
        const id = $0;
        const sel = $1; 
        const dom_id = UTF8ToString($2);
        const g = $3; 
        const label_str = UTF8ToString($4);
        const label_offset = UTF8ToString($5); 
        const orient = UTF8ToString($6); 
        const margin_top = $7;
        const margin_left = $10;
        
        const margin_default = 60;

        var axis_range = emp_d3.objects[id].scale().range();
        emp_d3.objects[g] = emp_d3.objects[sel].append("g");
        emp_d3.objects[g].append("g")
                    .attr("id", dom_id)
                    .call(emp_d3.objects[id]);

        var canvas_width = emp_d3.objects[sel].attr("width");
        var canvas_height = emp_d3.objects[sel].attr("height");
        
        var dy = "0em";
        var x_divisor = 2;
        var text_orient = 0;
        if (orient == "top") {
          dy = "-2.5em";
          emp_d3.objects[g].attr("transform", "translate(0,"+margin_default+")");   
        } else if (orient == "left") {
          x_divisor = -2;
          dy = "-2.5em";
          text_orient = -90;
          emp_d3.objects[g].attr("transform", "translate("+margin_default+",0)");
        } else if (orient == "right") {
          dy = "-2.5em";
          text_orient = 90;
          emp_d3.objects[g].attr("transform", "translate("+(canvas_width - margin_default)+",0)");
        } else {
          dy = "2.5em";
          emp_d3.objects[g].attr("transform", "translate(0,"+(canvas_height - margin_default)+")");
        }

        if (margin_top != "") {
          emp_d3.objects[g].attr("transform", "translate("+margin_left+","+margin_top+")");
        }

        if (label_offset != "") {
          dy = label_offset;
        }

        var label_x = (axis_range[0] < axis_range[1]) 
                    ? axis_range[0] + (axis_range[1]-axis_range[0])/x_divisor 
                    : axis_range[1] + (axis_range[0]-axis_range[1])/x_divisor;

        emp_d3.objects[g].append("text")
                     .attr("id", dom_id+"_label")
                     .attr("x", 0).attr("y", 0)
                     .attr("transform", "rotate("+text_orient+")")
                     .attr("x", label_x)
                     .attr("dy", dy)
                     .style("text-anchor", "middle")
                     .text(label_str);

      }, this->id, selection.GetID(), dom_id.c_str(), group.GetID(), 
         label.c_str(), label_offset.c_str(), orientation.c_str(), 
         margin_top, margin_left);

      return *this; 
    }

    template <typename T>
    Axis& ApplyAxis(internal::SelectionOrTransition<T> & selection) {
      EM_ASM({
	      emp_d3.objects[$1].call(emp_d3.objects[$0]);
	    }, this->id, selection.GetID());
      return *this;
    }

    /// An axis must have a scale. By default, a scale of SCALE_TYPE will be constructed, but
    /// usually you want an axis to depict a specific scale. This method points this object's
    /// scale member variable at [scale].
    Axis& SetScale(SCALE_TYPE & scale) {
      this->scale = scale;
      EM_ASM({
        emp_d3.objects[$0].scale(emp_d3.objects[$1]);
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
    Axis& AdjustLabelOffset(const std::string & offset) {
      label_offset = offset;
      if (dom_id != "") {   // we've already drawn stuff
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
    Axis& SetTickValues(const emp::array<T, SIZE> & values) {
      emp::pass_array_to_javascript(values);
      EM_ASM({
  	    emp_d3.objects[$0].tickValues(emp_i.__incoming_array);
	    }, this->id);
      return *this;
    }

    Axis& SetTickSize(float size) {
      EM_ASM({
	      emp_d3.objects[$0].tickSize($1);
      }, this->id, size);
      return *this;
    }

    Axis& SetTickSizeInner(float size) {
      EM_ASM({
	      emp_d3.objects[$0].tickSizeInner($1);
	    }, this->id, size);
      return *this;
    }

    Axis& SetTickSizeOuter(float size) {
      EM_ASM({
	      emp_d3.objects[$0].tickSizeOuter($1);
  	  }, this->id, size);
      return *this;
    }

    Axis& SetTickPadding(int padding) {
      EM_ASM({
	      emp_d3.objects[$0].tickPadding($1);
	    }, this->id, padding);
      return *this;
    }

    //TODO add  format param option

    /// Set the number of ticks along the axis
    Axis& SetTicks(int count){
      EM_ASM({
	      emp_d3.objects[$0].ticks($1);
	    }, this->id, count);
      return *this;
    }

    /// Set the format for displaying numbers assoiated with ticks. [format] should be a format following
    /// [the rules for d3.format()](https://github.com/d3/d3-3.x-api-reference/blob/master/Formatting.md#d3_format)
    Axis& SetTickFormat(const std::string & format) {
      EM_ASM({
        emp_d3.objects[$0].tickFormat(d3.format(UTF8ToString($1)));
      }, this->id, format.c_str());
      return *this;
    }

    /// Adjust scale and axis to accomodate the new range of data specified by [new_min],
    /// and [new_max]. [svg] is a Selection or Transition containing the current axis. 
    /// If it's a transition, then the rescaling will be animated.
    template <typename T>
    Axis& Rescale(double new_min, double new_max, const internal::SelectionOrTransition<T> & svg){
      this->scale.SetDomain(new_min, new_max);
      D3::Selection s = svg.Select("#"+dom_id);
      ApplyAxis(s);
      return *this;
    } 

  }; 

  /// Helper function to draw a standard set of x and y axes
  /// Takes the desired x axis, y axis, and the selection on which to draw them
  template <typename SCALE_X_TYPE = D3::LinearScale, typename SCALE_Y_TYPE = D3::LinearScale>
  void DrawAxes(Axis<SCALE_X_TYPE> & x_axis, Axis<SCALE_Y_TYPE> & y_axis, Selection & selection){
    x_axis.Draw(selection);
    y_axis.Draw(selection);

    EM_ASM({
      const x_axis_id = $0;
      const y_axis_id = $1;
      const x_axis_g = $2;
      const y_axis_g = $3;

      x_range = emp_d3.objects[x_axis_id].scale().range();
      y_range = emp_d3.objects[y_axis_id].scale().range();

      emp_d3.objects[x_axis_g].attr("transform", "translate(0,"+d3.max(y_range)+")");
      emp_d3.objects[y_axis_g].attr("transform", "translate("+x_range[0]+",0)");

    }, x_axis.GetID(), y_axis.GetID(), x_axis.group.GetID(), y_axis.group.GetID());
  }

} 

#endif