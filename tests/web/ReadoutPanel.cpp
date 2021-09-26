/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file ReadoutPanel.cpp
 */

#include "emp/base/assert.hpp"
#include "emp/prefab/ReadoutPanel.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/web.hpp"

int num_anim_steps;
int live_variable;
 /*
  * TODO: can't figure out which events happen in what order so unfortunately
  * even with stopping animation, can't guarantee num_anim_steps is zero
  * when asserts are called. Similarly for after updating live_variable, doesn't
  * seem like asserts detect it.
  */

struct Test_Readout_Panel_HTMLLayout : public emp::web::BaseTest {

  emp::prefab::ReadoutPanel readout_panel;
  // A value to test as a live variable in the panel

  Test_Readout_Panel_HTMLLayout()
  : BaseTest({"emp_test_container"}),
  readout_panel{"Readout", 10000, "INIT_OPEN", true, "readout"} {
    // For the sake of control, manually step animation forward only
    readout_panel.Animate(readout_panel.GetID()).Stop();
    // Set flags since AdvanceFrame has been called at least once
    num_anim_steps = 0;
    live_variable = 0;

    Doc("emp_test_container") << readout_panel;

    // A function to test live variables in panel
    std::function<std::string()> get_anim_steps =
    [&]() mutable {
      return emp::to_string(++num_anim_steps);
    };
    readout_panel.AddValues(
      "Counter function", "A function that counts upwards every call", get_anim_steps,
      "Counter variable", "A variable we increment", live_variable
    );
  }

  void Describe() override {
    EM_ASM({
      describe("emp::prefab::ValueBox HTML Layout Scenario", function() {
        const readout_panel = document.getElementById("readout");
        it('should exist', function() {
          chai.assert.isNotNull(readout_panel);
        });
        describe("#readout_values", function() {
          const values_div = document.getElementById("readout_values");
          it('should exist', function() {
            chai.assert.isNotNull(values_div);
          });
          it('should have 2 children', function() {
            chai.assert.equal(values_div.childElementCount, 2);
          });
          describe('first child', function() {
            const first = values_div.children[0];
            it('should exist', function() {
              chai.assert.isNotNull(first);
            });
            it('should have ID "readout_counter_function"', function() {
              chai.assert.equal(first.id, "readout_counter_function");
            });
            describe('value view', function() {
              const first_val = document.getElementById("readout_counter_function_view").children[0];
              it('should have a span', function() {
                chai.assert.equal(first_val.nodeName, "SPAN");
              });
            });
          });
          describe('second child', function() {
            const second = values_div.children[1];
            it('should exist', function() {
              chai.assert.isNotNull(second);
            });
            it('should have ID "readout_counter_variable"', function() {
              chai.assert.equal(second.id, "readout_counter_variable");
            });
            describe('value view', function() {
              const second_val = document.getElementById("readout_counter_variable_view").children[0];
              it('should have a span', function() {
                chai.assert.equal(second_val.nodeName, "SPAN");
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
  test_runner.AddTest<Test_Readout_Panel_HTMLLayout>("Test ReadoutPanel HTMLLayout");
  test_runner.Run();
}
