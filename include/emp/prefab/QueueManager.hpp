/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  queue-manager.h
 *  @brief A tool for processing multiple simulation runs onto a figure and table, displaying real-time statistics
 *  @note Status:
 *  @author Juan Chavez and Emily Dolson
 */

/// The goal of creating this tool is to alleviate the process of running multiple simulations of the same kind within
/// a queue structure. A user is able to visually see the results of their simulations, and each run held within the queue
/// runs subsequently after the other. Real-time statistics, that the user is able to display, are posted within a table as each run is carried out.
/// Here is a link to a blogpost that describes this tool's inpsiration, purpose, and required instructions:
/// https://mmore500.com/waves/blog/queuemanager.html

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

/// Information of each element within queue. This info represents the information required for each run to be processed.
struct RunInfo {
    SettingConfig runinfo_config;
    size_t id;
    size_t cur_epoch;
    size_t epochs;

    RunInfo(SettingConfig _config, size_t _id)
        : runinfo_config(_config), id(_id), cur_epoch(0) { ; }
};

/// Primary class that establishes queue for runs and processes them accordingly
class QueueManager {
   private:
    SettingConfig queue_config;
    std::queue<RunInfo> runs;
    emp::web::Div display_div;
    emp::web::TextArea run_input;
    std::string table_id;

    // ordered names of dependant headers with associated column #'s
    // emp::vector<std::string> ordered_param_names;
    emp::vector<std::string> ordered_metric_names;
    emp::vector<std::function<std::string()>> metric_funs;

    size_t epoch_ = 0;
    size_t num_runs = 10;

   public:
    void SetEpoch(size_t epoch) { epoch_ = epoch; }

    /// Default constructor
    QueueManager() = default;

    /// Config constructor
    QueueManager(SettingConfig user_config) : queue_config(user_config) { ; }

    /// Checks if queue is empty
    bool IsEmpty() {
        return runs.empty();
    }

    /// Checks how runs are in the queue
    size_t RunsRemaining() {
        return runs.size();
    }

    /// Adds run to queue with run info for paramters
    void AddRun(SettingConfig other, size_t _epochs) {
        RunInfo new_run(other, runs.size());
        new_run.epochs = _epochs;
        runs.push(new_run);
    }

    /// Remove run from front of queue
    void RemoveRun() {
        emp_assert(!IsEmpty(), "Queue is empty! Cannot remove!");
        runs.pop();
    }

    /// Front Run Getter
    RunInfo& FrontRun() {
        emp_assert(!IsEmpty(), "Queue is empty! Cannot access Front!");
        return runs.front();
    }

    /// Returns this dic
    emp::web::Div GetDiv() {
        return display_div;
    }
    /// Clears the content of this div
    void ResetDiv() {
        display_div.Clear();
    }

    /// Initializes table to web
    void DivAddTable(size_t row, size_t col, std::string id) {
        table_id = id;
        emp::web::Table result_tab(row, col, id);
        result_tab.SetCSS("border-collapse", "collapse");
        result_tab.SetCSS("border", "3px solid black");
        result_tab.CellsCSS("border", "1px solid black");

        result_tab.GetCell(0, 0).SetHeader() << "Run";
        int column_count = 1;
        emp::vector<std::string> setting_names = queue_config.GetSettingMapNames();
        for (const auto& p : setting_names) {
            result_tab.GetCell(0, column_count).SetHeader() << "<i>" << p << "</i>";
            ++column_count;
        }

        result_tab.GetCell(0, column_count).SetHeader() <<  "Epoch";
        ++column_count;
        /* if adding more features after this point, keep in mind of where
        the col count will be */
        for (size_t i = 0; i < ordered_metric_names.size(); i++) {
            result_tab.GetCell(0, column_count + i).SetHeader() << ordered_metric_names[i];
        }

        display_div << result_tab;
    }

    /// Extends table once button is clicked
    void DivButtonTable(int run_id) {
        emp::web::Table my_table = display_div.Find(table_id);

        // Update the table.
        int line_id = my_table.GetNumRows();
        my_table.Rows(line_id + 1);
        int col_count = 0;
        my_table.GetCell(line_id, col_count) << run_id;
        for (auto p : runs.back().runinfo_config.GetSettingMapBase()) {
            my_table.GetCell(line_id, ++col_count) << (*p).AsString();
        }

        for (int i = 0; i < ordered_metric_names.size(); i++) {
            my_table.GetCell(line_id, ++col_count) << "Waiting..."; 
        }

        // Draw the new table.
        my_table.CellsCSS("border", "1px solid black");
        my_table.Redraw();
    }

    /// Calculations required for updating table
    void DivTableCalc() {
        size_t id = FrontRun().id;
        RunInfo& current_run = FrontRun();

        emp::web::Table my_table = display_div.Find(table_id);
        my_table.Freeze();
        my_table.GetCell(id + 1, 5).ClearChildren() << emp::to_string(current_run.cur_epoch);

        // user function configuration
        for (int i = 0; i < metric_funs.size(); i++) {
            my_table.GetCell(id + 1, 6 + i).ClearChildren() << metric_funs[i]();            
        }


        if (current_run.cur_epoch >= current_run.epochs) {  // Are we done with this run?
            RemoveRun();                                    // Updates to the next run
        }

        my_table.Activate();
    }


    /// Creates queue button
    void AddQueueButton(std::function<emp::SettingConfig()> get_conf, std::function<size_t()> get_epochs) {
        run_input = emp::web::TextArea([this](const std::string & str){
            this->num_runs = emp::from_string<size_t>(str);
        }, "run_count");
        run_input.SetText(emp::to_string(num_runs));
        display_div << run_input;

        emp::web::Button my_button([this, get_conf, get_epochs]() {
            for (int run_id = 0; run_id < this->num_runs; run_id++) {
                AddRun(get_conf(), get_epochs());
                DivButtonTable(run_id);
            }
        }, "Queue", "queue_but");
        display_div << my_button;
    }

    /// Adds dependant variables to tables
    void AddDepVariable(std::function<std::string()> func, std::string header_name) {
        // ordered_param_names.push_back(header_name);
        ordered_metric_names.push_back(header_name);
        metric_funs.push_back(func);
    }
};

}  // namespace emp
