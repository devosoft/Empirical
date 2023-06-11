/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file Modal.cpp
 */

#include <functional>
#include <unordered_map>

#include "emp/base/assert.hpp"
#include "emp/prefab/Modal.hpp"
#include "emp/web/Button.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/web.hpp"

struct Test_Modal : emp::web::BaseTest {

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
          const modal = document.getElementById("modal");
          it('should have parent #emp_test_container', function() {
            const parent_id = modal.parentNode.id;
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have one child', function() {
            const num_children = modal.childElementCount;
            chai.assert.equal(num_children, 1);
          });

          it('should have class modal', function() {
            chai.assert.isTrue(modal.classList.contains("modal"));
          });

          it('should have static data-backdrop', function() {
            const backdrop = modal.getAttribute("data-backdrop");
            chai.assert.equal(backdrop, "static");
          });

          it('should have -1 tabindex', function() {
            const tabindex = modal.getAttribute("tabindex");
            chai.assert.equal(tabindex, "-1");
          });

          it('should not have aria-hidden set yet', function() {
            const aria_hidden = $("#modal").attr("aria-hidden");
            chai.assert.equal(aria_hidden, undefined);
          });

          it('should not have style set yet', function() {
            const style = $("#modal").attr("style");
            chai.assert.equal(style, undefined);
          });
        });

        describe(".modal-dialog", function() {
          const modal_dialog = document.getElementById(dialog_id);
          it('should have parent #modal', function() {
            const parent_id = modal_dialog.parentNode.id;
            chai.assert.equal(parent_id, "modal");
          });

          it('should have one child', function() {
            chai.assert.equal(modal_dialog.childElementCount, 1);
          });

          it('should have class modal-dialog', function() {
            chai.assert.isTrue(modal_dialog.classList.contains("modal-dialog"));
          });
        });

        describe(".modal-content", function() {
          const modal_content = document.getElementById(content_id);
          it('should have parent #modal', function() {
            const parent_id = modal_content.parentNode.id;
            chai.assert.equal(parent_id, dialog_id);
          });

          it('should have three children', function() {
            chai.assert.equal(modal_content.childElementCount, 3);
          });

          it('should have class modal-content', function() {
            chai.assert.isTrue(modal_content.classList.contains("modal-content"));
          });
        });

        describe(".modal-header", function() {
          const modal_header = document.getElementById(header_id);
          it('should have parent .modal-content', function() {
            const parent_id = modal_header.parentNode.id;
            chai.assert.equal(parent_id, content_id);
          });

          it('should have two children', function() {
            chai.assert.equal(modal_header.childElementCount, 2);
          });

          it('should have modal-header class', function() {
            chai.assert.isTrue(modal_header.classList.contains("modal-header"));
          });

          describe("Header Text", function() {
            const title_element = modal_header.children[0];

            it('should have parent .modal-header', function() {
              chai.assert.equal(title_element.parentNode.id, header_id);
            });

            it('should be a span element', function() {
              chai.assert.equal(title_element.nodeName, "SPAN");
            });

            it('should have one child', function() {
              const num_children = title_element.childElementCount;
              chai.assert.equal(num_children, 1);
            });

            it('should have one child with element type h3', function() {
              chai.assert.equal(title_element.children[0].nodeName, "H3");
            });
          });

          describe("X Close Button", function() {
            const x_element = modal_header.children[1];

            it('should have parent .modal-header', function() {
              chai.assert.equal(x_element.parentNode.id, header_id);
            });

            it('should be a button element', function() {
              chai.assert.equal(x_element.nodeName, "BUTTON");
            });

            it('should have no children', function() {
              chai.assert.equal(x_element.childElementCount, 0);
            });

            it('should have class close', function() {
              chai.assert.isTrue(x_element.classList.contains("close"));
            });

            it('should have class close', function() {
              chai.assert.isTrue(x_element.classList.contains("float-right"));
            });

            it('should have data-dismiss = modal', function() {
              chai.assert.equal(x_element.getAttribute("data-dismiss"), "modal");
            });

            it('should have aria-label = Close', function() {
              chai.assert.equal(x_element.getAttribute("aria-label"), "Close");
            });
          });
        });


        describe(".modal-body", function() {
          const modal_body = document.getElementById(body_id);

          it('should have parent .modal-content', function() {
            const parent_id = modal_body.parentNode.id;
            chai.assert.equal(parent_id, content_id);
          });

          it('should have one child', function() {
            const num_children = modal_body.childElementCount;
            chai.assert.equal(num_children, 1);
          });

          it('should have a child span element', function() {
            const child_ele =modal_body.children[0].nodeName;
            chai.assert.equal(child_ele, "SPAN");
          });

          it('should have modal-body class', function() {
            chai.assert.isTrue(modal_body.classList.contains("modal-body"));
          });
        });

        describe(".modal-footer", function() {
          const modal_footer = document.getElementById(footer_id);

          it('should have parent .modal-content', function() {
            const parent_id = modal_footer.parentNode.id;
            chai.assert.equal(parent_id, content_id);
          });

          it('should have two children', function() {
            chai.assert.equal(modal_footer.childElementCount, 2);
          });

          it('should have a child span element', function() {
            chai.assert.equal(modal_footer.children[0].nodeName, "SPAN");
          });

          it('should have a child button element', function() {
            chai.assert.equal(modal_footer.children[1].nodeName, "BUTTON");
          });

          it('should have class modal-footer', function() {
            chai.assert.isTrue(modal_footer.classList.contains("modal-footer"));
          });

          describe("Modal Close Button in Footer", function() {
            const close_button = modal_footer.children[1];

            it('should have class btn', function() {
              chai.assert.isTrue(close_button.classList.contains("btn"));
            });

            it('should have btn-secondary', function() {
              chai.assert.isTrue(close_button.classList.contains("btn-secondary"));
            });

            it('should have data-toggle set to modal', function() {
              chai.assert.equal(close_button.getAttribute("data-toggle"), "modal");
            });

            it('should have data-target set to #modal', function() {
              chai.assert.equal(close_button.getAttribute("data-target"), "#modal");
            });
          });
        });

        describe("button#modal_trigger", function() {
          const modal_trigger = document.getElementById("modal_trigger");
          it('should have parent #emp_test_container', function() {
            const parent_id = modal_trigger.parentNode.id;
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have no children', function() {
            chai.assert.equal(modal_trigger.childElementCount, 0);
          });

          it('should have button type', function() {
            chai.assert.equal(modal_trigger.nodeName, "BUTTON");
          });

          it('should have class btn ', function() {
            chai.assert.isTrue(modal_trigger.classList.contains("btn"));
          });

          it('should have class btn-primary', function() {
            chai.assert.isTrue(modal_trigger.classList.contains("btn-primary"));
          });

          it('should have data-toggle set to modal', function() {
            chai.assert.equal(modal_trigger.getAttribute("data-toggle"), "modal");
          });

          it('should have data-target #modal', function() {
            chai.assert.equal(modal_trigger.getAttribute("data-target"), "#modal");
          });
        });
      });


      // TODO: The describe on line 322 passes all checks, but the entire test
      //  seems to exit and not execute the remaining describes (lines 335, 350, 363).

      describe("Open modal, div#modal, 1st time", function() {
        const modal = document.getElementById("modal");

        it('modal should have the class show', function() {
          document.getElementById("modal_trigger").click(); // open modal
          chai.assert.isTrue(modal.classList.contains("show"));
        });

        it('modal should have style display: block', function() {
          chai.assert.equal(modal.getAttribute("style"), "display: block;");
        });
      });

      describe("Close modal with x button in header, div#modal", function() {
        const x_close = document.getElementById("modal").children[0].children[0].children[0].children[1];
        const modal = document.getElementById("modal");

        it('modal should not have class show', function() {
          x_close.click();
          chai.assert.isFalse(modal.classList.contains("show"));
        });

        it('modal should have style display: none', function() {
          chai.assert.equal(modal.getAttribute("style"), "display: none;");
        });
      });

      // we may not be able to click this again...
      describe("Open modal, div#modal, 2nd time", function() {
        const modal = document.getElementById("modal");

        it('modal should have class show', function() {
          document.getElementById("modal_trigger").click(); // close modal
          chai.assert.isTrue(modal.classList.contains("show"));
        });

        it('modal should have style display: block', function() {
          chai.assert.equal(modal.getAttribute("style"), "display: block;");
        });
      });

      describe("Close modal with close button in footer, div#modal", function() {
        const close_btn = document.getElementById("modal").children[0].children[0].children[2].children[1];
        const modal = document.getElementById("modal");

        it('modal should not have class show', function() {
          close_btn.click();
          chai.assert.isFalse(modal.classList.contains("show"));
        });

        it('modal should have style display: none', function() {
          chai.assert.equal(modal.getAttribute("style"), "display: none;");
        });
      });

    });
  }
};

emp::web::MochaTestRunner test_runner;
int main() {

  test_runner.Initialize({"emp_test_container"});
  test_runner.AddTest<Test_Modal>("Test emp::prefab::Modal");
  test_runner.Run();

}
