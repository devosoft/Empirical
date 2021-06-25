/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2020
 *
 *  @file  dataset.hpp
 *  @brief Tools to maintain data in D3.
 */

#ifndef EMP_D3_LOAD_DATA_H
#define EMP_D3_LOAD_DATA_H

#include <functional>

#include "d3_init.hpp"

#include "../JSWrap.hpp"
#include "../js_utils.hpp"
#include "../../tools/string_utils.hpp"

namespace D3 {

  class Dataset : public D3_Base {
  public:
    Dataset() {;}
    Dataset(int i) : D3_Base(i) {;}

    template <typename T>
    emp::sfinae_decoy<double, decltype(&T::operator())>
    Min(T comp) {
      const uint32_t fun_id = emp::JSWrap(comp, "", false);

      double min = MAIN_THREAD_EM_ASM_DOUBLE({
        return d3.min(emp_d3.objects[$0], function(d) {return emp.Callback($1, d);});
      }, this->id, fun_id);

      emp::JSDelete(fun_id);

      return min;
    }

    template <typename T>
    emp::sfinae_decoy<double, decltype(&T::operator())>
    Max(T comp) {
      const uint32_t fun_id = emp::JSWrap(comp, "", false);

      double max = MAIN_THREAD_EM_ASM_DOUBLE({
        return d3.max(emp_d3.objects[$0], function(d) {return emp.Callback($1, d);});
      }, this->id, fun_id);

      emp::JSDelete(fun_id);

      return max;
    }

  };

// TODO: Support init field for loading data
//   struct RequestInit {
//         EMP_BUILD_INTROSPECTIVE_TUPLE( std::string, RequestMode,
//                                        std::string, RequestCache
//                                      )
//   };

  class CSVDataset : public Dataset {
  public:
    CSVDataset() {;}
    CSVDataset(int i) : Dataset(i) {;}

    void LoadDataFromFile(const std::string & location, const std::string & row_callback, bool header=true) {
      emp_assert(
        MAIN_THREAD_EM_ASM_INT({
          return emp_d3.is_function(UTF8ToString($0));
        }, row_callback.c_str()),
        "Row callback specify an actual function in Javascript."
      );

      MAIN_THREAD_EM_ASM({
        const location = UTF8ToString($0);
        const row_callback_str = UTF8ToString($1);
        const id = $2;
        const header = $3;

        var row_callback_func = emp_d3.find_function(row_callback_string);
        if (header) {
          d3.csv(location, row_callback_func).then(function(data) {
              emp_d3.objects[id] = data;
          });
        } else {
          d3.text(location).then(function(data) {
              emp_d3.objects[id] = d3.csvParseRows(data, row_callback_func);
          });
        }
      }, location.c_str(), row_callback.c_str(),  this->id, header);
    }

    /// Put the last row of the array into arr
    template <size_t N, typename T>
    void GetLastRow(emp::array<T, N> & arr, int n) {
      emp_assert(MAIN_THREAD_EM_ASM_INT({return emp_d3.objects[$0].length > n;}, GetID(), n));

      MAIN_THREAD_EM_ASM({
        emp_i.__outgoing_array = emp_d3.objects[$0][$1];
      }, GetID(), n);
      emp::pass_array_to_cpp(arr);
    }

  };

  class JSONDataset : public Dataset {
  public:

    JSONDataset(int i) : Dataset(i) {;}
    JSONDataset() {
      MAIN_THREAD_EM_ASM({emp_d3.objects[$0] = [];}, this->id);
    };

    void LoadDataFromFile(const std::string & filename) {
      MAIN_THREAD_EM_ASM({
        d3.json(UTF8ToString($1)).then(function(data){
            emp_d3.objects[$0] = data;
        });
      }, this->id, filename.c_str());
    }

    template <typename DATA_TYPE>
    void LoadDataFromFile(const std::string & filename, std::function<void(DATA_TYPE)> fun) {
      emp::JSWrap(fun, "__json_load_fun__"+emp::to_string(id), true);

      MAIN_THREAD_EM_ASM({
        d3.json(UTF8ToString($1)).then(function(data){
            emp_d3.objects[$0] = data;
            emp["__json_load_fun__"+$0](data);
        });
      }, this->id, filename.c_str());
    }

    void LoadDataFromFile(const std::string & filename, std::function<void(void)> fun) {
      emp::JSWrap(fun, "__json_load_fun__"+emp::to_string(id), true);

      MAIN_THREAD_EM_ASM({
        var filename = UTF8ToString($1);
        d3.json(filename).then(function(data){
            emp_d3.objects[$0] = data;
            emp["__json_load_fun__"+$0]();
        });
      }, this->id, filename.c_str());
    }

    void Append(const std::string & json) {
      MAIN_THREAD_EM_ASM({
        emp_d3.objects[$0].push(JSON.parse(UTF8ToString($1)));
      }, this->id, json.c_str());
    }

    void AppendNested(const std::string & json) {

      int fail = MAIN_THREAD_EM_ASM_INT({
        var obj = JSON.parse(UTF8ToString($1));

        var result = null;
        for (var i in emp_d3.objects[$0]) {
          result = emp_d3.find_in_hierarchy(emp_d3.objects[$0][i], obj.parent);
          if (result) {
            break;
          }
        }
        if (!result) {
          return 1;
        }
        result.children.push(obj);
        return 0;
      }, this->id, json.c_str());

      if (fail) {
        emp::NotifyWarning("Append to JSON failed - parent not found");
      }
    }

    // Appends into large trees can be sped up by maintaining a list of
    // possible parent nodes
    int AppendNestedFromList(const std::string & json, JSObject & options) {
      int pos = MAIN_THREAD_EM_ASM_INT({
        var parent_node = null;
        var pos = -1;
        var child_node = JSON.parse(UTF8ToString($1));
        for (var item in emp_d3.objects[$0]) {
          if (emp_d3.objects[$0][item].name == child_node.parent) {
            parent_node = emp_d3.objects[$0][item];
            pos = item;
            break;
          }
        }

        if (!parent_node.hasOwnProperty("children")){
          parent_node.children = [];
        }
        parent_node.children.push(child_node);
        return pos;
      }, options.GetID(), json.c_str());

      return pos;
    }

  };
}

#endif
