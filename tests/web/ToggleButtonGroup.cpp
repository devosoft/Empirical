//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2021.
//  Released under the MIT Software license; see doc/LICENSE

#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/Document.hpp"
#include "emp/prefab/ToggleButtonGroup.hpp"
#include "emp/prefab/FontAwesomeIcon.hpp"

struct Test_Toggle_Button_Group : emp::web::BaseTest {
  Test_Toggle_Button_Group()
  : BaseTest({"emp_test_container"}) {
    emp::prefab::ToggleButtonGroup icon_and_string(
      emp::prefab::FontAwesomeIcon{"fa-play"}, "Pause",
      "primary", "secondary",
      true, false, "icon_and_string"
    );
    Doc("emp_test_container") << icon_and_string;
  }

  void Describe() override {
    EM_ASM({
      describe("ToggleButtonGroup HTML Layout", function() {
        const toggle = document.getElementById("icon_and_string");
        it("should exists", function() {
          chai.assert.isNotNull(toggle);
        });
        it("should have parent #emp_test_container", function() {
          chai.assert.equal(toggle.parentElement.getAttribute("id"), "emp_test_container");
        });
        it("should have 2 children", function() {
          chai.assert.equal(toggle.childElementCount, 2);
        });
        describe("first label (activate)", function() {
          const label1 = toggle.children[0];
          it("should have ID #icon_and_string_activate", function() {
            chai.assert.equal(label1.getAttribute("id"), "icon_and_string_activate");
          });
          it("should have two children", function() {
            chai.assert.equal(label1.childElementCount, 2);
          });
          const activate_radio = label1.children[0];
          it('should have first child be a radio input', function() {
            chai.assert.equal(activate_radio.nodeName, "INPUT");
          });
          const activate_indicator = label1.children[1];
          it('should have second child be a span', function() {
            chai.assert.equal(activate_indicator.nodeName, "SPAN");
          });
        });
        describe("second label (deactivate)", function() {
          const label2 = toggle.children[1];
          it("should have ID #icon_and_string_deactivate", function() {
            chai.assert.equal(label2.getAttribute("id"), "icon_and_string_deactivate");
          });
          it("should have two children", function() {
            chai.assert.equal(label2.childElementCount, 2);
          });
          const deactivate_radio = label2.children[0];
          it('should have first child be a radio input', function() {
            chai.assert.equal(deactivate_radio.nodeName, "INPUT");
          });
          const deactivate_indicator = label2.children[1];
          it('should have second child be a span', function() {
            chai.assert.equal(deactivate_indicator.nodeName, "SPAN");
          });
        });
      });
    });
  }
};

emp::web::MochaTestRunner test_runner;
int main() {
  test_runner.Initialize({"emp_test_container"});
  test_runner.AddTest<Test_Toggle_Button_Group>("Test emp::prefab::ToggleButtonGroup HTML Layout");
  test_runner.Run();
}
