#include "web/Document.h"
#include "web/d3/d3_init.h"
#include "web/d3/transition.h"
#include "d3_testing_utils.h"

struct Test_Transition : BaseTest {

  void Setup() { ; }

  void Describe() {
    EM_ASM({
      describe("transition", function() {
        it("should work", function() {
          chai.assert(false);
        });
      });
    });
  }
};

emp::web::Document doc("test_d3_init");
TestManager manager;

int main() {
  D3::internal::get_emp_d3();
  manager.AddTest<Test_Transition>();

  manager.OnBeforeEachTest([]() { ResetD3Context(); });
  manager.Run();

  return 0;
}