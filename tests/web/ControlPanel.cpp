//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2021.
//  Released under the MIT Software license; see doc/LICENSE

#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/Div.hpp"
#include "emp/prefab/ButtonGroup.hpp"
#include "emp/prefab/ToggleButtonGroup.hpp"
#include "emp/prefab/ControlPanel.hpp"

// Tests the integration between the Control Panel, Buttons, Button Groups,
// and ToggleButtonGroups
struct Test_Control_Panel : emp::web::BaseTest {
/*
 * Creates extra div + the following control panel structure
 *    +------------------+------+-----+-------------------+  +---+---+---+  +--+
 *    | +------+-------+ |      |     | +------+--------+ |  |   |   |   |  |  |
 *    | | Play | Pause | | Step |  C  | | Auto | Manual | |  | D | A | B |  |  |
 *    | +------+-------+ |      |     | +------+--------+ |  |   |   |   |  |  |
 *    +------------------+------+-----+-------------------+  +---+---+---+  +--+
 *     ToggleButtonGroup (default) ToggleButtonGroup (added)
 */
  Test_Control_Panel()
  : BaseTest({"emp_test_container"}) {

    emp::prefab::ControlPanel cp{5, "FRAMES", "ctrl"};
    emp::web::Div sim_area{"sim_area"};
    cp.AddToRefreshList(sim_area);
    cp.SetRefreshRate(500, "MILLISECONDS");

    emp::prefab::ButtonGroup husk{"husk"};
    husk << emp::web::Button{[](){;}, "A", "a_button"};
    husk << emp::web::Button{[](){;}, "B", "b_button"};

    cp << emp::web::Button([](){;}, "C", "c_button");
    emp::prefab::ToggleButtonGroup toggle{
      "Auto", "Manual",
      "primary", "secondary",
      true, false,
      "mode_toggle"
    };
    cp << toggle;
    toggle.SetCallback([](bool active) {
      if (active) {
        std::cout << "Auto!" << std::endl;
      } else {
        std::cout << "Manual!" << std::endl;
      }
    });
    emp::prefab::ButtonGroup real{"real"};
    real << emp::web::Button([](){;}, "D", "d_button");
    real.TakeChildren(husk);
    cp << real;
    cp << husk;

    Doc("emp_test_container") << sim_area;
    Doc("emp_test_container") << cp;
  }

  void Describe() override {
    EM_ASM({
      describe("Control Panel HTML layout", function() {
        const cp = document.getElementById('ctrl');
        it("should have three children (3 main button groups)", function() {
          chai.assert.equal(cp.childElementCount, 3);
        });

        describe("first button group (#ctrl_main)", function() {
          const bg1 = document.getElementById('ctrl_main');
          it("should exist", function() {
            chai.assert.isNotNull(bg1);
          });
          it("should have control panel (#ctrl) as parent", function() {
            chai.assert.equal(bg1.parentElement.getAttribute("id"), "ctrl");
          });
          describe("group's children", function() {
            // has 2 toggle button group bookending two buttons
            it("has children elements: div, 2 buttons, div", function() {
              const nodeNames = Array.from(bg1.children).map(child => child.nodeName);
              chai.assert.deepEqual(nodeNames, ["DIV", "BUTTON", "BUTTON", "DIV"]);
            });
            it("has main toggle button group (#ctrl_main_toggle)", function() {
              const main_toggle = document.getElementById('ctrl_main_toggle');
              chai.assert.equal(main_toggle.parentElement.getAttribute("id"), "ctrl_main");
            });
            it("has step button (#ctrl_main_step)", function() {
              const step = document.getElementById('ctrl_main_step');
              chai.assert.equal(step.parentElement.getAttribute("id"), "ctrl_main");
            });
            it("has C button (#c_button)", function() {
              const c_button = bg1.children[2];
              chai.assert.equal(c_button.getAttribute("id"), "c_button");
            });
            it("has auto/manual toggle", function() {
              const mode_toggle = bg1.children[3];
              chai.assert.equal(mode_toggle.getAttribute("id"), "mode_toggle");
            });
          });
        });

        describe("second button group (#real)", function() {
          const bg2 = document.getElementById('real');
          it("should exist", function() {
            chai.assert.isNotNull(bg2);
          });
          it("should have control panel (#ctrl) as parent", function() {
            chai.assert.equal(bg2.parentElement.getAttribute("id"), "ctrl");
          });
          it("should have 3 children (due to TakeChildren)", function() {
            chai.assert.equal(bg2.childElementCount, 3);
          });
          describe("group's children", function() {
            it("has only buttons", function() {
              const nodeNames = Array.from(bg2.children).map(child => child.nodeName);
              chai.assert.deepEqual(nodeNames, ["BUTTON", "BUTTON","BUTTON"]);
            });
            it("has first child D (#d_button)", function() {
              const d_button = bg2.children[0];
              chai.assert.equal(d_button.getAttribute("id"), "d_button");
            });
            it("has second child A (#a_button)", function() {
              const a_button = bg2.children[1];
              chai.assert.equal(a_button.getAttribute("id"), "a_button");
            });
            it("has third child B (#b_button)", function() {
              const b_button = bg2.children[2];
              chai.assert.equal(b_button.getAttribute("id"), "b_button");
            });
          });
        });

        describe("third button group (#husk)", function() {
          const bg3 = document.getElementById('husk');
          it("should exist", function() {
            chai.assert.isNotNull(bg3);
          });
          it("should have control panel (#ctrl) as parent", function() {
            chai.assert.equal(bg3.parentElement.getAttribute("id"), "ctrl");
          });
          it("should have no children (due to TakeChildren)", function() {
            chai.assert.equal(bg3.childElementCount, 0);
          });
        });
      });
    });
  }
};

emp::web::MochaTestRunner test_runner;
int main() {
  test_runner.Initialize({"emp_test_container"});
  test_runner.AddTest<Test_Control_Panel>("Test emp::prefab::ControlPanel HTML Layout");
  test_runner.Run();
}

