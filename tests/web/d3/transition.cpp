#include "web/Document.hpp"
#include "web/d3/d3_init.hpp"
#include "web/d3/selection.hpp"
#include "web/d3/transition.hpp"
#include "web/_MochaTestRunner.hpp"
#include "d3_testing_utils.hpp"

struct Test_Transition : emp::web::BaseTest {

  void Setup() { ; }

  void Describe() {
    EM_ASM({
      describe("transition", function() {
        it("should work", function() {
          chai.assert(true);
        });
      });
    });
  }
};

emp::web::Document doc("test_d3_init");
emp::web::MochaTestRunner manager;

int main() {
  D3::internal::get_emp_d3();
  manager.AddTest<Test_Transition>("Test transition");

  manager.OnBeforeEachTest([]() { ResetD3Context(); });
  manager.Run();

  return 0;
}