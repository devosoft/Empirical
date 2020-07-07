#include "web/Document.h"
#include "web/d3/d3_init.h"
#include "web/d3/scales.h"
#include "web/_MochaTestRunner.h"
#include "d3_testing_utils.h"

// This file tests:
// - D3_Scales

struct Test_Scale : emp::web::BaseTest {
  D3::LinearScale testLinear;
  int result1;
  int result2;
  int result1i;
  int result2i;

  Test_Scale() : emp::web::BaseTest({"emp_test_container"}) { Setup(); }
  // Test_Scale() { Setup(); }

  void Setup() { 
    testLinear.SetDomain(10, 130);
    testLinear.SetRange(0, 960);
    result1 = testLinear.ApplyScale<int>(20);
    result2 = testLinear.ApplyScale<int>(50);
    result1i = testLinear.Invert<int>(80);
    result2i = testLinear.Invert<int>(320); 
  }
  
  void Describe() override {

    EM_ASM({  
      const id = $0;
      const result1 = $1;
      const result2 = $2;
      const result1i = $3;
      const result2i = $4;

      var x = d3.scaleLinear()
                  .domain([ 10, 130 ])
                  .range([ 0, 960 ]);

      describe("creating a linear scale", function() {
        it ("should apply the scale correctly", function() {
          chai.assert.equal(result1, x(20)); // 80
        });
        it("should apply another scale correctly", function() {
          chai.assert.equal(result2, x(50)); // 320
        });
        it("should invert the scale correctly", function() {
          chai.assert.equal(result1i, x.invert(80)); // 20
        });
        it("should inver another scale correctly", function() {
          chai.assert.equal(result2i, x.invert(320)); // 50
        });
      });
    }, testLinear.GetID(), result1, result2, result1i, result2i);
  }

};


emp::web::MochaTestRunner test_runner;

int main() {

  test_runner.Initialize({"emp_test_container"});
  D3::internal::get_emp_d3();
  test_runner.AddTest<Test_Scale>("Scale");
  
  test_runner.OnBeforeEachTest([]() {
    ResetD3Context();
  });
  
  test_runner.Run();

  return 0;
}