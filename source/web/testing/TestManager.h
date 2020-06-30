
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

  /// Base test class that all web tests managed by TestManager should inherit from.
  struct BaseTest {

    BaseTest() { ; }

    // Remember to clean up after your test!
    virtual ~BaseTest() { }

    /// Setup is run immediately after construction and before Describe.
    /// Setup should run any configuration/setup (e.g., dom manipulation, object creation/configuration)
    /// necessary for test.
    virtual void Setup() { ; }

    /// Describe is run after Setup.
    /// Describe should contain the Mocha testing statements (e.g., 'describes', 'its', etc)
    /// [https://mochajs.org/#getting-started](https://mochajs.org/#getting-started)
    virtual void Describe() { ; }

  };

  /// Utility class for managing software tests written for Emscripten web code.
  /// IMPORTANT: This utility assumes the Karma + Mocha javascript testin framework.
  // TestManager is useful because emscripten does not play very nice with the browser event queue (i.e.,
  // it does not relinquish control back to the browser until it finishes executing the compiled 'C++'
  // code). This interacts poorly with Mocha because Mocha's 'describe' statements do not execute when
  // they are called; instead, they are added to the browser's event queue.
  // The TestManager exploits Mocha's describe statements + the browser's event queue to chain together
  // the tests added to the TestManager.
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
      test_runners[cur_test].create();    // Create test object
      test_runners[cur_test].describe();  // This will queue up describe clause for this test and either (1) queue up the next test or (2) queue up manager cleanup
      ++cur_test;                         // NOTE: this will execute before the test is run
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

    /// Add a test type to be run. The TestManager creates, runs, and cleans up each test.
    /// This function should be called with the test type (which should inherit from BaseTest) as a
    /// template argument (e.g., AddTest<TEST_TYPE>(...) ).
    /// Arguments:
    /// - test_name specifies the name of the test (this is only used when printing which test is running
    ///   and does not need to be unique across tests).
    /// - All subsequent arguments are forwarded to the TEST_TYPE constructor.
    // For variatic capture: https://stackoverflow.com/questions/47496358/c-lambdas-how-to-capture-variadic-parameter-pack-from-the-upper-scope
    //  - NOTE: this can get cleaned up quite a bit w/C++ 20!
    template<typename TEST_TYPE, typename... Args>
    void AddTest(const std::string & test_name, Args&&... constructor_args) {
      const size_t runner_id = test_runners.size();

      // create a new test runner for this test
      test_runners.emplace_back();
      auto & runner = test_runners[runner_id];

      // name it!
      runner.test_name = test_name;

      // configure the function that, when called, will create a new instance of TEST_TYPE (using forwarded
      // arguments as constructor arguments) and then call TEST_TYPE::Setup.
      runner.create = [constructor_args = std::make_tuple(std::forward<Args>(constructor_args)...), runner_id, this]() mutable {
        std::apply([this, runner_id](auto&&... constructor_args) {
          // Allocate memory for test
          this->test_runners[runner_id].test = emp::NewPtr<TEST_TYPE>(std::forward<Args>(constructor_args)...);
          this->test_runners[runner_id].done = false;
          // Run test setup
          this->test_runners[runner_id].test->Setup();
        }, std::move(constructor_args));
      };

      // configure the function that, when called, will call TEST_TYPE::Describe, which should queue
      // up a Mocha describe function (containing js tests). Next, this function will either (1) queue
      // up the next test to be run or (2) queue up general TestManager cleanup if there are no more
      // tests to run.
      // This function takes advantage of Mocha describe functions to use 'browser' events to chain
      // together tests (which is necessary because js 'describe' calls add themselves to the browser's
      // event queue instead of executing as soon as you call them).
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

        // If there are still more tests to do, queue them
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

      // configure the function that, when called, triggers the 'after each test' signal, and cleans
      // up the dynamically allocated TEST_TYPE object.
      runner.cleanup = [runner_id, this]() {
        this->test_runners[runner_id].done = true;
        this->after_each_test_sig.Trigger();
        this->test_runners[runner_id].test.Delete();
        this->test_runners[runner_id].test = nullptr;
      };
    }

    /// Run all tests that have been added to the TestManager thus far.
    /// Running a test consumes it (i.e., executing Run a second time will not re-run previously run
    /// tests).
    void Run() { NextTest(); }

    /// Provide a function for TestManager to call before each test is created and run.
    void OnBeforeEachTest(const std::function<void()> & fun) { before_each_test_sig.AddAction(fun); };

    /// Provide a function for TestManager to call before after each test runs (but before it is deleted).
    void OnAfterEachTest(const std::function<void()> & fun) { after_each_test_sig.AddAction(fun); };

  };


}
}

#endif