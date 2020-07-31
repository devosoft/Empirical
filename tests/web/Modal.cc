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

#include "prefab/Modal.h"


struct Test_Modal : emp::web::BaseTest {
  // Construct the following HTML structure:
  /**
    <span id="switch_on" style="clear: none; display: inline;" class="custom-control custom-switch">
        <input type="checkbox" onchange="emp.Callback(32, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" checked="checked" class="custom-control-input">
        <label class="custom-control-label" for="[input_id]">
            <span>Switch Defult On</span>
        </label>
    </span>
 */
  Test_Modal()
  : BaseTest({"emp_test_container"})
  {
    emp::prefab::Modal modal("modal");
    Doc("emp_test_container") << modal;

    modal.AddHeaderContent("<h3>Modal Header Section</h3>");
    modal.AddBodyContent("This is the content of the modal");

    modal.AddFooterContent("Modal Footer Section");
    emp::web::Button close_btn([](){;}, "Close");
    close_btn.SetAttr("class", "btn btn-secondary");
    modal.AddFooterContent(close_btn);
    modal.AddButton(close_btn);

    modal.AddClosingX();

    emp::web::Button modal_btn([](){;}, "Show Modal", "modal_trigger");
    Doc("emp_test_container") << modal_btn;
    modal_btn.SetAttr("class", "btn btn-primary");
    modal.AddButton(modal_btn);
  }

  void Describe() override {

    // Test that the HTML components created in constructor are correct.
    EM_ASM({
      describe("emp::prefab::Modal Initial HTML Structure", function() {
        const dialog_id = document.getElementById("modal").children[0].id;
        const content_id = document.getElementById(dialog_id).children[0].id;
        const header_id = document.getElementById(content_id).children[0].id;
        const body_id = document.getElementById(content_id).children[1].id;
        const footer_id = document.getElementById(content_id).children[2].id;

        describe("div#emp_test_container", function() {
          it('should have two children', function() {
            const num_children = document.getElementById("emp_test_container").childElementCount;
            chai.assert.equal(num_children, 2);
          });

          it('first child should be div#modal', function() {
            const first_child = document.getElementById("emp_test_container").children[0].id;
            chai.assert.equal(first_child, "modal");
          });

          it('second child should be button#modal_trigger', function() {
            const first_child = document.getElementById("emp_test_container").children[1].id;
            chai.assert.equal(first_child, "modal_trigger");
          });
        });

        describe("div#modal", function() {
          it('should have parent #emp_test_container', function() {
            const parent_id = document.getElementById("modal").parentNode.id;
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have one child', function() {
            const num_children = document.getElementById("modal").childElementCount;
            chai.assert.equal(num_children, 1);
          });

          it('should have "modal" class', function() {
            const classes = document.getElementById("modal").className;
            chai.assert.equal(classes, "modal");
          });

          it('should have static data-backdrop', function() {
            const backdrop = document.getElementById("modal").getAttribute("data-backdrop");
            chai.assert.equal(backdrop, "static");
          });

          it('should have -1 tabindex', function() {
            const tabindex = document.getElementById("modal").getAttribute("tabindex");
            chai.assert.equal(tabindex, "-1");
          });

          it('should not have aria-hidden set yet', function() { // fail [null]
            const aria_hidden = $("#modal").attr("aria-hidden");
            chai.assert.equal(aria_hidden, undefined);
          });

          it('should not have style set yet', function() { // fail [null]
            const style = $("#modal").attr("style");
            chai.assert.equal(style, undefined);
          });
        });

        describe(".modal-dialog", function() {
          it('should have parent #modal', function() {
            const parent_id = document.getElementById(dialog_id).parentNode.id;
            chai.assert.equal(parent_id, "modal");
          });

          it('should have one child', function() {
            const num_children = document.getElementById(dialog_id).childElementCount;
            chai.assert.equal(num_children, 1);
          });

          it('should have modal-dialog class', function() {
              const classes = document.getElementById(dialog_id).className;
            chai.assert.equal(classes, "modal-dialog");
          });
        });

        describe(".modal-content", function() {
          it('should have parent #modal', function() {
            const parent_id = document.getElementById(content_id).parentNode.id;
            chai.assert.equal(parent_id, dialog_id);
          });

          it('should have three children', function() {
            const num_children = document.getElementById(content_id).childElementCount;
            chai.assert.equal(num_children, 3);
          });

          it('should have modal-content class', function() {
              const classes = document.getElementById(content_id).className;
            chai.assert.equal(classes, "modal-content");
          });
        });

        describe(".modal-header", function() {
          it('should have parent .modal-content', function() {
            const parent_id = document.getElementById(header_id).parentNode.id;
            chai.assert.equal(parent_id, content_id);
          });

          it('should have two children', function() {
            const num_children = document.getElementById(header_id).childElementCount;
            chai.assert.equal(num_children, 2);
          });

          it('should have modal-header class', function() {
            const classes = document.getElementById(header_id).className;
            chai.assert.equal(classes, "modal-header");
          });

          describe("Header Text", function() {
            const title_id = document.getElementById(header_id).children[0].id;

            it('should have parent .modal-header', function() {
              const parent_id = document.getElementById(title_id).parentNode.id;
              chai.assert.equal(parent_id, header_id);
            });

            it('should be a span element', function() {
              const element_type = document.getElementById(title_id).nodeName;
              chai.assert.equal(element_type, "SPAN");
            });

            it('should have one child', function() {
              const num_children = document.getElementById(title_id).childElementCount;
              chai.assert.equal(num_children, 1);
            });

            it('should have one child with element type h3', function() {
              const child_ele = document.getElementById(title_id).children[0].nodeName;
              chai.assert.equal(child_ele, "H3");
            });
          });

          describe("X Close Button", function() {
            const btn_id = document.getElementById(header_id).children[1].id;

            it('should have parent .modal-header', function() {
              const parent_id = document.getElementById(btn_id).parentNode.id;
              chai.assert.equal(parent_id, header_id);
            });

            it('should be a button element', function() {
              const element_type = document.getElementById(btn_id).nodeName;
              chai.assert.equal(element_type, "BUTTON");
            });

            it('should have no children', function() {
              const num_children = document.getElementById(btn_id).childElementCount;
              chai.assert.equal(num_children, 0);
            });

            it('should have classes "close" and "float-right"', function() {
              const classes = document.getElementById(btn_id).className;
              chai.assert.equal(classes, "close float-right");
            });

            it('should have data-dismiss = modal', function() {
              const dismiss = document.getElementById(btn_id).getAttribute("data-dismiss");
              chai.assert.equal(dismiss, "modal");
            });

            it('should have aria-label = Close', function() {
              const label = document.getElementById(btn_id).getAttribute("aria-label");
              chai.assert.equal(label, "Close");
            });
          });
        });


        describe(".modal-body", function() {
          it('should have parent .modal-content', function() {
            const parent_id = document.getElementById(body_id).parentNode.id;
            chai.assert.equal(parent_id, content_id);
          });

          it('should have one child', function() {
            const num_children = document.getElementById(body_id).childElementCount;
            chai.assert.equal(num_children, 1);
          });

          it('should have a child span element', function() {
            const child_ele = document.getElementById(body_id).children[0].nodeName;
            chai.assert.equal(child_ele, "SPAN");
          });

          it('should have modal-body class', function() {
            const classes = document.getElementById(body_id).className;
            chai.assert.equal(classes, "modal-body");
          });
        });

        describe(".modal-footer", function() {
          it('should have parent .modal-content', function() {
            const parent_id = document.getElementById(footer_id).parentNode.id;
            chai.assert.equal(parent_id, content_id);
          });

          it('should have two children', function() {
            const num_children = document.getElementById(footer_id).childElementCount;
            chai.assert.equal(num_children, 2);
          });

          it('should have a child span element', function() {
            const child_ele = document.getElementById(footer_id).children[0].nodeName;
            chai.assert.equal(child_ele, "SPAN");
          });

          it('should have a child button element', function() {
            const child_ele = document.getElementById(footer_id).children[1].nodeName;
            chai.assert.equal(child_ele, "BUTTON");
          });

          it('should have modal-footer class', function() {
            const classes = document.getElementById(footer_id).className;
            chai.assert.equal(classes, "modal-footer");
          });

          describe("Modal Close Button in Footer", function() {
            const btn_id = document.getElementById(footer_id).children[1].id;

            it('should have classes "btn" and "btn-secondary"', function() {
              const classes = document.getElementById(btn_id).className;
              chai.assert.equal(classes, "btn btn-secondary");
            });

            it('should have data-toggle set to modal', function() {
              const data_toggle = document.getElementById(btn_id).getAttribute("data-toggle");
              chai.assert.equal(data_toggle, "modal");
            });

            it('should have data-target set to #modal', function() {
              const data_toggle = document.getElementById(btn_id).getAttribute("data-target");
              chai.assert.equal(data_toggle, "#modal");
            });
          });
        });

        describe("button#modal_trigger", function() {
          it('should have parent #emp_test_container', function() {
            const parent_id = document.getElementById("modal_trigger").parentNode.id;
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have no children', function() {
            const num_children = document.getElementById("modal_trigger").childElementCount;
            chai.assert.equal(num_children, 0);
          });

          it('should have button type', function() {
            const btn_type = document.getElementById("modal_trigger").nodeName;
            chai.assert.equal(btn_type, "BUTTON");
          });

          it('should have classes "btn" and "btn-primary"', function() {
            const classes = document.getElementById("modal_trigger").className;
            chai.assert.equal(classes, "btn btn-primary");
          });

          it('should have data-toggle set to modal', function() {
            const data_toggle = document.getElementById("modal_trigger").getAttribute("data-toggle");
            chai.assert.equal(data_toggle, "modal");
          });

          it('should have data-target #modal', function() {
            const data_toggle = document.getElementById("modal_trigger").getAttribute("data-target");
            chai.assert.equal(data_toggle, "#modal");
          });
        });
      });

      /*
      TODO: Find a way to trigger a click and check the DOM
      These tests should pass once we find a way to do this

      describe("Open modal, div#modal, 1st time", function() {
        $("#modal_trigger").trigger("click");
        it('should add the class show', function() { // fail [modal]
          const has_show = $("#modal").hasClass("show");
          chai.assert.equal(has_show, true);
        });
        it('should display block', function() { // fail [undefined]
          const style = $("#modal").attr("style");
          chai.assert.equal(style, "display: block;");
        });
      });

      describe("Close modal with x button in header, div#modal", function() {
        const x_id = document.getElementById("#modal").children[0].children[0].children[0].children[1].id;
        $("#"+x_id).trigger("click");
        it('should remove the class show', function() {
          const classes = $("#modal").attr("class");
          chai.assert.equal(classes, "modal");
        });
        it('should display none', function() {
          const style = $("#modal").attr("style");
          chai.assert.equal(style, "display: none;");
        });
      });

      describe("Open modal, div#modal, 2nd time", function() {
        $("#modal_trigger").click();
        it('should add the class show', function() {
          const classes = $("#modal").attr("class");
          chai.assert.equal(classes, "modal show");
        });
        it('should display block', function() {
          const style = $("#modal").attr("style");
          chai.assert.equal(style, "display: block;");
        });
      });

      describe("Close modal with close button in footer, div#modal", function() {
        const close_btn_id = document.getElementById("#modal").children[0].children[0].children[2].children[1].id;
        $("#"+close_btn_id).click();
        it('should remove the class show', function() {
          const classes = $("#modal").attr("class");
          chai.assert.equal(classes, "modal");
        });
        it('should display none', function() {
          const style = $("#modal").attr("style");
          chai.assert.equal(style, "display: none;");
        });
      });
      */
    });
  }
};

emp::web::MochaTestRunner test_runner;
int main() {

  test_runner.Initialize({"emp_test_container"});

  test_runner.AddTest<Test_Modal>("Test emp::prefab::Modal");
  test_runner.Run();
}
