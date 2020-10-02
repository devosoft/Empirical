//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <functional>
#include <unordered_map>

#include "base/assert.h"
#include "web/_MochaTestRunner.h"
#include "web/Document.h"
#include "web/web.h"

#include "web/Button.h"

int x = 0; 

// Test that the user can trigger multiple clicks of a web element.
struct Test_Click : public emp::web::BaseTest {
  // Construct the following HTML structure:
  /**
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
            chai.assert.equal(value.textContent, "2");
          }, 1500);
        });
      });

    }); // end EM_ASM
  }

};

// Create a MochaTestRunner object in the global namespace so that it hangs around after main finishes.
emp::web::MochaTestRunner test_runner;

int main() {

  test_runner.Initialize({"emp_test_container"});

  test_runner.AddTest<Test_Click>("Test Increment Button");

  test_runner.Run();
}
