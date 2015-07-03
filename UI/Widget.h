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

    public:
      const std::string & GetDivID() const { return div_id; }      
      const std::string & GetObjExt() const { return obj_ext; }
      std::string GetFullID() const { return div_id + obj_ext; }
      static std::string TypeName() { return "Widget"; }

      std::string CSS(const std::string & setting) { return style.Get(setting); }
      template <typename SETTING_TYPE>
      RETURN_TYPE & CSS(const std::string & setting, SETTING_TYPE && value) {
        style.Set(setting, value);
        return (RETURN_TYPE &) *this;
      }
      RETURN_TYPE & ID(const std::string & in_id) { div_id = in_id; return (RETURN_TYPE &) *this; }

      RETURN_TYPE & Width(int w) { return CSS("width", emp::to_string(w, "px") ); }
      RETURN_TYPE & Height(int h) { return CSS("height", emp::to_string(h, "px") ); }
      RETURN_TYPE & Size(int w, int h) { Width(w); Height(h); return *this; }
      
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

