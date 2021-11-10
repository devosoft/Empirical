/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file Widget.cpp
 */

#include <functional>
#include <unordered_map>

#include "emp/base/assert.hpp"
#include "emp/web/Div.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/Element.hpp"
#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/web.hpp"

struct Test_WidgetWrapWith : emp::web::BaseTest {


  // Construct the following HTML structure:
  // <div id="wrapper2"> <!-- inserted with WrapWith -->
  //   <p id="parent">
  //     parent
  //     <div id="wrapper"> <!-- inserted with WrapWith -->
  //       wrapper
  //       <button id="child"></button>
  //     </div>
  //   </p>
  //   <br/><br/>
  // </div>
  Test_WidgetWrapWith()
  : BaseTest({"emp_test_container"})
  {

    emp::web::Element parent("p", "parent");
    parent << "parent";
    Doc("emp_test_container") << parent;

    emp::web::Button child(
      []() {
        EM_ASM({
          $("#child_button").attr("clicked", "yes");
        });
      },
      "child"
    );
    child.SetAttr("clicked", "no");
    child.SetAttr("id", "child_button");
    parent << child;

    emp::web::Div wrapper("wrapper");
    wrapper << "wrapper";
    child.WrapWith(wrapper);

    parent.WrapWith(
      emp::web::Div("wrapper2").SetCSS("background-color", "red")
    ).SetCSS("background-color", "blue");

    Doc("emp_test_container").Div("wrapper2") << "<br/><br/>";

  }

  void Describe() override {

    // Test that the HTML components created in constructor are correct.
    EM_ASM({

      describe("Widget::WrapWith", function() {
        describe("#wrapper2", function() {
          it('should have parent #emp_test_container', function() {
            const parent_id = $("#wrapper2").parent().attr("id");
            chai.assert.equal(parent_id, "emp_test_container");
          });
          it('should have child #parent', function() {
            var children = $("#wrapper2").children();
            // Get ids of child
            var child_ids = [];
            for (i = 0; i < children.length; i++) {
              child_ids.push(children[i].getAttribute("id"));
            }
            chai.assert.include(child_ids, "parent");
            chai.assert.equal($("#wrapper2").children("#parent").length, 1);
          });
        });
        describe("#parent", function() {
          it('should have parent #wrapper2', function() {
            const parent_id = $("#parent").parent().attr("id");
            chai.assert.equal(parent_id, "wrapper2");
          });
          it('should have child #wrapper', function() {
            chai.assert.equal($("#parent").children("#wrapper").length, 1);
          });
        });
        describe("#wrapper", function() {
          it('should have parent #parent', function() {
            const parent_id = $("#wrapper").parent().attr("id");
            chai.assert.equal(parent_id, "parent");
          });
          it('should have child #child_button', function() {
            chai.assert.equal($("#wrapper").children("#child_button").length, 1);
          });
        });
        describe("#child_button", function() {
          it('should have parent #wrapper', function() {
            const parent_id = $("#child_button").parent().attr("id");
            chai.assert.equal(parent_id, "wrapper");
          });
        });
        describe("button#child_button", function() {
          it('should do stuff when clicked', function() {
            const before = $("#child_button").attr("clicked");
            chai.assert.equal(before, "no", "check initial clicked value");
            $("#child_button").trigger( "click" );
            const after = $("#child_button").attr("clicked");
            chai.assert.equal(after, "yes", "check post-click clicked value");
          });
        });
      });
    });
  }

};

struct Test_TemplatedAppending : emp::web::BaseTest {

  Test_TemplatedAppending()
  : BaseTest({"emp_test_container"})
  {
    emp::web::Div widgets("widgets");
    emp::web::Div strings("strings");
    emp::web::Div invoked("invoked");
    Doc("emp_test_container") << widgets;
    Doc("emp_test_container") << strings;
    Doc("emp_test_container") << invoked;

    emp::vector<emp::web::Div> divVec({emp::web::Div("one"), emp::web::Div("two"), emp::web::Div("three")});
    widgets << divVec;
    emp::vector<std::string> stringVec({"hi", "why", "bye"});
    strings << stringVec;
    invoked << []() { return "Handles lambdas"; };
  }

  void Describe() override {
    EM_ASM({
      describe("Widget::Append templated handling", function() {
        describe("#widgets", function() {
          const widgets = document.getElementById("widgets");
          // Divs remain distinct
          it("should have 3 children", function() {
            chai.assert.equal(widgets.childElementCount, 3);
          });
        });
        describe("#strings", function() {
          const strings = document.getElementById("strings");
          // Strings get appended together in the span
          it("should have 1 child", function() {
            chai.assert.equal(strings.childElementCount,1);
          });
          const child = strings.children[0];
          describe("child", function() {
            it("should be a span", function() {
              chai.assert.equal(child.nodeName, "SPAN");
            });
          });
        });
        describe("#invoked", function() {
          const invoked = document.getElementById("invoked");
          it("should have 1 child", function() {
            chai.assert.equal(invoked.childElementCount, 1);
          });
          const child = invoked.children[0];
          describe("child", function() {
            it("should be a span", function() {
              chai.assert.equal(child.nodeName, "SPAN");
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

  test_runner.AddTest<Test_WidgetWrapWith>("Test Widget::WrapWith");
  test_runner.AddTest<Test_TemplatedAppending>("Test Widget::Append's handling of various types");

  test_runner.Run();
}
