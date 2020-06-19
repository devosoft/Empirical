#include "web/JSWrap.h"
#include "web/d3/d3_init.h"
#include "web/Document.h"
#include <iostream>
#include <functional>
#include <type_traits>
#include <utility>

#include "d3_testing_utils.h"

// This file tests:
// - D3_Base
//   - reference counting
//   - correct assignment of next id
// - library_d3.js
//   - find_function
//   - is_function

// This struct exists to allow us to instantiate a D3_Base object.
struct BaseTester : D3::D3_Base {
  public:
    BaseTester() : D3::D3_Base() { ; }
    BaseTester(int id) : D3::D3_Base(id) {}
};

struct Test_BaseObjectIDAssignment : BaseTest {
  // All persistent data structures necessary for test should be member variables.
  BaseTester test1{}; // This will get created 1st.
  BaseTester test2{}; // This will get created 2nd.
  BaseTester test3{}; // This will get created 3rd.
  emp::Ptr<BaseTester> test4;
  uint32_t test4_del_func_id=0;

  // This code will run in the 'before' call for this function's describe clause.
  void Setup() override {
    test4 = emp::NewPtr<BaseTester>(D3::internal::NextD3ID());
    test4_del_func_id = emp::JSWrap([this]() { test4.Delete(); }, "TestDeleteBaseObject");

    emp_assert(test1.GetID() == 0);
    emp_assert(test2.GetID() == 1);
    emp_assert(test3.GetID() == 2);
    emp_assert(test4->GetID() == 3);
  }

  // Describe test (should only contain 'describe' call)
  void Describe() override { // option => move this into 'Setup'
    EM_ASM({
      const done_func_id = $0;
      const setup_func_id = $1;
      describe('Base Object', function() {

        before(function() {
          emp.Callback(setup_func_id); // This will call setup!
        });

        it('should create objects in Javascript', function() {
            chai.assert.equal(Object.keys(emp_d3.objects).length, 4, "emp_d3.objects");
            chai.assert.equal(Object.keys(emp_d3.counts).length, 4, "emp_d3.counts");
        });

        it('should track next_id correctly', function(){
            chai.assert.equal(emp_d3.next_id, 4, "emp_d3.next_id");
        });

        it("should do reference counting", function() {
            chai.assert.equal(emp_d3.counts[0], 1);
            chai.assert.equal(emp_d3.counts[1], 1);
            chai.assert.equal(emp_d3.counts[2], 1);
            chai.assert.equal(emp_d3.counts[3], 1);

            emp.TestDeleteBaseObject();

            chai.assert.equal(emp_d3.counts[0], 1);
            chai.assert.equal(emp_d3.counts[1], 1);
            chai.assert.equal(emp_d3.counts[2], 1);
            chai.assert.equal(3 in emp_d3.counts, false);
            chai.assert.equal(3 in emp_d3.objects, false);
        });

        after(function() {
          emp.Callback(done_func_id); // This will mark this test as done
        });

      });
    }, this->GetDoneJSFuncID(), this->GetSetupJSFuncID()); // -- end EM_ASM --
  }

  ~Test_BaseObjectIDAssignment() {
    emp::JSDelete(test4_del_func_id);
  }
};

// Proof of concept (will delete later)
struct TestThingy : BaseTest {
  size_t num;
  TestThingy(double i) : num(i) { ; }
  void Describe() override {
    before_test_sig.Trigger();
    std::cout << "  > Testing thingy!" << std::endl;
    std::cout << "      num = " << num << std::endl;
    after_test_sig.Trigger();
  }
};

emp::web::Document doc("test_d3_init");
TestManager test_manager;

int main() {
  D3::internal::get_emp_d3();
  std::cout << "Creating test manager." << std::endl;

  test_manager.AddTest<Test_BaseObjectIDAssignment>();
  test_manager.AddTest<TestThingy>(1);
  test_manager.AddTest<Test_BaseObjectIDAssignment>();
  test_manager.AddTest<TestThingy>(2);
  test_manager.AddTest<Test_BaseObjectIDAssignment>();
  test_manager.AddTest<TestThingy>(3);
  test_manager.AddTest<Test_BaseObjectIDAssignment>();

  test_manager.OnBeforeEachTest([]() { ResetD3Context(); });
  test_manager.Run();
}