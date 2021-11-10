/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file Card.cpp
 */

#include <functional>
#include <unordered_map>

#include "emp/base/assert.hpp"
#include "emp/prefab/Card.hpp"
#include "emp/web/Div.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/web.hpp"

// Test that the Card class properly gets attached and laid out via emp::web::Document.
struct Test_Card_STATIC_HTMLLayout : public emp::web::BaseTest {

  /**
   * Construct the following HTML structure:
   *
   * <div id="emp_test_container">
   * <div id="static" class="card">
   *
   *  <div id="static_card_header" class="card-header">
   *    <span id="emp__0">Static Card Title</span>
   *  </div>
   *
   *  <div id="static_card_body" class="card-body">
   *    <div id="div_body">
   *      <span id="emp__1">body content 1</span>
   *    </div>
   *    <span id="emp__1">body content 2</span>
   *  </div>
   *
   * </div>
   * </div>
   */

  Test_Card_STATIC_HTMLLayout()
  : BaseTest({"emp_test_container"}) // we can tell BaseTest that we want to create a set of emp::web::Document
                                     // objects for each given html element ids.
  {
    emp::prefab::Card static_card("STATIC", false, "static");
    Doc("emp_test_container") << static_card;
    static_card.AddHeaderContent("Static Card Title");
    emp::web::Div body("div_body");
    static_card.AddBodyContent(body);
    body << "body content 1";
    static_card << "body content 2";
  }

  void Describe() override {

    EM_ASM({
      describe("emp::prefab::Card STATIC HTML Layout Scenario", function() {

        // test that everything got layed out correctly in the HTML document
        describe("div#emp_test_container", function() {
          it('should exist', function() {
            chai.assert.equal($( "div#emp_test_container" ).length, 1);
          });

          it('should have child div#static', function() {
            chai.assert.equal($("div#emp_test_container").children("div#static").length, 1);
          });
        });

        describe("div#static [Card]", function() {
          it('should exist', function() {
            chai.assert.equal($( "div#static" ).length, 1);
          });

          it('should have parent #test_div', function() {
            const parent_id = $("#static").parent().attr("id");
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have the class "card"', function() {
            chai.assert.isTrue(document.getElementById('static').classList.contains('card'));
          });

          it('should have two children', function() {
            chai.assert.equal($("div#static").children().length, 2);
          });

          it('should have child div#static_card_header', function() {
            chai.assert.equal($("div#static").children("div#static_card_header").length, 1);
          });

          it('should have child div#static_card_body', function() {
            chai.assert.equal($("div#static").children("div#static_card_body").length, 1);
          });
        });

        describe("div#static_card_header", function() {
          it('should exist', function() {
            chai.assert.equal($( "div#static_card_header" ).length, 1);
          });

          it('should have parent #static', function() {
            const parent_id = $("#static_card_header").parent().attr("id");
            chai.assert.equal(parent_id, "static");
          });

          it('should have the class card-header', function() {
            chai.assert.isTrue(document.getElementById('static_card_header').classList.contains('card-header'));
          });

          it('should have one child', function() {
            chai.assert.equal($("#static_card_header").children().length, 1);
          });

          describe("Child", function() {
            const child = $("#static_card_header").children()[0];
            it('should have parent #static_card_header', function() {
              const parent_id = child.parentElement.id;
              chai.assert.equal(parent_id, "static_card_header");
            });

            it('should not have children', function() {
              chai.assert.equal(child.childElementCount, 0);
            });

            it('should be a span element', function() {
              chai.assert.equal(child.nodeName, "SPAN");
            });
          });
        });

        describe("div#static_card_body", function() {
          it('should exist', function() {
            chai.assert.equal($( "div#static_card_body" ).length, 1);
          });

          it('should have parent #static', function() {
            const parent_id = $("#static_card_body").parent().attr("id");
            chai.assert.equal(parent_id, "static");
          });

          it('should have the class card-body', function() {
            chai.assert.isTrue(document.getElementById('static_card_body').classList.contains('card-body'));
          });

          it('should have two children', function() {
            chai.assert.equal($("#static_card_body").children().length, 2);
          });

          it('should have child div#div_body', function() {
            chai.assert.equal($("#static_card_body").children()[0].getAttribute("id"), "div_body");
          });

          describe("Child [#div_body]", function() {
            const child = document.getElementById("div_body");
            it('should have parent #static_card_body', function() {
              const parent_id = child.parentElement.id;
              chai.assert.equal(parent_id, "static_card_body");
            });

            it('should be a div element', function() {
              chai.assert.equal(child.nodeName, "DIV");
            });

            it('should have 1 child', function() {
              chai.assert.equal(child.childElementCount, 1);
            });

            it('should have child of type span', function() {
              chai.assert.equal(child.children[0].nodeName, "SPAN");
            });
          });

          it('should have a child of type span', function() {
            chai.assert.equal($("#static_card_body").children()[1].nodeName, "SPAN");
          });
        });
      });
    });
  }

};
// Test that the Card class properly gets attached and laid out via emp::web::Document.
struct Test_Card_INIT_OPEN_HTMLLayout : public emp::web::BaseTest {

  /*
   * Construct the following HTML structure:
   *
   * <div id="emp_test_container">
   * <div id="open" class="card">
   *
   *  <div id="open_card_header" aria-controls=".open_card_collapse" aria-expanded="true" class="card-header , collapse_toggle , collapse_toggle_card_header" data-target=".open_card_collapse" data-toggle="collapse" role="button">
   *    <span id="emp__3" class="fa fa-angle-double-up , toggle_glyph"></span>
   *    <span id="emp__4" class="fa fa-angle-double-down , toggle_glyph"></span>
   *    <div id="div_header">
   *      <span id="emp__6">
   *        <h3>init open title</h3>
   *      </span>
   *    </div>
   *  </div>
   *
   *  <div id="open_card_body" class="card-body , collapse show , open_card_collapse">
   *    <span id="emp__5">
   *      <p>collapsible card body</p>
   *      <p>alternatively inserted text</p>
   *    </span>
   *  </div>
   *
   * </div>
   * </div>
   */

  Test_Card_INIT_OPEN_HTMLLayout()
  : BaseTest({"emp_test_container"}) // we can tell BaseTest that we want to create a set of emp::web::Document
                                     // objects for each given html element ids.
  {
    emp::prefab::Card open_card("INIT_OPEN", true, "open");
    Doc("emp_test_container") << open_card;
    emp::web::Div header("div_header");
    open_card.AddHeaderContent(header);
    open_card.AddBodyContent("<p>collapsible card body</p>");
    open_card << "<p>alternatively insterted text</p>";
    header << "<h3>init open title</h3>";

  }

  void Describe() override {

    EM_ASM({
      describe("emp::prefab::Card INIT_OPEN HTML Layout Scenario", function() {

        // test that everything got layed out correctly in the HTML document
        describe("div#emp_test_container", function() {
          it('should exist', function() {
            chai.assert.equal($( "div#emp_test_container" ).length, 1);
          });

          it('should have child div#open', function() {
            chai.assert.equal($("div#emp_test_container").children("div#open").length, 1);
          });
        });

        describe("div#open [Card]", function() {
          it('should exist', function() {
            chai.assert.equal($( "div#open" ).length, 1);
          });

          it('should have parent #test_div', function() {
            const parent_id = $("#open").parent().attr("id");
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have the class card', function() {
            chai.assert.isTrue(document.getElementById('open').classList.contains('card'));
          });

          it('should have two children', function() {
            chai.assert.equal($("div#open").children().length, 2);
          });

          it('should have child div#open_card_header', function() {
            chai.assert.equal($("div#open").children("div#open_card_header").length, 1);
          });

          it('should have child div#open_card_body', function() {
            chai.assert.equal($("div#open").children("div#open_card_body").length, 1);
          });
        });

        describe("div#open_card_header", function() {
          const header = document.getElementById('open_card_header');
          it('should exist', function() {
            chai.assert.equal($("div#open_card_header").length, 1);
          });

          it('should have parent #open', function() {
            const parent_id = header.parentNode.getAttribute("id");
            chai.assert.equal(parent_id, "open");
          });

          it('should have class card-header', function() {
            chai.assert.isTrue(header.classList.contains('card-header'));
          });

          it('should have class collapse_toggle', function() {
            chai.assert.isTrue(header.classList.contains('collapse_toggle'));
          });

          it('should have class collapse_toggle_card_header', function() {
            chai.assert.isTrue(header.classList.contains('collapse_toggle_card_header'));
          });

          it('should have aria-controls set to .open_card_collapse', function() {
            chai.assert.equal(header.getAttribute("aria-controls"), ".open_card_collapse");
          });

          it('should have aria-expaned set to true', function() {
            chai.assert.equal(header.getAttribute("aria-expanded"), "true");
          });

          it('should have data-target set to .open_card_collapse', function() {
            chai.assert.equal(header.getAttribute("data-target"), ".open_card_collapse");
          });

          it('should have data-toggle set to collapse', function() {
            chai.assert.equal(header.getAttribute("data-toggle"), "collapse");
          });

          it('should have role set to button', function() {
            chai.assert.equal(header.getAttribute("role"), "button");
          });

          it('should have three children', function() {
            chai.assert.equal(header.childElementCount, 3);
          });
        });

        describe("Glyph Icons [in card header]", function() {
          describe("arrow_up glyph [header child 1]", function() {
            const glyph = $("#open_card_header").children()[0];
            it('should have parent #open_card_header', function() {
              const parent_id = glyph.parentElement.id;
              chai.assert.equal(parent_id, "open_card_header");
            });

            it('should not have children', function() {
              chai.assert.equal(glyph.childElementCount, 0);
            });

            it('should be a span element', function() {
              chai.assert.equal(glyph.nodeName, "SPAN");
            });

            it('should have class fa', function() {
              chai.assert.isTrue(glyph.classList.contains("fa"));
            });

            it('should have class fa-angle-double-up', function() {
              chai.assert.isTrue(glyph.classList.contains("fa-angle-double-up"));
            });

            it('should have class toggle_glyph', function() {
              chai.assert.isTrue(glyph.classList.contains("toggle_glyph"));
            });
          });

          describe("arrow_down glyph [header child 2]", function() {
            const glyph = $("#open_card_header").children()[1];
            it('should have parent #open_card_header', function() {
              const parent_id = glyph.parentElement.id;
              chai.assert.equal(parent_id, "open_card_header");
            });

            it('should not have children', function() {
              chai.assert.equal(glyph.childElementCount, 0);
            });

            it('should be a span element', function() {
              chai.assert.equal(glyph.nodeName, "SPAN");
            });

            it('should have class fa', function() {
              chai.assert.isTrue(glyph.classList.contains("fa"));
            });

            it('should have class fa-angle-double-down', function() {
              chai.assert.isTrue(glyph.classList.contains("fa-angle-double-down"));
            });

            it('should have class toggle_glyph', function() {
              chai.assert.isTrue(glyph.classList.contains("toggle_glyph"));
            });
          });
        });

        describe("div#div_header [header child 3]", function() {
          const header_div = document.getElementById("div_header");
          it('should exist', function(){
            chai.assert.equal($( "div#div_header" ).length, 1);
          });

          it('should have parent #open_card_header', function() {
            const parent_id = header_div.parentElement.id;
            chai.assert.equal(parent_id, "open_card_header");
          });

          it('should have one child', function() {
            chai.assert.equal(header_div.childElementCount, 1);
          });

          describe("div#div_header child", function() {
            const child = header_div.children[0];
            it('should have parent #div_header', function(){
              chai.assert.equal(child.parentElement.id, "div_header");
            });

            it('should be a span element', function() {
              chai.assert.equal(child.nodeName, "SPAN");
            });

            it('should have one child', function() {
              chai.assert.equal(child.childElementCount, 1);
            });

            describe("div#div_header grandchild", function() {
              const grandchild = child.children[0];
              it('should be a h3 element', function() {
                chai.assert.equal(grandchild.nodeName, "H3");
              });

              it('should not have children', function() {
                chai.assert.equal(grandchild.childElementCount, 0);
              });
            });
          });
        });

        describe("div#open_card_body", function() {
          const body = document.getElementById("open_card_body");
          it('should exist', function() {
            chai.assert.equal($( "div#open_card_body" ).length, 1);
          });

          it('should have parent #open', function() {
            const parent_id = $("#open_card_body").parent().attr("id");
            chai.assert.equal(parent_id, "open");
          });

          it('should have class card-body', function() {
            chai.assert.isTrue(body.classList.contains("card-body"));
          });

          it('should have class collapse', function() {
            chai.assert.isTrue(body.classList.contains("collapse"));
          });

          it('should have class show', function() {
            chai.assert.isTrue(body.classList.contains("show"));
          });

          it('should have class open_card_collapse', function() {
            chai.assert.isTrue(body.classList.contains("open_card_collapse"));
          });

          it('should have one child', function() {
            chai.assert.equal($("#open_card_body").children().length, 1);
          });

          describe("Child", function() {
            const child = $("#open_card_body").children()[0];
            it('should have parent #open_card_body', function() {
              chai.assert.equal(child.parentElement.id, "open_card_body");
            });

            it('should be a span element', function() {
              chai.assert.equal(child.nodeName, "SPAN");
            });

            it('should have two children', function() {
              chai.assert.equal(child.childElementCount, 2);
            });
          });

          describe("Grandchild", function() {
            const grandchild1 = $("#open_card_body").children()[0].children[0];

            it('should be a p element', function() {
              chai.assert.equal(grandchild1.nodeName, "P");
            });

            it('should have no children', function() {
              chai.assert.equal(grandchild1.childElementCount, 0);
            });

            const grandchild2 = $("#open_card_body").children()[0].children[1];

            it('should be a p element', function() {
              chai.assert.equal(grandchild2.nodeName, "P");
            });

            it('should have no children', function() {
              chai.assert.equal(grandchild2.childElementCount, 0);
            });
          });
        });
      });

      // When #open_card_header is clicked first time --->
      describe("Click to collapse card", function() {
        const card_head = document.getElementById('open_card_header');
        it('card header should have class collapsed', function() {
          card_head.click(); // collapses card
          chai.assert.isTrue(card_head.classList.contains("collapsed"));
        });

        it('card header should have aria-expanded set to false', function() {
          chai.assert.equal(card_head.getAttribute('aria-expanded'), 'false');
        });

        it('card body should not have class show', function() {
          const body = document.getElementById('open_card_body');
          chai.assert.isFalse(body.classList.contains("show"));
        });
      });

      // TODO: When Issue 368 is resolved, implement here
      // https://github.com/devosoft/Empirical/issues/368

      // When #open_card_header is clicked second time --->
      // describe("Click to expand card", function() {
      //   const card_head = document.getElementById('open_card_header');
      //   it('card header should not have class collapsed', function() {
      //     card_head.click(); // expands card
      //     chai.assert.isFalse(card_head.classList.contains("collapses"));
      //   });

      //   it('card header should have aria-expanded set to true', function() {
      //     chai.assert.equal(card_head.getAttribute('aria-expanded'), 'true');
      //   });

      //   it('card body should have class show', function() {
      //     const body = document.getElementById('open_card_body');
      //     chai.assert.isTrue(body.classList.contains("show"));
      //   });
      // });

    });
  }

};

struct Test_Card_INIT_CLOSED_HTMLLayout : public emp::web::BaseTest {

  /**
   * Construct the following HTML structure:
   *
   * <div id="emp_test_container">
   * <div id="closed" class="card">
   *
   *  <div id="closed_card_header" aria-controls=".closed_card_collapse" aria-expanded="false" class="card-header , collapse_toggle , collapsed , collapse_toggle_card_header" data-target=".closed_card_collapse" data-toggle="collapse" role="button">
   *    <div id="emp__8" class="btn-link">
   *      <span id="emp__9">linked Header</span>
   *    </div>
   *  </div>
   *
   *  <div id="closed_card_body" class="card-body , collapse , closed_card_collapse"></div>
   *
   * </div>
   * </div>
   */

  Test_Card_INIT_CLOSED_HTMLLayout()
  : BaseTest({"emp_test_container"}) // we can tell BaseTest that we want to create a set of emp::web::Document
                                     // objects for each given html element ids.
  {
    emp::prefab::Card closed_card("INIT_CLOSED", false, "closed");
    Doc("emp_test_container") << closed_card;
    closed_card.AddHeaderContent("linked Header", true);
  }

  void Describe() override {

    EM_ASM({
      describe("emp::prefab::Card INIT_CLOSED HTML Layout Scenario", function() {

        // test that everything got layed out correctly in the HTML document
        describe("div#emp_test_container", function() {
          it('should exist', function() {
            chai.assert.equal($( "div#emp_test_container" ).length, 1);
          });

          it('should have child div#closed', function() {
            chai.assert.equal($("div#emp_test_container").children("div#closed").length, 1);
          });
        });

        describe("div#closed [Card]", function() {
          it('should exist', function() {
            chai.assert.equal($( "div#closed" ).length, 1);
          });

          it('should have parent #test_div', function() {
            const parent_id = $("#closed").parent().attr("id");
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have the class card', function() {
            chai.assert.isTrue(document.getElementById("closed").classList.contains("card"));
          });

          it('should have two children', function() {
            chai.assert.equal($("div#closed").children().length, 2);
          });

          it('should have child div#closed_card_header', function() {
            chai.assert.equal($("div#closed").children("div#closed_card_header").length, 1);
          });

          it('should have child div#closed_card_body', function() {
            chai.assert.equal($("div#closed").children("div#closed_card_body").length, 1);
          });
        });

        describe("div#closed_card_header", function() {
          const header = document.getElementById("closed_card_header");
          it('should exist', function() {
            chai.assert.equal($("div#closed_card_header").length, 1);
          });

          it('should have parent #closed', function() {
            const parent_id = $("#closed_card_header").parent().attr("id");
            chai.assert.equal(parent_id, "closed");
          });

          it('should have class card-header', function() {
            chai.assert.isTrue(header.classList.contains("card-header"));
          });

          it('should have class collapse_toggle', function() {
            chai.assert.isTrue(header.classList.contains("collapse_toggle"));
          });

          it('should have class collapse_toggle_card_header', function() {
            chai.assert.isTrue(header.classList.contains("collapse_toggle_card_header"));
          });

          it('should have aria-controls set to .closed_card_collapse', function() {
            chai.assert.equal(header.getAttribute("aria-controls"), ".closed_card_collapse");
          });

          it('should have aria-expaned set to false', function() {
            chai.assert.equal(header.getAttribute("aria-expanded"), "false");
          });

          it('should have data-target set to .closed_card_collapse', function() {
            chai.assert.equal(header.getAttribute("data-target"), ".closed_card_collapse");
          });

          it('should have data-toggle set to collapse', function() {
            chai.assert.equal(header.getAttribute("data-toggle"), "collapse");
          });

          it('should have role set to button', function() {
            chai.assert.equal(header.getAttribute("role"), "button");
          });

          it('should have one child', function() {
            chai.assert.equal($("#closed_card_header").children().length, 1);
          });

          describe("Child, link with bootstrap styles", function() {
            const link = $("#closed_card_header").children()[0];
            it('should be a div element', function() {
              chai.assert.equal(link.nodeName, "DIV");
            });

            it('should have class btn-link', function() {
              chai.assert.isTrue(link.classList.contains("btn-link"));
            });

            it('should have one child', function() {
              chai.assert.equal(link.childElementCount, 1);
            });
          });

          describe("Grandchild", function() {
            const grandchild = $("#closed_card_header").children()[0].children[0];
            it('should be a span element', function() {
              chai.assert.equal(grandchild.nodeName, "SPAN");
            });

            it('should have no children', function() {
              chai.assert.equal(grandchild.childElementCount, 0);
            });
          });
        });

        describe("div#closed_card_body", function() {
          const body = document.getElementById("closed_card_body");
          it('should exist', function() {
            chai.assert.equal($( "div#closed_card_body" ).length, 1);
          });

          it('should have parent #closed', function() {
            const parent_id = $("#closed_card_body").parent().attr("id");
            chai.assert.equal(parent_id, "closed");
          });

          it('should have class card-body', function() {
            chai.assert.isTrue(body.classList.contains("card-body"));
          });

          it('should have class collapse', function() {
            chai.assert.isTrue(body.classList.contains("collapse"));
          });

          it('should have class closed_card_collapse', function() {
            chai.assert.isTrue(body.classList.contains("closed_card_collapse"));
          });

          it('should not have class show', function() {
            chai.assert.isFalse(body.classList.contains("show"));
          });

          it('should have no children', function() {
            chai.assert.equal($("#closed_card_body").children().length, 0);
          });
        });
      });

      // When #closed_card_header is clicked first time --->
      describe("Click to expand card", function() {
        const card_head = document.getElementById('closed_card_header');
        it('card header should not have class collapsed', function() {
          card_head.click(); // expands card
          chai.assert.isFalse(card_head.classList.contains("collapsed"));
        });

        it('card header should have aria-expanded set to true', function() {
          chai.assert.equal(card_head.getAttribute('aria-expanded'), 'true');
        });

        it('card body should have class show', function() {
          const body = document.getElementById('closed_card_body');
          setTimeout(function() {
            chai.assert.isTrue(body.classList.contains("show"));
          }, 3000);
        });
      });

      // TODO: When Issue 368 is resolved, implement here
      // https://github.com/devosoft/Empirical/issues/368

      // When #closed_card_header is clicked second time --->
      // describe("Click to collapse card", function() {
      //   const card_head = document.getElementById('closed_card_header');

      //   it('card header should have class collapsed', function() {
      //     card_head.click(); // collapses card
      //     chai.assert.isTrue(card_head.classList.contains("collapses"));
      //   });

      //   it('card header should have aria-expanded set to false', function() {
      //     chai.assert.equal(card_head.getAttribute('aria-expanded'), 'false');
      //   });

      //   it('card body should not have class show', function() {
      //     const body = document.getElementById('closed_card_body');
      //     chai.assert.isFalse(body.classList.contains("show"));
      //   });
      // });

    });
  }

};
// Create a MochaTestRunner object in the global namespace so that it hangs around after main finishes.
emp::web::MochaTestRunner test_runner;

int main() {
  test_runner.Initialize({"emp_test_container"});

  test_runner.AddTest<Test_Card_STATIC_HTMLLayout>("Test Static Card HTML Layout");
  test_runner.AddTest<Test_Card_INIT_OPEN_HTMLLayout>("Test Initially Open Card HTML Layout");
  test_runner.AddTest<Test_Card_INIT_CLOSED_HTMLLayout>("Test Initially Closed Card HTML Layout");

  test_runner.Run();
}
