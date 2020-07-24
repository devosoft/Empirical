#include "web/Document.h"
#include "web/_MochaTestRunner.h"
#include "d3_testing_utils.h"

#include "web/d3/d3_init.h"
#include "web/d3/selection.h"
#include "web/d3/scales.h"
#include "web/d3/axis.h"

#include <iostream>

// This file tests:
// - D3::Axis
//   - default (construct axis with no arguments)
//     - set id to 'axis_<cpp_id>' since no label provided
//     - position the axis correctly (horizontal and 60px from bottom of svg)
//   - oriented (set bottomAxis, topAxis, leftAxis, or rightAxis)
//     - set id to '<cpp_label>_axis' since label provided
//     - position the axis correctly (horizontal/vertical, ticks up/down, and 60px from edge of svg)
//     - has label with id '<cpp_label>_axis_label' since label provided
//     - position the label correctly (centered above/below/beside the axis)
//     - rotate the label if the axis is vertical
//   - padded (pass in relative padding argument)
//     - set id to 'axis_<cpp_id>' or create label '<cpp_label>_axis_label' from id
//     - position the axis correctly (has correct padding from corresponding edge of svg)
//   - shifted (construct with shift_x and shift_y)
//     - set id to 'axis_<cpp_id>' or create label '<cpp_label>_axis_label' from id
//     - position the axis correctly (translate by the given x and y shifts)
//   - edited (change ticks, label, and scale)
//     - move axis to x, y
//     - rescale axis by new domain
//     - adjust label offset correctly
//     - set tick size, padding, number, format, and new values

struct Test_Axis : emp::web::BaseTest {

  // SVGs
  D3::Selection svg_default_axis;
  D3::Selection svg_oriented_axes;
  D3::Selection svg_padded_axes;
  D3::Selection svg_shifted_axes;
  D3::Selection svg_edited_axis;

  // Scale
  D3::LinearScale scale;

  // Axes
  D3::Axis<D3::LinearScale> default_axis;

  D3::Axis<D3::LinearScale> bottom_axis;
  D3::Axis<D3::LinearScale> top_axis;
  D3::Axis<D3::LinearScale> left_axis;
  D3::Axis<D3::LinearScale> right_axis;

  D3::Axis<D3::LinearScale> padded_bottom_axis;
  D3::Axis<D3::LinearScale> padded_top_axis;
  D3::Axis<D3::LinearScale> padded_left_axis;
  D3::Axis<D3::LinearScale> padded_right_axis;

  D3::Axis<D3::LinearScale> shifted_axis;
  D3::Axis<D3::LinearScale> shifted_labeled_axis;

  D3::Axis<D3::LinearScale> edited_axis;

  Test_Axis() : emp::web::BaseTest({"emp_test_container"}) {

    // create divs to organize the different axes we're testing and position them in separate svgs
    // FIXME: selecting #emp_test_container doesn't work since it's a Widget object and gets redrawn
    D3::Select("body").Append("div").SetAttr("id", "d3_testing_div");
    D3::Select("#d3_testing_div").Append("div").SetAttr("id", "default_axis_div");
    D3::Select("#d3_testing_div").Append("div").SetAttr("id", "oriented_axes_div");
    D3::Select("#d3_testing_div").Append("div").SetAttr("id", "padded_axes_div");
    D3::Select("#d3_testing_div").Append("div").SetAttr("id", "shifted_axes_div");
    D3::Select("#d3_testing_div").Append("div").SetAttr("id", "edited_axis_div");

    // set the svg for default axis testing to 600x100px
    svg_default_axis = D3::Select("#default_axis_div").Append("svg").SetAttr("id", "default_axis_svg").SetAttr("width", 600).SetAttr("height", 100);
    // set the svg for oriented axes testing to 600x600px (taller to fit vertical axes)
    svg_oriented_axes = D3::Select("#oriented_axes_div").Append("svg").SetAttr("id", "oriented_axes_svg").SetAttr("width", 600).SetAttr("height", 600);
    // set the svg for padded axes testing to 600x600px (taller to fit vertical axes)
    svg_padded_axes = D3::Select("#padded_axes_div").Append("svg").SetAttr("id", "padded_axes_svg").SetAttr("width", 600).SetAttr("height", 600);
    // set the svg for shifted axes testing to 600x200px (taller to allow for vertical shift)
    svg_shifted_axes = D3::Select("#shifted_axes_div").Append("svg").SetAttr("id", "shifted_axes_svg").SetAttr("width", 600).SetAttr("height", 200);
    // set the svg for edited axis testing to 600x100px
    svg_edited_axis = D3::Select("#edited_axis_div").Append("svg").SetAttr("id", "edited_axis_svg").SetAttr("width", 600).SetAttr("height", 100);

    // set up a simple scale that all of the axes will be constructed on
    scale = D3::LinearScale();
    scale.SetDomain(0, 100).SetRange(0, 500);

    // set up axis for testing the default axis constructor
    default_axis = D3::Axis<D3::LinearScale>().SetScale(scale).Draw(svg_default_axis);
    // set up oriented axes for tests specific to location
    bottom_axis = D3::Axis<D3::LinearScale>("bottom", "Bottom Axis").SetScale(scale).Draw(svg_oriented_axes);
    top_axis = D3::Axis<D3::LinearScale>("top", "Top Axis").SetScale(scale).Draw(svg_oriented_axes);
    left_axis = D3::Axis<D3::LinearScale>("left", "Left Axis").SetScale(scale).Draw(svg_oriented_axes);
    right_axis = D3::Axis<D3::LinearScale>("right", "Right Axis").SetScale(scale).Draw(svg_oriented_axes);
    // set up padded axes for tests specific to passed-in relative padding argument
    padded_bottom_axis = D3::Axis<D3::LinearScale>("bottom", "", 0).SetScale(scale).Draw(svg_padded_axes);
    padded_top_axis = D3::Axis<D3::LinearScale>("top", "Padded Top", 80).SetScale(scale).Draw(svg_padded_axes);
    padded_left_axis = D3::Axis<D3::LinearScale>("left", "Padded Left", 70).SetScale(scale).Draw(svg_padded_axes);
    padded_right_axis = D3::Axis<D3::LinearScale>("right", "Padded Right", -10).SetScale(scale).Draw(svg_padded_axes);
    // set up shifted axes for tests specific to constructor that sets position with shift_x and shift_y
    shifted_axis = D3::Axis<D3::LinearScale>(0, 75).SetScale(scale).Draw(svg_shifted_axes);
    shifted_labeled_axis = D3::Axis<D3::LinearScale>(30, 55, "top", "Labeled Shifted").SetScale(scale).Draw(svg_shifted_axes);
    // set up axis to test other functions that can be called to edit a default axis
    edited_axis = D3::Axis<D3::LinearScale>("bottom", "Edited Axis").SetScale(scale).Draw(svg_edited_axis);

    // call various modifying functions on edited_axis to test them
    edited_axis.AdjustLabelOffset("4em");
    edited_axis.SetTicks(5).SetTickSize(10.5).SetTickSizeInner(10.5).SetTickSizeOuter(0);
    edited_axis.SetTickPadding(10).SetTickFormat(",.2r");
    emp::array<int, 6> new_tick_values({1122,2075,3086,4454,6894,9223});
    edited_axis.SetTickValues(new_tick_values);
    edited_axis.Move(60,0);
    edited_axis.Rescale(1000, 10000, svg_edited_axis);

  }

  void Describe() override {

    // Test the default axis constructor
    EM_ASM({

      describe("Default Axis", function() {

        var d_axis_container = d3.select("#default_axis_svg>g");
        var d_axis = d3.select("#default_axis_svg>g>g");

        it("should set id to 'axis_<cpp_id>' since no label provided", function() {
          var supposed_id = "axis_" + $0;
          chai.assert.equal(d_axis.attr("id"), supposed_id);
        });
        it("should position the axis correctly (horizontal and 60px from bottom of svg)", function() {
          chai.assert.equal(d_axis.select("path").attr("d"), "M0.5,6V0.5H500.5V6");
          chai.assert.equal(d_axis_container.attr("transform"), "translate(0,40)");
        });
      });

    }, default_axis.GetID());

    // Test oriented axes: axisBottom, axisTop, axisLeft, and axisRight
    EM_ASM({

      describe("axisBottom", function() {

        var b_axis_container = d3.select("#oriented_axes_svg>g:nth-child(1)");
        var b_axis = d3.select("#oriented_axes_svg>g:nth-child(1)>g");
        var b_axis_label = d3.select("#oriented_axes_svg>g:nth-child(1)>text");

        it("should set id to '<cpp_label>_axis' since label provided", function() {
          chai.assert.equal(b_axis.attr("id"), "BottomAxis_axis");
        });
        it("should position the axis correctly (horizontal, ticks down, and 60px from bottom of svg)", function() {
          chai.assert.equal(b_axis_container.attr("transform"), "translate(0,540)");
          chai.assert.equal(b_axis.select("path").attr("d"), "M0.5,6V0.5H500.5V6");
        });
        it("should have a label with id '<cpp_label>_axis_label' since label provided", function() {
          chai.assert.equal(b_axis_label.attr("id"), "BottomAxis_axis_label");
        });
        it("should position the label correctly (centered below the axis)", function() {
          chai.assert.equal(b_axis_label.attr("x"), "250");
          chai.assert.equal(b_axis_label.attr("y"), "0");
          chai.assert.equal(b_axis_label.attr("dy"), "2.5em");
          chai.assert.equal(b_axis_label.attr("style"), "text-anchor: middle;");
        });
        it("should not rotate the label since the axis is horizontal", function() {
          chai.assert.equal(b_axis_label.attr("transform"), "rotate(0)");
        });

      });

      describe("axisTop", function() {

        var t_axis_container = d3.select("#oriented_axes_svg>g:nth-child(2)");
        var t_axis = d3.select("#oriented_axes_svg>g:nth-child(2)>g");
        var t_axis_label = d3.select("#oriented_axes_svg>g:nth-child(2)>text");

        it("should set id to '<cpp_label>_axis' since label provided", function() {
          chai.assert.equal(t_axis.attr("id"), "TopAxis_axis");
        });
        it("should position the axis correctly (horizontal, ticks up, and 60px from top of svg)", function() {
          chai.assert.equal(t_axis_container.attr("transform"), "translate(0,60)");
          chai.assert.equal(t_axis.select("path").attr("d"), "M0.5,-6V0.5H500.5V-6");
        });
        it("should have a label with id '<cpp_label>_axis_label' since label provided", function() {
          chai.assert.equal(t_axis_label.attr("id"), "TopAxis_axis_label");
        });
        it("should position the label correctly (centered above the axis)", function() {
          chai.assert.equal(t_axis_label.attr("x"), "250");
          chai.assert.equal(t_axis_label.attr("y"), "0");
          chai.assert.equal(t_axis_label.attr("dy"), "-2.5em");
          chai.assert.equal(t_axis_label.attr("style"), "text-anchor: middle;");
        });
        it("should not rotate the label since the axis is horizontal", function() {
          chai.assert.equal(t_axis_label.attr("transform"), "rotate(0)");
        });

      });

      describe("axisLeft", function() {

        var l_axis_container = d3.select("#oriented_axes_svg>g:nth-child(3)");
        var l_axis = d3.select("#oriented_axes_svg>g:nth-child(3)>g");
        var l_axis_label = d3.select("#oriented_axes_svg>g:nth-child(3)>text");

        it("should set id to '<cpp_label>_axis' since label provided", function() {
          chai.assert.equal(l_axis.attr("id"), "LeftAxis_axis");
        });
        it("should position the axis correctly (vertical, ticks left, and 60px from left of svg)", function() {
          chai.assert.equal(l_axis_container.attr("transform"), "translate(60,0)");
          chai.assert.equal(l_axis.select("path").attr("d"), "M-6,0.5H0.5V500.5H-6");
        });
        it("should have a label with id '<cpp_label>_axis_label' since label provided", function() {
          chai.assert.equal(l_axis_label.attr("id"), "LeftAxis_axis_label");
        });
        it("should position the label correctly (centered to the left of the axis)", function() {
          chai.assert.equal(l_axis_label.attr("x"), "-250");
          chai.assert.equal(l_axis_label.attr("y"), "0");
          chai.assert.equal(l_axis_label.attr("dy"), "-2.5em");
          chai.assert.equal(l_axis_label.attr("style"), "text-anchor: middle;");
        });
        it("should rotate the label counterclockwise since the axis is vertical and leftward", function() {
          chai.assert.equal(l_axis_label.attr("transform"), "rotate(-90)");
        });

      });

      describe("axisRight", function() {

        var r_axis_container = d3.select("#oriented_axes_svg>g:nth-child(4)");
        var r_axis = d3.select("#oriented_axes_svg>g:nth-child(4)>g");
        var r_axis_label = d3.select("#oriented_axes_svg>g:nth-child(4)>text");

        it("should set id to '<cpp_label>_axis' since label provided", function() {
          chai.assert.equal(r_axis.attr("id"), "RightAxis_axis");
        });
        it("should position the axis correctly (vertical, ticks right, and 60px from right of svg)", function() {
          chai.assert.equal(r_axis_container.attr("transform"), "translate(540,0)");
          chai.assert.equal(r_axis.select("path").attr("d"), "M6,0.5H0.5V500.5H6");
        });
        it("should have a label with id '<cpp_label>_axis_label' since label provided", function() {
          chai.assert.equal(r_axis_label.attr("id"), "RightAxis_axis_label");
        });
        it("should position the label correctly (centered to the right of the axis)", function() {
          chai.assert.equal(r_axis_label.attr("x"), "250");
          chai.assert.equal(r_axis_label.attr("y"), "0");
          chai.assert.equal(r_axis_label.attr("dy"), "-2.5em");
          chai.assert.equal(r_axis_label.attr("style"), "text-anchor: middle;");
        });
        it("should rotate the label counterclockwise since the axis is vertical and rightward", function() {
          chai.assert.equal(r_axis_label.attr("transform"), "rotate(90)");
        });

      });

    });

    // Test axes that were constructed with specific padding values
    EM_ASM({

      describe("Padded axisBottom", function() {

        var pad_b_axis_container = d3.select("#padded_axes_svg>g:nth-child(1)");
        var pad_b_axis = d3.select("#padded_axes_svg>g:nth-child(1)>g");

        it("should set id to 'axis_<cpp_id>' since no label provided", function() {
          var supposed_pad_id = "axis_" + $0;
          chai.assert.equal(pad_b_axis.attr("id"), supposed_pad_id);
        });
        it("should position the axis correctly (horizontal, ticks down, 0px padding from bottom)", function() {
          chai.assert.equal(pad_b_axis_container.attr("transform"), "translate(0,600)");
          chai.assert.equal(pad_b_axis.select("path").attr("d"), "M0.5,6V0.5H500.5V6");
        });

      });

      describe("Padded axisTop", function() {

        var pad_t_axis_container = d3.select("#padded_axes_svg>g:nth-child(2)");
        var pad_t_axis = d3.select("#padded_axes_svg>g:nth-child(2)>g");
        var pad_t_axis_label = d3.select("#padded_axes_svg>g:nth-child(2)>text");

        it("should have a label with id '<cpp_label>_axis_label' since label provided", function() {
          chai.assert.equal(pad_t_axis_label.attr("id"), "PaddedTop_axis_label");
        });
        it("should position the axis correctly (horizontal, ticks up, 80px padding from top)", function() {
          chai.assert.equal(pad_t_axis_container.attr("transform"), "translate(0,80)");
          chai.assert.equal(pad_t_axis.select("path").attr("d"), "M0.5,-6V0.5H500.5V-6");
        });

      });

      describe("Padded axisLeft", function() {

        var pad_l_axis_container = d3.select("#padded_axes_svg>g:nth-child(3)");
        var pad_l_axis = d3.select("#padded_axes_svg>g:nth-child(3)>g");
        var pad_l_axis_label = d3.select("#padded_axes_svg>g:nth-child(3)>text");

        it("should have a label with id '<cpp_label>_axis_label' since label provided", function() {
          chai.assert.equal(pad_l_axis_label.attr("id"), "PaddedLeft_axis_label");
        });
        it("should position the axis correctly (vertical, ticks left, 70px padding from left)", function() {
          chai.assert.equal(pad_l_axis_container.attr("transform"), "translate(70,0)");
          chai.assert.equal(pad_l_axis.select("path").attr("d"), "M-6,0.5H0.5V500.5H-6");
        });

      });

      describe("Padded axisRight", function() {

        var pad_r_axis_container = d3.select("#padded_axes_svg>g:nth-child(4)");
        var pad_r_axis = d3.select("#padded_axes_svg>g:nth-child(4)>g");
        var pad_r_axis_label = d3.select("#padded_axes_svg>g:nth-child(4)>text");

        it("should have a label with id '<cpp_label>_axis_label' since label provided", function() {
          chai.assert.equal(pad_r_axis_label.attr("id"), "PaddedRight_axis_label");
        });
        it("should position the axis correctly (vertical, ticks right, -10px padding from right (outside svg))", function() {
          chai.assert.equal(pad_r_axis_container.attr("transform"), "translate(610,0)");
          chai.assert.equal(pad_r_axis.select("path").attr("d"), "M6,0.5H0.5V500.5H6");
        });

      });

    }, padded_bottom_axis.GetID());

    // Test axes that were constructed with shift_x and shift_y to specify their initial positions
    EM_ASM({
      // position the axis correctly (translate by the given x and y shifts)
      describe("Shifted Axes", function() {

        var shift_axis_container = d3.select("#shifted_axes_svg>g:nth-child(1)");
        var shift_axis = d3.select("#shifted_axes_svg>g:nth-child(1)>g");

        var labeled_shift_axis_container = d3.select("#shifted_axes_svg>g:nth-child(2)");
        var labeled_shift_axis = d3.select("#shifted_axes_svg>g:nth-child(2)>g");
        var labeled_shift_axis_label = d3.select("#shifted_axes_svg>g:nth-child(2)>text");

        it("should set default id to 'axis_<cpp_id>' since no label provided", function() {
          var supposed_shift_id = "axis_" + $0;
          chai.assert.equal(shift_axis.attr("id"), supposed_shift_id);
        });
        it("should position the default axis correctly (horizontal, ticks down, and shifted 75px down)", function() {
          chai.assert.equal(shift_axis_container.attr("transform"), "translate(0,75)");
          chai.assert.equal(shift_axis.select("path").attr("d"), "M0.5,6V0.5H500.5V6");
        });

        it("should have a label for labeled axis with id '<cpp_label>_axis_label' since label provided", function() {
          chai.assert.equal(labeled_shift_axis_label.attr("id"), "LabeledShifted_axis_label");
        });
        it("should position the labeled axis correctly (horizontal, ticks up, and shifted 30px right and 55px down)", function() {
          chai.assert.equal(labeled_shift_axis_container.attr("transform"), "translate(30,55)");
          chai.assert.equal(labeled_shift_axis.select("path").attr("d"), "M0.5,-6V0.5H500.5V-6");
        });

      });

    }, shifted_axis.GetID());

    // Test an axis that has been modified with other functions
    EM_ASM({

      describe("Edited Axis", function() {

        var e_axis_container = d3.select("#edited_axis_svg>g");
        var e_axis = d3.select("#edited_axis_svg>g>g");
        var e_axis_label = d3.select("#edited_axis_svg>g>text");

        var e_axis_tick_container = d3.select("#edited_axis_svg>g>g>g");
        var e_axis_tick = d3.select("#edited_axis_svg>g>g>g>line");
        var e_axis_tick_label = d3.select("#edited_axis_svg>g>g>g>text");
        var e_axis_last_tick_container = d3.select("#edited_axis_svg>g>g>g:nth-last-child(1)");
        var e_axis_last_tick = d3.select("#edited_axis_svg>g>g>g:nth-last-child(1)>line");
        var e_axis_last_tick_label = d3.select("#edited_axis_svg>g>g>g:nth-last-child(1)>text");

        it("should move the axis to specified location (x, y)", function() {
          chai.assert.equal(e_axis_container.attr("transform"), "translate(60,0)");
        });
        it("should rescale the axis to have a domain of (1000,10000)", function() {
          chai.assert.equal(e_axis_tick_container.attr("transform"), "translate(7.277777777777778,0)");
          chai.assert.equal(e_axis_tick_label.text(), "1,100");
          chai.assert.equal(e_axis_last_tick_container.attr("transform"), "translate(457.3333333333333,0)");
          chai.assert.equal(e_axis_last_tick_label.text(), "9,200");
        });
        it("should adjust the label's offset from the axis to 4em", function() {
          chai.assert.equal(e_axis_label.attr("dy"), "4em");
        });
        it("should change the number of ticks to 6", function() {
          chai.assert.equal(e_axis.selectAll(".tick").size(), 6);
        });
        it("should change tick size to 10.5px and outer tick size to 0px", function() {
          chai.assert.equal(e_axis_tick.attr("y2"), "10.5");
          chai.assert.equal(e_axis.select("path").attr("d"), "M0.5,0.5H500.5");
        });
        it("should change tick padding to 10px", function() {
          chai.assert.equal(e_axis_tick_label.attr("y"), Number(e_axis_tick.attr("y2"))+10);
        });
        it("should change tick values (set first tick to '1122') and set formatting to ',.2r'", function() {
          chai.assert.equal(e_axis_tick_label.text(), "1,100");
        });

      });

    });

  }

};

emp::web::MochaTestRunner test_runner;

int main() {
  test_runner.Initialize({"emp_test_container"});
  D3::internal::get_emp_d3();

  test_runner.AddTest<Test_Axis>("Axis");

  test_runner.OnBeforeEachTest([]() { ResetD3Context(); });
  test_runner.Run();

  return 0;
}