/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file LoadingIcon.cpp
 */

#include <functional>
#include <unordered_map>

#include "emp/base/assert.hpp"
#include "emp/prefab/LoadingIcon.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/web.hpp"

// Test that the Element class properly gets attached and laid out via emp::web::Document.
struct Test_Loading_Icon_HTMLLayout : public emp::web::BaseTest {

  /*
   * Construct the following HTML structure:
   *
   * <div id="emp_test_container">
   *  <div id="test_div">
   *    <span id="loading_icon">
   *      <span class="fa fa-spinner fa-pulse fa-3x fa-fw"></span>
   *      <span class="sr-only"><span>Loading...</span></span>
   *    </span>
   *  </div>
   * </div>
   */

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
        });

        describe("span#loading_icon child #1", function() {
          const child_one = document.getElementById("loading_icon").children[0];
          it('should have class fa', function() {
            chai.assert.isTrue(child_one.classList.contains("fa"));
          });

          it('should have class fa-spinner', function() {
            chai.assert.isTrue(child_one.classList.contains("fa-spinner"));
          });

          it('should have class fa-pulse', function() {
            chai.assert.isTrue(child_one.classList.contains("fa-pulse"));
          });

          it('should have class fa-3x', function() {
            chai.assert.isTrue(child_one.classList.contains("fa-3x"));
          });

          it('should have class fa-fw', function() {
            chai.assert.isTrue(child_one.classList.contains("fa-fw"));
          });
        });

        describe("span#loading_icon child #2", function() {
          it('should be alternative loading text', function() {
            const child_two = document.getElementById("loading_icon").children[1];
            chai.assert.isTrue(child_two.classList.contains("sr-only"));
          });

          it('should have one child', function() {
            const num_children = document.getElementById("loading_icon").children[1].childElementCount;
            chai.assert.equal(num_children, 1);
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
  test_runner.AddTest<Test_Loading_Icon_HTMLLayout>("Test Loading Icon HTML Layout");
  test_runner.Run();

}
