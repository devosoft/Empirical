#ifndef D3_INIT_H
#define D3_INIT_H

#include <iostream>

#include "../../base/errors.hpp"
#include "../init.hpp"
#include "../JSWrap.hpp"
#include "utils.hpp"

// #ifdef __EMSCRIPTEN__
// extern "C" {
//     extern void D3_Initialize();
// }
// #endif

namespace D3 {


    // #ifdef __EMSCRIPTEN__
    // static void InitD3() {
    //     static bool init = false;
    //     if (!init) {
    //         D3_Initialize();
    //         init = true;
    //     }
    // }
    //
    // #endif

    int NextD3ID() {
        return MAIN_THREAD_EM_ASM_INT({
            id = js.next_id++;
            js.counts[id] = 0;
            js.objects[id] = -1;
            return id;
        });
    }

  /// A base object that all D3 objects inherit from. Handles storing the object in Javascript
  /// You probably don't want to instantiate this directly

  class D3_Base {
  protected:
    int id;

    /// Default constructor - adds placeholder to js.objects array in Javascript
    D3_Base(){
      this->id = NextD3ID();
    //   std::cout << "Default constructor: " << this->id << std::endl;
      EM_ASM_ARGS({
          js.counts[$0] = 1;
      }, this->id);
    }

    /// Construct an object pointing to a pre-determined location in js.objects.
    /// Warning: This trusts that you know what you're doing in choosing an id.
    D3_Base(int id){
      this->id = id;
    //   std::cout << "Int constructor: " << id << std::endl;
      emp_assert(EM_ASM_INT({return $0 in js.counts;}, this->id));

      EM_ASM_ARGS({
          js.counts[$0]++;
      }, this->id);
    }

    D3_Base(const D3_Base & other) {
        //TODO: Make this a deep copy
        // std::cout << "Copying: " << other.id << std::endl;
        this->id = other.id;
        EM_ASM_ARGS({js.counts[$0]++;}, this->id);
    }

    D3_Base& operator= (const D3_Base & other) {
        // std::cout << "Calling assingment: " << this->id << " " << other.id << std::endl;
        this->id = other.id;
        EM_ASM_ARGS({js.counts[$0]++;}, this->id);
        return (*this);
    }

    ~D3_Base() {
        // std::cout << "Destructing: " <<this->id << std::endl;
        EM_ASM_ARGS({
            js.counts[$0]--;
            if (js.counts[$0] == 0) {
                // console.log("Deleting: ", $0);
                delete js.objects[$0];
                delete js.counts[$0];
            }
        }, this->id);
    }

  public:
    // Get this object's ID (i.e. it's location in the js.objects array in Javascript)
    int GetID() const {
      return this->id;
    }

    void Log() const {
        EM_ASM_ARGS({console.log($0+":", js.objects[$0]);}, id);
    }
  };

  /// Create a tooltup using the d3.tip Javascript library
  class ToolTip : public D3_Base {
  public:
    /// Default constructor - displays whatever data is bound on mouseover
    ToolTip() {
      EM_ASM_ARGS({
          js.objects[$0] = d3.tip().attr('class', 'd3-tip')
                                    .offset([-10, 0])
                                    .html(function(d, i) { return d; });
      }, this->id);
    }

    /// Cosntructor that allows you to specify a function that returns the html for the tooltip.
    /// As input, this function should take 3 parameters: the bound data, the index of this item
    /// in the selection (int), and a placeholder (int).
    ///
    /// Example:
    ///
    /// `D3::FormatFunction rounded = D3::FormatFunction(".2f");
    ///
    ///  std::function<double, int, int)> tooltip_display =
    ///    [this](double d, int i, int k) {return "Data: " + to_string(rounded(d));}
    ///
    ///  D3::ToolTip tip = D3::ToolTip(tooltip_display);
    ///
    ///  D3::Selection example_selection = D3::SelectAll("circle");
    ///
    ///  example_selection.SetupToolTip(tip);'
    ///
    /// Mousing over a circle in the example selection will display
    /// "Data: " followed by the value of d, rounded to two decimal points.
    ToolTip(std::string func) {
      EM_ASM_ARGS({
        var in_string = UTF8ToString($1);
        if (typeof window["d3"][in_string] === "function"){
          in_string = window["d3"][in_string];
        } else if (typeof window["emp"][in_string] === "function"){
          in_string = window["emp"][in_string];
        } else if (typeof window[in_string] === "function"){
          in_string = window[in_string];
        }

        js.objects[$0] = d3.tip().attr('class', 'd3-tip').offset([-10, 0]).html(in_string);
      }, this->id, func.c_str());
    }

    /// @cond TEMPLATES
    template <typename T>
    ToolTip(T func) {
      emp::JSWrap(func, emp::to_string(this->id)+"_html_func");
      EM_ASM_ARGS({
          js.objects[$0] = d3.tip().attr('class', 'd3-tip')
                                   .offset([-10, 0])
                                   .html(emp[$0+"_html_func"]);
      }, id);

    }
    /// @endcond

    void SetHtml(std::string func) {
      EM_ASM_ARGS({
        var in_string = UTF8ToString($1);
        if (typeof window["d3"][in_string] === "function"){
          in_string = window["d3"][in_string];
        } else if (typeof window["emp"][in_string] === "function"){
          in_string = window["emp"][in_string];
        } else if (typeof window[in_string] === "function"){
          in_string = window[in_string];
        }

        js.objects[$0].html(in_string);
      }, this->id, func.c_str());
    }

    /// @cond TEMPLATES
    template <typename T>
    emp::sfinae_decoy<void, decltype(&T::operator())>
    SetHtml(T func) {
      emp::JSWrap(func, emp::to_string(id)+"_html_func");
      EM_ASM_ARGS({js.objects[$0].html(emp[$0+"_html_func"]);}, id);
    }
    /// @endcond

  };

  /// A callable string d3.format() string formatting function
  class FormatFunction : public D3_Base {
  public:
      FormatFunction(std::string format = "") {
        EM_ASM_ARGS({
          js.objects[$1] = d3.format(UTF8ToString($0));
        }, format.c_str(), this->id);
      }

      std::string operator() (double d){
        char * buffer = (char *) EM_ASM_INT({
          var text = js.objects[$0]($1);
          var buffer = Module._malloc(text.length+1);
    	  Module.stringToUTF8(text, buffer, lengthBytesUTF8(text)+1);
    	  return buffer;
        }, this->id, d);
        std::string result = std::string(buffer);
        free(buffer);
        return result;
      }
  };

  /// Catch-all object for storing references to things created in JS
  class JSObject : public D3_Base {
  public:
    JSObject(){;};
};

  /// Wrapper for creating functions in javascript and calling them there
  class JSFunction : public D3_Base {
  public:
    JSFunction() {;}
    JSFunction(std::string name) {
      int fail = EM_ASM_INT({
        var fn = window["d3"][UTF8ToString($2)];
        if (typeof fn === "function") {
          js.objects[$0] = fn;
          return 0;
        } else {
          var fn = window["emp"][UTF8ToString($2)];
          if (typeof fn === "function") {
            js.objects[$0] = fn;
            return 0;
          } else {
            var fn = window[UTF8ToString($2)];
            if (typeof fn === "function") {
              js.objects[$0] = fn;
              return 0;
            }
          }
        }
        return 1;
      }, this->id, name.c_str());

      if (fail) {
        emp::NotifyWarning("Invalid function name passed to JSFunction");
      }
    }

    /// Only works if function has no arguments or returns
    void operator() () {
        EM_ASM_ARGS({
          js.objects[$0]();
      }, this->id);
    }

  };
}

#endif
