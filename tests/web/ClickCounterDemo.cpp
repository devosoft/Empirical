/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file ClickCounterDemo.cpp
 *  @brief This file provides a simple example of clicking a web
 * element at multiple points within a test.
 *
 * In this case, the web page consists of a numerical "live" variable,
 * x, (http://mmore500.com/waves/tutorials/lesson04.html#live-variables)
 * and an emp::web::Button. When the button is clicked, the
 * value of x will increment by 1 and the page will be redrawn.
 *
 * We have found that we can trigger a mouse click using
 * the .click() JavaScript function. In order for the click
 * to occur when we expect, we must call this function
 * within an it statement. Additionally, since we are redrawing
 * the page with each click, we must trigger every click within
 * a setTimeout() call. All asserts that depend on the click
 * to occur first must also be called within the setTimeout().
 *
 * The first parameter to setTimeout() is a function containing
 * the code you want to run after a certain amount of time has
 * passed. The second parameter is the where you specify the
 * amount of time to elapse (in milliseconds) before running the
 * function.
 *
 * Note: If the web element that you are clicking does not
 * require a redraw, the first click does not need to be called
 * in a setTimeout(). However, all other click must be within
 * a setTimeout().
 */

#include <functional>
#include <unordered_map>

#include "emp/base/assert.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/web.hpp"

#include "emp/web/Button.hpp"

int x = 0; // live variable

// Test that the user can trigger multiple clicks of a web element.
struct Test_Click : public emp::web::BaseTest {
  /*
   * Construct the following HTML structure:
   *
   * <span id="emp__0">0</span>
   * <button id="counter_id" onclick="emp.Callback(2)">Increment</button>
   */

  Test_Click()
  : BaseTest({"emp_test_container"}) // we can tell BaseTest that we want to create a set of emp::web::Document
                                     // objects for each given html element ids.
  {
    Doc("emp_test_container") << emp::web::Live(x);

    emp::web::Button counter( [this](){ x+=1; Doc("emp_test_container").Redraw(); }, "Increment", "counter_id" );
    Doc("emp_test_container") << counter;

  }

  void Describe() override {

    EM_ASM({

      describe("Basic emp_test_container HTML format", function() {
        const emp_container = document.getElementById("emp_test_container");
        it('should have 2 children', function() {
          chai.assert.equal(emp_container.childElementCount, 2);
        });

        describe("Child 1 - value", function() {
          it('should have a value of 0 before any clicks are triggered', function() {
            chai.assert.equal(emp_container.children[0].textContent, "0");
          });
        });

        describe("Child 2 - button", function() {
          const btn = emp_container.children[1];
          it('should have id counter_id', function() {
            chai.assert.equal(btn.id, "counter_id");
          });

          it('should be a BUTTON element', function() {
            chai.assert.equal(btn.nodeName, "BUTTON");
          });
        });
      }); // end basic HTML describe

      const btn = document.getElementById("counter_id");
      const value =  document.getElementById("emp_test_container").children[0];

      /*
       * Here, I've staggered the setTimeout times slightly
       * to ensure that the clicks and their corresponing
       * asserts occur in the order that I expect.
       */
      describe("First click", function() {
        it('should increment the value to 1', function() {
          setTimeout(function() {
            btn.click();
            chai.assert.equal(value.textContent, "1");
          }, 0);
        });
      });

      describe("Second click", function() {
        it('should increment the value to 2', function() {
          setTimeout(function() {
            btn.click();
            chai.assert.equal(value.textContent, "2");
          }, 1000);
        });
      });

      describe("Third click", function() {
        it('should increment the value to 3', function() {
          setTimeout(function() {
            btn.click();
            chai.assert.equal(value.textContent, "3");
          }, 2000);
        });
      });

    }); // end EM_ASM
  }

};

emp::web::MochaTestRunner test_runner;

int main() {

  test_runner.Initialize({"emp_test_container"});
  test_runner.AddTest<Test_Click>("Test Increment Button");
  test_runner.Run();

}
