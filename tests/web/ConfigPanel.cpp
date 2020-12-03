//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <functional>
#include <unordered_map>

#include "emp/base/assert.hpp"
#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/web.hpp"
#include "emp/web/UrlParams.hpp"

#include "emp/config/command_line.hpp"
#include "emp/config/ArgManager.hpp"
#include "emp/prefab/ConfigPanel.hpp"
#include "emp/prefab/LoadingModal.hpp"

#include "assets/Config.hpp"

Config cfg; // config class used in all test to construct config panel

// Test that the ConfigPanel class properly gets attached and laid out via emp::web::Document.
struct Test_Config_Panel_HTMLLayout : public emp::web::BaseTest {

  /*
   * Construct the following HTML structure:
   *
   * <div id="emp_base">
   *  <div id="emp__1">
   *    <div id="settings_MAIN">
   *      <div id="emp__3" class="card">
   *        <div id="emp__3_card_header" aria-controls=".emp__3_card_collapse" aria-expanded="true" class="card-header , collapse_toggle , collapse_toggle_card_header" data-target=".emp__3_card_collapse" data-toggle="collapse" role="button">
   *          <span id="emp__5" class="fa fa-angle-double-up , float-right btn-link collapse_toggle setting_heading"></span>
   *          <span id="emp__6" class="fa fa-angle-double-down , float-right btn-link collapse_toggle setting_heading"></span>
   *          <div id="emp__7" class="setting_heading"><span id="emp__8"><h3>Global settings</h3></span></div>
   *        </div>
   *
   *      <div id="emp__3_card_body" class="card-body , collapse show , emp__3_card_collapse">
   *        <div id="emp__9">
   *          <div id="BOOL_EX_row" class="setting_element">
   *            <button id="emp__11" aria-controls=".BOOL_EX_dropdown" aria-expanded="false" class="title_area , btn btn-link , collapse_toggle , collapsed" data-target=".BOOL_EX_dropdown" data-toggle="collapse" role="button">
   *              <span id="emp__12" class="fa fa-angle-double-right , toggle_icon_right_margin"></span>
   *              <span id="emp__13" class="fa fa-angle-double-up , toggle_icon_right_margin"></span>
   *              <span id="emp__14">Bool Ex</span>
   *            </button>
   *            <span id="BOOL_EX_input_checkbox" style="clear: none; display: inline;" class="custom-control custom-switch , input_bool">
   *              <input type="checkbox" id="emp__19" onchange="emp.Callback(2, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" checked="checked" class="custom-control-input">
   *              <label id="emp__18" class="custom-control-label" for="emp__19"></label>
   *            </span>
   *          </div>
   *          <div id="emp__15" class="collapse , BOOL_EX_dropdown">
   *            <div id="emp__15_triangle" class="commentbox_triangle"></div>
   *            <div id="emp__15_all_content" class="commentbox_content">
   *              <div id="emp__15_desktop_content"><span id="emp__16">example description</span></div>
   *              <div id="emp__15_mobile_content" class="mobile_commentbox"></div>
   *            </div>
   *          </div>
   *        </div>
   *
   *        <div id="emp__20">
   *          <div id="SEED_row" class="setting_element">
   *            <button id="emp__22" aria-controls=".SEED_dropdown" aria-expanded="false" class="title_area , btn btn-link , collapse_toggle , collapsed" data-target=".SEED_dropdown" data-toggle="collapse" role="button">
   *              <span id="emp__23" class="fa fa-angle-double-right , toggle_icon_right_margin"></span>
   *              <span id="emp__24" class="fa fa-angle-double-up , toggle_icon_right_margin"></span>
   *              <span id="emp__25">Seed</span>
   *            </button>
   *            <input type="range" min="-10" max="10" value="-1" id="SEED_input_slider" onchange="emp.Callback(3, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" class="input_slider">
   *            <input type="number" min="-10" max="10" value="-1" id="SEED_input_number" onchange="emp.Callback(4, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" class="input_number">
   *          </div>
   *          <div id="emp__26" class="collapse , SEED_dropdown">
   *            <div id="emp__26_triangle" class="commentbox_triangle"></div>
   *            <div id="emp__26_all_content" class="commentbox_content">
   *              <div id="emp__26_desktop_content"><span id="emp__27">Random number generator seed</span></div>
   *              <div id="emp__26_mobile_content" class="mobile_commentbox">
   *                <span id="emp__29"><hr></span>
   *                <input type="range" min="-10" max="10" value="-1" id="SEED_input_mobile_slider" onchange="emp.Callback(5, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);">
   *              </div>
   *            </div>
   *          </div>
   *        </div>
   *
   *        <div id="emp__30">
   *          <div id="RADIATION_PRESCRIPTION_FILE_row" class="setting_element">
   *            <button id="emp__32" aria-controls=".RADIATION_PRESCRIPTION_FILE_dropdown" aria-expanded="false" class="title_area , btn btn-link , collapse_toggle , collapsed" data-target=".RADIATION_PRESCRIPTION_FILE_dropdown" data-toggle="collapse" role="button">
   *              <span id="emp__33" class="fa fa-angle-double-right , toggle_icon_right_margin"></span>
   *              <span id="emp__34" class="fa fa-angle-double-up , toggle_icon_right_margin"></span>
   *              <span id="emp__35">Radiation Prescription File</span>
   *            </button>
   *            <input type="text" value="none" id="RADIATION_PRESCRIPTION_FILE_input_textbox" onchange="emp.Callback(6, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" class="input_text">
   *          </div>
   *          <div id="emp__36" class="collapse , RADIATION_PRESCRIPTION_FILE_dropdown">
   *            <div id="emp__36_triangle" class="commentbox_triangle"></div>
   *            <div id="emp__36_all_content" class="commentbox_content">
   *              <div id="emp__36_desktop_content"><span id="emp__37">File containing radiation prescription</span></div>
   *              <div id="emp__36_mobile_content" class="mobile_commentbox"></div>
   *            </div>
   *          </div>
   *        </div>
   *      </div>
   *    </div>
   *  </div>
   *
   *  <div id="settings_CELL">
   *    <div id="emp__40" class="card">
   *      <div id="emp__40_card_header" aria-controls=".emp__40_card_collapse" aria-expanded="true" class="card-header , collapse_toggle , collapse_toggle_card_header" data-target=".emp__40_card_collapse" data-toggle="collapse" role="button">
   *        <span id="emp__42" class="fa fa-angle-double-up , float-right btn-link collapse_toggle setting_heading"></span>
   *        <span id="emp__43" class="fa fa-angle-double-down , float-right btn-link collapse_toggle setting_heading"></span>
   *        <div id="emp__44" class="setting_heading"><span id="emp__45"><h3>Cell settings</h3></span></div>
   *      </div>
   *
   *    <div id="emp__40_card_body" class="card-body , collapse show , emp__40_card_collapse">
   *      <div id="emp__46">
   *        <div id="NEUTRAL_MUTATION_RATE_row" class="setting_element">
   *          <button id="emp__48" aria-controls=".NEUTRAL_MUTATION_RATE_dropdown" aria-expanded="false" class="title_area , btn btn-link , collapse_toggle , collapsed" data-target=".NEUTRAL_MUTATION_RATE_dropdown" data-toggle="collapse" role="button">
   *            <span id="emp__49" class="fa fa-angle-double-right , toggle_icon_right_margin"></span>
   *            <span id="emp__50" class="fa fa-angle-double-up , toggle_icon_right_margin"></span>
   *            <span id="emp__51">Neutral Mutation Rate</span>
   *          </button>
   *          <input type="range" min="0" max="5" value="0.05" step="0.005" id="NEUTRAL_MUTATION_RATE_input_slider" onchange="emp.Callback(7, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" class="input_slider">
   *          <input type="number" min="0" max="5" value="0.05" step="0.005" id="NEUTRAL_MUTATION_RATE_input_number" onchange="emp.Callback(8, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" class="input_number">
   *        </div>
   *
   *        <div id="emp__52" class="collapse , NEUTRAL_MUTATION_RATE_dropdown">
   *          <div id="emp__52_triangle" class="commentbox_triangle"></div>
   *          <div id="emp__52_all_content" class="commentbox_content">
   *          <div id="emp__52_desktop_content"><span id="emp__53">Probability of a neutral mutation (only relevant for phylogenetic signature)</span></div>
   *            <div id="emp__52_mobile_content" class="mobile_commentbox">
   *              <span id="emp__55"><hr></span>
   *              <input type="range" min="0" max="5" value="0.05" step="0.005" id="NEUTRAL_MUTATION_RATE_input_mobile_slider" onchange="emp.Callback(9, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);">
   *            </div>
   *          </div>
   *        </div>
   *      </div>
   *
   *    <div id="emp__56"><div id="ASYMMETRIC_DIVISION_PROB_row" class="setting_element">
   *      ....
   * </div>
   */

  emp::prefab::ConfigPanel config_panel{cfg};

  Test_Config_Panel_HTMLLayout()
  : BaseTest({"emp_test_container"})
  {
    // apply configuration query params and config files to Config
    auto specs = emp::ArgManager::make_builtin_specs(&cfg);
    emp::ArgManager am(emp::web::GetUrlParams(), specs);
    // cfg.Read("config.cfg");
    am.UseCallbacks();
    if (am.HasUnused()) std::exit(EXIT_FAILURE);

    // setup configuration panel
    config_panel.Setup();
    Doc("emp_test_container") << config_panel.GetConfigPanelDiv();
    emp::prefab::CloseLoadingModal();

  }

  void Describe() override {

    EM_ASM({
      describe("emp::prefab::ConfigPanel HTML Layout Scenario", function() {

        // test that everything got layed out correctly in the HTML document
        describe("div#emp_test_container", function() {
          it('should exist', function() {
            chai.assert.equal($( "div#emp_test_container" ).length, 1);
          });

          it('should have one child', function() {
            chai.assert.equal($("div#emp_test_container").children().length, 1);
          });
        });

        // user doesn't define config panel ID, find object here
        const config_panel = document.getElementById('emp_test_container').children[0];

        describe("ConfigPanel (div#emp_test_container Child)", function() {
          it('should exist', function() {
            chai.assert.notEqual(config_panel, null);
          });

          it('should have parent #emp_test_container', function() {
            const parent_id = config_panel.parentElement.id;
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have 3 children', function() {
            chai.assert.equal(config_panel.childElementCount, 3);
          });

          it('should have child #settings_MAIN', function() {
            chai.assert.equal(config_panel.children[0].id, "settings_MAIN");
          });

          it('should have child #settings_CELL', function() {
            chai.assert.equal(config_panel.children[1].id, "settings_CELL");
          });

          it('should have child #settings_TREATMENT', function() {
            chai.assert.equal(config_panel.children[2].id, "settings_TREATMENT");
          });
        });

        describe("#settings_MAIN", function() {
          const main =  document.getElementById("settings_MAIN");
          it('should have parent ConfigPanel', function() {
            const parent_id = main.parentElement.id;
            chai.assert.equal(parent_id, config_panel.id);
          });

          it('should have 1 child ', function() {
            chai.assert.equal(main.childElementCount, 1);
          });

          const card = main.children[0];
          describe("Basic card layout", function() {
            it('should have class card', function() {
              chai.assert.isTrue(card.classList.contains("card"));
            });

            it('should have 2 children', function() {
              chai.assert.equal(card.childElementCount, 2);
            });

            it('should have card header child', function() {
              chai.assert.isTrue(card.children[0].classList.contains("card-header"));
            });

            it('should have card body child', function() {
              chai.assert.isTrue(card.children[1].classList.contains("card-body"));
            });

            // Note: Not checking card toggle functionality here because
            // it is tested in Card.cc mocha tests

            describe("Card Header", function() {
              const card_header = card.children[0];
              it('should have 3 children', function() {
                chai.assert.equal(card_header.childElementCount, 3);
              });

              it('should have an arrow up glyph', function() {
                chai.assert.isTrue(card_header.children[0].classList.contains("fa-angle-double-up"));
              });

              it('should have an arrow down glyph', function() {
                chai.assert.isTrue(card_header.children[1].classList.contains("fa-angle-double-down"));
              });

              it('should have a title with class setting_heading', function() {
                chai.assert.isTrue(card_header.children[2].classList.contains("setting_heading"));
              });
            });

            describe("Card Body", function() {
              const card_body = card.children[1];
              it('should have 3 settings', function() {
                chai.assert.equal(card_body.childElementCount, 3);
              });
            });
          });
        });

        describe("#setting_CELL", function() {
          const cell  =  document.getElementById("settings_CELL");
          it('should have parent Config Panel div', function() {
            chai.assert.equal(cell.parentElement.id, config_panel.id);
          });

          it('should have 1 child', function() {
            chai.assert.equal(cell.childElementCount, 1);
          });

          const card = cell.children[0];
          describe("Basic card layout", function() {
            it('should have card class', function() {
              chai.assert.isTrue(card.classList.contains("card"));
            });

            it('should have 2 children', function() {
              chai.assert.equal(card.childElementCount, 2);
            });

            it('should have card header child', function() {
              chai.assert.isTrue(card.children[0].classList.contains("card-header"));
            });

            it('should have card body child', function() {
              chai.assert.isTrue(card.children[1].classList.contains("card-body"));
            });

            // Note: Not checking card toggle functionality here because
            // it is tested in Card.cc mocha tests

            describe("Card Header", function() {
              const card_header = card.children[0];
              it('should have 3 children', function() {
                chai.assert.equal(card_header.childElementCount, 3);
              });

              it('should have an arrow up glyph', function() {
                chai.assert.isTrue(card_header.children[0].classList.contains("fa-angle-double-up"));
              });

              it('should have an arrow down glyph', function() {
                chai.assert.isTrue(card_header.children[1].classList.contains("fa-angle-double-down"));
              });

              it('should have a title with class setting_heading', function() {
                chai.assert.isTrue(card_header.children[2].classList.contains("setting_heading"));
              });
            });

            describe("Card Body", function() {
              const card_body = card.children[1];
              it('should have 2 settings', function() {
                chai.assert.equal(card_body.childElementCount, 2);
              });
            });
          });

        });

        describe("#setting_TREATMENT", function() {
          const treatment = document.getElementById("settings_TREATMENT");
          it('should have parent Config Panel div', function() {
            chai.assert.equal(treatment.parentElement.id, config_panel.id);
          });

          it('should have 1 child', function() {
            chai.assert.equal(treatment.childElementCount, 1);
          });

          const card = treatment.children[0];
          describe("Basic card layout", function() {
            it('should have card class', function() {
              chai.assert.isTrue(card.classList.contains("card"));
            });

            it('should have 2 children', function() {
              chai.assert.equal(card.childElementCount, 2);
            });

            it('should have card header child', function() {
              chai.assert.isTrue(card.children[0].classList.contains("card-header"));
            });

            it('should have card body child', function() {
              chai.assert.isTrue(card.children[1].classList.contains("card-body"));
            });

            // Note: Not checking card toggle functionality here because
            // it is tested in Card.cc mocha tests

            describe("Card Header", function() {
              const card_header = card.children[0];
              it('should have 3 children', function() {
                chai.assert.equal(card_header.childElementCount, 3);
              });

              it('should have an arrow up glyph', function() {
                chai.assert.isTrue(card_header.children[0].classList.contains("fa-angle-double-up"));
              });

              it('should have an arrow down glyph', function() {
                chai.assert.isTrue(card_header.children[1].classList.contains("fa-angle-double-down"));
              });

              it('should have a title with class setting_heading', function() {
                chai.assert.isTrue(card_header.children[2].classList.contains("setting_heading"));
              });
            });

            describe("Card Body", function() {
              const card_body = card.children[1];
              it('should have 2 settings', function() {
                chai.assert.equal(card_body.childElementCount, 2);
              });
            });
          });
        });
      });
    });
  }
};

struct Test_Config_Panel_Int_HTMLLayout : public emp::web::BaseTest {

  /*
   * Construct the following HTML structure:
   *
   *        <div id="emp__21">
   *          <div id="SEED_row" class="setting_element">
   *            <button id="emp__23" aria-controls=".SEED_dropdown" aria-expanded="false" class="title_area , btn btn-link , collapse_toggle , collapsed" data-target=".SEED_dropdown" data-toggle="collapse" role="button">
   *              <span id="emp__24" class="fa fa-angle-double-right , toggle_icon_right_margin"></span>
   *              <span id="emp__25" class="fa fa-angle-double-up , toggle_icon_right_margin"></span>
   *              <span id="emp__26">Seed</span>
   *            </button>
   *            <label for="SEED_input_slider"></label>
   *            <input for="SEED_input_number" type="range" min="-10" max="10" value="-1" id="SEED_input_slider" onchange="emp.Callback(3, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" class="input_slider">
   *            <label for="SEED_input_number"></label>
   *            <input type="number" min="-10" max="10" value="-1" id="SEED_input_number" onchange="emp.Callback(4, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" class="input_number">
   *          </div>
   *          <div id="emp__27" class="collapse , SEED_dropdown">
   *            <div id="emp__27_triangle" class="commentbox_triangle"></div>
   *            <div id="emp__27_all_content" class="commentbox_content">
   *              <div id="emp__27_desktop_content"><span id="emp__28">Random number generator seed</span></div>
   *              <div id="emp__27_mobile_content" class="mobile_commentbox">
   *                <span id="emp__29"><hr></span>
   *                <label for="SEED_input_mobile_slider"></label>
   *                <input type="range" min="-10" max="10" value="-1" id="SEED_input_mobile_slider" onchange="emp.Callback(5, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);">
   *              </div>
   *            </div>
   *          </div>
   *        </div>
   */

  emp::prefab::ConfigPanel config_panel{cfg};

  Test_Config_Panel_Int_HTMLLayout()
  : BaseTest({"emp_test_container"}) // we can tell BaseTest that we want to create a set of emp::web::Document
                                     // objects for each given html element ids.
  {
    // apply configuration query params and config files to Config
    auto specs = emp::ArgManager::make_builtin_specs(&cfg);
    emp::ArgManager am(emp::web::GetUrlParams(), specs);
    // cfg.Read("config.cfg");
    am.UseCallbacks();
    if (am.HasUnused()) std::exit(EXIT_FAILURE);

    // setup configuration panel
    config_panel.Setup();
    Doc("emp_test_container") << config_panel.GetConfigPanelDiv();
    emp::prefab::CloseLoadingModal();
  }

  void Describe() override {

    EM_ASM({
      describe("emp::prefab::ConfigPanel Integer Setting", function() {
        // Must use "children chain" because we rely on emscripten to generate
        // IDs for setting elements.
        const setting = document.getElementById("settings_MAIN").children[0].children[1].children[1];
        it('should have 2 children', function() {
          chai.assert.equal(setting.childElementCount, 2);
        });

        it('should have a child with class setting_element', function() {
          chai.assert.isTrue(setting.children[0].classList.contains("setting_element"));
        });

        it('should have a child with class collapse (comment box)', function() {
          chai.assert.isTrue(setting.children[1].classList.contains("collapse"));
        });

        describe("Setting row", function() {
          const row = setting.children[0];
          it('should have 3 children (title area, input number, input slider)', function() {
            chai.assert.equal(row.childElementCount, 3);
          });

          describe("Title area (child 1)", function(){
            const title_area = row.children[0];
            it('should have class title_area', function() {
              chai.assert.isTrue(title_area.classList.contains("title_area"));
            });

            it('should have 3 children (title + 2 glyphs)', function() {
              chai.assert.equal(title_area.childElementCount, 3);
            });

            it('should be a button element', function() {
              chai.assert.equal(title_area.nodeName, "BUTTON");
            });

            it('should have a right arrow glyph', function() {
              chai.assert.isTrue(title_area.children[0].classList.contains("fa-angle-double-right"));
            });

            it('should have an up arrow glyph', function() {
              chai.assert.isTrue(title_area.children[1].classList.contains("fa-angle-double-up"));
            });

            it('should have a span with title text', function() {
              chai.assert.equal(title_area.children[2].nodeName, "SPAN");
            });
          });

          describe("Slider input (child 2)", function() {
            const slider = row.children[1];
            it('should be an input element', function() {
              chai.assert.equal(slider.nodeName, "INPUT");
            });

            it('should have type "range"', function() {
              chai.assert.equal(slider.getAttribute("type"), "range");
            });

            it('should initially have a value of -1', function() {
              chai.assert.equal(slider.getAttribute("value"), "-1");
            });

            it('should have class "input_slider"', function() {
              chai.assert.isTrue(slider.classList.contains("input_slider"));
            });
          });

          describe("Numerical input (child 3)", function() {
            const number = row.children[2];
            it('should be an input element', function() {
              chai.assert.equal(number.nodeName, "INPUT");
            });

            it('should have type "number"', function() {
              chai.assert.equal(number.getAttribute("type"), "number");
            });

            it('should initially have a value of -1', function() {
              chai.assert.equal(number.getAttribute("value"), "-1");
            });

            it('should have class "input_number"', function() {
              chai.assert.isTrue(number.classList.contains("input_number"));
            });

          // TODO: check that the callback functions actually synchronize with the
          // all inputs (desktop slider, mobile slider [in dropdown], number input)
          });
        });

        describe("Setting description box (comment box)", function() {
          const box = setting.children[1];
          it('should have a child with class commentbox_triangle', function() {
            chai.assert.isTrue(box.children[0].classList.contains("commentbox_triangle"));
          });

          it('should have a child with class commentbox_content', function() {
            chai.assert.isTrue(box.children[1].classList.contains("commentbox_content"));
          });

          // TODO: When the mobile prefab tools is created, need to rethink how to
          // obtain the mobile slider object
          describe("Mobile slider", function() {
            const mobile_slider = box.children[1].children[1].children[1];
            it('should be an input element', function() {
              chai.assert.equal(mobile_slider.nodeName, "INPUT");
            });

            it('should have type "range"', function() {
              chai.assert.equal(mobile_slider.getAttribute("type"), "range");
            });

            it('should initially have a value of -1', function() {
              chai.assert.equal(mobile_slider.getAttribute("value"), "-1");
            });

            // TODO: check that the callback functions actually synchronize with the
            // all inputs (desktop slider, mobile slider [in dropdown], number input)
          });

          // Note: Not checking comment box collapsibility because this functionality should
          // be tested throughly with Collapse.cc
        });
      });
    });
  }
};

struct Test_Config_Panel_Double_HTMLLayout : public emp::web::BaseTest {

  /*
   * Construct the following HTML structure:
   *
   *      <div id="emp__47">
   *        <div id="NEUTRAL_MUTATION_RATE_row" class="setting_element">
   *          <button id="emp__49" aria-controls=".NEUTRAL_MUTATION_RATE_dropdown" aria-expanded="false" class="title_area , btn btn-link , collapse_toggle , collapsed" data-target=".NEUTRAL_MUTATION_RATE_dropdown" data-toggle="collapse" role="button">
   *            <span id="emp__50" class="fa fa-angle-double-right , toggle_icon_right_margin"></span>
   *            <span id="emp__51" class="fa fa-angle-double-up , toggle_icon_right_margin"></span>
   *            <span id="emp__52">Neutral Mutation Rate</span>
   *          </button>
   *          <label for="NEUTRAL_MUTATION_RATE_input_slider"></label>
   *          <input type="range" min="0" max="5" value="0.05" step="0.005" id="NEUTRAL_MUTATION_RATE_input_slider" onchange="emp.Callback(7, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" class="input_slider">
   *          <label for="NEUTRAL_MUTATION_RATE_input_number"></label>
   *          <input type="number" min="0" max="5" value="0.05" step="0.005" id="NEUTRAL_MUTATION_RATE_input_number" onchange="emp.Callback(8, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" class="input_number">
   *        </div>
   *
   *        <div id="emp__53" class="collapse , NEUTRAL_MUTATION_RATE_dropdown">
   *          <div id="emp__53_triangle" class="commentbox_triangle"></div>
   *          <div id="emp__53_all_content" class="commentbox_content">
   *          <div id="emp__53_desktop_content"><span id="emp__54">Probability of a neutral mutation (only relevant for phylogenetic signature)</span></div>
   *            <div id="emp__53_mobile_content" class="mobile_commentbox">
   *              <span id="emp__55"><hr></span>
   *              <label for="NEUTRAL_MUTATION_RATE_input_mobile_slider"></label>
   *              <input type="range" min="0" max="5" value="0.05" step="0.005" id="NEUTRAL_MUTATION_RATE_input_mobile_slider" onchange="emp.Callback(9, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);">
   *            </div>
   *          </div>
   *        </div>
   *      </div>
   */

  emp::prefab::ConfigPanel config_panel{cfg};

  Test_Config_Panel_Double_HTMLLayout()
  : BaseTest({"emp_test_container"}) // we can tell BaseTest that we want to create a set of emp::web::Document
                                     // objects for each given html element ids.
  {
    // apply configuration query params and config files to Config
    auto specs = emp::ArgManager::make_builtin_specs(&cfg);
    emp::ArgManager am(emp::web::GetUrlParams(), specs);
    // cfg.Read("config.cfg");
    am.UseCallbacks();
    if (am.HasUnused()) std::exit(EXIT_FAILURE);

    // setup configuration panel
    config_panel.Setup();
    Doc("emp_test_container") << config_panel.GetConfigPanelDiv();
    emp::prefab::CloseLoadingModal();
  }

  void Describe() override {

    EM_ASM({
      describe("Double Setting", function() {
      const setting = document.getElementById("settings_CELL").children[0].children[1].children[0];
      it('should have 2 children', function() {
        chai.assert.equal(setting.childElementCount, 2);
      });

      it('should have a child with class setting_element', function() {
        chai.assert.isTrue(setting.children[0].classList.contains("setting_element"));
      });

      it('should have a child with class collapse (comment box)', function() {
        chai.assert.isTrue(setting.children[1].classList.contains("collapse"));
      });

      describe("Setting row", function() {
        const row = setting.children[0];
        it('should have 3 children (title area, input number, input slider)', function() {
          chai.assert.equal(row.childElementCount, 3);
        });

        describe("Title area (child 1)", function(){
          const title_area = row.children[0];
          it('should have class title_area', function() {
            chai.assert.isTrue(title_area.classList.contains("title_area"));
          });

          it('should have 3 children (title + 2 glyphs)', function() {
            chai.assert.equal(title_area.childElementCount, 3);
          });

          it('should be a button element', function() {
            chai.assert.equal(title_area.nodeName, "BUTTON");
          });

          it('should have a right arrow glyph', function() {
            chai.assert.isTrue(title_area.children[0].classList.contains("fa-angle-double-right"));
          });

          it('should have an up arrow glyph', function() {
            chai.assert.isTrue(title_area.children[1].classList.contains("fa-angle-double-up"));
          });

          it('should have a span with title text', function() {
            chai.assert.equal(title_area.children[2].nodeName, "SPAN");
          });
        });

        describe("Slider input (child 2)", function() {
          const slider = row.children[1];
          it('should be an input element', function() {
            chai.assert.equal(slider.nodeName, "INPUT");
          });

          it('should have type "range"', function() {
            chai.assert.equal(slider.getAttribute("type"), "range");
          });

          it('should initially have a value of 0.05', function() {
            chai.assert.equal(slider.getAttribute("value"), "0.05");
          });

          it('should have class "input_slider"', function() {
            chai.assert.isTrue(slider.classList.contains("input_slider"));
          });
        });

        describe("Numerical input (child 3)", function() {
          const number = row.children[2];
          it('should be an input element', function() {
            chai.assert.equal(number.nodeName, "INPUT");
          });

          it('should have type "number"', function() {
            chai.assert.equal(number.getAttribute("type"), "number");
          });

          it('should initially have a value of 0.05', function() {
            chai.assert.equal(number.getAttribute("value"), "0.05");
          });

          it('should have class "input_number"', function() {
            chai.assert.isTrue(number.classList.contains("input_number"));
          });

        // TODO: check that the callback functions actually synchronize with the
        // all inputs (desktop slider, mobile slider [in dropdown], number input)
        });
      });

      describe("Setting description box (comment box)", function() {
        const box = setting.children[1];
        it('should have a child with class commentbox_triangle', function() {
          chai.assert.isTrue(box.children[0].classList.contains("commentbox_triangle"));
        });

        it('should have a child with class commentbox_content', function() {
          chai.assert.isTrue(box.children[1].classList.contains("commentbox_content"));
        });

        // TODO: When the mobile prefab tools is created, need to rethink how to
        // obtain the mobile slider object
        describe("Mobile slider", function() {
          const mobile_slider = box.children[1].children[1].children[1];
          it('should be an input element', function() {
            chai.assert.equal(mobile_slider.nodeName, "INPUT");
          });

          it('should have type "range"', function() {
            chai.assert.equal(mobile_slider.getAttribute("type"), "range");
          });

          it('should initially have a value of 0.05', function() {
            chai.assert.equal(mobile_slider.getAttribute("value"), "0.05");
          });

          // TODO: check that the callback functions actually synchronize with the
          // all inputs (desktop slider, mobile slider [in dropdown], number input)
        });

        // Note: Not checking comment box collapsibility because this functionality should
        // be tested throughly with Collapse.cc
      });
    });
    });
  }
};

struct Test_Config_Panel_Text_HTMLLayout : public emp::web::BaseTest {

  /*
   * Construct the following HTML structure:
   *
   *        <div id="emp__31">
   *          <div id="RADIATION_PRESCRIPTION_FILE_row" class="setting_element">
   *            <button id="emp__33" aria-controls=".RADIATION_PRESCRIPTION_FILE_dropdown" aria-expanded="false" class="title_area , btn btn-link , collapse_toggle , collapsed" data-target=".RADIATION_PRESCRIPTION_FILE_dropdown" data-toggle="collapse" role="button">
   *              <span id="emp__34" class="fa fa-angle-double-right , toggle_icon_right_margin"></span>
   *              <span id="emp__35" class="fa fa-angle-double-up , toggle_icon_right_margin"></span>
   *              <span id="emp__36">Radiation Prescription File</span>
   *            </button>
   *            <label for="RADIATION_PRESCRIPTION_FILE_input_textbox"></label>
   *            <input type="text" value="none" id="RADIATION_PRESCRIPTION_FILE_input_textbox" onchange="emp.Callback(6, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" class="input_text">
   *          </div>
   *          <div id="emp__37" class="collapse , RADIATION_PRESCRIPTION_FILE_dropdown">
   *            <div id="emp__37_triangle" class="commentbox_triangle"></div>
   *            <div id="emp__37_all_content" class="commentbox_content">
   *              <div id="emp__37_desktop_content"><span id="emp__38">File containing radiation prescription</span></div>
   *              <div id="emp__37_mobile_content" class="mobile_commentbox"></div>
   *            </div>
   *          </div>
   *        </div>
   */


  emp::prefab::ConfigPanel config_panel{cfg};

  Test_Config_Panel_Text_HTMLLayout()
  : BaseTest({"emp_test_container"}) // we can tell BaseTest that we want to create a set of emp::web::Document
                                     // objects for each given html element ids.
  {
    // apply configuration query params and config files to Config
    auto specs = emp::ArgManager::make_builtin_specs(&cfg);
    emp::ArgManager am(emp::web::GetUrlParams(), specs);
    // cfg.Read("config.cfg");
    am.UseCallbacks();
    if (am.HasUnused()) std::exit(EXIT_FAILURE);

    // setup configuration panel
    config_panel.Setup();
    Doc("emp_test_container") << config_panel.GetConfigPanelDiv();
    emp::prefab::CloseLoadingModal();
  }

  void Describe() override {

    EM_ASM({
      describe("Text Setting", function() {
      const setting = document.getElementById("settings_MAIN").children[0].children[1].children[2];
      it('should have 2 children', function() {
        chai.assert.equal(setting.childElementCount, 2);
      });

      it('should have a child with class setting_element', function() {
        chai.assert.isTrue(setting.children[0].classList.contains("setting_element"));
      });

      it('should have a child with class collapse (comment box)', function() {
        chai.assert.isTrue(setting.children[1].classList.contains("collapse"));
      });

      describe("Setting row", function() {
        const row = setting.children[0];
        it('should have 2 children (title area and text input)', function() {
          chai.assert.equal(row.childElementCount, 2);
        });

        describe("Title area (child 1)", function(){
          const title_area = row.children[0];
          it('should have class title_area', function() {
            chai.assert.isTrue(title_area.classList.contains("title_area"));
          });

          it('should have 3 children (title + 2 glyphs)', function() {
            chai.assert.equal(title_area.childElementCount, 3);
          });

          it('should be a button element', function() {
            chai.assert.equal(title_area.nodeName, "BUTTON");
          });

          it('should have a right arrow glyph', function() {
            chai.assert.isTrue(title_area.children[0].classList.contains("fa-angle-double-right"));
          });

          it('should have an up arrow glyph', function() {
            chai.assert.isTrue(title_area.children[1].classList.contains("fa-angle-double-up"));
          });

          it('should have a span with title text', function() {
            chai.assert.equal(title_area.children[2].nodeName, "SPAN");
          });
        });

        describe("Text input (child 2)", function() {
          const text = row.children[1];
          it('should be an input element', function() {
            chai.assert.equal(text.nodeName, "INPUT");
          });

          it('should have type "text"', function() {
            chai.assert.equal(text.getAttribute("type"), "text");
          });

          it('should initially have a value of "none', function() {
            chai.assert.equal(text.getAttribute("value"), "none");
          });

          it('should have class "input_text"', function() {
            chai.assert.isTrue(text.classList.contains("input_text"));
          });
        });
      });

      describe("Setting description box (comment box)", function() {
        const box = setting.children[1];
        it('should have a child with class commentbox_triangle', function() {
          chai.assert.isTrue(box.children[0].classList.contains("commentbox_triangle"));
        });

        it('should have a child with class commentbox_content', function() {
          chai.assert.isTrue(box.children[1].classList.contains("commentbox_content"));
        });

        // Not checking comment box collapsibility because this functionality should
        // be tested throughly with Collapse.cc
      });
    });
    });
  }
};

struct Test_Config_Panel_Bool_HTMLLayout : public emp::web::BaseTest {

  /*
   * Construct the following HTML structure:
   *
   *        <div id="emp__9">
   *          <div id="BOOL_EX_row" class="setting_element">
   *            <button id="emp__11" aria-controls=".BOOL_EX_dropdown" aria-expanded="false" class="title_area , btn btn-link , collapse_toggle , collapsed" data-target=".BOOL_EX_dropdown" data-toggle="collapse" role="button">
   *              <span id="emp__12" class="fa fa-angle-double-right , toggle_icon_right_margin"></span>
   *              <span id="emp__13" class="fa fa-angle-double-up , toggle_icon_right_margin"></span>
   *              <span id="emp__14">Bool Ex</span>
   *            </button>
   *            <span id="BOOL_EX_input_checkbox" style="clear: none; display: inline;" class="custom-control custom-switch , input_bool">
   *              <input type="checkbox" id="emp__19" onchange="emp.Callback(2, ['checkbox', 'radio'].includes(this.type) ? this.checked.toString() : this.value);" checked="checked" class="custom-control-input">
   *              <label id="emp__18" class="custom-control-label" for="emp__19"></label>
   *            </span>
   *          </div>
   *          <div id="emp__15" class="collapse , BOOL_EX_dropdown">
   *            <div id="emp__15_triangle" class="commentbox_triangle"></div>
   *            <div id="emp__15_all_content" class="commentbox_content">
   *              <div id="emp__15_desktop_content"><span id="emp__16">example description</span></div>
   *              <div id="emp__15_mobile_content" class="mobile_commentbox"></div>
   *            </div>
   *          </div>
   *        </div>
   */

  emp::prefab::ConfigPanel config_panel{cfg};

  Test_Config_Panel_Bool_HTMLLayout()
  : BaseTest({"emp_test_container"}) // we can tell BaseTest that we want to create a set of emp::web::Document
                                     // objects for each given html element ids.
  {
    // apply configuration query params and config files to Config
    auto specs = emp::ArgManager::make_builtin_specs(&cfg);
    emp::ArgManager am(emp::web::GetUrlParams(), specs);
    // cfg.Read("config.cfg");
    am.UseCallbacks();
    if (am.HasUnused()) std::exit(EXIT_FAILURE);

    // setup configuration panel
    config_panel.Setup();
    Doc("emp_test_container") << config_panel.GetConfigPanelDiv();
    emp::prefab::CloseLoadingModal();
  }

  void Describe() override {

    EM_ASM({
      describe("Boolean setting", function() {
        const setting = document.getElementById("settings_MAIN").children[0].children[1].children[0];
        it('should have 2 children', function() {
          chai.assert.equal(setting.childElementCount, 2);
        });

        it('should have a child with class setting_element', function() {
          chai.assert.isTrue(setting.children[0].classList.contains("setting_element"));
        });

        it('should have a child with class collapse (comment box)', function() {
          chai.assert.isTrue(setting.children[1].classList.contains("collapse"));
        });

        describe("Setting row", function() {
          const row = setting.children[0];
          it('should have 2 children (title area and toggle switch input)', function() {
            chai.assert.equal(row.childElementCount, 2);
          });

          describe("Title area (child 1)", function(){
            const title_area = row.children[0];
            it('should have class title_area', function(){
              chai.assert.isTrue(title_area.classList.contains("title_area"));
            });

            it('should have 3 children (title + 2 glyphs)', function() {
              chai.assert.equal(title_area.childElementCount, 3);
            });

            it('should have a button element as the child', function() {
              chai.assert.equal(title_area.nodeName, "BUTTON");
            });

            it('should have a right arrow glyph', function() {
              chai.assert.isTrue(title_area.children[0].classList.contains("fa-angle-double-right"));
            });

            it('should have a up arrow glyph', function() {
              chai.assert.isTrue(title_area.children[1].classList.contains("fa-angle-double-up"));
            });

            it('should have a span with title text', function() {
              chai.assert.equal(title_area.children[2].nodeName, "SPAN");
            });
          });

          describe("Toggle switch (child 2)", function() {
            const checkbox = row.children[1];
            it('should have class input_bool', function() {
              chai.assert.isTrue(checkbox.classList.contains("input_bool"));
            });

            it('should have two children', function() {
              chai.assert.equal(checkbox.childElementCount, 2);
            });

            it('should have a checkbox input as a child', function() {
              chai.assert.equal(checkbox.children[0].nodeName, "INPUT");
              chai.assert.equal(checkbox.children[0].getAttribute("type"), "checkbox");
            });

            it('should be initially toggled on', function() {
              chai.assert.isTrue(checkbox.children[0].hasAttribute("checked"));
            });
          });
        });

        describe("Setting description box (comment box)", function() {
          const box = setting.children[1];
          it('should have a child with class commentbox_triangle', function() {
            chai.assert.isTrue(box.children[0].classList.contains("commentbox_triangle"));
          });

          it('should have a child with class commentbox_content', function() {
            chai.assert.isTrue(box.children[1].classList.contains("commentbox_content"));
          });
          /*
           * Not checking comment box collapsibility because this functionality should
           * be tested throughly with Collapse.cc
           */
        });
      });
    });
  }
};

emp::web::MochaTestRunner test_runner;

int main() {
  test_runner.Initialize({"emp_test_container"});

  test_runner.AddTest<Test_Config_Panel_HTMLLayout>("Test ConfigPanel HTML Layout");
  test_runner.AddTest<Test_Config_Panel_Int_HTMLLayout>("Test ConfigPanel Integer Input HTML Layout");
  test_runner.AddTest<Test_Config_Panel_Double_HTMLLayout>("Test ConfigPanel Double Input HTML Layout");
  test_runner.AddTest<Test_Config_Panel_Text_HTMLLayout>("Test ConfigPanel Text Input HTML Layout");
  test_runner.AddTest<Test_Config_Panel_Bool_HTMLLayout>("Test ConfigPanel Boolean Input HTML Layout");

  test_runner.Run();
}
