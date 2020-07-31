//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

/**
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
 *  - Be able to click an element multiple times.
 * 
 * Thoughts:
 *  - Give control back to the browser for click and take it back for tests.
 *  - Sleep between clicks so bowser doesn't think we're double clicking
 *  - Compile tests inside of docker, and serve them in regular browser
 */

#include <functional>
#include <unordered_map>
#include <string>

#include "base/assert.h"
#include "web/_MochaTestRunner.h"
#include "web/Document.h"
#include "web/Element.h"
#include "web/web.h"
#include "web/js_utils.h"

#include "prefab/Collapse.h"

struct Test_Collapse_Click : emp::web::BaseTest {

  // Construct the following HTML structure:
  /**
   * <div id="emp_test_container">
   *
   * <span aria-controls=".set1" aria-expanded="true" class="collapse_toggle" data-target=".set1" data-toggle="collapse" role="button">Controller 1</span>
   *
   * <span class="collapse show , set1">[1] Target Content (set1)</span>
   *
   * </div>
   */

  // variables to pass to javascript
  std::string class_name;
  std::vector<std::string> controller1_classes;

  Test_Collapse_Click()
  : BaseTest({"emp_test_container"})
  {
    emp::prefab::CollapseCoupling couple1("Controller 1", "[1] Target Content (set1)", true, "set1");

    Doc("emp_test_container") << couple1.GetControllerDiv();
    Doc("emp_test_container") << couple1.GetTargetDiv();

  }

  void Describe() override {
    // Pass vector of controllers to JavaScript
    emp::pass_array_to_javascript(controller1_classes);
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

      describe("Controller 1st click, collapse target", function() {
        const controller = document.getElementsByTagName("span")[1];
        const target = document.getElementsByTagName("span")[2];
        // TODO: Click controller here
        // controller.click();
        it('should make the controller have class "collapsed"', function() {
          chai.assert.isTrue(controller.classList.contains("collapsed"));
        });

        it('should make the controller have aria-expanded = false', function() {
          chai.assert.equal(controller.getAttribute("aria-expanded"), "false");
        });

        it('should cause the target to not have the class "show"', function() {
          chai.assert.isFalse(target.classList.contains("show"));
        });
      });

      // function sleepFor( sleepDuration ){
      //     var now = new Date().getTime();
      //     while(new Date().getTime() < now + sleepDuration){ /* do nothing */ } 
      // }

      describe("Controller 2nd click, expand target", function() {
        const controller = document.getElementsByTagName("span")[1];
        const target = document.getElementsByTagName("span")[2];
        // TODO: Click controller here
        // controller.click();
        it('should make the controller not have class "collapsed"', function() {
          chai.assert.isFalse(controller.classList.contains("collapsed"));
        });

        it('should make the controller have aria-expanded = true', function() {
          chai.assert.equal(controller.getAttribute("aria-expanded"), "true");
        });

        it('should cause the target to have the class "show"', function() {
          chai.assert.isTrue(target.classList.contains("show"));
        });
      });
    });
  }
};


emp::web::MochaTestRunner test_runner;
int main() {

  test_runner.Initialize({"emp_test_container"});

  test_runner.AddTest<Test_Collapse_Click>("Test emp::prefab::Collapse for Click Response");
  test_runner.Run();
}
