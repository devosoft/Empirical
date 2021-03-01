//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <functional>
#include <unordered_map>

#include "emp/base/assert.hpp"
#include "emp/web/_MochaTestRunner.hpp"
#include "emp/web/Document.hpp"
#include "emp/prefab/QueueManager.hpp"
#include "emp/config/SettingConfig.hpp"
#include "emp/web/web.hpp"


struct Test_QueueManager : public emp::web::BaseTest {

  std::function<emp::SettingConfig()> get_setting_config = [](){
      emp::SettingConfig my_settings;
      my_settings.AddSetting<int>("my_param") = {9};
      return my_settings;
  };

  std::function<int()> get_epochs = [](){return 50;};
  std::function<std::string()> metric_fun = [](){return "Hello";};

  emp::SettingConfig config;
  emp::Ptr<emp::QueueManager> queue_manager;

  Test_QueueManager()
  : BaseTest({"emp_test_container"}) // we can tell BaseTest that we want to create a set of emp::web::Document
                                     // objects for each given html element ids.
  {
    
    config.AddSetting<int>("my_param") = {6};
    queue_manager.New(config);

    queue_manager->AddMetric(metric_fun, "Hello metric");

    queue_manager->AddQueueButton(get_setting_config, get_epochs); // Add button and text input
    queue_manager->BuildTable("my_table"); // Add progress table
    

    Doc("emp_test_container")
      << queue_manager->GetDiv();

    queue_manager->AddRun(get_setting_config(), get_epochs());
    queue_manager->AddNewQueuedRunToTable();

    emp_assert(!queue_manager->IsEmpty());
    emp_assert(queue_manager->RunsRemaining() == 1);
    emp_assert(queue_manager->FrontRun().GetEpoch() == 0);        
    emp_assert(queue_manager->FrontRun().GetConfig().GetValue<int>("my_param") == 9);
    emp_assert(queue_manager->FrontRun().GetNEpochs() == 50);

    queue_manager->FrontRun().IncEpoch();
    queue_manager->Update();

    queue_manager->AddRun(config, 20);
    queue_manager->AddNewQueuedRunToTable();

    emp_assert(queue_manager->RunsRemaining() == 2);
    queue_manager->FrontRun().IncEpoch(49);
    queue_manager->Update();
    
    // Previous run should have finished
    emp_assert(queue_manager->RunsRemaining() == 1);
  }

  ~Test_QueueManager() {
    queue_manager.Delete();
  }

  void Describe() override {

    EM_ASM({
      describe("emp::QueueManager GUI", function() {

        describe("data table", function() {

          it('should exist and be a table', function() {
            chai.assert.equal($( "table#my_table" ).length, 1);
          });

          it('should have grandparent #emp_test_container', function() {
            const grand_parent_id = $("#my_table").parent().parent().attr("id");
            chai.assert.equal(grand_parent_id, "emp_test_container");
          });

          it('should have 4 columns', function() {
            columns = $("#my_table").find("th>span");
            chai.assert.equal(columns.length, 4);
            chai.assert.equal(columns[0].firstChild.textContent, "Run");
            chai.assert.equal(columns[1].firstChild.textContent, "my_param");
            chai.assert.equal(columns[2].firstChild.textContent, "Epoch");
            chai.assert.equal(columns[3].firstChild.textContent, "Hello metric");
          });

          it('should have 3 rows', function() {
            rows = $("#my_table").find("tr");
            chai.assert.equal(rows.length, 3);
          });

          it('should have the correct data', function() {
            rows = $("#my_table").find("td");
            chai.assert.equal(rows[0].children[0].firstChild.textContent, "0");
            chai.assert.equal(rows[1].children[0].firstChild.textContent, "9");
            chai.assert.equal(rows[2].children[0].firstChild.textContent, "50");
            chai.assert.equal(rows[3].children[0].firstChild.textContent, "Hello");
            chai.assert.equal(rows[4].children[0].firstChild.textContent, "1");
            chai.assert.equal(rows[5].children[0].firstChild.textContent, "6");
            chai.assert.equal(rows[6].children[0].firstChild.textContent, "Waiting...");
            chai.assert.equal(rows[7].children[0].firstChild.textContent, "Waiting...");
          });


        });

        describe("Queue button", function() {

          it('button should exist', function() {
            chai.assert.equal($( "button#queue_but" ).length, 1);
          });

          it('text area should exist', function() {
            chai.assert.equal($( "textarea#run_count" ).length, 1);
          });

          it('should respond correctly to clicks', function() {
            $( "button#queue_but" ).click();
            rows = $("#my_table").find("tr");
            chai.assert.equal(rows.length, 13);
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
  test_runner.AddTest<Test_QueueManager>(
    "Test QueueManager"
  );

  // Once we add all of the tests we want to run in this file, run them!
  test_runner.Run();
}
