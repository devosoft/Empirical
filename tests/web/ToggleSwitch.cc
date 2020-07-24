//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <functional>
#include <unordered_map>

#include "base/assert.h"
#include "web/_MochaTestRunner.h"
#include "web/Document.h"
#include "web/web.h"
#include "web/Div.h"

#include "prefab/ToggleSwitch.h"


struct Test_ToggleSwitch_init_on : emp::web::BaseTest {
  // Construct the following HTML structure:
  /**
    <span id="switch_on" style="clear: none; display: inline;" class="custom-control custom-switch">
        <input type="checkbox" onchange="emp.Callback(32, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" checked="checked" class="custom-control-input">
        <label class="custom-control-label" for="[input_id]">
            <span>Switch Defult On</span>
        </label>
    </span>
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

          it('should have two children', function() {
            const num_children = document.getElementById("switch_on").childElementCount;
            chai.assert.equal(num_children, 2);
          });

          it('should have custom-control and custom-switch classes', function() {
            const classes = document.getElementById("switch_on").className;
            chai.assert.equal(classes, "custom-control custom-switch");
          });

        });
        
        describe("input element", function() {
          it('should have parent #switch_on', function() {
            const parent_id = $("#"+input_id).parent().attr("id");
            chai.assert.equal(parent_id, "switch_on");
          });

          it('should have type checkbox', function() {
            chai.assert.equal($("#"+input_id).attr("type"), "checkbox");
          });

          it('should have attribute checked', function() {
            chai.assert.notEqual($("#"+input_id).attr("checked"), undefined);
          });

          it('should have custom-control-input class', function() {
            chai.assert.equal($("#"+input_id).attr("class"), "custom-control-input");
          });
        });

        describe("label element", function() {
          it('should have parent #switch_on', function() {                       /// fail (undefined)
            const parent_id = document.getElementById(label_id).parentNode.id;
            chai.assert.equal(parent_id, "switch_on");
          });

          it('should have one span child', function() {                       /// fail (0)
            chai.assert.equal($("#"+label_id).children().length, 1);
            chai.assert.equal($("#"+label_id).children()[0].nodeName, "SPAN");
          });

          it('should have custom-control-label class', function() {
            chai.assert.equal($("#"+label_id).attr("class"), "custom-control-label");
          });

          it('should have for attribute equal to input id', function() {                       /// fail (undefined)
            chai.assert.equal($("#"+label_id).attr("for"), input_id);
          });

        });

      });
    });
  }

};
struct Test_ToggleSwitch_init_off : emp::web::BaseTest {

  // Construct the following HTML structure:
  /**
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

          it('should have two children', function() {
            const num_children = document.getElementById("switch_off").childElementCount;
            chai.assert.equal(num_children, 2);
          });

          it('should have custom-control and custom-switch classes', function() {
            const classes = document.getElementById("switch_off").className;
            chai.assert.equal(classes, "custom-control custom-switch");
          });

        });

        describe("input element", function() {
          it('should have parent #switch_off', function() {
            const parent_id = $("#"+input_id).parent().attr("id");
            chai.assert.equal(parent_id, "switch_off");
          });

          it('should have type checkbox', function() {
            chai.assert.equal($("#"+input_id).attr("type"), "checkbox");
          });

          it('should not have attribute checked', function() {
            chai.assert.equal($("#"+input_id).attr("checked"), undefined);
          });

          it('should have custom-control-input class', function() {
            chai.assert.equal($("#"+input_id).attr("class"), "custom-control-input");
          });
        });

        describe("label element", function() {
          it('should have parent #switch_off', function() {
            const parent_id = document.getElementById(label_id).parentNode.id;
            chai.assert.equal(parent_id, "switch_off");
          });

          it('should have one span child', function() {
            chai.assert.equal($("#"+label_id).children().length, 1);
            chai.assert.equal($("#"+label_id).children()[0].nodeName, "SPAN");
          });

          it('should have custom-control-label class', function() {
            chai.assert.equal($("#"+label_id).attr("class"), "custom-control-label");
          });

          it('should have "for" attribute equal to input id', function() {
            chai.assert.equal($("#"+label_id).attr("for"), input_id);
          });

        });

      });
    });
  }

};
struct Test_ToggleSwitch_add_label : emp::web::BaseTest {


  // Construct the following HTML structure:
  /**
    <span id="switch" style="clear: none; display: inline;" class="custom-control custom-switch">
        <input type="checkbox" onchange="emp.Callback(32, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" class="custom-control-input">
        <label class="custom-control-label">
            <div id="added_label">
              <span><h3>Text</h3></span>
            </div>
        </label>
    </span>
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

          it('should have two children', function() {
            const num_children = document.getElementById("switch").childElementCount;
            chai.assert.equal(num_children, 2);
          });

          it('should have custom-control and custom-switch classes', function() {
            const classes = document.getElementById("switch").className;
            chai.assert.equal(classes, "custom-control custom-switch");
          });

        });

        describe("input element", function() {
          it('should have parent #switch', function() {
            const parent_id = $("#"+input_id).parent().attr("id");
            chai.assert.equal(parent_id, "switch");
          });

          it('should have type checkbox', function() {
            chai.assert.equal($("#"+input_id).attr("type"), "checkbox");
          });

          it('should not have attribute checked', function() {
            chai.assert.equal($("#"+input_id).attr("checked"), undefined);
          });

          it('should have custom-control-input class', function() {
            chai.assert.equal($("#"+input_id).attr("class"), "custom-control-input");
          });
        });

        describe("label element", function() {
          it('should have parent #switch', function() {
            const parent_id = document.getElementById(label_id).parentNode.id;
            chai.assert.equal(parent_id, "switch");
          });

          it('should have one child', function() {
            chai.assert.equal($("#"+label_id).children().length, 1);
          });

          it('the child is the div added with AddLabel()', function() {
            chai.assert.equal($("#"+label_id).children()[0].nodeName, "DIV");
          });

          it('should have custom-control-label class', function() {
            chai.assert.equal($("#"+label_id).attr("class"), "custom-control-label");
          });

          it('should have "for" attribute equal to input id', function() {
            chai.assert.equal($("#"+label_id).attr("for"), input_id);
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
