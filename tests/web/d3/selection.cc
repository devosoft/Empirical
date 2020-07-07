#include "web/Document.h"
#include "web/d3/d3_init.h"
#include "web/d3/selection.h"
#include "web/_MochaTestRunner.h"
#include "d3_testing_utils.h"

// namespace UI = emp::web;
// UI::Document doc("emp_d3_test");

// This file tests:
// - D3_Selection
//   - empty selection
//   - non empty selection

struct Test_Selection : emp::web::BaseTest {
  D3::Selection empty_selection;
  D3::Selection svg_selection;
  D3::Selection circle_selection;

  Test_Selection() : emp::web::BaseTest({"emp_test_container"}) {
    std::cout << empty_selection.GetID() << std::endl;
    EM_ASM({
      $("#emp_test_container").append("<svg id='test_svg'><circle/><circle/></svg>");
    });
    svg_selection = D3::Select("#test_svg");
    circle_selection = svg_selection.SelectAll("circle");

    circle_selection.SetAttr("r", 5);
  }

  void Describe() override {

    EM_ASM({
      describe("creating an empty selection", function() {
        it ("should be a selection", function() {
          chai.assert(emp_d3.objects[$0] instanceof d3.selection);
        });
        it("should be empty", function() {
          chai.assert(emp_d3.objects[$0].empty());
          chai.assert.equal(emp_d3.objects[$0].size(), 0);
        });
      });
    }, empty_selection.GetID());

    EM_ASM({
      describe("selecting an svg with some circles in it!", function() {
        it("should be a selection", function() {
          chai.assert(emp_d3.objects[$0] instanceof d3.selection);
          chai.assert(emp_d3.objects[$1] instanceof d3.selection);
        });
        it ("should be selected", function() {
          chai.assert.equal(emp_d3.objects[$0].size(), 1);
          chai.assert.equal(emp_d3.objects[$1].size(), 2);
        });
        it ("should let you set attributes with attr", function() {
          chai.assert.equal(emp_d3.objects[$1].attr("r"), "5");
        });
      });
    }, svg_selection.GetID(), circle_selection.GetID());

    // tests:
    // - Data (all the different versions?)
    // - Enter
    // - Exit
    // - Insert
    // - Append
    // - EnterAppend
    // - ExitRemove
    // - SetProperty
    // - SetHTML
    // - SetClassed
    // - GetHtml
    // - MakeTransition
    // - Interrupt
    // - Move
    // - Rotate
    // - Order
    // - Raise
    // - Lower
    // - On
    // - Sort
  }
};


emp::web::MochaTestRunner test_runner;

int main() {

  test_runner.Initialize({"emp_test_container"});

  D3::internal::get_emp_d3();

  test_runner.AddTest<Test_Selection>("Selection");

  test_runner.OnBeforeEachTest([]() {
    ResetD3Context();
  });
  test_runner.Run();

  return 0;
}