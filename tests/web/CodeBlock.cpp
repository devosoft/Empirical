/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file CodeBlock.cpp
 */

#include <functional>
#include <unordered_map>

#include "emp/base/assert.hpp"
#include "emp/prefab/CodeBlock.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/web.hpp"


// Test that the Element class properly gets attached and laid out via emp::web::Document.
struct Test_Code_Block_HTMLLayout : public emp::web::BaseTest {

  /*
   * Construct the following HTML structure:
   *
   * <div id="emp_test_container">
   *  <div id="test_div">
   *    <pre id="code_block" class="c++">
   *      <code class="hljs cpp">
   *        <span>[contents of code block]</span>
   *      </code>
   *    </pre>
   *  </div>
   * </div>
   */
  Test_Code_Block_HTMLLayout()
  : BaseTest({"emp_test_container"})
  {
    std::string code_str = "// c++ comment\nint i = 3;\nstd::cout << i << std::endl;";
    emp::prefab::CodeBlock code(code_str, "c++", "code_block");

    Doc("emp_test_container")
      << emp::web::Div("test_div")
      << code;

  }

  void Describe() override {

    EM_ASM({
      describe("emp::prefab::CodeBlock HTML Layout Scenario", function() {

        // test that everything got layed out correctly in the HTML document
        describe("div#test_div", function() {

          it('should exist', function() {
            chai.assert.equal($( "div#test_div" ).length, 1);
          });

          it('should have parent #emp_test_container', function() {
            const parent_id = $("#test_div").parent().attr("id");
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have child pre#code_block', function() {
            chai.assert.equal($("div#test_div").children("pre#code_block").length, 1);
          });
        });

        describe("pre#code_block", function() {

          it('should exist', function() {
            chai.assert.equal($( "pre#code_block" ).length, 1);
          });

          it('should have parent #test_div', function() {
            const parent_id = $("pre#code_block").parent().attr("id");
            chai.assert.equal(parent_id, "test_div");
          });

          const code_block = document.getElementById("code_block");

          it('should have element type pre', function() {
            chai.assert.equal(code_block.nodeName, "PRE");
          });

          it("should have class c++", function() {
            chai.assert.isTrue(code_block.classList.contains("c++"));
          });

          it('should have one child', function() {
            chai.assert.equal(code_block.childElementCount, 1);
          });
        });

        describe("pre#code_block child", function() {
          const cb_child = document.getElementById("code_block").children[0];
          it("should have element type code", function() {
            chai.assert.equal(cb_child.nodeName, "CODE");
          });

          it("should have class hljs", function() {
            chai.assert.isTrue(cb_child.classList.contains("hljs"));
          });

          it("should have class cpp", function() {
            chai.assert.isTrue(cb_child.classList.contains("cpp"));
          });

          it("should have one child (span that contains code)", function() {
            chai.assert.equal(cb_child.childElementCount, 1);
          });
        });

        describe("span containing code", function() {
          it('should have at least one child', function() {
            const num_children = document.getElementById("code_block").children[0].children[0].children.length;
            chai.assert.isAtLeast(num_children, 1);
          });

          it('should have called highlightjs', function() {
            chai.assert.equal(hljs.initHighlighting.called, true);
          });
        });

      });
    });
  }
};

emp::web::MochaTestRunner test_runner;

int main() {

  test_runner.Initialize({"emp_test_container"});
  test_runner.AddTest<Test_Code_Block_HTMLLayout>("Test emp::prefab::CodeBlock");
  test_runner.Run();

}
