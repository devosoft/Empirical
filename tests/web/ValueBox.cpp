/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file ValueBox.cpp
 */

#include <functional>
#include <unordered_map>

#include "emp/base/assert.hpp"
#include "emp/prefab/ValueBox.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/Element.hpp"
#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/web.hpp"

// Test that the ValueBox class is constructed correctly
struct Test_Value_Box_HTMLLayout : public emp::web::BaseTest {

  Test_Value_Box_HTMLLayout()
  : BaseTest({"emp_test_container"}) {
    Doc("emp_test_container") << emp::prefab::ValueBox("label", "description", "value_box");
  }

  void Describe() override {
    EM_ASM({
      describe("emp::prefab::ValueBox HTML Layout Scenario", function() {
        const value_display = document.getElementById("value_box");

        describe("Value box (#value_box)", function() {
          it('should exist', function() {
            chai.assert.notEqual(value_box, null);
          });

          it('should have parent #emp_test_container', function() {
            const parent_id = value_box.parentElement.id;
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have 3 children', function() {
            chai.assert.equal(value_box.childElementCount, 3);
          });
        });
      });
    });
  }
};

// Test that the ValueDisplay class is constructed correctly
struct Test_Value_Display_HTMLLayout : public emp::web::BaseTest {

  Test_Value_Display_HTMLLayout()
  : BaseTest({"emp_test_container"}) {
    Doc("emp_test_container") << emp::prefab::ValueDisplay(
      "label",
      "description",
      "value",
      "value_display"
    );
  }

  void Describe() override {
    EM_ASM({
      describe("emp::prefab::ValueDisplay HTML Layout Scenario", function() {
        const value_display = document.getElementById("value_display");

        describe("#value_display", function() {
          it('should exist', function() {
            chai.assert.notEqual(value_display, null);
          });

          it('should have parent #emp_test_container', function() {
            const parent_id = value_display.parentElement.id;
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have 3 children', function() {
            chai.assert.equal(value_display.childElementCount, 3);
          });

          describe("view", function() {
            const value_view = value_display.children[2];
            it('should exist', function() {
              chai.assert.notEqual(value_view, null);
            });

            it("should have a child", function() {
              chai.assert.equal(value_view.childElementCount, 1);
            });

            describe("view's child", function() {
              it("should be a span", function() {
                chai.assert(value_display.children[0].nodeName, "SPAN");
              });
            });
          });
        });
      });
    });
  }
};

// Test that the BoolValueControl class is constructed correctly
struct Test_Bool_Value_Control_HTMLLayout : public emp::web::BaseTest {

  Test_Bool_Value_Control_HTMLLayout()
  : BaseTest({"emp_test_container"}) {
    Doc("emp_test_container") << emp::prefab::BoolValueControl(
      "label",
      "description",
      true,
      [](std::string val) { ; },
      "bool_value_control");
  }

  void Describe() override {
    EM_ASM({
      describe("emp::prefab::BoolValueControl HTML Layout Scenario", function() {
        const bool_value_control = document.getElementById("bool_value_control");

        describe("#value_display", function() {
          it('should exist', function() {
            chai.assert.notEqual(bool_value_control, null);
          });

          it('should have parent #emp_test_container', function() {
            const parent_id = bool_value_control.parentElement.id;
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have 3 children', function() {
            chai.assert.equal(bool_value_control.childElementCount, 3);
          });

          describe("view", function() {
            const value_view = bool_value_control.children[2];
            it('should exist', function() {
              chai.assert.notEqual(value_view, null);
            });

            it("should have a child", function() {
              chai.assert.equal(value_view.childElementCount, 1);
            });

            describe("view's child", function() {
              it("should be a span (toggle)", function() {
                chai.assert(value_display.children[0].nodeName, "SPAN");
              });
            });
          });
        });
      });
    });
  }
};

// Test that the TextValueControl class is constructed correctly
struct Test_Text_Value_Control_HTMLLayout : public emp::web::BaseTest {

  Test_Text_Value_Control_HTMLLayout()
  : BaseTest({"emp_test_container"}) {
    Doc("emp_test_container") << emp::prefab::TextValueControl(
      "label",
      "description",
      "value",
      [](std::string val) { ; },
      "text_value_control"
    );
  }

  void Describe() override {
    EM_ASM({
      describe("emp::prefab::TextValueControl HTML Layout Scenario", function() {
        const text_value_control = document.getElementById("text_value_control");

        describe("#text_value_control", function() {
          it('should exist', function() {
            chai.assert.notEqual(text_value_control, null);
          });

          it('should have parent #emp_test_container', function() {
            const parent_id = text_value_control.parentElement.id;
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have 3 children', function() {
            chai.assert.equal(text_value_control.childElementCount, 3);
          });

          describe("view", function() {
            const value_view = text_value_control.children[2];
            it('should exist', function() {
              chai.assert.notEqual(value_view, null);
            });

            it("should have a child", function() {
              chai.assert.equal(value_view.childElementCount, 1);
            });

            describe("view's child", function() {
              const text_box = value_view.children[0];
              it("should be a span", function() {
                chai.assert(text_box.nodeName, "INPUT");
              });
              it('should have type "text"', function() {
                chai.assert.equal(text_box.getAttribute("type"), "text");
              });
            });
          });
        });
      });
    });
  }
};

// Test that the NumericValueControl class is constructed correctly
struct Test_Numeric_Value_Control_HTMLLayout : public emp::web::BaseTest {

  Test_Numeric_Value_Control_HTMLLayout()
  : BaseTest({"emp_test_container"}) {
    Doc("emp_test_container") << emp::prefab::NumericValueControl(
      "label",
      "description",
      ".1",
      "float",
      [](std::string val) { ; },
      "numeric_value_control"
    );
  }

  void Describe() override {
    EM_ASM({
      describe("emp::prefab::NumericValueControl HTML Layout Scenario", function() {
        const numeric_value_control = document.getElementById("numeric_value_control");

        describe("#numeric_value_control", function() {
          it('should exist', function() {
            chai.assert.notEqual(numeric_value_control, null);
          });

          it('should have parent #emp_test_container', function() {
            const parent_id = numeric_value_control.parentElement.id;
            chai.assert.equal(parent_id, "emp_test_container");
          });

          it('should have 3 children', function() {
            chai.assert.equal(numeric_value_control.childElementCount, 3);
          });

          describe("view", function() {
            const value_view = numeric_value_control.children[2];
            it('should exist', function() {
              chai.assert.notEqual(value_view, null);
            });

            it("should have 2 children", function() {
              chai.assert.equal(value_view.childElementCount, 2);
            });

            describe("view's children", function() {
              const range = value_view.children[0];
              it("first should be an input", function() {
                chai.assert(range.nodeName, "INPUT");
              });
              it('should have type "range"', function() {
                chai.assert.equal(range.getAttribute("type"), "range");
              });
              it('should have min "0"', function() {
                chai.assert.equal(range.getAttribute("min"), "0");
              });
              it('should have max "1"', function() {
                chai.assert.equal(range.getAttribute("max"), "1");
              });
              it('should have step "0.01"', function() {
                chai.assert.equal(range.getAttribute("step"), "0.01");
              });
              const number = value_view.children[1];
              it("second should be an input", function() {
                chai.assert(number.nodeName, "INPUT");
              });
              it('should have type "number"', function() {
                chai.assert.equal(number.getAttribute("type"), "number");
              });
            });
          });
        });
      });
    });
  }
};

// Test that the NumericValueControl class is constructed correctly
struct Test_Numeric_Value_Control_Override_Default_Ranges : public emp::web::BaseTest {

  Test_Numeric_Value_Control_Override_Default_Ranges()
  : BaseTest({"emp_test_container"}) {
    emp::prefab::NumericValueControl::setDefaultRangeMaker(
      [](const std::string & value, const std::string & type, emp::web::Input & in) {
        const double val = emp::from_string<double>(value);
        in.Min(-val);
        in.Max(val);
        in.Step(val/100);
      }
    );
    Doc("emp_test_container") << emp::prefab::NumericValueControl(
      "label",
      "description",
      ".1",
      "float",
      [](std::string val) { ; },
      "numeric_value_control"
    );
  }

  void Describe() override {
    EM_ASM({
      describe("emp::prefab::NumericValueControl with different default range maker", function() {
        const range = document.getElementById("numeric_value_control_view").children[0];
        it("first should be an input", function() {
          chai.assert(range.nodeName, "INPUT");
        });
        it('should have type "range"', function() {
          chai.assert.equal(range.getAttribute("type"), "range");
        });
        it('should have min "-0.1"', function() {
          chai.assert.equal(range.getAttribute("min"), "-0.1");
        });
        it('should have max "0.1"', function() {
          chai.assert.equal(range.getAttribute("max"), "0.1");
        });
        it('should have step "0.001"', function() {
          chai.assert.equal(range.getAttribute("step"), "0.001");
        });
      });
    });
  }
};

emp::web::MochaTestRunner test_runner;

int main() {
  test_runner.Initialize({"emp_test_container"});

  test_runner.AddTest<Test_Value_Box_HTMLLayout>("Test ValueBox HTML Layout");
  test_runner.AddTest<Test_Value_Display_HTMLLayout>("Test ValueDisplay HTML Layout");
  test_runner.AddTest<Test_Text_Value_Control_HTMLLayout>("Test TextValueControl HTML Layout");
  test_runner.AddTest<Test_Numeric_Value_Control_HTMLLayout>("Test NumericValueControl HTML Layout");
  test_runner.AddTest<Test_Numeric_Value_Control_Override_Default_Ranges>(
    "Test NumericValueControl Override of Default Ranges"
  );

  test_runner.Run();
}
