//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <functional>
#include <unordered_map>

#include "base/assert.h"
#include "web/_MochaTestRunner.h"
#include "web/Document.h"
#include "web/web.h"

#include "prefab/FontAwesomeIcon.h"


// Test that the Element class properly gets attached and laid out via emp::web::Document.
struct Test_FontAwesome_Icon_HTMLLayout : public emp::web::BaseTest {


  // Construct the following HTML structure:
  // <div id="emp_test_container">
  //   <div id="test_div">
  //        <span id="checkbox_icon" class="fa fa-check-square-o"></span>
  //    </div>
  // </div>

  Test_FontAwesome_Icon_HTMLLayout()
  : BaseTest({"emp_test_container"}) // we can tell BaseTest that we want to create a set of emp::web::Document
                                     // objects for each given html element ids.
  {
    emp::prefab::FontAwesomeIcon icon("fa-check-square-o", "checkbox_icon");

    Doc("emp_test_container")
      << emp::web::Div("test_div")
      << icon;

  }

  void Describe() override {

    EM_ASM({
      describe("emp::prefab::FontAwesome HTML Layout Scenario", function() {

        // test that everything got layed out correctly in the HTML document
        describe("div#test_div", function() {

          it('should exist', function() {
            chai.assert.equal($( "div#test_div" ).length, 1);
          });

          it('should have parent #emp_test_container', function() {
            const parent_id = $("#test_div").parent().attr("id");
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have child span#checkbox_icon', function() {
            chai.assert.equal($("div#test_div").children("span#checkbox_icon").length, 1);
          });

        });

        describe("span#checkbox_icon", function() {

          it('should exist', function() {
            chai.assert.equal($( "span#checkbox_icon" ).length, 1);
          });

          it('should have parent #test_div', function() {
            const parent_id = $("span#checkbox_icon").parent().attr("id");
            chai.assert.equal(parent_id, "test_div");
          });

          // it should have classes fa and fa-check-square-o
          // this should fail
          it('should have fontawesome classes', function(){
            const classes = document.getElementById("checkbox_icon").className;
            chai.assert.equal(classes, "fa-check-square-o");
          });
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
  test_runner.AddTest<Test_FontAwesome_Icon_HTMLLayout>(
    "Test FontAwesome Icon HTML Layout"
  );

  // Once we add all of the tests we want to run in this file, run them!
  test_runner.Run();
}
