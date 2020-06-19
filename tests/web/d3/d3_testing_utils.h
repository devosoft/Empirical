#ifndef D3_WEB_TESTING_UTILS_H
#define D3_WEB_TESTING_UTILS_H

#include "control/Signal.h"
#include "base/vector.h"
#include "web/JSWrap.h"
#include "web/Animate.h"
#include <functional>
#include <type_traits>
#include <utility>
#include <algorithm>

/// Convenience function to reset D3 js namespace.
void ResetD3Context() {
  EM_ASM({
    emp_d3.clear_emp_d3();  // Reset the emp_d3 object tracker
  });
}

/// Base test class that all tests should inherit from.
struct BaseTest {
  emp::Signal<void()> before_test_sig;  ///< NOTE: This *must* get triggered by the before function!
  emp::Signal<void()> after_test_sig;   ///< NOTE: This *must* get triggered by the after function!
  uint32_t test_setup_func_id=0;
  uint32_t test_done_func_id=0;

  BaseTest() {
    test_done_func_id = emp::JSWrap([this](){
      after_test_sig.Trigger();
    });
    test_setup_func_id = emp::JSWrap([this]() {
      before_test_sig.Trigger();
    });
  }

  virtual ~BaseTest() {
    emp::JSDelete(test_done_func_id);
    emp::JSDelete(test_setup_func_id);
  }

  /// Put code to actually run the test here.
  virtual void Describe() { ; }
  virtual void Setup() { ; }

  uint32_t GetDoneJSFuncID() const { return test_done_func_id; }
  uint32_t GetSetupJSFuncID() const { return test_setup_func_id; }
};

class TestManager {
protected:

  struct TestRunner {
    emp::Ptr<BaseTest> test{nullptr};
    std::function<void()> create{};
    std::function<void()> describe{};
    std::function<void()> cleanup{};
    bool done=false;
  };

  emp::Signal<void()> before_each_test_sig;
  emp::Signal<void()> after_each_test_sig;
  emp::vector<TestRunner> test_runners;
  size_t cur_test=0;
  size_t next_tests_js_func_id=0;

  /// Cleanup previous test, if necessary. Queue next test's 'describe'.
  void NextTest() {
    if (cur_test > 0) {
      // Cleanup previous test
      emp_assert(test_runners[cur_test-1].done);
      test_runners[cur_test-1].cleanup();
    }
    if (cur_test >= test_runners.size()) {
      // finished running all tests. Make sure all dynamically allocated memory is cleaned up.
      std::for_each(
        test_runners.begin(),
        test_runners.end(),
        [](TestRunner & runner) {
          if (runner.test != nullptr) runner.test.Delete();
        }
      );
      // Clear test runners.
      test_runners.clear();
      return; // Relinquish execution control (don't queue anymore NextTest calls)
    }
    // ResetD3Context();                   // Reset D3 namespace
    before_each_test_sig.Trigger();
    test_runners[cur_test].create();    // Create test object in clean namespace
    test_runners[cur_test].describe();  // this will queue up describe clause
    ++cur_test; // note: this will execute before the test is run
  }

public:

  TestManager() {
    next_tests_js_func_id = emp::JSWrap([this](){ this->NextTest(); }, "NextTest");
  }

  ~TestManager() {
    emp::JSDelete(next_tests_js_func_id);
  }

  // Arguments are forwarded to the constructor.
  // For variatic capture: https://stackoverflow.com/questions/47496358/c-lambdas-how-to-capture-variadic-parameter-pack-from-the-upper-scope
  //  - NOTE: this can get cleaned up quite a bit w/C++ 20!
  // All the crazy lambda capture stuff is for the minor flexibility of being able to use non-default constructors...
  template<typename TEST_TYPE, typename... Args>
  void AddTest(Args&&... constructor_args) {
    const size_t runner_id = test_runners.size();

    test_runners.emplace_back();
    auto & runner = test_runners[runner_id];

    runner.create = [constructor_args = std::make_tuple(std::forward<Args>(constructor_args)...), runner_id, this]() mutable {
      std::apply([this, runner_id](auto&&... constructor_args) {
        // Allocate memory for test
        this->test_runners[runner_id].test = emp::NewPtr<TEST_TYPE>(std::forward<Args>(constructor_args)...);
        // Hook setup signal
        auto & tst = *(this->test_runners[runner_id].test);
        this->test_runners[runner_id].done = false;
        tst.before_test_sig.AddAction([this, runner_id]() {
          this->test_runners[runner_id].test->Setup();
        });
        tst.after_test_sig.AddAction([this, runner_id]() {
          this->test_runners[runner_id].done=true;
          this->after_each_test_sig.Trigger();
        });
      }, std::move(constructor_args));
    };

    runner.describe = [runner_id, this]() {
      this->test_runners[runner_id].test->Describe(); // this will run c++ setup code & queue up the describe
      EM_ASM({
        describe("NextTest", function() {
          it("should queue the next test", function() {
            emp.NextTest();
          });
        });
      });
    };

    runner.cleanup = [runner_id, this]() {
      this->test_runners[runner_id].test.Delete();
      this->test_runners[runner_id].test = nullptr;
      ResetD3Context();
    };
  }

  void Run() { NextTest(); }

  void OnBeforeEachTest(const std::function<void()> & fun) { before_each_test_sig.AddAction(fun); };
  void OnAfterEachTest(const std::function<void()> & fun) { after_each_test_sig.AddAction(fun); };

};

#endif