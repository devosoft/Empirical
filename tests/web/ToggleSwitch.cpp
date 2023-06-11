/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file ToggleSwitch.cpp
 */

#include <functional>
#include <unordered_map>

#include "emp/base/assert.hpp"
#include "emp/prefab/ToggleSwitch.hpp"
#include "emp/web/Div.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/web.hpp"

struct Test_ToggleSwitch_init_on : emp::web::BaseTest {

  /*
   * Construct the following HTML structure:
   *
   * <span id="switch_on" style="clear: none; display: inline;" class="custom-control custom-switch">
   *  <input type="checkbox" onchange="emp.Callback(32, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" checked="checked" class="custom-control-input">
   *  <label class="custom-control-label" for="[input_id]">
   *    <span>Switch Defult On</span>
   *  </label>
   * </span>
   */
  Test_ToggleSwitch_init_on()
  : BaseTest({"emp_test_container"})
  {
    emp::prefab::ToggleSwitch on_switch([](std::string val){},"Switch Defult On", true, "switch_on");
    Doc("emp_test_container") << on_switch;
  }

  void Describe() override {

    // Test that the HTML components created in constructor are correct.
    EM_ASM({
      const input_id = document.getElementsByTagName("input")[0].getAttribute("id");
      const label_id = document.getElementsByTagName("label")[0].getAttribute("id");

      describe("emp::prefab::ToggleSwitch Default On", function() {
        describe("span#switch_on", function() {
          it('should have parent #emp_test_container', function() {
            const parent_id = $("span#switch_on").parent().attr("id");
            chai.assert.equal(parent_id, "emp_test_container");
          });

          const t_switch = document.getElementById("switch_on");

          it('should have two children', function() {
            const num_children = t_switch.childElementCount;
            chai.assert.equal(num_children, 2);
          });

          it('should have class custom-control', function() {
            chai.assert.isTrue(t_switch.classList.contains("custom-control"));
          });

          it('should have class custom-switch', function() {
            chai.assert.isTrue(t_switch.classList.contains("custom-switch"));
          });

        });

        describe("input element", function() {
          it('should have parent #switch_on', function() {
            const parent_id = $("#"+input_id).parent().attr("id");
            chai.assert.equal(parent_id, "switch_on");
          });

          const input_element = document.getElementById(input_id);

          it('should have type checkbox', function() {
            chai.assert.equal(input_element.getAttribute("type"), "checkbox");
          });

          it('should have attribute checked', function() {
            chai.assert.notEqual(input_element.getAttribute("checked"), undefined);
          });

          it('should have class custom-control-input', function() {
            chai.assert.isTrue(input_element.classList.contains("custom-control-input"));
          });
        });

        describe("label element", function() {
          const label_element = document.getElementById(label_id);

          it('should have parent #switch_on', function() {
            const parent_id = label_element.parentNode.id;
            chai.assert.equal(parent_id, "switch_on");
          });

          it('should have one span child', function() {
            chai.assert.equal(label_element.childElementCount, 1);
            chai.assert.equal(label_element.children[0].nodeName, "SPAN");
          });

          it('should have class custom-control-label', function() {
            chai.assert.isTrue(label_element.classList.contains("custom-control-label"));
          });

          it('should have for attribute equal to input id', function() {
            chai.assert.equal(label_element.getAttribute("for"), input_id);
          });

        });

      });
    });
  }

};
struct Test_ToggleSwitch_init_off : emp::web::BaseTest {

  /*
   * Construct the following HTML structure:
   *
   * <span id="switch_off" style="clear: none; display: inline;" class="custom-control custom-switch">
   *     <input type="checkbox" onchange="emp.Callback(32, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" class="custom-control-input">
   *     <label class="custom-control-label">
   *         <span>Switch Defult Off</span>
   *     </label>
   * </span>
   */
  Test_ToggleSwitch_init_off()
  : BaseTest({"emp_test_container"})
  {
    emp::prefab::ToggleSwitch off_switch([](std::string val){},"Switch Defult Off", false, "switch_off");
    Doc("emp_test_container") << off_switch;
  }

  void Describe() override {

    // Test that the HTML components created in constructor are correct.
    EM_ASM({
      const input_id = document.getElementsByTagName("input")[0].getAttribute("id");
      const label_id = document.getElementsByTagName("label")[0].getAttribute("id");

      describe("emp::prefab::ToggleSwitch Default Off", function() {
        describe("span#switch_off", function() {
          it('should have parent #emp_test_container', function() {
            const parent_id = $("span#switch_off").parent().attr("id");
            chai.assert.equal(parent_id, "emp_test_container");
          });

          const t_switch = document.getElementById("switch_off");

          it('should have two children', function() {
            const num_children = t_switch.childElementCount;
            chai.assert.equal(num_children, 2);
          });

          it('should have class custom-control and custom-switch', function() {
            chai.assert.isTrue(t_switch.classList.contains("custom-control"));
          });

          it('should have class custom-switch', function() {
            chai.assert.isTrue(t_switch.classList.contains("custom-switch"));
          });

        });

        describe("input element", function() {
          it('should have parent #switch_off', function() {
            const parent_id = $("#"+input_id).parent().attr("id");
            chai.assert.equal(parent_id, "switch_off");
          });

          const input_element = document.getElementById(input_id);

          it('should have type checkbox', function() {
            chai.assert.equal(input_element.getAttribute("type"), "checkbox");
          });

          it('should not have attribute checked', function() {
            chai.assert.equal(input_element.getAttribute("checked"), undefined);
          });

          it('should have class custom-control-input', function() {
            chai.assert.isTrue(input_element.classList.contains("custom-control-input"));
          });
        });

        describe("label element", function() {
          it('should have parent #switch_off', function() {
            const parent_id = document.getElementById(label_id).parentNode.id;
            chai.assert.equal(parent_id, "switch_off");
          });

          const label_element = document.getElementById(label_id);

          it('should have one span child', function() {
            chai.assert.equal(label_element.childElementCount, 1);
            chai.assert.equal(label_element.children[0].nodeName, "SPAN");
          });

          it('should have class custom-control-label', function() {
            chai.assert.isTrue(label_element.classList.contains("custom-control-label"));
          });

          it('should have "for" attribute equal to input id', function() {
            chai.assert.equal(label_element.getAttribute("for"), input_id);
          });

        });

      });
    });
  }

};
struct Test_ToggleSwitch_add_label : emp::web::BaseTest {

  /*
   * Construct the following HTML structure:
   *
   * <span id="switch" style="clear: none; display: inline;" class="custom-control custom-switch">
   *  <input type="checkbox" onchange="emp.Callback(32, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" class="custom-control-input">
   *  <label class="custom-control-label">
   *    <div id="added_label">
   *      <span><h3>Text</h3></span>
   *    </div>
   *  </label>
   * </span>
   */
  Test_ToggleSwitch_add_label()
  : BaseTest({"emp_test_container"})
  {
    emp::prefab::ToggleSwitch my_switch([](std::string val){},"", false, "switch");
    Doc("emp_test_container") << my_switch;
    emp::web::Div my_label("added_label");
    my_switch.AddLabel(my_label);
    my_label << "<h3>Text</h3>";
  }

  void Describe() override {

    // Test that the HTML components created in constructor are correct.
    EM_ASM({
      const input_id = document.getElementsByTagName("input")[0].getAttribute("id");
      const label_id = document.getElementsByTagName("label")[0].getAttribute("id");

      describe("emp::prefab::ToggleSwitch AddLabel()", function() {
        describe("span#switch", function() {
          it('should have parent #emp_test_container', function() {
            const parent_id = $("span#switch").parent().attr("id");
            chai.assert.equal(parent_id, "emp_test_container");
          });

          const t_switch = document.getElementById("switch");

          it('should have two children', function() {
            chai.assert.equal(t_switch.childElementCount, 2);
          });

          it('should have class custom-control', function() {
            chai.assert.isTrue(t_switch.classList.contains("custom-control"));
          });

          it('should have class custom-switch', function() {
            chai.assert.isTrue(t_switch.classList.contains("custom-switch"));
          });

        });

        describe("input element", function() {
          it('should have parent #switch', function() {
            const parent_id = $("#"+input_id).parent().attr("id");
            chai.assert.equal(parent_id, "switch");
          });

          const input_element = document.getElementById(input_id);

          it('should have type checkbox', function() {
            chai.assert.equal(input_element.getAttribute("type"), "checkbox");
          });

          it('should not have attribute checked', function() {
            chai.assert.equal(input_element.getAttribute("checked"), undefined);
          });

          it('should have class custom-control-input', function() {
            chai.assert.isTrue(input_element.classList.contains("custom-control-input"));
          });
        });

        describe("label element", function() {
          it('should have parent #switch', function() {
            const parent_id = document.getElementById(label_id).parentNode.id;
            chai.assert.equal(parent_id, "switch");
          });

          const label_element = document.getElementById(label_id);

          it('should have one child', function() {
            chai.assert.equal(label_element.childElementCount, 1);
          });

          it('the child is the div added with AddLabel()', function() {
            chai.assert.equal(label_element.children[0].nodeName, "DIV");
          });

          it('should have class custom-control-label', function() {
            chai.assert.isTrue(label_element.classList.contains("custom-control-label"));
          });

          it('should have "for" attribute equal to input id', function() {
            chai.assert.equal(label_element.getAttribute("for"), input_id);
          });

        });

        describe("div#added_label", function() {
          it('should have parent label element', function() {
            const parent_id = document.getElementById("added_label").parentNode.id;
            chai.assert.equal(parent_id, label_id);
          });

          it('should have one span child', function() {
            chai.assert.equal($("#added_label").children().length, 1);
            chai.assert.equal($("#added_label").children()[0].nodeName, "SPAN");
          });

        });

        describe("span in div#added_label", function() {
          const span_id = $("#added_label").children()[0].id;
          it('should have parent label element', function() {
            const parent_id = document.getElementById(span_id).parentNode.id;
            chai.assert.equal(parent_id, "added_label");
          });

          it('should have one h3 child', function() {
            chai.assert.equal($("#"+span_id).children().length, 1);
            chai.assert.equal($("#"+span_id).children()[0].nodeName, "H3");
          });

        });

      });
    });
  }

};
emp::web::MochaTestRunner test_runner;
int main() {
  test_runner.Initialize({"emp_test_container"});

  test_runner.AddTest<Test_ToggleSwitch_init_on>("Test emp::prefab::ToggleSwitch default on");
  test_runner.AddTest<Test_ToggleSwitch_init_off>("Test emp::prefab::ToggleSwitch default off");
  test_runner.AddTest<Test_ToggleSwitch_add_label>("Test emp::prefab::ToggleSwitch test AddLabel()");

  test_runner.Run();
}
