//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <functional>
#include <unordered_map>

#include "emp/base/assert.hpp"
#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/Div.hpp"
#include "emp/web/web.hpp"

#include "emp/prefab/CommentBox.hpp"


// Test that the Element class properly gets attached and laid out via emp::web::Document.
struct Test_Comment_Box_HTMLLayout : public emp::web::BaseTest {

  /*
   * Construct the following HTML structure:
   *
   * <div id="emp_base">
   *  <div id="comment_box">
   *    <div id="comment_box_triangle" class="commentbox_triangle"></div>
   *
   *    <div id="comment_box_all_content" class="commentbox_content">
   *
   *      <div id="comment_box_desktop_content">
   *        <span id="emp__0"><h1>Comment Box Title</h1></span>
   *        <div id="body"><span id="emp__1">More content for comment box</span></div>
   *      </div>
   *      <div id="comment_box_mobile_content" class="mobile_commentbox"></div>
   *
   *    </div>
   *  </div>
   * </div>
   */

  Test_Comment_Box_HTMLLayout()
  : BaseTest({"emp_test_container"}) // we can tell BaseTest that we want to create a set of emp::web::Document
                                     // objects for each given html element ids.
  {
    emp::prefab::CommentBox box("comment_box");
    Doc("emp_test_container") << box;
    box.AddContent("<h1>Comment Box Title</h1>");
    emp::web::Div body("body");
    box.AddContent(body);
    body << "More content for comment box";
  }

  void Describe() override {

    EM_ASM({
      describe("emp::prefab::CommentBox HTML Layout Scenario", function() {

        describe("div#emp_test_container", function() {
          it('should exist', function() {
            chai.assert.equal($("div#emp_test_container").length, 1);
          });

          it('should have child div#comment_box', function() {
            chai.assert.equal($("div#emp_test_container").children("div#comment_box").length, 1);
          });
        });

        describe("div#comment_box", function() {
          it('should exist', function() {
            chai.assert.equal($("div#comment_box").length, 1);
          });

          it('should have parent div#emp_test_container', function() {
            const parent_id = $("div#comment_box").parent().attr("id");
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have child div#comment_box_triangle', function() {
            chai.assert.equal($("div#comment_box").children("div#comment_box_triangle").length, 1);
          });

          it('should have child div#comment_box_all_content', function() {
            chai.assert.equal($("div#comment_box").children("div#comment_box_all_content").length, 1);
          })
        });

        describe("div#comment_box_all_content", function() {

          it('should exist', function() {
            chai.assert.equal($("div#comment_box_all_content").length, 1);
          });

          it('should have parent div#comment_box', function() {
            const parent_id = $("div#comment_box_all_content").parent().attr("id");
            chai.assert.equal(parent_id, "comment_box");
          });

          // TODO: When AddMobileContent() is removed from CommentBox, remove this test
          it('should have child div#comment_box_desktop_content', function() {
            chai.assert.equal($("div#comment_box_all_content").children("div#comment_box_desktop_content").length, 1);
          });

          it('should have class commentbox_content', function() {
            chai.assert.isTrue(document.getElementById('comment_box_all_content').classList.contains('commentbox_content'));
          });
        });

        describe("div#comment_box_triangle", function() {
          it('should exist', function() {
            chai.assert.equal($("div#comment_box_triangle").length, 1);
          });

          it('should have parent div#comment_box', function() {
            const parent_id = $("div#comment_box_triangle").parent().attr("id");
            chai.assert.equal(parent_id, "comment_box");
          });

          it('should have class commentbox_triangle', function() {
            chai.assert.isTrue(document.getElementById('comment_box_triangle').classList.contains('commentbox_triangle'));
          });
        });

        // TODO: When AddMobileContent() is removed from CommentBox, the remaining tests
        // will apply to #comment_box_all_content instead of #comment_box_desktop_content
        describe("div#comment_box_desktop_content", function() {
          it('should exist', function() {
            chai.assert.equal($("div#comment_box_desktop_content").length, 1);
          });

          it('should have parent #comment_box_all_content', function() {
            const parent_id = $("#comment_box_desktop_content").parent().attr("id");
            chai.assert.equal(parent_id, "comment_box_all_content");
          });

          it('should have two children', function() {
            chai.assert.equal(document.getElementById("comment_box_desktop_content").childElementCount, 2);
          });

          it('should have child div#body', function() {
            chai.assert.equal($("div#comment_box_desktop_content").children("div#body").length, 1);
          });
        });

        describe("first child of div#comment_box_desktop_content", function() {
          const child = document.getElementById("comment_box_desktop_content").children[0];
          it('should have type span', function() {
            chai.assert.equal(child.nodeName, "SPAN");
          });

          it('should have one child', function() {
            chai.assert.equal(child.childElementCount, 1);
          });

          it('should have one child of type h1', function() {
            chai.assert.equal(child.children[0].nodeName, "H1");
          });
        });

        describe("second child of div#comment_box_desktop_content", function() {
          const child = document.getElementById("comment_box_desktop_content").children[1];
          it('should have an id of body', function() {
            chai.assert.equal(child.getAttribute("id"), "body");
          });

          it('should have type div', function() {
            chai.assert.equal(child.nodeName, "DIV");
          });

          it('should have one child', function() {
            chai.assert.equal(child.childElementCount, 1);
          });

          it('should have one child of type span', function() {
            chai.assert.equal(child.children[0].nodeName, "SPAN");
          });
        });
      });
    });
  }

};

emp::web::MochaTestRunner test_runner;

int main() {

  test_runner.Initialize({"emp_test_container"});
  test_runner.AddTest<Test_Comment_Box_HTMLLayout>("Test emp::prefab::CommentBox HTML Layout");
  test_runner.Run();

}
