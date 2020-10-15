//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <functional>
#include <unordered_map>

#include "base/assert.h"
#include "web/_MochaTestRunner.h"
#include "web/Document.h"
#include "web/Element.h"
#include "web/Tutorial.h"
#include "web/web.h"


  // event listener trigger - events added on enter state, added on manual activation, removed on manual deactivation, removed on removal

//Test events added on state enter
struct Test_EventListenerTrigger_0 : emp::web::BaseTest {

  Tutorial tut;

  Test_EventListenerTrigger_0(): BaseTest({"emp_test_container"})
  {
    emp::web::Div& doc = Doc("emp_test_container");
    
    emp::web::Div div("testdiv");
    div << "this is a Div";
    doc << div;

    tut.AddState("state1");
    tut.AddState("state2");
    tut.AddEventListenerTrigger("state1", "state2", div, "click", "clicktrigger");
    tut.AddEventListenerTrigger("state1", "state2", div, "hover", "hovertrigger");
    tut.StartAtState("state1");
  }


  void Describe() override 
  {
    EM_ASM
    ({
      describe("Tutorial::AddEventListenerTrigger events added on state enter", function() 
      {

        describe("#testdiv", function() {

          it('should have an event listener on click', function() {
            var testdiv = document.getElementById('testdiv');
            chai.assert.notEqual(jQuery._data(testdiv, "events" )['click'], null);
          });

          it('should have an event listener on hover', function() {
            var testdiv = document.getElementById('testdiv');
            chai.assert.notEqual(jQuery._data(testdiv, "events" )['hover'], null);
          });
        });
      });
    });
  } 
};


// Test listeners removed on exit state
struct Test_EventListenerTrigger_1 : emp::web::BaseTest {

  Tutorial tut;

  Test_EventListenerTrigger_1(): BaseTest({"emp_test_container"})
  {
    emp::web::Div& doc = Doc("emp_test_container");
    
    emp::web::Div div("testdiv");
    div << "this is a Div";
    doc << div;

    tut.AddState("state1");
    tut.AddState("state2");
    tut.AddEventListenerTrigger("state1", "state2", div, "click", "clicktrigger");
    tut.AddEventListenerTrigger("state1", "state2", div, "hover", "hovertrigger");
    tut.AddManualTrigger("state1", "state2", "manualtrigger");
    tut.StartAtState("state1");
    tut.FireTrigger("manualtrigger");
  }

  void Describe() override 
  {
    EM_ASM
    ({
      describe("Tutorial::AddEventListenerTrigger events removed on state exit", function() 
      {

        describe("#testdiv", function() {

          it('should NOT have any event listeners', function() {
            var testdiv = document.getElementById('testdiv');
            chai.assert.equal(jQuery._data(testdiv, "events"), null);
          });
        });
      });
    });
  } 
};



// Test listeners removed on manual deactivation
struct Test_EventListenerTrigger_2 : emp::web::BaseTest {

  Tutorial tut;

  Test_EventListenerTrigger_2(): BaseTest({"emp_test_container"})
  {
    emp::web::Div& doc = Doc("emp_test_container");
    
    emp::web::Div div("testdiv");
    div << "this is a Div";
    doc << div;

    tut.AddState("state1");
    tut.AddState("state2");
    tut.AddEventListenerTrigger("state1", "state2", div, "click", "clicktrigger");
    tut.StartAtState("state1");
    tut.DeactivateTrigger("clicktrigger"); 
  }

  void Describe() override 
  {
    EM_ASM
    ({
      describe("Tutorial::AddEventListenerTrigger events removed on manual deactivation", function() 
      {

        describe("#testdiv", function() {

          it('should NOT have any event listeners', function() {
            var testdiv = document.getElementById('testdiv');
            chai.assert.equal(jQuery._data(testdiv, "events"), null);
          });
        });
      });
    });
  } 

};


// Test listeners re-added on manual activation
struct Test_EventListenerTrigger_3 : emp::web::BaseTest {

  Tutorial tut;

  Test_EventListenerTrigger_3(): BaseTest({"emp_test_container"})
  {
    emp::web::Div& doc = Doc("emp_test_container");
    
    emp::web::Div div("testdiv");
    div << "this is a Div";
    doc << div;

    tut.AddState("state1");
    tut.AddState("state2");
    tut.AddEventListenerTrigger("state1", "state2", div, "click", "clicktrigger");
    tut.StartAtState("state1");
    tut.DeactivateTrigger("clicktrigger"); 
    tut.ActivateTrigger("clicktrigger");
  }

  void Describe() override 
  {
    EM_ASM
    ({
      describe("Tutorial::AddEventListenerTrigger events added on manual activation", function() 
      {

        describe("#testdiv", function() {

          it('should have an event listener on click after manual activation', function() {
            var testdiv = document.getElementById('testdiv');
            chai.assert.notEqual(jQuery._data(testdiv, "events") ["click"], null);
          });
        });
      });
    });
  } 

};


// Test event listeners removed after trigger removal
struct Test_EventListenerTrigger_4 : emp::web::BaseTest {

  Tutorial tut;

  Test_EventListenerTrigger_4(): BaseTest({"emp_test_container"})
  {
    emp::web::Div& doc = Doc("emp_test_container");
    
    emp::web::Div div("testdiv");
    div << "this is a Div";
    doc << div;

    tut.AddState("state1");
    tut.AddState("state2");
    tut.AddEventListenerTrigger("state1", "state2", div, "click", "clicktrigger");
    tut.StartAtState("state1");
    tut.RemoveTrigger("clicktrigger", "state1"); 
  }

  void Describe() override 
  {
    EM_ASM
    ({
      describe("Tutorial::AddEventListenerTrigger events removed on trigger removal", function() 
      {

        describe("#testdiv", function() {

          it('should NOT have an event listener after trigger removal', function() {
            var testdiv = document.getElementById('testdiv');
            chai.assert.equal(jQuery._data(testdiv, "events"), null);
          });
        });
      });
    });
  } 

};


// overlayeffect - added to parent div on enter state, removed on exit state, added on manual activation, removed on manual deactivation, removed on removal

// Test overlay added to parent div on enter state
struct Test_OverlayEffect_0 : emp::web::BaseTest {

  Tutorial tut;

  Test_OverlayEffect_0(): BaseTest({"emp_test_container"})
  {
    emp::web::Div& doc = Doc("emp_test_container");
    
    emp::web::Div div("testdiv");
    div << "this is a Div";
    doc << div;

    tut.AddState("state1");
    tut.AddState("state2");
    tut.AddManualTrigger("state1", "state2", "manualtrigger"); // needed so the tutorial doesn't start and immediately stop
    tut.AddOverlayEffect("state1", doc);
    tut.StartAtState("state1");
  }

  void Describe() override 
  {
    EM_ASM
    ({
      describe("Tutorial::AddOverlayEffect overlay added on state enter", function() 
      {

        describe("#testdiv", function() {

          it('doc should have one overlay as a child', function() {
            var overlays = document.getElementsByClassName("Tutorial-Overlay-Effect");
            chai.assert.equal(overlays.length, 1);
            chai.assert(overlays[0].parentNode = document);
          });
        });
      });
    });
  } 

};



// Test overlay removed on exit state
struct Test_OverlayEffect_1 : emp::web::BaseTest {

  Tutorial tut;

  Test_OverlayEffect_1(): BaseTest({"emp_test_container"})
  {
    emp::web::Div& doc = Doc("emp_test_container");
    
    emp::web::Div div("testdiv");
    div << "this is a Div";
    doc << div;

    tut.AddState("state1");
    tut.AddState("state2");
    tut.AddManualTrigger("state1", "state2", "manualtrigger"); // needed so the tutorial doesn't start and immediately stop
    tut.AddOverlayEffect("state1", doc);
    tut.StartAtState("state1");
    tut.FireTrigger("manualtrigger");
  }

  void Describe() override 
  {
    EM_ASM
    ({
      describe("Tutorial::AddOverlayEffect overlay removed on state exit", function() 
      {

        describe("#testdiv", function() {

          it('doc should NOT have an overlay as a child', function() {
            var overlays = document.getElementsByClassName("Tutorial-Overlay-Effect");
            chai.assert.equal(overlays.length, 0);
          });
        });
      });
    });
  } 

};


// Test overlay removed on manual deactivation
struct Test_OverlayEffect_2 : emp::web::BaseTest {

  Tutorial tut;

  Test_OverlayEffect_2(): BaseTest({"emp_test_container"})
  {
    emp::web::Div& doc = Doc("emp_test_container");
    
    emp::web::Div div("testdiv");
    div << "this is a Div";
    doc << div;

    tut.AddState("state1");
    tut.AddState("state2");
    tut.AddManualTrigger("state1", "state2", "manualtrigger"); // needed so the tutorial doesn't start and immediately stop
    tut.AddOverlayEffect("state1", doc, "blue", 0.4, 1000, false, "overlay");
    tut.StartAtState("state1");
    tut.DeactivateVisualEffect("overlay");
  }

  void Describe() override 
  {
    EM_ASM
    ({
      describe("Tutorial::AddOverlayEffect overlay removed on manual deactivation", function() 
      {

        describe("#testdiv", function() {

          it('doc should NOT have an overlay as a child', function() {
            var overlays = document.getElementsByClassName("Tutorial-Overlay-Effect");
            chai.assert.equal(overlays.length, 0);
          });
        });
      });
    });
  } 

};


// Test overlay added on manual activation after manual deactivation
struct Test_OverlayEffect_3 : emp::web::BaseTest {

  Tutorial tut;

  Test_OverlayEffect_3(): BaseTest({"emp_test_container"})
  {
    emp::web::Div& doc = Doc("emp_test_container");
    
    emp::web::Div div("testdiv");
    div << "this is a Div";
    doc << div;

    tut.AddState("state1");
    tut.AddState("state2");
    tut.AddManualTrigger("state1", "state2", "manualtrigger"); // needed so the tutorial doesn't start and immediately stop
    tut.AddOverlayEffect("state1", doc, "blue", 0.4, 1000, false, "overlay");
    tut.StartAtState("state1");
    tut.DeactivateVisualEffect("overlay");
    tut.ActivateVisualEffect("overlay");
  }

  void Describe() override 
  {
    EM_ASM
    ({
      describe("Tutorial::AddOverlayEffect overlay added on manual activation after manual deactivation", function() 
      {

        describe("#testdiv", function() {

          it('doc should have one overlay as a child', function() {
            var overlays = document.getElementsByClassName("Tutorial-Overlay-Effect");
            chai.assert.equal(overlays.length, 1);
            chai.assert(overlays[0].parentNode = document);
          });
        });
      });
    });
  } 

};



// Test overlay removed on removal
struct Test_OverlayEffect_4 : emp::web::BaseTest {

  Tutorial tut;

  Test_OverlayEffect_4(): BaseTest({"emp_test_container"})
  {
    emp::web::Div& doc = Doc("emp_test_container");
    
    emp::web::Div div("testdiv");
    div << "this is a Div";
    doc << div;

    tut.AddState("state1");
    tut.AddState("state2");
    tut.AddManualTrigger("state1", "state2", "manualtrigger"); // needed so the tutorial doesn't start and immediately stop
    tut.AddOverlayEffect("state1", doc, "blue", 0.4, 1000, false, "overlay");
    tut.StartAtState("state1");
    tut.RemoveVisualEffect("overlay", "state1");
  }

  void Describe() override 
  {
    EM_ASM
    ({
      describe("Tutorial::AddOverlayEffect overlay removed on manual deactivation", function() 
      {

        describe("#testdiv", function() {

          it('doc should NOT have an overlay as a child', function() {
            var overlays = document.getElementsByClassName("Tutorial-Overlay-Effect");
            chai.assert.equal(overlays.length, 0);
          });
        });
      });
    });
  } 

};




// css effect - attr changed on enter state, manual activation. reverted on exit state, manual deactivation, removal.

// Test css attribute changed on enter state
struct Test_CSSEffect_0 : emp::web::BaseTest {

  Tutorial tut;

  Test_CSSEffect_0(): BaseTest({"emp_test_container"})
  {
    emp::web::Div& doc = Doc("emp_test_container");
    
    emp::web::Div div("testdiv");
    div << "this is a Div";
    doc << div;

    tut.AddState("state1");
    tut.AddState("state2");
    tut.AddManualTrigger("state1", "state2", "manualtrigger"); // needed so the tutorial doesn't start and immediately stop
    tut.AddCSSEffect("state1", div, "background-color", "seagreen", "css_effect");
    tut.StartAtState("state1");
  }

  void Describe() override 
  {
    EM_ASM
    ({
      describe("Tutorial::AddCSSEffect css attribute changed on enter state", function() 
      {

        describe("#testdiv", function() {

          it('div background color should be seagreen', function() {
            var testdiv = document.getElementById('testdiv');
            chai.assert.equal(testdiv.style.backgroundColor, "seagreen");
          });
        });
      });
    });
  } 

};


// Test css attribute reverted on exit state
struct Test_CSSEffect_1 : emp::web::BaseTest {

  Tutorial tut;

  Test_CSSEffect_1(): BaseTest({"emp_test_container"})
  {
    emp::web::Div& doc = Doc("emp_test_container");
    
    emp::web::Div div("testdiv");
    div << "this is a Div";
    doc << div;

    tut.AddState("state1");
    tut.AddState("state2");
    tut.AddManualTrigger("state1", "state2", "manualtrigger"); // needed so the tutorial doesn't start and immediately stop
    tut.AddCSSEffect("state1", div, "background-color", "seagreen", "css_effect");
    tut.StartAtState("state1");
    tut.FireTrigger("manualtrigger");
  }

  void Describe() override 
  {
    EM_ASM
    ({
      describe("Tutorial::AddCSSEffect css attribute reverted on exit state", function() 
      {

        describe("#testdiv", function() {

          it('div background color should be transparent', function() {
            var testdiv = document.getElementById('testdiv');
            console.log(testdiv.style.backgroundColor);
            chai.assert.equal(testdiv.style.backgroundColor, '');
          });
        });
      });
    });
  } 

};




// Test css attribute changed on manual activation after manual deactivation
struct Test_CSSEffect_2 : emp::web::BaseTest {

  Tutorial tut;

  Test_CSSEffect_2(): BaseTest({"emp_test_container"})
  {
    emp::web::Div& doc = Doc("emp_test_container");
    
    emp::web::Div div("testdiv");
    div << "this is a Div";
    doc << div;

    tut.AddState("state1");
    tut.AddState("state2");
    tut.AddManualTrigger("state1", "state2", "manualtrigger"); // needed so the tutorial doesn't start and immediately stop
    tut.AddCSSEffect("state1", div, "background-color", "seagreen", "css_effect");
    tut.StartAtState("state1");
    tut.DeactivateVisualEffect("css_effect");
    tut.ActivateVisualEffect("css_effect");
  }

  void Describe() override 
  {
    EM_ASM
    ({
      describe("Tutorial::AddCSSEffect css attribute changed on enter state", function() 
      {

        describe("#testdiv", function() {

          it('div background color should be seagreen', function() {
            var testdiv = document.getElementById('testdiv');
            chai.assert.equal(testdiv.style.backgroundColor, "seagreen");
          });
        });
      });
    });
  } 

};



// Test css attribute reverted on manual deactivation
struct Test_CSSEffect_3 : emp::web::BaseTest {

  Tutorial tut;

  Test_CSSEffect_3(): BaseTest({"emp_test_container"})
  {
    emp::web::Div& doc = Doc("emp_test_container");
    
    emp::web::Div div("testdiv");
    div << "this is a Div";
    doc << div;

    tut.AddState("state1");
    tut.AddState("state2");
    tut.AddManualTrigger("state1", "state2", "manualtrigger"); // needed so the tutorial doesn't start and immediately stop
    tut.AddCSSEffect("state1", div, "background-color", "seagreen", "css_effect");
    tut.StartAtState("state1");
    tut.DeactivateVisualEffect("css_effect");
  }

  void Describe() override 
  {
    EM_ASM
    ({
      describe("Tutorial::AddCSSEffect css attribute reverted on manual deactivation", function() 
      {

        describe("#testdiv", function() {

          it('div background color should be transparent', function() {
            var testdiv = document.getElementById('testdiv');
            console.log(testdiv.style.backgroundColor);
            chai.assert.equal(testdiv.style.backgroundColor, '');
          });
        });
      });
    });
  } 

};


// Test css attribute reverted on effect removal
struct Test_CSSEffect_4 : emp::web::BaseTest {

  Tutorial tut;

  Test_CSSEffect_4(): BaseTest({"emp_test_container"})
  {
    emp::web::Div& doc = Doc("emp_test_container");
    
    emp::web::Div div("testdiv");
    div << "this is a Div";
    doc << div;

    tut.AddState("state1");
    tut.AddState("state2");
    tut.AddManualTrigger("state1", "state2", "manualtrigger"); // needed so the tutorial doesn't start and immediately stop
    tut.AddCSSEffect("state1", div, "background-color", "seagreen", "css_effect");
    tut.StartAtState("state1");
    tut.RemoveVisualEffect("css_effect", "state1");
  }

  void Describe() override 
  {
    EM_ASM
    ({
      describe("Tutorial::AddCSSEffect css attribute reverted on removal", function() 
      {

        describe("#testdiv", function() {

          it('div background color should be transparent', function() {
            var testdiv = document.getElementById('testdiv');
            console.log(testdiv.style.backgroundColor);
            chai.assert.equal(testdiv.style.backgroundColor, '');
          });
        });
      });
    });
  } 

};




emp::web::MochaTestRunner test_runner;
int main() {

  test_runner.Initialize({"emp_test_container"});

  test_runner.AddTest<Test_EventListenerTrigger_0>("Test Tutorial::AddEventListenerTrigger");
  test_runner.AddTest<Test_EventListenerTrigger_1>("Test Tutorial::AddEventListenerTrigger");
  test_runner.AddTest<Test_EventListenerTrigger_2>("Test Tutorial::AddEventListenerTrigger");
  test_runner.AddTest<Test_EventListenerTrigger_3>("Test Tutorial::AddEventListenerTrigger");
  test_runner.AddTest<Test_EventListenerTrigger_4>("Test Tutorial::AddEventListenerTrigger");
  test_runner.AddTest<Test_OverlayEffect_0>("Test Tutorial::AddOverlayEffect");
  test_runner.AddTest<Test_OverlayEffect_1>("Test Tutorial::AddOverlayEffect");
  test_runner.AddTest<Test_OverlayEffect_2>("Test Tutorial::AddOverlayEffect");
  test_runner.AddTest<Test_OverlayEffect_3>("Test Tutorial::AddOverlayEffect");
  test_runner.AddTest<Test_OverlayEffect_4>("Test Tutorial::AddOverlayEffect");
  test_runner.AddTest<Test_CSSEffect_0>("Test Tutorial::AddCSSEffect");
  test_runner.AddTest<Test_CSSEffect_1>("Test Tutorial::AddCSSEffect");
  test_runner.AddTest<Test_CSSEffect_2>("Test Tutorial::AddCSSEffect");
  test_runner.AddTest<Test_CSSEffect_3>("Test Tutorial::AddCSSEffect");
  test_runner.AddTest<Test_CSSEffect_4>("Test Tutorial::AddCSSEffect");


  test_runner.Run();
}
