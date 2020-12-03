//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

/*
 * This file can be used to test triggering a mouse click by the user.
 * It appears that .click() can be used to trigger a click.
 * Issues encountered:
 *  - When click() is called  on line 100 (1st call), it looks
 *    like it triggers this function before any of the tests execute.
 *    As a result, the it statements in the 1st and 3rd describes
 *    all fail.
 *
 *  - When click() is called on line 123 (2nd call), the it statements
 *    in the 1st and 3rd describes all fail too. Maybe we can't click
 *    the same element multiple time in one test?
 *
 * Goals:
 *  - Be able to trigger a click at a particular point in a test
 *      [can click once when triggered in an it statement]
 *  - Be able to click an element multiple times. <----------- TODO:
 *
 * Thoughts:
 *  - Give control back to the browser for click and take it back for tests.
 *  - Sleep between clicks so bowser doesn't think we're double clicking
 *  - Compile tests inside of docker, and serve them in regular browser
 */

#include <functional>
#include <unordered_map>
#include <string>

#include "emp/base/assert.hpp"
#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/Element.hpp"
#include "emp/web/web.hpp"
#include "emp/web/js_utils.hpp"

#include "emp/prefab/Collapse.hpp"

struct Test_Collapse_Click_Initial : emp::web::BaseTest {

  /*
   * Construct the following HTML structure:
   *
   * <div id="emp_test_container">
   *
   * <span aria-controls=".set1" aria-expanded="true" class="collapse_toggle" data-target=".set1" data-toggle="collapse" role="button">Controller 1</span>
   *
   * <span class="collapse show , set1">[1] Target Content (set1)</span>
   *
   * </div>
   */

  Test_Collapse_Click_Initial()
  : BaseTest({"emp_test_container"})
  {
    emp::prefab::CollapseCoupling couple1("Controller 1", "[1] Target Content (set1)", true, "set1");

    Doc("emp_test_container") << couple1.GetControllerDiv();
    Doc("emp_test_container") << couple1.GetTargetDiv();

  }

  void Describe() override {
    // Test that the HTML components created in constructor are correct.
    EM_ASM({

      describe("Initial HTML", function() {
        // Note: If the loading modal is removed from DOM, decrement all indicies by 1
        const controller = document.getElementsByTagName("span")[1];
        const target = document.getElementsByTagName("span")[2];

        it('Controller should have aria-expanded set to true', function() {
          chai.assert.equal(controller.getAttribute("aria-expanded"), "true");
        });

        it('Controller should not have class collapsed', function() {
          chai.assert.isFalse(controller.classList.contains("collapsed"));
        });

        it('Target should have class collapse', function() {
          chai.assert.isTrue(target.classList.contains("collapse"));
        });

        it('Target should have class show', function() {
          chai.assert.isTrue(target.classList.contains("show"));
        });
      });

    });
  }
};

struct Test_Collapse_One_Click : emp::web::BaseTest {

  /*
   * Construct the following HTML structure:
   *
   * <div id="emp_test_container">
   *
   * <span aria-controls=".set1" aria-expanded="true" class="collapse_toggle" data-target=".set1" data-toggle="collapse" role="button">Controller 1</span>
   *
   * <span class="collapse show , set1">[1] Target Content (set1)</span>
   *
   * </div>
   */



  Test_Collapse_One_Click()
  : BaseTest({"emp_test_container"})
  {
    emp::prefab::CollapseCoupling couple1("Controller 1", "[1] Target Content (set1)", true, "set1");

    Doc("emp_test_container") << couple1.GetControllerDiv();
    Doc("emp_test_container") << couple1.GetTargetDiv();

  }

  void Describe() override {
    // Test that the HTML components created in constructor are correct.
    EM_ASM({

      describe("Controller 1st click, collapse target", function() {
        const controller = document.getElementsByTagName("span")[1];
        const target = document.getElementsByTagName("span")[2];
        it('should make the controller have class "collapsed" after first click', function() {
          controller.click(); // click to collapse
          chai.assert.isTrue(controller.classList.contains("collapsed"));
        });

        it('should make the controller have aria-expanded = false', function() {
          chai.assert.equal(controller.getAttribute("aria-expanded"), "false");
        });

        it('should cause the target to not have the class "show"', function() {
          chai.assert.isFalse(target.classList.contains("show"));
        });
      });

    });
  }
};

struct Test_Collapse_Two_Clicks : emp::web::BaseTest {

  /*
   * Construct the following HTML structure:
   *
   * <div id="emp_test_container">
   *
   * <span aria-controls=".set1" aria-expanded="true" class="collapse_toggle" data-target=".set1" data-toggle="collapse" role="button">Controller 1</span>
   *
   * <span class="collapse show , set1">[1] Target Content (set1)</span>
   *
   * </div>
   */

  Test_Collapse_Two_Clicks()
  : BaseTest({"emp_test_container"})
  {
    emp::prefab::CollapseCoupling couple1("Controller 1", "[1] Target Content (set1)", true, "set1");

    Doc("emp_test_container") << couple1.GetControllerDiv();
    Doc("emp_test_container") << couple1.GetTargetDiv();

  }

  /* TODO: Test successfully clicks multiple times, but repetitive
   * https://github.com/devosoft/Empirical/issues/368
   *
   * Methods tried (unsuccessful):
   *  - wait 1 second before clicking the element a second time
   *  - click element a second time immediately after first click
   *  - double click element after first click (no time delay)
   *  - click element twice after first click (1 sec time delay)
   *  - put whole describe in setTimeout()
   *  - put whole it or multile it statements in setTimeout()
   *
   * Suggestions to try next:
   *  - change settimeout to 1 ms
   *  - instead of settimeout append events to event queue
   *  - create a macro to reduce repeated boilerplate code
   */
  void Describe() override {
    // Test that the HTML components created in constructor are correct.
    EM_ASM({

        describe("Controller 2nd click, expand target", function() {
          const controller = document.getElementsByTagName("span")[1];
          const target = document.getElementsByTagName("span")[2];

          it('should make the controller not have class "collapsed"', function() {
            controller.click();

            setTimeout(function() {
              controller.click();
            }, 1000);

            setTimeout(function() {
              chai.assert.isFalse(controller.classList.contains("collapsed"));
            }, 1000);
          });

          it('should make the controller have aria-expanded = true', function() {
            setTimeout(function() {
              chai.assert.equal(controller.getAttribute("aria-expanded"), "true");
            }, 1000);
          });

          it('should cause the target to have the class "show"', function() {
            setTimeout(function() {
              chai.assert.isTrue(target.classList.contains("show"));
            }, 1000);
          });

      });

    });
  }
};


struct Test_show_timing : emp::web::BaseTest {

  /*
   * Construct the following HTML structure:
   *
   * <div id="emp_test_container">
   *
   * <span aria-controls=".set1" aria-expanded="false" class="collapse_toggle" data-target=".set1" data-toggle="collapse" role="button">Controller 1</span>
   *
   * <span class="collapse , set1">[1] Target Content (set1)</span>
   *
   * </div>
   */

  Test_show_timing()
  : BaseTest({"emp_test_container"})
  {
    emp::prefab::CollapseCoupling couple1("Controller 1", "[1] Target Content (set1)", false, "set1");

    Doc("emp_test_container") << couple1.GetControllerDiv();
    Doc("emp_test_container") << couple1.GetTargetDiv();

  }

  /*
   * Use setTimeout() when needing to check if an element has the
   * class show after triggering a click. If not, the assert may
   * incorrectly return false. This is becuase Bootstrap briefly
   * gives elements that are in the process of collapsing or expanding
   * the class "collapsing" before the "show" class is removed or added,
   * respectively. If setTimeout() is not used, the element will be
   * tested for the assert during this transition period.
   */
  void Describe() override {
    // Test that the HTML components created in constructor are correct.
    EM_ASM({

      describe("Target after clicking controller", function() {
        // Note: If the loading modal is removed from DOM, decrement all indicies by 1
        const controller = document.getElementsByTagName("span")[1];
        const target = document.getElementsByTagName("span")[2];

        it('should have class show', function() {
          controller.click();
          // chai.assert.isTrue(target.classList.contains("show") <---- This fails
          setTimeout(function() {
            chai.assert.isTrue(target.classList.contains("show"));
          }, 3000);
        });
      });

    });
  }
};


emp::web::MochaTestRunner test_runner;
int main() {
  test_runner.Initialize({"emp_test_container"});

  test_runner.AddTest<Test_Collapse_Click_Initial>("Test DOM of original emp::prefab::Collapse element"); // Passes
  test_runner.AddTest<Test_Collapse_One_Click>("Test DOM after 1st click of emp::prefab::Collapse element"); // Passes
  test_runner.AddTest<Test_Collapse_Two_Clicks>("Test DOM after 2nd click of emp::prefab::Collapse element"); // Passes, but not efficient
  test_runner.AddTest<Test_show_timing>("Test existance of class show after expanding"); // Passes

  test_runner.Run();
}
