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
    double shift_x;
    double shift_y;

  public:

    /// There are a lot of graphical elements associated with an axis, so it's best to group them all
    /// together into an HTML group element. This selection holds a pointer to the group for this axis.
    Selection group;

    /// Padding values set the distance between an axis line and its corresponding side of the svg.
    /// They're also used by DrawAxes().
    bool has_padding = false;
    double padding;

    /// Consruct an axis - this doesn't draw anything yet, but sets up the necessary infrastructure
    /// to draw it when you call the Draw method. Optionally takes a label to label the axis with.
    /// This label will also be used to create an id for the axis, to make it easier to select it
    /// later. The id will be the same as [label], but with all whitespace removed and "_axis"
    /// appended to the end.
    ///
    /// For example, if your label was "Per capita mortality", you could select the axis with:
    /// `D3::Select("#Percapitamortality_axis");`.
    ///
    /// The padding argument shifts the axis a given distance (in px) away from the side of the
    /// svg that it corresponds with. For example, setting the padding to 80 means than an
    /// axisLeft would have an 80px gap between the axis line and the left side of its svg;
    /// an axisBottom would have an 80px gap between the axis line and the bottom of its svg.
    /// To set the exact initial position of the axis yourself, use the constructor that takes
    /// shift_x and shift_y arguments.
    ///
    /// By default, this constructor will create an axisBottom with no label, and will set the
    /// axis's id to "axis_[d3_index]." It will also set the padding to 60px, meaning that it
    /// will shift the axis 60px away from the side of the svg that it corresponds with.
    Axis(const std::string & type = "bottom", const std::string & label = "", double padding = 60) {
      // The scale got added to the list of objects before this one
      this->label = label;
      this->orientation = type;
      this->has_padding = true;
      this->padding = padding;

      // if invalid type, sets to axisBottom and prints warning in debug mode
      emp_assert(std::regex_match(type, std::regex("left|right|bottom|top"))
        && "WARNING: Invalid type given to axis constructor");

      MAIN_THREAD_EM_ASM({
        const id = $0;
        const scale = emp_d3.objects[$1];
        const type = UTF8ToString($2);
        emp_d3.objects[id] = (type == "left") ? (d3.axisLeft(scale))
                           : (type == "right") ? (d3.axisRight(scale))
                           : (type == "top") ? (d3.axisTop(scale))
                           : (d3.axisBottom(scale));
      }, this->id, scale.GetID(), type.c_str());
    }

    /// Construct an axis and specify its initial position in px with shift_x and shift_y.
    /// For example, given a shift_x of 50 and shift_y of 100, the axis will be shifted
    /// 50px to the right and 100px down from its origin. It's very helpful to use this
    /// constructor to position your axes if you're using a scale range minimum that isn't zero.
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
    Axis(double shift_x, double shift_y,
         const std::string & type = "bottom", const std::string & label = "") {
      // The scale got added to the list of objects before this one
      this->label = label;
      this->orientation = type;
      this->shift_x = shift_x;
      this->shift_y = shift_y;

      // if invalid type, sets to axisBottom and prints warning in debug mode
      emp_assert(std::regex_match(type, std::regex("left|right|bottom|top"))
        && "WARNING: Invalid type given to axis constructor");

      MAIN_THREAD_EM_ASM({
        const id = $0;
        const scale = emp_d3.objects[$1];
        const type = UTF8ToString($2);
        emp_d3.objects[id] = (type == "left") ? (d3.axisLeft(scale))
                           : (type == "right") ? (d3.axisRight(scale))
                           : (type == "top") ? (d3.axisTop(scale))
                           : (d3.axisBottom(scale));
      }, this->id, scale.GetID(), type.c_str());
    }

    /// Draw axis on [selection] with intelligent default positioning or positioned with
    /// specified shift_x and shift_y (selection must contain a single SVG element).
    /// Returns a reference to this object.
    Axis& Draw(Selection & selection){

      // if no label given, dom_id is set to <id>_axis
      std::string nospace_label = label;
      emp::remove_whitespace(nospace_label); // DOM ids can't contain whitespace
      dom_id = (label != "") ? nospace_label + "_axis"
             : "axis_" + emp::to_string(this->id);

      MAIN_THREAD_EM_ASM({
        const id = $0;
        const sel = $1;
        const dom_id = UTF8ToString($2);
        const g = $3;
        const label_str = UTF8ToString($4);
        const label_offset = UTF8ToString($5);
        const orient = UTF8ToString($6);
        const has_padding = $7;
        const padding = $8;
        const shift_x = $9;
        const shift_y = $10;

        emp_d3.objects[g] = emp_d3.objects[sel].append("g");
        emp_d3.objects[g].append("g")
                         .attr("id", dom_id)
                         .call(emp_d3.objects[id]);

        var svg_width = emp_d3.objects[sel].attr("width");
        var svg_height = emp_d3.objects[sel].attr("height");

        var axis_range_low = d3.min(emp_d3.objects[id].scale().range());
        var axis_range_high = d3.max(emp_d3.objects[id].scale().range());

        var dy = "0em";
        var text_orient = 0;
        var padding_translation = "";
        if (orient == "top") {
          dy = "-2.5em";
          padding_translation = "translate(0,"+padding+")";
        } else if (orient == "left") {
          axis_range_low *= -1;  // since left axis label is rotated -90,
          axis_range_high *= -1; // range values must be given opposite sign
          dy = "-2.5em";
          text_orient = -90;
          padding_translation = "translate("+padding+",0)";
        } else if (orient == "right") {
          dy = "-2.5em";
          text_orient = 90;
          padding_translation = "translate("+(svg_width - padding)+",0)";
        } else {
          dy = "2.5em";
          padding_translation = "translate(0,"+(svg_height - padding)+")";
        }

        if (has_padding) {
          emp_d3.objects[g].attr("transform", padding_translation);
        } else {
          emp_d3.objects[g].attr("transform", "translate("+shift_x+","+shift_y+")");
        }

        if (label_offset != "") {
          dy = label_offset;
        }

        var label_x = axis_range_low + (axis_range_high - axis_range_low) / 2;

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
         has_padding, padding, shift_x, shift_y);

      return *this;
    }

    template <typename T>
    Axis& ApplyAxis(internal::SelectionOrTransition<T> & selection) {
      MAIN_THREAD_EM_ASM({
	      emp_d3.objects[$1].call(emp_d3.objects[$0]);
	    }, this->id, selection.GetID());
      return *this;
    }

    /// An axis must have a scale. By default, a scale of SCALE_TYPE will be constructed, but
    /// usually you want an axis to depict a specific scale. This method points this object's
    /// scale member variable at [scale].
    Axis& SetScale(SCALE_TYPE & scale) {
      this->scale = scale;
      MAIN_THREAD_EM_ASM({
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
      // prints warning in debug mode if this method is called before Draw() since it will overwrite changes
      emp_assert((dom_id != "") && "WARNING: Calling Draw() after this method will overwrite your changes");

      group.Move(x,y);
      return *this;
    }

    template <typename T, std::size_t SIZE>
    Axis& SetTickValues(const emp::array<T, SIZE> & values) {
      // prints warning in debug mode if this method is called before Draw() since it will overwrite changes
      emp_assert((dom_id != "") && "WARNING: Calling Draw() after this method will overwrite your changes");

      emp::pass_array_to_javascript(values);
      MAIN_THREAD_EM_ASM({
  	    emp_d3.objects[$0].tickValues(emp_i.__incoming_array);
	    }, this->id);
      return *this;
    }

    Axis& SetTickSize(float size) {
      // prints warning in debug mode if this method is called before Draw() since it will overwrite changes
      emp_assert((dom_id != "") && "WARNING: Calling Draw() after this method will overwrite your changes");

      MAIN_THREAD_EM_ASM({
	      emp_d3.objects[$0].tickSize($1);
      }, this->id, size);
      return *this;
    }

    Axis& SetTickSizeInner(float size) {
      // prints warning in debug mode if this method is called before Draw() since it will overwrite changes
      emp_assert((dom_id != "") && "WARNING: Calling Draw() after this method will overwrite your changes");

      MAIN_THREAD_EM_ASM({
	      emp_d3.objects[$0].tickSizeInner($1);
	    }, this->id, size);
      return *this;
    }

    Axis& SetTickSizeOuter(float size) {
      // prints warning in debug mode if this method is called before Draw() since it will overwrite changes
      emp_assert((dom_id != "") && "WARNING: Calling Draw() after this method will overwrite your changes");

      MAIN_THREAD_EM_ASM({
	      emp_d3.objects[$0].tickSizeOuter($1);
  	  }, this->id, size);
      return *this;
    }

    Axis& SetTickPadding(int padding) {
      // prints warning in debug mode if this method is called before Draw() since it will overwrite changes
      emp_assert((dom_id != "") && "WARNING: Calling Draw() after this method will overwrite your changes");

      MAIN_THREAD_EM_ASM({
	      emp_d3.objects[$0].tickPadding($1);
	    }, this->id, padding);
      return *this;
    }

    /// Set the number of ticks along the axis
    Axis& SetTicks(int count){
      // prints warning in debug mode if this method is called before Draw() since it will overwrite changes
      emp_assert((dom_id != "") && "WARNING: Calling Draw() after this method will overwrite your changes");

      MAIN_THREAD_EM_ASM({
	      emp_d3.objects[$0].ticks($1);
	    }, this->id, count);
      return *this;
    }
    /// Version of ticks() to set the number of ticks along the axis as well as the format
    /// for displaying numbers assoiated with ticks. [format] should be a format following
    /// [the rules for d3.format()](https://github.com/d3/d3-3.x-api-reference/blob/master/Formatting.md#d3_format)
    Axis& SetTicks(int count, const std::string & format){
      // prints warning in debug mode if this method is called before Draw() since it will overwrite changes
      emp_assert((dom_id != "") && "WARNING: Calling Draw() after this method will overwrite your changes");

      MAIN_THREAD_EM_ASM({
	      emp_d3.objects[$0].ticks($1, d3.format(UTF8ToString($2)));
	    }, this->id, count, format.c_str());
      return *this;
    }

    /// Set the format for displaying numbers assoiated with ticks. [format] should be a format following
    /// [the rules for d3.format()](https://github.com/d3/d3-3.x-api-reference/blob/master/Formatting.md#d3_format)
    Axis& SetTickFormat(const std::string & format) {
      // prints warning in debug mode if this method is called before Draw() since it will overwrite changes
      emp_assert((dom_id != "") && "WARNING: Calling Draw() after this method will overwrite your changes");

      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].tickFormat(d3.format(UTF8ToString($1)));
      }, this->id, format.c_str());
      return *this;
    }

    /// Adjust scale and axis to accomodate the new range of data specified by [new_min],
    /// and [new_max]. [svg] is a Selection or Transition containing the current axis.
    /// If it's a transition, then the rescaling will be animated.
    /// Prints a warning in debug mode if this method is called before Draw()
    /// since Draw() creates the dom_id required to select the axis.
    /// There are three versions since SetDomain() can take three types of arguments.
    template <typename A, size_t SIZE, typename T>
    Axis& Rescale(const emp::array<A, SIZE> & new_min_and_max, const internal::SelectionOrTransition<T> & svg){
      emp_assert((dom_id != "") && "WARNING: Rescale() cannot be called before Draw()");
      this->scale.SetDomain(new_min_and_max);
      D3::Selection s = svg.Select("#"+dom_id);
      ApplyAxis(s);
      return *this;
    }
    template <typename T>
    Axis& Rescale(double new_min, double new_max, const internal::SelectionOrTransition<T> & svg){
      emp_assert((dom_id != "") && "WARNING: Rescale() cannot be called before Draw()");
      this->scale.SetDomain(new_min, new_max);
      D3::Selection s = svg.Select("#"+dom_id);
      ApplyAxis(s);
      return *this;
    }
    template <typename T>
    Axis& Rescale(const std::string & lower, const std::string & upper, const internal::SelectionOrTransition<T> & svg){
      emp_assert((dom_id != "") && "WARNING: Rescale() cannot be called before Draw()");
      this->scale.SetDomain(lower, upper);
      D3::Selection s = svg.Select("#"+dom_id);
      ApplyAxis(s);
      return *this;
    }

  };

  /// Helper function to draw a standard set of x and y axes at bottom and left.
  /// Takes the desired x axis, y axis, and the selection on which to draw them.
  ///
  /// Only takes padding into account (not shift). The padding on the bottom axis
  /// will be their distance from the bottom of the svg, and the padding on the left axis
  /// will be their distance from the svg's left edge. The axes will meet at their origins.
  /// By default, they will have 60px padding.
  ///
  /// The axes' scale ranges' lower limits must be zero for the axes' origins to match up.
  /// TODO: make this function work with non-zero scale range minimums
  template <typename SCALE_X_TYPE = D3::LinearScale, typename SCALE_Y_TYPE = D3::LinearScale>
  void DrawAxes(Axis<SCALE_X_TYPE> & x_axis, Axis<SCALE_Y_TYPE> & y_axis, Selection & selection){

    double x_axis_padding;
    double y_axis_padding;

    x_axis.Draw(selection);
    y_axis.Draw(selection);

    if (x_axis.has_padding) {
      x_axis_padding = x_axis.padding;
    } else {
      x_axis_padding = 60;
    }
    if (y_axis.has_padding) {
      y_axis_padding = y_axis.padding;
    } else {
      y_axis_padding = 60;
    }

    MAIN_THREAD_EM_ASM({
      const y_axis = $0;
      const x_axis_g = $1;
      const y_axis_g = $2;
      const svg = $3;
      const x_axis_padding = $4;
      const y_axis_padding = $5;

      var svg_width = emp_d3.objects[svg].attr("width");
      var svg_height = emp_d3.objects[svg].attr("height");

      var y_axis_range_low = d3.min(emp_d3.objects[y_axis].scale().range());
      var y_axis_range_high = d3.max(emp_d3.objects[y_axis].scale().range());

      var y_axis_height = y_axis_range_high - y_axis_range_low;

      emp_d3.objects[x_axis_g].attr("transform", "translate("+y_axis_padding+","+(svg_height - x_axis_padding)+")");
      emp_d3.objects[y_axis_g].attr("transform", "translate("+y_axis_padding+","+(svg_height - y_axis_height - x_axis_padding)+")");

    }, y_axis.GetID(), x_axis.group.GetID(), y_axis.group.GetID(), selection.GetID(), x_axis_padding, y_axis_padding);
  }

}

#endif