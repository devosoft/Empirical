/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file  queue-manager.h
 *  @brief A web widget for managing queued runs with different settings and displaying real-time statistics
 *  @note Status:
 *  @author Juan Chavez and Emily Dolson
 * 
 * This tool provides a web interface that allows users of the web version of a program to queue up multiple 
 * runs of the program with different settings. It can produce a table that displays the progress of these
 * runs and user-defined statistics about them.
 * 
 * QueueManager uses SettingConfig objects to keep track of parameter values for each run. It requires
 * a SettingConfig on construction in order to initialize the table header correctly. When runs are queued,
 * they will each require their own SettingConfig option. All SettingConfigs used in the same QueueManager 
 * must have the same parameters, although those parameters can have different values.
 * 
 * Example of constructing a QueueManager:
 * 
 * emp::SettingConfig my_settings;
 * my_settings.AddSetting<double>("my_param") = {.5};
 * emp::QueueManager my_queue_manager = emp::QueueManager(my_settings);
 * 
 * Once a QueueManager has been constructed, it can be told to keep track of additional metrics about the
 * world via the AddMetric() method. This method takes a function to calculate the metric and a name for
 * the column in the table containing the metric as input.
 * 
 * Example of adding metrics:
 * 
 * my_queue_manager.AddMetric([](){return emp::to_string(std::chrono::system_clock::now());}, "Wall time");
 * 
 * Once a QueueManager has all the metrics set up, a button and text input for queueing runs and a table for 
 * displaying progress can be added to a web page. To add the button/text input for queueing, the QueueManager
 * needs to know two things: where to get the SettingConfig for the newly-queued runs, and how to figure out 
 * how many epochs (time steps) to run them for. Both can be specified with functions.
 * 
 * Example:
 * 
 * int epochs = 0;
 * std::function<emp::SettingConfig()> get_setting_config = [](){
 *      emp::SettingConfig my_settings;
 *      my_settings.AddSetting<double>("my_param") = {.9};
 *      return my_settings;
 * };
 * std::function<int()> get_epochs = [&epochs](){return epochs;};
 * 
 * emp::web::Document doc("emp_base");
 * my_queue_manager.AddQueueButton(get_setting_config, get_epochs); // Add button and text input
 * my_queue_manager.BuildTable(); // Add progress table
 * doc << my_queue_manager.GetDiv(); // Get the div storing QueueManager stuff and put it on the document
 * 
 * Once your QueueManager is set up, you'll need integrate it with the rest of your code so that it
 * appropriately pulls new runs from the queue when appropriate, does set up for each run at the beginning,
 * and updates the current epoch and table when appropriate. The way to do this will vary based on the rest
 * of your code. Assuming each run of your code involves a loop that occurs for a certain number of time
 * steps (which correspond to epochs in the QueueManger), here is a template for how you might set up
 * the body of that loop:
 * 
 * if (!run_list.IsEmpty()) { // If there isn't stuff in the queue, support running program normally
 *   emp::QueueManager::RunInfo & run = run_list.FrontRun();
 *   if (run.GetEpoch() == 0) {  // Are we starting a new run?
 *     // Add beginning of run set up here
 *     // e.g. load in parameter settings from run.GetConfig()
 *     // If you want to do an initial visualization of something,
 *     // that goes here too.
 *     // Make sure to fully clean up from any previous run
 *   } 
 *   run.IncEpoch();  // Assumes the loop this is inside runs once per time step
 * }
 * 
 * // Add the rest of the work for an individual time step of your code here
 * // e.g. update the world, draw stuff, etc.
 *
 * if (!run_list.IsEmpty()) { // If we're running something from the queue, update the table
 *   run_list.Update();       // and check whether we have finished the current job
 * }
 * 
 * For an example of this template in action, see the SpatialCoop2017 demo.
 * 
 * More information is available in this blog post: https://mmore500.com/waves/blog/queuemanager.html (this 
 * tool was written by Juan Chavez as part of WAVES 2020)
 */

#pragma once

#include <functional>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>

#include "emp/base/vector.hpp"
#include "emp/config/SettingConfig.hpp"
#include "emp/math/Random.hpp"
#include "emp/math/math.hpp"
#include "emp/web/Div.hpp"
#include "emp/web/web.hpp"

namespace emp {

/// Primary class that establishes queue for runs and processes them accordingly
class QueueManager {
    public:

    /// Information of each element within queue. This info represents the information required for each run to be processed.
    struct RunInfo {
        SettingConfig runinfo_config; // Holds all program-specific settings
        size_t id; // The id of this run in the queue
        size_t cur_epoch; // The current epoch that this run is on (will either be 0 or `epochs` unless this run is in progress)
        size_t epochs; // The number of epochs this run is supposed to run for

        RunInfo(SettingConfig _config, size_t _id)
            : runinfo_config(_config), id(_id), cur_epoch(0) { ; }

        /// @returns current epoch
        size_t GetEpoch() {return cur_epoch;}
        /// Increment current epoch by @param x
        void IncEpoch(int x = 1) {cur_epoch += x;}
        /// @returns configuration for this run
        SettingConfig GetConfig() {return runinfo_config;}
    };


   private:
    SettingConfig queue_config;
    std::queue<emp::QueueManager::RunInfo> runs;
    emp::web::Div display_div;
    emp::web::TextArea run_input;
    emp::web::Button queue_button;
    emp::web::Table display_table;

    emp::vector<std::string> ordered_metric_names;
    emp::vector<std::function<std::string()>> metric_funs;

    size_t num_runs = 10;
    bool table_built = false;

   public:

    /// @param user_config An example configuration file for this program. Used to initialize table headers.
    QueueManager(SettingConfig user_config) : queue_config(user_config) { ; }

    /// @returns True if queue is empty, false if it is not
    bool IsEmpty() {
        return runs.empty();
    }

    /// @returns Number of runs remaining in the queue
    size_t RunsRemaining() {
        return runs.size();
    }

    /// Adds new run to queue using settings specified in @param settings.
    /// @param epochs indicates how many epochs this run should run for.  
    void AddRun(SettingConfig settings, size_t epochs) {
        RunInfo new_run(settings, runs.size());
        new_run.epochs = epochs;
        runs.push(new_run);
    }

    /// Removes run from front of queue
    void RemoveRun() {
        emp_assert(!IsEmpty(), "Queue is empty! Cannot remove!");
        runs.pop();
    }

    /// @returns The a reference to the first run in the queue
    /// (i.e. the one that is running currently or, if none are
    ///  in progress, the next run)
    RunInfo& FrontRun() {
        emp_assert(!IsEmpty(), "Queue is empty! Cannot access Front!");
        return runs.front();
    }

    /// @returns the Div associated with this QueueManager.
    emp::web::Div GetDiv() {
        return display_div;
    }
    /// Clears the content of the div associated with this QueueManager
    void ResetDiv() {
        display_div.Clear();
        table_built = false;
    }

    /// Adds table containing information for this QueueManager to the
    /// Div associated with this QueueManager.
    ///
    /// @param id optionally allows you to choose the table's element id,
    /// for ease of finding it from other parts of your code.
    ///
    /// Note that you still need to add this div to your document,
    /// e.g. `my_doc << my_queue_manager.GetDiv()`;
    void BuildTable(const std::string & id = "") {
        emp_assert(!table_built && 
                   "Trying to add QueueManager table but QueueManager table already built");

        // Get parameter names
        emp::vector<std::string> setting_names = queue_config.GetSettingMapNames();

        // Total number of columns is number of params + number of metrics +
        // a column for the run id and a column for the current epoch.
        size_t col = 2 + setting_names.size() + ordered_metric_names.size();

        // Make and style table
        display_table = emp::web::Table(1, col, id);
        display_table.SetCSS("border-collapse", "collapse");
        display_table.SetCSS("border", "3px solid black");
        display_table.CellsCSS("border", "1px solid black");

        // Fill out header
        display_table.GetCell(0, 0).SetHeader() << "Run";
        int column_count = 1;
        for (const auto& p : setting_names) {
            display_table.GetCell(0, column_count).SetHeader() << "<i>" << p << "</i>";
            ++column_count;
        }

        display_table.GetCell(0, column_count).SetHeader() <<  "Epoch";
        ++column_count;

        // if adding more features after this point, keep in mind of where
        // the col count will be
        for (size_t i = 0; i < ordered_metric_names.size(); i++) {
            display_table.GetCell(0, column_count + i).SetHeader() << ordered_metric_names[i];
        }

        display_div << display_table;
        table_built = true;
    }

    /// Helper function to add the last run in the queue to the table
    /// Called by queue button.
    void AddNewQueuedRunToTable() {
        emp_assert(table_built && 
                   "Trying to add run to QueueManager table but table hasn't been initialized. Call BuildTable first.");

        // Update the table.
        int line_id = display_table.GetNumRows();
        display_table.Rows(line_id + 1);
        int col_count = 0;
        display_table.GetCell(line_id, col_count) << runs.back().id;

        // Add correct parameter values
        for (auto p : runs.back().runinfo_config.GetSettingMapBase()) {
            display_table.GetCell(line_id, ++col_count) << (*p).AsString();
        }

        // Add placeholders for metrics and epoch column
        for (int i = 0; i < ordered_metric_names.size() + 1; i++) {
            display_table.GetCell(line_id, ++col_count) << "Waiting..."; 
        }

        // Draw the new table.
        display_table.CellsCSS("border", "1px solid black");
        display_table.Redraw();
    }

    /// Update QueueManager to reflect current status of runs and metrics.
    /// Handles updating table and updating queue (checking if current run is done).
    void Update() {
        emp_assert(table_built && 
                "Trying to update QueueManager table but table hasn't been initialized. Call BuildTable first.");

        size_t id = FrontRun().id;
        RunInfo& current_run = FrontRun();

        display_table.Freeze();
        display_table.GetCell(id + 1, 5).ClearChildren() << emp::to_string(current_run.cur_epoch);

        // user function configuration
        for (int i = 0; i < metric_funs.size(); i++) {
            display_table.GetCell(id + 1, 6 + i).ClearChildren() << metric_funs[i]();            
        }


        if (current_run.cur_epoch >= current_run.epochs) {  // Are we done with this run?
            RemoveRun();                                    // Updates to the next run
        }

        display_table.Activate();
    }


    /// Adds a button and text input to this QueueManager's div, allowing the user to queue runs.
    /// The text input allows the user to enter a number specifying the number of runs to be queued.
    /// Clicking the button queues the runs.
    ///
    /// @param get_conf is a function that creates and returns a SettingConfig object containing all of
    /// the parameters that should be used for the run to be queued.
    /// @param get_epochs is a function that will be used to determine how many epochs/time steps 
    /// the run is supposed to go for 
    void AddQueueButton(std::function<emp::SettingConfig()> get_conf, std::function<size_t()> get_epochs) {
        run_input = emp::web::TextArea([this](const std::string & str){
            this->num_runs = emp::from_string<size_t>(str);
        }, "run_count");
        run_input.SetText(emp::to_string(num_runs));
        display_div << run_input;

        queue_button = emp::web::Button([this, get_conf, get_epochs]() {
            for (int i = 0; i < this->num_runs; i++) {
                AddRun(get_conf(), get_epochs());
                AddNewQueuedRunToTable();
            }
        }, "Queue", "queue_but");
        display_div << queue_button;
    }

    /// Adds new metric to table
    /// @param func the function to be called to calculate this metric. Must take
    /// no arguments and return a string. If you need arguments, we reccomend using
    /// a lambda function and capturing the information you need instead.
    /// @param header_name the name this column should have in the table
    ///
    void AddMetric(std::function<std::string()> func, std::string header_name) {
        ordered_metric_names.push_back(header_name);
        metric_funs.push_back(func);

        if (table_built) {
            size_t col_id = display_table.GetNumCols();
            display_table.Cols(col_id + 1);
            display_table.GetCell(0, col_id).SetHeader() << header_name;
        }
    }
};

}  // namespace emp
