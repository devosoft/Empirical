/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file FontAwesomeIcon.cpp
 */

#include <functional>
#include <unordered_map>

#include "emp/base/assert.hpp"
#include "emp/prefab/FontAwesomeIcon.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/web.hpp"

// Test that the Element class properly gets attached and laid out via emp::web::Document.
struct Test_FontAwesome_Icon_HTMLLayout : public emp::web::BaseTest {

  /*
   * Construct the following HTML structure:
   *
   * <div id="emp_test_container">
   *  <div id="test_div">
   *    <span id="checkbox_icon" class="fa fa-check-square-o"></span>
   *  </div>
   * </div>
   */

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

          const icon = document.getElementById("checkbox_icon");

          it('should have class fa', function(){
            chai.assert.isTrue(icon.classList.contains("fa"));
          });

          it('should have class fa-check-square-o', function(){
            chai.assert.isTrue(icon.classList.contains("fa-check-square-o"));
          });
        });

      });
    });
  }

};

emp::web::MochaTestRunner test_runner;

int main() {

  test_runner.Initialize({"emp_test_container"});
  test_runner.AddTest<Test_FontAwesome_Icon_HTMLLayout>("Test FontAwesome Icon HTML Layout");
  test_runner.Run();

}
