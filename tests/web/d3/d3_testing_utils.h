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

  BaseTest() { ; }

  virtual ~BaseTest() { }

  /// Put code to actually run the test here.
  virtual void Describe() { ; }
  virtual void Setup() { ; }
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
  size_t next_test_js_func_id=0;
  size_t cleanup_test_js_func_id=0;
  size_t cleanup_all_js_func_id=0;

  /// Cleanup previous test, if necessary. Queue next test's 'describe'.
  void NextTest() {
    emp_assert(cur_test < test_runners.size());
    // ResetD3Context();                   // Reset D3 namespace
    before_each_test_sig.Trigger();
    test_runners[cur_test].create();    // Create test object in clean namespace
    test_runners[cur_test].describe();  // this will queue up describe clause
    ++cur_test; // note: this will execute before the test is run
  }

  void CleanupTest(size_t runner_id) {
    emp_assert(runner_id < test_runners.size());
    test_runners[runner_id].cleanup();
  }

  void Cleanup() {
    // finished running all tests. Make sure all dynamically allocated memory is cleaned up.
    std::for_each(
      test_runners.begin(),
      test_runners.end(),
      [](TestRunner & runner) {
        emp_assert(runner.done);
        if (runner.test != nullptr) runner.test.Delete();
      }
    );
    // Clear test runners.
    test_runners.clear();
  }

public:

  TestManager() {
    next_test_js_func_id = emp::JSWrap(
      [this]() { this->NextTest(); },
      "NextTest"
    );
    cleanup_test_js_func_id = emp::JSWrap(
      [this](size_t test_id) { this->CleanupTest(test_id); },
      "CleanupTest"
    );
    cleanup_all_js_func_id = emp::JSWrap(
      [this]() { this->Cleanup(); },
      "CleanupManager"
    );
  }

  ~TestManager() {
    emp::JSDelete(next_test_js_func_id);
    emp::JSDelete(cleanup_test_js_func_id);
    emp::JSDelete(cleanup_all_js_func_id);
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
        this->test_runners[runner_id].done = false;
        // Run test setup
        this->test_runners[runner_id].test->Setup();
      }, std::move(constructor_args));
    };

    runner.describe = [runner_id, this]() {
      this->test_runners[runner_id].test->Describe(); // this will queue up the next test's describe
      const size_t next_test_id = runner_id + 1;

      EM_ASM({
        const test_id = $0;
        // Queue up cleanup for this test
        describe("Cleanup test " + test_id, function() {
          it('should clean up test ' + test_id, function() {
            emp.CleanupTest($0);
          });
        });
      }, runner_id);

      // If there are still more tests to do, queue them...
      // otherwise, queue up a cleanup
      if (next_test_id < this->test_runners.size()) {
        EM_ASM({
          const next_test_id = $0;
          // Queue up next test
          describe("Queue test " + next_test_id, function() {
            it("should queue the next test " + next_test_id, function() {
              emp.NextTest();
            });
          });
        }, next_test_id);
      } else {
        EM_ASM({
          describe("Finished running tests.", function() {
            it("should cleanup test manager", function() {
              emp.CleanupManager();
            });
          });
        });
      }
    };

    runner.cleanup = [runner_id, this]() {
      this->test_runners[runner_id].done = true;
      this->after_each_test_sig.Trigger();
      this->test_runners[runner_id].test.Delete();
      this->test_runners[runner_id].test = nullptr;
    };
  }

  void Run() { NextTest(); }

  void OnBeforeEachTest(const std::function<void()> & fun) { before_each_test_sig.AddAction(fun); };
  void OnAfterEachTest(const std::function<void()> & fun) { after_each_test_sig.AddAction(fun); };

};

#endif