#include "web/Document.h"
#include "web/d3/d3_init.h"
#include "web/d3/selection.h"
#include "d3_testing_utils.h"

// namespace UI = emp::web;
// UI::Document doc("emp_d3_test");

struct Test_Selection : BaseTest {

  void Setup() {
    D3::Selection empty_selection;
    std::cout << empty_selection.GetID() << std::endl;
  }

  void Describe() {
    EM_ASM({
      describe("selection", function() {
        it("should work", function() {
          chai.assert(false);
        });
      });
    });
  }
};

// emp::web::Document doc("test_d3_init");
TestManager manager;

int main() {
  EM_ASM({
    $("body").append('<div id="emp_test_container"></div>');
  });

  emp::Initialize();
  D3::internal::get_emp_d3();

  manager.AddTest<Test_Selection>();

  manager.OnBeforeEachTest([]() {
    EM_ASM({
      $("#emp_test_container").empty();
    });
    ResetD3Context();
  });
  manager.Run();

  return 0;
}