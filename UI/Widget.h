#ifndef EMP_UI_WIDGET_H
#define EMP_UI_WIDGET_H

#include <string>

#include "Style.h"

namespace emp {
namespace UI {

  namespace internal {
  
    // Provide a quick method for generating unique IDs when not otherwise specified.
    static int NextWidgetID() {
      static int next_id = 0;
      return next_id++;
    }
    
    // Widget is a base class containing information needed by all GUI widget classes
    // (Buttons, Images, etc...).  It take in a return type to be cast to for accessors.
    
    template <typename RETURN_TYPE>
    class Widget {
    protected: 
      std::string div_id;  // ID used for the div surrounding this element.
      std::string obj_ext; // Extension for internal object if eeds own id: div_id + '__but'
      
      UI::Style style;

      Widget(const std::string & in_name="") : div_id(in_name) {
        if (div_id == "") {
          div_id = emp::to_string("emp__", NextWidgetID());
        }
      }

      virtual RETURN_TYPE & CSS_impl(const std::string & setting, const std::string & value) {
        style.Set(setting, value);
        return (RETURN_TYPE &) *this;
      }

    public:
      const std::string & GetDivID() const { return div_id; }      
      const std::string & GetObjExt() const { return obj_ext; }
      std::string GetFullID() const { return div_id + obj_ext; }
      static std::string TypeName() { return "Widget"; }

      virtual std::string CSS(const std::string & setting) { return style.Get(setting); }
      
      template <typename SETTING_TYPE>
      RETURN_TYPE & CSS(const std::string & setting, SETTING_TYPE && value) {
        return CSS_impl(setting, emp::to_string(value));
      }

      // Allow multiple CSS settings to be grouped.
      template <typename T1, typename T2, typename... OTHER_SETTINGS>
      RETURN_TYPE & CSS(const std::string & setting1, T1 && val1,
                        const std::string & setting2, T2 && val2,
                        OTHER_SETTINGS... others) {
        CSS(setting1, val1);              // Set the first CSS value.
        return CSS(setting2, val2, others...);   // Recurse to the others.
      }

      RETURN_TYPE & ID(const std::string & in_id) { div_id = in_id; return (RETURN_TYPE &) *this; }

      // Size Manipulation
      RETURN_TYPE & Width(int w) { return CSS("width", emp::to_string(w, "px") ); }
      RETURN_TYPE & Height(int h) { return CSS("height", emp::to_string(h, "px") ); }
      RETURN_TYPE & Size(int w, int h) { Width(w); Height(h); return (RETURN_TYPE &) *this; }


      // Position Manipulation
      RETURN_TYPE & Center() { return CSS("margin", "auto"); }
      RETURN_TYPE & SetPosition(int x, int y) {
        CSS("position", "fixed",
            "left", emp::to_string(x, "px"),
            "top", emp::to_string(y, "px"));
        return (RETURN_TYPE &) *this;
      }
      RETURN_TYPE & SetPositionRT(int x, int y) {
        CSS("position", "fixed",
            "right", emp::to_string(x, "px"),
            "top", emp::to_string(y, "px"));
        return (RETURN_TYPE &) *this;
      }
      RETURN_TYPE & SetPositionRB(int x, int y) {
        CSS("position", "fixed",
            "right", emp::to_string(x, "px"),
            "bottom", emp::to_string(y, "px"));
        return (RETURN_TYPE &) *this;
      }
      RETURN_TYPE & SetPositionLB(int x, int y) {
        CSS("position", "fixed",
            "left", emp::to_string(x, "px"),
            "bottom", emp::to_string(y, "px"));
        return (RETURN_TYPE &) *this;
      }

      // Text Manipulation
      RETURN_TYPE & Font(const std::string & font) { return CSS("font-family", font); }
      RETURN_TYPE & FontSize(int s) { return CSS("font-size", emp::to_string(s, "px")); }
      RETURN_TYPE & CenterText() { return CSS("text-align", "center"); }

      // Color Manipulation
      RETURN_TYPE & Background(const std::string & v) { return CSS("background-color", v); }
      RETURN_TYPE & Color(const std::string & v) { return CSS("color", v); }
      RETURN_TYPE & Opacity(double v) { return CSS("opacity", v); }
      
      void TriggerCSS() {
        std::string obj_id = div_id + obj_ext;
        style.Apply(obj_id);
      }
      void TriggerJS() {
      }
    };
    
  };

};
};  


#endif

