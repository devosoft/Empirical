//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <functional>
#include <unordered_map>

#include "base/assert.h"
#include "web/_MochaTestRunner.h"
#include "web/Document.h"
#include "web/web.h"

#include "prefab/LoadingIcon.h"


// Test that the Element class properly gets attached and laid out via emp::web::Document.
struct Test_Loading_Icon_HTMLLayout : public emp::web::BaseTest {


  // Construct the following HTML structure:
  // <div id="emp_test_container">
  //   <div id="test_div">
  //        <span id="loading_icon">
  //            <span class="fa fa-spinner fa-pulse fa-3x fa-fw"></span>
  //            <span class="sr-only"><span>Loading...</span></span>
  //        </span>
  //    </div>
  // </div>

  Test_Loading_Icon_HTMLLayout()
  : BaseTest({"emp_test_container"}) // we can tell BaseTest that we want to create a set of emp::web::Document
                                     // objects for each given html element ids.
  {
    emp::prefab::LoadingIcon icon("loading_icon");

    Doc("emp_test_container")
      << emp::web::Div("test_div")
      << icon;

  }

  void Describe() override {

    EM_ASM({
      describe("emp::prefab::LoadingIcon HTML Layout Scenario", function() {

        // test that everything got layed out correctly in the HTML document
        describe("div#test_div", function() {

          it('should exist', function() {
            chai.assert.equal($( "div#test_div" ).length, 1);
          });

          it('should have parent #emp_test_container', function() {
            const parent_id = $("#test_div").parent().attr("id");
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have child span#loading_icon', function() {
            chai.assert.equal($("div#test_div").children("span#loading_icon").length, 1);
          });

        });

        describe("span#loading_icon", function() {

          it('should exist', function() {
            chai.assert.equal($( "span#loading_icon" ).length, 1);
          });

          it('should have parent #test_div', function() {
            const parent_id = $("span#loading_icon").parent().attr("id");
            chai.assert.equal(parent_id, "test_div");
          });

          it('should have two children', function() {
            const num_children = document.getElementById("loading_icon").childElementCount;
            chai.assert.equal(num_children, 2);
          });

          it('should have a FontAwesome spinner as the first child', function() {
            const child_one = document.getElementById("loading_icon").children[0];
            const child_classes = child_one.className;
            chai.assert.equal(child_classes, "fa fa-spinner fa-pulse fa-3x fa-fw");
          });

          it('should have alternative loading text as the second child', function() {
            const child_two = document.getElementById("loading_icon").children[1];
            const child_classes = child_two.className;
            chai.assert.equal(child_classes, "sr-only");
          });

        });

        describe("span#loading_icon child#2", function() {
          it('should have one child', function() {
            const num_children = document.getElementById("loading_icon").children[1].childElementCount;
            chai.assert.equal(num_children, 1);
          })
        });


      });
    });
  }

};

// Create a MochaTestRunner object in the global namespace so that it hangs around after main finishes.
emp::web::MochaTestRunner test_runner;

int main() {

  // MochaTestRunner::Initialize will make sure empirical's web environment is initialized, and will
  // append a set of div elements (with the given string ids) to the HTML document body.
  // Between tests, the MochaTestRunner clears the contents of these div elements.
  // Remember, karma is generating our HTML file, so this is useful for attaching any HTML divs that
  // you want to interact with in your tests.
  test_runner.Initialize({"emp_test_container"});

  // We add tests to the test runner like this:
  //  where "Test Element" is the name of the test (and does not need to be unique)
  test_runner.AddTest<Test_Loading_Icon_HTMLLayout>(
    "Test Loading Icon HTML Layout"
  );

  // Once we add all of the tests we want to run in this file, run them!
  test_runner.Run();
}
