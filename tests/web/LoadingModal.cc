//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <functional>
#include <unordered_map>

#include "base/assert.h"
#include "web/_MochaTestRunner.h"
#include "web/Document.h"
#include "web/Div.h"
#include "web/web.h"

#include "prefab/LoadingModal.h"


// Test that the Element class properly gets attached and laid out via emp::web::Document.
struct Test_Loading_Modal_Open_HTMLLayout : public emp::web::BaseTest {


  // Construct the following HTML structure:
  /**
   * <div class="modal bd-example-modal-lg show" id="loading_modal" data-backdrop="static" data-keyboard="false" tabindex="-1" style="display: block;">
   *  <div class="modal-dialog modal-sm">
   *    <div class="modal-content" style="width: 256px">
   *      <div class="modal-body">
   *        <div class="spinner-border text-light" style="width: 256px; height: 256px;" role="status">
   *          <span class="sr-only">Loading...</span>
   *        </div>
   *      </div>
   *    </div>
   *  </div>
   * </div>
   */

  Test_Loading_Modal_Open_HTMLLayout()
  : BaseTest({"emp_test_container"}) // we can tell BaseTest that we want to create a set of emp::web::Document
                                     // objects for each given html element ids.
  {
  }

  void Describe() override {

    EM_ASM({
      describe("emp::prefab::LoadingModal Open HTML Layout Scenario", function() {
        const modal = $("div#loading_modal");
        const modal_dialog = modal.children()[0];
        const modal_content = modal_dialog.children[0];
        const modal_body = modal_content.children[0];
        const spinner = modal_body.children[0];

        describe("body tag", function() {
          it("should have modal-open class", function() {
            chai.assert.isTrue(document.getElementsByTagName("body")[0].classList.contains("modal-open"));
          });
        });

        describe("div#loading_modal", function() {
          it('should exist', function() {
            chai.assert.equal(modal.length, 1);
          });

          it('should have parent <body>', function() {
            chai.assert.equal(modal.parent()[0].tagName, "BODY");
          });

          it('should have classes modal, bd-example-modal-lg, show', function() {
            chai.assert.equal(modal.attr("class"), "modal bd-example-modal-lg show");
          });

          it('should have static data-backdrop', function() {
            chai.assert.equal(modal.attr("data-backdrop"), "static");
          });

          it('should have false data-keyboard', function() {
            chai.assert.equal(modal.attr("data-keyboard"), "false");
          });

          it('should have tabindex -1', function() {
            chai.assert.equal(modal.attr("tabindex"), "-1");
          });

          it('should display block', function() {
            chai.assert.equal(modal.attr("style"), "display: block;");
          });

          it('should have one child', function() {
            chai.assert.equal(modal.children().length, 1);
          });
        });

        describe(".modal-dialog (child of #loading_modal)", function() {
          it('should exist', function() {
            chai.assert.isFalse(modal_dialog.hidden);
          });

          it('should have parent #loading_modal', function() {
            chai.assert.equal(modal_dialog.parentElement.id, "loading_modal");
          });

          it('should have class modal-dialog and modal-sm', function() {
            chai.assert.equal(modal_dialog.getAttribute("class"), "modal-dialog modal-sm");
          });

          it('should have one child', function() {
            chai.assert.equal(modal_dialog.childElementCount, 1);
          });
        });

        describe(".modal-content (child of .modal-dialog)", function() {
          it('should exist', function() {
            chai.assert.isFalse(modal_content.hidden);
          });

          it('should have parent .modal-dialog', function() {
            chai.assert.isTrue(modal_content.parentNode.classList.contains("modal-dialog"));
          });

          it('should have class modal-content', function() {
            chai.assert.equal(modal_content.getAttribute("class"), "modal-content");
          });

          it('should have width = 256px', function() {
            chai.assert.equal(modal_content.getAttribute("style"), "width: 256px");
          });

          it('should have one child', function() {
            chai.assert.equal(modal_content.childElementCount, 1);
          });
        });

        describe(".modal-body (child of .modal-content)", function() {
          it('should exist', function() {
            chai.assert.isFalse(modal_body.hidden);
          });

          it('should have parent .modal-content', function() {
            chai.assert.isTrue(modal_body.parentNode.classList.contains("modal-content"));
          });

          it('should have class modal-body', function() {
            chai.assert.equal(modal_body.getAttribute("class"), "modal-body");
          });

          it('should have one child', function() {
            chai.assert.equal(modal_body.childElementCount, 1);
          });
        });

        describe(".spinner-border (child of .modal-body)", function() {
          it('should exist', function() {
            chai.assert.isFalse(spinner.hidden);
          });

          it('should have parent .modal-body', function() {
            chai.assert.isTrue(spinner.parentNode.classList.contains("modal-body"));
          });

          it('should have class spinner-border and text-light', function() {
            chai.assert.equal(spinner.getAttribute("class"), "spinner-border text-light");
          });

          it('should have width and height set to 256px', function() {
            chai.assert.equal(spinner.getAttribute("style"), "width: 256px; height: 256px;");
          });

          it('should have role of status', function() {
            chai.assert.equal(spinner.getAttribute("role"), "status");
          });

          it('should have one child', function() {
            chai.assert.equal(spinner.childElementCount, 1);
          });
        });

        describe(".spinner-border child", function() {
          const child = spinner.children[0];
          it('should exist', function() {
            chai.assert.isFalse(child.hidden, 1);
          });

          it('should have parent .spinner-border', function() {
            chai.assert.isTrue(child.parentNode.classList.contains("spinner-border"));
          });

          it('should be a span element', function() {
            chai.assert.equal(child.nodeName, "SPAN");
          });

          it('should have class sr-only', function() {
            chai.assert.equal(child.getAttribute("class"), "sr-only");
          });

          it('should have no children', function() {
            chai.assert.equal(child.childElementCount, 0);
          });
        });
      });
    });
  }

};

struct Test_Loading_Modal_Closed_HTMLLayout : public emp::web::BaseTest {


  // Construct the following HTML structure:
  /**
   * <div class="modal bd-example-modal-lg" id="loading_modal" data-backdrop="static" data-keyboard="false" tabindex="-1" style="display: none;" aria-hidden="true">
   *  <div class="modal-dialog modal-sm">
   *    <div class="modal-content" style="width: 256px">
   *      <div class="modal-body">
   *        <div class="spinner-border text-light" style="width: 256px; height: 256px;" role="status">
   *          <span class="sr-only">Loading...</span>
   *        </div>
   *      </div>
   *    </div>
   *  </div>
   * </div>
   */

  Test_Loading_Modal_Closed_HTMLLayout()
  : BaseTest({"emp_test_container"}) // we can tell BaseTest that we want to create a set of emp::web::Document
                                     // objects for each given html element ids.
  {
    emp::prefab::CloseLoadingModal();
  }

  void Describe() override {

    EM_ASM({
      describe("emp::prefab::LoadingModal Open HTML Layout Scenario", function() {
        const modal = $("div#loading_modal");
        const modal_dialog = modal.children()[0];
        const modal_content = modal_dialog.children[0];
        const modal_body = modal_content.children[0];
        const spinner = modal_body.children[0];

        describe("body tag", function() {
          it("should not have modal-open class", function() {
            chai.assert.isFalse(document.getElementsByTagName("body")[0].classList.contains("modal-open"));
          });
        });

        describe("div#loading_modal", function() {
          it('should exist', function() {
            chai.assert.equal(modal.length, 1);
          });

          it('should have parent <body>', function() {
            chai.assert.equal(modal.parent()[0].tagName, "BODY");
          });

          it('should have classes modal and bd-example-modal-lg', function() {
            chai.assert.equal(modal.attr("class"), "modal bd-example-modal-lg");
          });

          it('should have static data-backdrop', function() {
            chai.assert.equal(modal.attr("data-backdrop"), "static");
          });

          it('should have false data-keyboard', function() {
            chai.assert.equal(modal.attr("data-keyboard"), "false");
          });

          it('should have tabindex -1', function() {
            chai.assert.equal(modal.attr("tabindex"), "-1");
          });

          it('should display block', function() {
            chai.assert.equal(modal.attr("style"), "display: none;");
          });

          it('should have aria-hidden set to true', function() {
            chai.assert.equal(modal.attr("aria-hidden"), "true");
          });

          it('should have one child', function() {
            chai.assert.equal(modal.children().length, 1);
          });
        });

        describe(".modal-dialog (child of #loading_modal)", function() {
          it('should exist', function() {
            chai.assert.isFalse(modal_dialog.hidden);
          });

          it('should have parent #loading_modal', function() {
            chai.assert.equal(modal_dialog.parentElement.id, "loading_modal");
          });

          it('should have class modal-dialog and modal-sm', function() {
            chai.assert.equal(modal_dialog.getAttribute("class"), "modal-dialog modal-sm");
          });

          it('should have one child', function() {
            chai.assert.equal(modal_dialog.childElementCount, 1);
          });
        });

        describe(".modal-content (child of .modal-dialog)", function() {
          it('should exist', function() {
            chai.assert.isFalse(modal_content.hidden);
          });

          it('should have parent .modal-dialog', function() {
            chai.assert.isTrue(modal_content.parentNode.classList.contains("modal-dialog"));
          });

          it('should have class modal-content', function() {
            chai.assert.equal(modal_content.getAttribute("class"), "modal-content");
          });

          it('should have width = 256px', function() {
            chai.assert.equal(modal_content.getAttribute("style"), "width: 256px");
          });

          it('should have one child', function() {
            chai.assert.equal(modal_content.childElementCount, 1);
          });
        });

        describe(".modal-body (child of .modal-content)", function() {
          it('should exist', function() {
            chai.assert.isFalse(modal_body.hidden);
          });

          it('should have parent .modal-content', function() {
            chai.assert.isTrue(modal_body.parentNode.classList.contains("modal-content"));
          });

          it('should have class modal-body', function() {
            chai.assert.equal(modal_body.getAttribute("class"), "modal-body");
          });

          it('should have one child', function() {
            chai.assert.equal(modal_body.childElementCount, 1);
          });
        });

        describe(".spinner-border (child of .modal-body)", function() {
          it('should exist', function() {
            chai.assert.isFalse(spinner.hidden);
          });

          it('should have parent .modal-body', function() {
            chai.assert.isTrue(spinner.parentNode.classList.contains("modal-body"));
          });

          it('should have class spinner-border and text-light', function() {
            chai.assert.equal(spinner.getAttribute("class"), "spinner-border text-light");
          });

          it('should have width and height set to 256px', function() {
            chai.assert.equal(spinner.getAttribute("style"), "width: 256px; height: 256px;");
          });

          it('should have role of status', function() {
            chai.assert.equal(spinner.getAttribute("role"), "status");
          });

          it('should have one child', function() {
            chai.assert.equal(spinner.childElementCount, 1);
          });
        });

        describe(".spinner-border child", function() {
          const child = spinner.children[0];
          it('should exist', function() {
            chai.assert.isFalse(child.hidden, 1);
          });

          it('should have parent .spinner-border', function() {
            chai.assert.isTrue(child.parentNode.classList.contains("spinner-border"));
          });

          it('should be a span element', function() {
            chai.assert.equal(child.nodeName, "SPAN");
          });

          it('should have class sr-only', function() {
            chai.assert.equal(child.getAttribute("class"), "sr-only");
          });

          it('should have no children', function() {
            chai.assert.equal(child.childElementCount, 0);
          });
        });
      });
    });
  }

};


// Create a MochaTestRunner object in the global namespace so that it hangs around after main finishes.
emp::web::MochaTestRunner test_runner;

int main() {
  test_runner.Initialize({"emp_test_container"});

  test_runner.AddTest<Test_Loading_Modal_Open_HTMLLayout>("Test emp::prefab::LoadingModal Open HTML Layout");
  test_runner.AddTest<Test_Loading_Modal_Closed_HTMLLayout>("Test emp::prefab::LoadingModal Closed HTML Layout");

  test_runner.Run();
}
