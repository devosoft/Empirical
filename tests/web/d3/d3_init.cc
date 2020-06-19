//
#include "web/JSWrap.h"
#include "web/d3/d3_init.h"
#include "web/Document.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
#include <type_traits>
#include <utility>

// This file tests:
// - D3_Base
//   - reference counting
//   - correct assignment of next id
// - library_d3.js
//   - find_function
//   - is_function

emp::web::Document doc("test_d3_init");

// This struct exists to allow us to instantiate a D3_Base object.
struct BaseTester : D3::D3_Base {
  public:
    BaseTester() : D3::D3_Base() { ; }
    BaseTester(int id) : D3::D3_Base(id) {}
};

  void ResetContext() {
    EM_ASM({
      emp_d3.clear_emp_d3();  // Reset the emp_d3 object tracker
    });
  }


struct BaseTest {
  // emp::signal ondone
  emp::Signal<void()> after_test_sig;
  uint32_t test_done_func_id=0;
  // manager attaches signal!
  BaseTest() {
    std::cout << "> Base constructor" << std::endl;
    test_done_func_id = emp::JSWrap([this](){
      after_test_sig.Trigger();
    });
  }
  virtual ~BaseTest() {
    std::cout << "> Base destructor" << std::endl;
    emp::JSDelete(test_done_func_id);
  }
  virtual void Run() { ; }

  uint32_t GetDoneJSFuncID() const { return test_done_func_id; }
};

class TestManager {
public:
protected:
  // emp::vector<emp::Ptr<BaseTest>> tests;
  emp::Ptr<BaseTest> current_test=nullptr;
  emp::vector<std::function<void()>> test_runners;

  void NextTest(bool start=false) {
    // If current test points to something, delete it!
    if (current_test != nullptr) {
      current_test.Delete();
      current_test = nullptr;
    }
    if (!start && test_runners.size()) {
      test_runners.pop_back();
    }
    if (!test_runners.size()) { return; }
    ResetContext(); // Reset emp_d3 context
    (test_runners.back())(); // Run test!
  }

public:

  // Arguments are forwarded to the constructor.
  // For variatic capture: https://stackoverflow.com/questions/47496358/c-lambdas-how-to-capture-variadic-parameter-pack-from-the-upper-scope
  //  - NOTE: this can get cleaned up quite a bit w/C++ 20!
  // All the crazy lambda capture stuff is for the minor flexibility of being able to use non-default constructors...
  template<typename TEST_TYPE, typename... Args>
  void AddTest(Args&&... constructor_args) {
    test_runners.emplace_back(
      [constructor_args = std::make_tuple(std::forward<Args>(constructor_args)...), this]() mutable {
        std::apply([this](auto&&... constructor_args) {
          std::cout << ">>Test runner!<<" << std::endl;
          current_test = emp::NewPtr<TEST_TYPE>(std::forward<Args>(constructor_args) ...);
          current_test->after_test_sig.AddAction([this]() { this->NextTest(); });
          current_test->Run();
        }, std::move(constructor_args));
      }
    );
  }

  void RunTests() {
    std::cout << "-- Run tests! --" << std::endl;
    // Reverse order!
    std::reverse(test_runners.begin(), test_runners.end());
    // If there are more tests to run, run the next test (indicate this is the first test to run).
    if (test_runners.size()) {
      NextTest(true);
    }
  }

};


struct Test_BaseObjectIDAssignment : BaseTest {
  // All persistent data structures necessary for test should be member variables.
  BaseTester test1{}; // This will get created 1st.
  BaseTester test2{}; // This will get created 2nd.
  BaseTester test3{}; // This will get created 3rd.
  emp::Ptr<BaseTester> test4;
  uint32_t test4_del_func_id=0;

  void Run() override {
    std::cout << "==============================" << std::endl;
    std::cout << "Test_BaseObjectIDAssignment" << std::endl;

    test4 = emp::NewPtr<BaseTester>(D3::internal::NextD3ID());
    test4_del_func_id = emp::JSWrap([this]() { test4.Delete(); }, "TestDeleteBaseObject");

    std::cout << EM_ASM_INT({
      return Object.keys(emp_d3.objects).length;
    });
    std::cout << std::endl;

    emp_assert(test1.GetID() == 0);
    emp_assert(test2.GetID() == 1);
    emp_assert(test3.GetID() == 2);
    emp_assert(test4->GetID() == 3);

    EM_ASM({
      console.log("Inside EM_ASM!");

      describe('Base Object', function() {

        it('should create objects in Javascript', function() {
            console.log("running first 'it'");
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
          console.log("After function!");
          emp.Callback($0);
        });

      });
    }, this->GetDoneJSFuncID()); // -- end EM_ASM --

  }

  ~Test_BaseObjectIDAssignment() {
    emp::JSDelete(test4_del_func_id);
  }

};

struct TestThingy : BaseTest {
  size_t num;
  TestThingy(double i) : num(i) { ; }
  void Run() override {
    std::cout << "  > Testing thingy!" << std::endl;
    std::cout << "      num = " << num << std::endl;
  }
};

int main() {
  D3::internal::get_emp_d3();
  std::cout << "Creating test manager." << std::endl;
  TestManager test_manager;
  std::cout << "Adding Test_BaseObjectIDAssignment" << std::endl;
  test_manager.AddTest<Test_BaseObjectIDAssignment>();
  std::cout << "Adding TestThingy 1" << std::endl;
  test_manager.AddTest<TestThingy>(1);
  std::cout << "Adding TestThingy 2" << std::endl;
  test_manager.AddTest<TestThingy>(2);
  std::cout << "Adding TestThingy 3" << std::endl;
  test_manager.AddTest<TestThingy>(3);
  std::cout << "Adding TestThingy 4" << std::endl;
  test_manager.AddTest<TestThingy>(4);
  std::cout << "Run tests" << std::endl;
  test_manager.RunTests();

  std::cout << "FIN" << std::endl;
}