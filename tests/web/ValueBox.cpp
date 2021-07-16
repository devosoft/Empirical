//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <functional>
#include <unordered_map>

#include "emp/base/assert.hpp"
#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/Element.hpp"
#include "emp/web/web.hpp"

#include "emp/prefab/ValueBox.hpp"

// Test that the ValueBox class is constructed correctly
struct Test_Value_Box_HTMLLayout : public emp::web::BaseTest {

  Test_Value_Box_HTMLLayout()
  : BaseTest({"emp_test_container"}) {
    Doc("emp_test_container") << emp::prefab::ValueBox("label", "description", "value_box");
  }

  void Describe() override {
    EM_ASM({
      describe("emp::prefab::ValueBox HTML Layout Scenario", function() {
        const value_display = document.getElementById("value_box");

        describe("Value box (#value_box)", function() {
          it('should exist', function() {
            chai.assert.notEqual(value_box, null);
          });

          it('should have parent #emp_test_container', function() {
            const parent_id = value_box.parentElement.id;
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have 3 children', function() {
            chai.assert.equal(value_box.childElementCount, 3);
          });
        });
      });
    });
  }
};

// Test that the ValueDisplay class is constructed correctly
struct Test_Value_Display_HTMLLayout : public emp::web::BaseTest {

  Test_Value_Display_HTMLLayout()
  : BaseTest({"emp_test_container"}) {
    Doc("emp_test_container") << emp::prefab::ValueDisplay("label", "description", "value", "value_display");
  }

  void Describe() override {
    EM_ASM({
      describe("emp::prefab::ValueDisplay HTML Layout Scenario", function() {
        const value_display = document.getElementById("value_display");

        describe("#value_display", function() {
          it('should exist', function() {
            chai.assert.notEqual(value_display, null);
          });

          it('should have parent #emp_test_container', function() {
            const parent_id = value_display.parentElement.id;
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have 3 children', function() {
            chai.assert.equal(value_display.childElementCount, 3);
          });

          describe("view", function() {
            const value_view = value_display.children[2];
            it('should exist', function() {
              chai.assert.notEqual(value_view, null);
            });

            it("should have a child", function() {
              chai.assert.equal(value_view.childElementCount, 1);
            });

            describe("view's child", function() {
              it("should be a span", function() {
                chai.assert(value_display.children[0].nodeName, "SPAN");
              });
            });
          });
        });
      });
    });
  }
};

emp::web::MochaTestRunner test_runner;

int main() {
  test_runner.Initialize({"emp_test_container"});

  test_runner.AddTest<Test_Value_Box_HTMLLayout>("Test ValueBox HTML Layout");
  test_runner.AddTest<Test_Value_Display_HTMLLayout>("Test ValueDisplay HTML Layout");

  test_runner.Run();
}