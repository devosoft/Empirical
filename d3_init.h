#ifndef D3_INIT_H
#define D3_INIT_H

#include "../Empirical/web/web_init.h"
#include "utils.h"

#include <iostream>

namespace D3 {

  class D3_Base {
  protected:
    int id;
    D3_Base(){
      this->id = EM_ASM_INT_V({return js.objects.length});
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
      EM_ASM({js.objects.push(d3.tip().attr('class', 'd3-tip').offset([-10, 0]).html(function(d, i) { return d; }));});
    }

    ToolTip(std::string func) {
      EM_ASM_ARGS({
        var in_string = Pointer_stringify($0);
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
        js.objects.push(new_sel);
      }, func.c_str());
    }
  };

  class FormatFunction : public D3_Base {
  public:
      FormatFunction(std::string format = "") {
        EM_ASM_ARGS({
          js.objects.push(d3.format(Pointer_stringify($0)));
      }, format.c_str());
      }

      std::string Call(double d){
        int buffer = EM_ASM_INT({
          var text = js.objects[$0]($1);
          var buffer = Module._malloc(text.length+1);
    	  Module.writeStringToMemory(text, buffer);
    	  return buffer;
        }, this->id, d);
        return std::string((char *)buffer);
      }
  };
}

#endif
