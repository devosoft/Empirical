#include "web/Document.hpp"
#include "web/d3/d3_init.hpp"
#include "web/d3/scales.hpp"
#include "web/d3/selection.hpp"
#include "web/d3/svg_shapes.hpp"
#include "web/_MochaTestRunner.hpp"
#include "d3_testing_utils.hpp"

// namespace UI = emp::web;
// UI::Document doc("emp_d3_test");

// This file tests:
// - Shapes

struct Test_LineGenerator : emp::web::BaseTest {
  D3::LineGenerator sg;
  D3::Selection svg_selection;
  D3::LinearScale x_scale;
  D3::LinearScale y_scale;
  uint32_t append_func_id = 0;

  Test_LineGenerator() : emp::web::BaseTest({"emp_test_container"}) {
    std::cout << sg.GetID() << std::endl;

    EM_ASM({
      $("#emp_test_container").append("<svg id='test_svg'></svg>");
    });
    svg_selection = D3::Select("#test_svg");
    x_scale.SetRange(0, 10);
    x_scale.SetDomain(0, 1);
    y_scale.SetRange(0, 100);
    y_scale.SetDomain(0, 1);
    sg.AddXScale(x_scale);
    sg.AddYScale(y_scale);
    emp::vector<emp::vector<double> > data({{0,0},{.2, .5},{1,1}});
    this->Require(sg.Generate(data) == "M0,0L2,50L10,100");
    std::cout << sg.Generate(data)  << std::endl;
    append_func_id = emp::JSWrap(
      [this, data]() {
        svg_selection.Append("path").SetAttr("d", sg.Generate(data));
      },
      "DrawPath"
    );

  }

  ~Test_LineGenerator() {
    emp::JSDelete(append_func_id);
  }

  void Describe() override {

    EM_ASM({
      describe("create a line generator", function() {
        it ("should be a line generator", function() {
          chai.assert(typeof emp_d3.objects[$0] === "function" );
        });
        it("should draw a path", function() {
          emp.DrawPath();
          chai.assert(emp_d3.objects[$1].select("path").attr("d") == "M0,0L2,50L10,100");
        });
      });
    }, sg.GetID(), svg_selection.GetID());
  }
};

emp::web::MochaTestRunner test_runner;

int main() {

  test_runner.Initialize({"emp_test_container"});

  D3::internal::get_emp_d3();

  test_runner.AddTest<Test_LineGenerator>("ShapeGeneration");

  test_runner.OnBeforeEachTest([]() {
    ResetD3Context();
  });
  test_runner.Run();

  return 0;
}