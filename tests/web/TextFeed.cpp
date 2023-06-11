/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file TextFeed.cpp
 */

#include <functional>
#include <unordered_map>

#include "emp/base/assert.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/TextFeed.hpp"
#include "emp/web/web.hpp"


// Test that the TextFeed class properly gets attached and laid out via emp::web::Document.
struct Test_TextFeed_HTMLLayout : public emp::web::BaseTest {


  // Construct the following HTML structure:
  // <div id="emp_test_container">
  //   <div id="test_div">
  //     /* feed */
  //   </div>
  // </div>
  Test_TextFeed_HTMLLayout()
  : BaseTest({"emp_test_container"}) // we can tell BaseTest that we want to create a set of emp::web::Document
                                     // objects for each given html element ids.
  {
    emp::web::TextFeed feed{"test_feed"};

    Doc("emp_test_container")
      << emp::web::Div("test_div")
      << feed << "Header1!" << "Header2!";

  }

  void Describe() override {

    EM_ASM({
      describe("emp::web::TextFeed HTML Layout Scenario", function() {

        // test that everything got laid out correctly in the HTML document
        describe("div#test_div", function() {

          it('should exist', function() {
            chai.assert.equal($( "div#test_div" ).length, 1);
          });

          it('should have parent #emp_test_container', function() {
            const parent_id = $("#test_div").parent().attr("id");
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have child p#test_div', function() {
            chai.assert.equal($("div#test_div").children("span#test_feed").length, 1);
          });

        });

        describe("span#test_feed", function() {

          it('should exist', function() {
            chai.assert.equal($( "span#test_feed" ).length, 1);
          });

          it('should contain first text', function() {
            chai.assert.equal(
              $('span#test_feed').text().indexOf('Header1!') > -1,
              true
            );
          });

          it('should contain second text', function() {
            chai.assert.equal(
              $('span#test_feed').text().indexOf('Header2!') > -1,
              true
            );
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
  //  where "Test TextFeed" is the name of the test (and does not need to be unique)
  test_runner.AddTest<Test_TextFeed_HTMLLayout>(
    "Test TextFeed HTML Layout"
  );

  // Once we add all of the tests we want to run in this file, run them!
  test_runner.Run();
}
