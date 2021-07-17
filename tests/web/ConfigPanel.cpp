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

  Test_Config_Panel_HTMLLayout()
  : BaseTest({"emp_test_container"})
  {
    // apply configuration query params and config files to Config
    auto specs = emp::ArgManager::make_builtin_specs(&cfg);
    emp::ArgManager am(emp::web::GetUrlParams(), specs);
    // cfg.Read("config.cfg");
    am.UseCallbacks();
    if (am.HasUnused()) std::exit(EXIT_FAILURE);
      emp::prefab::ConfigPanel config_panel{cfg, true, "settings"};
    config_panel.ExcludeSetting("BOOL_EX");
    config_panel.ExcludeGroup("TREATMENT");

    Doc("emp_test_container") << config_panel;
    emp::prefab::CloseLoadingModal();
  }

  void Describe() override {

    EM_ASM({
      describe("emp::prefab::ConfigPanel HTML Layout Scenario", function() {

        // test that everything is laid out correctly in the HTML document
        describe("div#emp_test_container", function() {
          it('should exist', function() {
            chai.assert.equal($( "div#emp_test_container" ).length, 1);
          });

          it('should have one child', function() {
            chai.assert.equal($("div#emp_test_container").children().length, 1);
          });
        });

        // Config panel has id 'settings'
        const config_panel = document.getElementById('settings');

        describe("ConfigPanel (div#emp_test_container Child)", function() {
          it('should exist', function() {
            chai.assert.notEqual(config_panel, null);
          });

          it('should have parent #emp_test_container', function() {
            const parent_id = config_panel.parentElement.id;
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have 4 children (3 groups, 1 control panel)', function() {
            chai.assert.equal(config_panel.childElementCount, 4);
          });

          it('should have child #settings_MAIN_outer', function() {
            chai.assert.equal(config_panel.children[0].id, "settings_MAIN_outer");
          });

          it('should have child #settings_CELL_outer', function() {
            chai.assert.equal(config_panel.children[1].id, "settings_CELL_outer");
          });

          it('should have child #settings_TREATMENT_outer', function() {
            chai.assert.equal(config_panel.children[2].id, "settings_TREATMENT_outer");
          });
          it('should have child #settings_control', function() {
            chai.assert.equal(config_panel.children[3].id, "settings_controls");
          });
        });

        describe("#settings_MAIN_outer", function() {
          const main_card =  document.getElementById("settings_MAIN_outer");
          it('should have parent ConfigPanel', function() {
            const parent_id = main_card.parentElement.id;
            chai.assert.equal(parent_id, config_panel.id);
          });

          it('should have class card', function() {
            chai.assert.isTrue(main_card.classList.contains("card"));
          });

          it('should have 2 child (card header and body)', function() {
            chai.assert.equal(main_card.childElementCount, 2);
          });

          it('should have card header child', function() {
            chai.assert.isTrue(main_card.children[0].classList.contains("card-header"));
          });

          it('should have card body child', function() {
            chai.assert.isTrue(main_card.children[1].classList.contains("card-body"));
          });

          // Note: Not checking card toggle functionality here because
          // it is tested in Card.cc mocha tests

          describe("Card Header", function() {
            const card_header = main_card.children[0];
            it('should have 3 children', function() {
              chai.assert.equal(card_header.childElementCount, 3);
            });

            it('should have an arrow up glyph', function() {
              chai.assert.isTrue(card_header.children[0].classList.contains("fa-angle-double-up"));
            });

            it('should have an arrow down glyph', function() {
              chai.assert.isTrue(card_header.children[1].classList.contains("fa-angle-double-down"));
            });
          });

          describe("#settings_MAIN (card body's child)", function() {
            const main_settings_group = document.getElementById("settings_MAIN");
            it('should exist', function() {
              chai.assert.isNotNull(main_settings_group);
            });

            it('should have 3 settings', function() {
              chai.assert.equal(main_settings_group.childElementCount, 3);
            });
          });
        });

        describe("#setting_CELL_outer", function() {
          const cell_card  =  document.getElementById("settings_CELL_outer");
          it('should have parent Config Panel div', function() {
            chai.assert.equal(cell_card.parentElement.id, config_panel.id);
          });

          it('should have card class', function() {
              chai.assert.isTrue(cell_card.classList.contains("card"));
          });

          it('should have 2 children', function() {
            chai.assert.equal(cell_card.childElementCount, 2);
          });

          it('should have card header child', function() {
            chai.assert.isTrue(cell_card.children[0].classList.contains("card-header"));
          });

          it('should have card body child', function() {
            chai.assert.isTrue(cell_card.children[1].classList.contains("card-body"));
          });

          // Note: Not checking card toggle functionality here because
          // it is tested in Card.cc mocha tests

          describe("Card Header", function() {
            const card_header = cell_card.children[0];
            it('should have 3 children', function() {
              chai.assert.equal(card_header.childElementCount, 3);
            });

            it('should have an arrow up glyph', function() {
              chai.assert.isTrue(card_header.children[0].classList.contains("fa-angle-double-up"));
            });

            it('should have an arrow down glyph', function() {
              chai.assert.isTrue(card_header.children[1].classList.contains("fa-angle-double-down"));
            });
          });

          describe("#settings_CELL (card body's child)", function() {
            const cell_settings_group = document.getElementById("settings_CELL");
            it('should exist', function() {
              chai.assert.isNotNull(cell_settings_group);
            });

            it('should have 2 settings', function() {
              chai.assert.equal(cell_settings_group.childElementCount, 2);
            });
          });
        });

        describe("#settings_TREATMENT_outer", function() {
          const treatment_card = document.getElementById("settings_TREATMENT_outer");
          it('should have parent Config Panel div', function() {
            chai.assert.equal(treatment_card.parentElement.id, config_panel.id);
          });

          it('should have card class', function() {
            chai.assert.isTrue(treatment_card.classList.contains("card"));
          });

          it('should have 2 children', function() {
            chai.assert.equal(treatment_card.childElementCount, 2);
          });

          it('should have card header child', function() {
            chai.assert.isTrue(treatment_card.children[0].classList.contains("card-header"));
          });

          it('should have card body child', function() {
            chai.assert.isTrue(treatment_card.children[1].classList.contains("card-body"));
          });

          // Note: Not checking card toggle functionality here because
          // it is tested in Card.cc mocha tests

          describe("Card Header", function() {
            const card_header = treatment_card.children[0];
            it('should have 3 children', function() {
              chai.assert.equal(card_header.childElementCount, 3);
            });

            it('should have an arrow up glyph', function() {
              chai.assert.isTrue(card_header.children[0].classList.contains("fa-angle-double-up"));
            });

            it('should have an arrow down glyph', function() {
              chai.assert.isTrue(card_header.children[1].classList.contains("fa-angle-double-down"));
            });
          });

          describe("#settings_TREATMENT (card body's child)", function() {
            const treatment_settings_group = document.getElementById("settings_TREATMENT");
            it('should exist', function() {
              chai.assert.isNotNull(treatment_settings_group);
            });

            it('should have 2 settings', function() {
              chai.assert.equal(treatment_settings_group.childElementCount, 2);
            });
          });
        });

        // Control panel at bottom: buttons for refreshing page (+ more coming soon)
        describe("Controls panel", function() {
          const controls = document.getElementById("settings_controls");
          it('should exist', function() {
            chai.assert.isNotNull(controls);
          });

          it('should have parent #settings', function() {
            chai.assert.equal(controls.parentElement.id, config_panel.id);
          });

          it('should have 1 child',function() {
            chai.assert(controls.childElementCount, 1);
          });

          describe('child #settings_reset', function() {
            const reset = document.getElementById("settings_reset");
            it('should exist', function() {
              chai.assert.isNotNull(controls);
            });

            it('should have parent #settings_controls', function() {
              chai.assert.equal(reset.parentElement.id, controls.id);
            });

            it('should be a button', function() {
              chai.assert.equal(reset.nodeName, "BUTTON");
            });
          });

        });

        // Test that the "excluded" class is applied properly
        describe("setting and group exclusion checks", function() {
          const bool_ex_setting = document.getElementById("settings_BOOL_EX");
          it('#setting_BOOL_EX (single setting) should be excluded', function() {
            chai.assert.isTrue(bool_ex_setting.classList.contains("excluded"));
          });
          const treatment_group = document.getElementById("settings_TREATMENT_outer");
          it('#setting_BOOL_EX_outer (whole card) should be excluded', function() {
            chai.assert.isTrue(treatment_group.classList.contains("excluded"));
          });

        });
      });
    });
  }
};

emp::web::MochaTestRunner test_runner;

int main() {
  test_runner.Initialize({"emp_test_container"});
  test_runner.AddTest<Test_Config_Panel_HTMLLayout>("Test ConfigPanel HTML Layout");
  test_runner.Run();
}
