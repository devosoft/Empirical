#include "web/Document.hpp"
#include "web/d3/d3_init.hpp"
#include "web/d3/selection.hpp"
#include "web/_MochaTestRunner.hpp"
#include "d3_testing_utils.hpp"

// namespace UI = emp::web;
// UI::Document doc("emp_d3_test");

// This file tests:
// - D3_Selection
//   - empty selection
//   - non empty selection

// Test the Selection constructors
struct Test_SelectionConstruction : emp::web::BaseTest {
  D3::Selection empty_selection;
  D3::Selection svg_selection;
  D3::Selection circle_selection;

  Test_SelectionConstruction() : emp::web::BaseTest({"emp_test_container"}) {
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

// enter, append, etc
struct Test_SelectionEnter : emp::web::BaseTest {
  D3::Selection svg_selection;
  D3::Selection enter_selection;
  emp::vector<int32_t> data{1, 2, 4, 8, 16, 32, 64};
  // uint32_t enter_func_id = 0;
  uint32_t append_func_id = 0;


  Test_SelectionEnter() {
    EM_ASM({
      $("#emp_test_container").append("<svg id='test_svg'></svg>");
    });

    svg_selection = D3::Select("#test_svg");

    enter_selection = svg_selection.SelectAll("circle").Data(data).Enter();

    append_func_id = emp::JSWrap(
      [this]() {
        enter_selection.Append("circle").SetAttr("class", "test_circle");
      },
      "AppendSel"
    );
  }

  ~Test_SelectionEnter() {
    // emp::JSDelete(enter_func_id);
    emp::JSDelete(append_func_id);
  }

  void Describe() override {

    EM_ASM({
      describe("calling enter on a data-bound selection", function() {
        it("should have 7 things in it", function() {
          chai.assert.equal(emp_d3.objects[$0]._groups[0].length, 7);
        });
      });
    }, enter_selection.GetID());

    EM_ASM({
      describe("calling append on our enter selection", function() {
        it("should put circles in the svg", function() {
          emp.AppendSel();
          chai.assert.equal($("#test_svg").children(".test_circle").length, 7);
        });
      });
    });
  }
};

// Tests the following Selection actions: Move
struct Test_SelectionActions : emp::web::BaseTest {
  D3::Selection svg_selection;
  D3::Selection circle_selection;

  Test_SelectionActions() : emp::web::BaseTest({"emp_test_container"}) {
    EM_ASM({
      $("#emp_test_container").append("<svg id='test_svg'><circle/><circle/></svg>");
    });
    svg_selection = D3::Select("#test_svg");
    circle_selection = svg_selection.SelectAll("circle");

    circle_selection.SetAttr("r", 5);
    circle_selection.SetAttr("cx", 0);
    circle_selection.SetAttr("cy", 0);
    circle_selection.Move(100, 100);
  }

  void Describe() override {

    EM_ASM({
      describe("moving/transforming a selection", function() {

        it("should move the circle selection by (100, 100)", function() {
          chai.assert.equal(emp_d3.objects[$0].attr("transform"), "translate(100,100)");
        });

      });
    }, circle_selection.GetID());
  }

};


emp::web::MochaTestRunner test_runner;

int main() {

  test_runner.Initialize({"emp_test_container"});

  D3::internal::get_emp_d3();

  test_runner.AddTest<Test_SelectionConstruction>("SelectionConstruction");
  test_runner.AddTest<Test_SelectionEnter>("Selection::Enter");
  test_runner.AddTest<Test_SelectionActions>("SelectionActions");

  test_runner.OnBeforeEachTest([]() {
    ResetD3Context();
  });
  test_runner.Run();

  return 0;
}