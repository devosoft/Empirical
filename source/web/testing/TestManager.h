
/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020-2020
 *
 *  @file  TestManager.h
 *  @brief Utility class for managing software testing for Emscripten web code using the Karma + Mocha
 *         javascript testing framework.
 *
 */

#ifndef WEB_TESTING_UTILS_H
#define WEB_TESTING_UTILS_H

#include <functional>
#include <type_traits>
#include <utility>
#include <algorithm>
#include "control/Signal.h"
#include "base/vector.h"
#include "web/JSWrap.h"

namespace emp {
namespace web {

  /// Base test class that all tests should inherit from.
  struct BaseTest {

    BaseTest() { ; }

    virtual ~BaseTest() { }

    /// Put code to actually run the test here.
    virtual void Describe() { ; }
    virtual void Setup() { ; }
  };

  /// Utility class for managing software tests written for Emscripten web code.
  /// IMPORTANT: This utility assumes the Karma + Mocha javascript testin framework.
  // QUESTION: should this be the KarmaMochaTestingManager?
  class TestManager {
  protected:

    // TestRunner encapsulates everything needed to create, run, and cleanup a test.
    struct TestRunner {
      emp::Ptr<BaseTest> test{nullptr};
      std::function<void()> create{};
      std::function<void()> describe{};
      std::function<void()> cleanup{};
      std::string test_name{};
      bool done=false;
    };

    emp::Signal<void()> before_each_test_sig;   ///< Is triggered before each test.
    emp::Signal<void()> after_each_test_sig;    ///< Is triggered after each test (after test marked 'done', but before test is deleted).
    emp::vector<TestRunner> test_runners;       ///< Set of tests to be run.

    size_t cur_test=0;
    size_t next_test_js_func_id=0;
    size_t cleanup_test_js_func_id=0;
    size_t cleanup_all_js_func_id=0;

    /// Run the next test!
    void NextTest() {
      emp_assert(cur_test < test_runners.size());
      before_each_test_sig.Trigger();
      test_runners[cur_test].create();    // Create test object in clean namespace
      test_runners[cur_test].describe();  // this will queue up describe clause
      ++cur_test; // note: this will execute before the test is run
    }

    /// Cleanup test runner specified by runner_id.
    void CleanupTest(size_t runner_id) {
      emp_assert(runner_id < test_runners.size());
      test_runners[runner_id].cleanup();
    }

    /// Cleanup all test runners.
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
      Cleanup();
      emp::JSDelete(next_test_js_func_id);
      emp::JSDelete(cleanup_test_js_func_id);
      emp::JSDelete(cleanup_all_js_func_id);
    }

    /// Add
    // Arguments are forwarded to the constructor.
    // For variatic capture: https://stackoverflow.com/questions/47496358/c-lambdas-how-to-capture-variadic-parameter-pack-from-the-upper-scope
    //  - NOTE: this can get cleaned up quite a bit w/C++ 20!
    // All the crazy lambda capture stuff is for the minor flexibility of being able to use non-default constructors...
    template<typename TEST_TYPE, typename... Args>
    void AddTest(const std::string & test_name, Args&&... constructor_args) {
      const size_t runner_id = test_runners.size();

      test_runners.emplace_back();
      auto & runner = test_runners[runner_id];

      runner.test_name = test_name;

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
        auto & test_name = this->test_runners[runner_id].test_name;
        EM_ASM({
          const test_id = $0;
          const test_name = UTF8ToString($1);
          // Queue up cleanup for this test
          describe("Cleanup " + test_name + " (test id " + test_id + ")", function() {
            it('should clean up test id ' + test_id, function() {
              emp.CleanupTest($0);
            });
          });
        }, runner_id, test_name.c_str());

        // If there are still more tests to do, queue them...
        // otherwise, queue up a cleanup
        if (next_test_id < this->test_runners.size()) {
          auto & next_test_name = this->test_runners[next_test_id].test_name;
          EM_ASM({
            const next_test_id = $0;
            const next_test_name = UTF8ToString($1);
            // Queue up next test
            describe("Queue " + next_test_name + " (test id " + next_test_id + ")", function() {
              it("should queue the next test " + next_test_id, function() {
                emp.NextTest();
              });
            });
          }, next_test_id, next_test_name.c_str());
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

    /// TODO
    void Run() { NextTest(); }

    /// TODO
    void OnBeforeEachTest(const std::function<void()> & fun) { before_each_test_sig.AddAction(fun); };

    /// TODO
    void OnAfterEachTest(const std::function<void()> & fun) { after_each_test_sig.AddAction(fun); };

  };


}
}

#endif