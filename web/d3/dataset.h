#ifndef __LOAD_DATA_H__
#define __LOAD_DATA_H__

#include <functional>

#include "d3_init.h"

#include "../../web/JSWrap.h"
#include "../../web/js_utils.h"
#include "../../tools/string_utils.h"

namespace D3 {

  class Dataset : public D3_Base {
  public:
    Dataset(){;};
    Dataset(int i) : D3_Base(i) {;};

    void CaptureIncoming(){
        EM_ASM({js.objects[$0] = emp.__incoming_data;}, this->id);
    };
  };


  class JSONDataset : public Dataset {
  public:
    //This should probably be static, but emscripten complains when it is
    JSFunction FindInHierarchy;

    JSONDataset(int i) : Dataset(i) {;}
    JSONDataset() {
      EM_ASM_ARGS({js.objects[$0] = [];}, this->id);

      //Useful function for dealing with nested JSON data structures
      //Assumes nested objects are stored in an array called children
      EM_ASM_ARGS({
        //Inspired by Niels' answer to
        //http://stackoverflow.com/questions/12899609/how-to-add-an-object-to-a-nested-javascript-object-using-a-parent-id/37888800#37888800
        js.objects[$0] = function(root, id) {
          if (root.name == id){
            return root;
          }
          if (root.children) {
            for (var k in root.children) {
              if (root.children[k].name == id) {
                return root.children[k];
              }
              else if (root.children[k].children) {
                result = js.objects[$0](root.children[k], id);
                if (result) {
                  return result;
                }
              }
            }
          }
        };
      }, FindInHierarchy.GetID());
    };

    void LoadDataFromFile(std::string filename) {
      EM_ASM_ARGS ({
        d3.json(Pointer_stringify($1), function(data){js.objects[$0]=data;});
      }, id, filename.c_str());
    }

    template <typename DATA_TYPE>
    void LoadDataFromFile(std::string filename, std::function<void(DATA_TYPE)> fun) {
      emp::JSWrap(fun, "__json_load_fun__"+emp::to_string(id));

      EM_ASM_ARGS ({
        d3.json(Pointer_stringify($1), function(data){
            js.objects[$0]=data;
            emp["__json_load_fun__"+$0](data);
        });
      }, id, filename.c_str());
    }

    void LoadDataFromFile(std::string filename, std::function<void(void)> fun) {
      emp::JSWrap(fun, "__json_load_fun__"+emp::to_string(id));

      EM_ASM_ARGS ({
        d3.json(Pointer_stringify($1), function(data){
            js.objects[$0]=data;
            emp["__json_load_fun__"+$0]();
        });
      }, id, filename.c_str());
    }


    void Append(std::string json) {
      EM_ASM_ARGS({
        js.objects[$0].push(JSON.parse(Pointer_stringify($1)));
      }, this->id, json.c_str());
    }

    void AppendNested(std::string json) {

      int fail = EM_ASM_INT({
        var result = null;
        for (var i in js.objects[$0]) {
          result = js.objects[$1](js.objects[$0][i]);
          if (result) {
            break;
          }
        }
        if (!result) {
          return 1;
        }
        result.children.append(JSON.parse(Pointer_stringify($2)));
        return 0;
    }, this->id, FindInHierarchy.GetID(), json.c_str());

      if (fail) {
        emp::NotifyWarning("Append to JSON failed - parent not found");
      }
    }

    //Appends into large trees can be sped up by maintaining a list of
    //possible parent nodes
    int AppendNestedFromList(std::string json, JSObject & options) {
      int pos = EM_ASM_INT({
        var parent_node = null;
        var pos = -1;
        var child_node = JSON.parse(Pointer_stringify($1));
        for (var item in js.objects[$0]) {
          if (js.objects[$0][item].name == child_node.parent) {
            parent_node = js.objects[$0][item];
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

  class CSVDataset : public Dataset {
  public:
    CSVDataset(){;}
    CSVDataset(int i) : Dataset(i) {;}


    void LoadDataFromFile(std::string location, std::string callback, bool header=true) {
        std::cout << "In loaddata " << callback << std::endl;
      EM_ASM_ARGS({
        var acc = function(d){
            return ([+d[0], +d[1]]);
        };

        var arg1 = Pointer_stringify($0);
        var in_string = Pointer_stringify($1);
        console.log("about to convert to function", in_string);
        if (typeof window[in_string] === "function"){
          in_string = window[in_string];
        } else if (typeof window["d3"][in_string] === "function") {
          in_string = window["d3"][in_string];
        } else if (typeof window["emp"][in_string] === "function") {
          in_string = window["emp"][in_string];
        }
        console.log(in_string);
        if ($3) {
          d3.csv(arg1, acc, function(d){
            js.objects[$2] = d;
            in_string($2);
          });
        } else {
          d3.text(arg1, function(d){
            js.objects[$2] = d3.csv.parseRows(d, acc);
            in_string($2);
          });
        }
      }, location.c_str(), callback.c_str(), this->id, header);
    }

    void Parse(std::string contents, std::string accessor){
      D3_CALLBACK_FUNCTION_2_ARGS(d3.csv.parse, contents.c_str(),	\
				  accessor.c_str())
	}

    void ParseRows(std::string contents, std::string accessor){
      D3_CALLBACK_FUNCTION_2_ARGS(d3.csv.parseRows, contents.c_str(),	\
				  accessor.c_str())
	}

    /// Put the last row of the array into arr
    template <std::size_t N, typename T>
    void GetLastRow(std::array<T, N> & arr) {
      EM_ASM_ARGS({
        emp_i.__outgoing_array = js.objects[$0][js.objects[$0].length - 1];
        console.log(emp_i.__outgoing_array);
      }, GetID());
      emp::pass_array_to_cpp(arr);
      std::cout << emp::to_string(arr) << std::endl;
    }

    //TODO Format and FormatRows

  };

};


#endif
