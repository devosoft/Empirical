#ifndef D3_INIT_H
#define D3_INIT_H

#include "..//web_init.h"
#include "../../tools/errors.h"
#include "utils.h"

#include <iostream>

namespace D3 {

  class D3_Base {
  protected:
    int id;
    D3_Base(){
      this->id = EM_ASM_INT_V({return js.objects.length});
      EM_ASM({js.objects.push(-1);});
    }
    D3_Base(int id){
      this->id = id;
    }

  public:
    int GetID() {
      return this->id;
    }
  };

  class ToolTip : public D3_Base {
  public:
    ToolTip() {
      EM_ASM_ARGS({
          js.objects[$0] = d3.tip().attr('class', 'd3-tip')
                                    .offset([-10, 0])
                                    .html(function(d, i) { return d; });
      }, this->id);
    }

    ToolTip(std::string func) {
      EM_ASM_ARGS({
        var in_string = Pointer_stringify($1);
        var fn = window["d3"][in_string];
        if (typeof fn === "function"){
          var new_sel = d3.tip().attr('class', 'd3-tip').offset([-10, 0]).html(fn);
        } else {
          var fn = window["emp"][in_string];
          if (typeof fn === "function"){
            var new_sel = d3.tip().attr('class', 'd3-tip').offset([-10, 0]).html(fn);
          } else {
            var fn = window[in_string];
            if (typeof fn === "function"){
              var new_sel = d3.tip().attr('class', 'd3-tip').offset([-10, 0]).html(fn);
            } else {
              var new_sel = d3.tip().attr('class', 'd3-tip').offset([-10, 0]).html(in_string);
            }
          }
        }
        js.objects[$0] = new_sel;
    }, this->id, func.c_str());
    }
  };

  class FormatFunction : public D3_Base {
  public:
      FormatFunction(std::string format = "") {
        EM_ASM_ARGS({
          js.objects[$1] = d3.format(Pointer_stringify($0));
      }, format.c_str(), this->id);
      }

      std::string operator() (double d){
        char * buffer = (char *) EM_ASM_INT({
          var text = js.objects[$0]($1);
          var buffer = Module._malloc(text.length+1);
    	  Module.writeStringToMemory(text, buffer);
    	  return buffer;
        }, this->id, d);
        std::string result = std::string(buffer);
        free(buffer);
        return result;
      }
  };

  //Catch-all object for storing references to things created in JS
  class JSObject : public D3_Base {
  public:
    JSObject(){;};
};

  //Wrapper for creating functions in javascript and calling them there
  class JSFunction : public D3_Base {
  public:
    JSFunction() {;}
    JSFunction(std::string name) {
      int fail = EM_ASM_INT({
        var fn = window["d3"][Pointer_stringify($2)];
        if (typeof fn === "function") {
          js.objects[$0] = fn;
          return 0;
        } else {
          var fn = window["emp"][Pointer_stringify($2)];
          if (typeof fn === "function") {
            js.objects[$0] = fn;
            return 0;
          } else {
            var fn = window[Pointer_stringify($2)];
            if (typeof fn === "function") {
              js.objects[$0] = fn;
              return 0;
            }
          }
        }
        return 1;
      }, this->id, name.c_str());

      if (fail) {
        emp::NotifyWarning("Invalid function name passed to D3Function");
      }
    }

    //Only works if function has no arguments or returns
    void operator() () {
        EM_ASM_ARGS({
          js.objects[$0]();
      }, this->id);
    }

  };
}

#endif
