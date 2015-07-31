#ifndef EMP_WEB_WIDGET_H
#define EMP_WEB_WIDGET_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Widgets maintain individual components on a web page and link to Elements
//
//
//  Widget is the generic base class.
//  WidgetCore is a template to allow Set methods in Widget to return derived return-type.
//
//  In other files, Widgets will be used to define specific elements.
//  ELEMENTCore maintains all information about the specific widget.
//  ELEMENT creates an interface to ELEMENTCore so multiple elements can use the same core.
//


#include <string>

#include "Style.h"


namespace emp {
namespace web {

  namespace internal {
  
    // Provide a quick method for generating unique IDs when not otherwise specified.
    static int NextWidgetID() {
      static int next_id = 0;
      return next_id++;
    }
    
    // Widget is a base class containing information needed by all GUI widget classes
    // (Buttons, Images, etc...).  It take in a return type to be cast to for accessors.
    
    class Widget {
    protected:
      std::string id;     // ID used for this element.
      UI::Style style;

      // Widget cannot be built unless within derived class, so constructors are protected
      Widget(const std::string & in_id="") : id(in_id) {
        EMP_TRACK_CONSTRUCT(WebWidget);
        if (id == "") id = emp::to_string("emp__", NextWidgetID());
      }
      Widget(const Widget & in) : id(in.id), style(in.style) {
        EMP_TRACK_CONSTRUCT(WebWidget);
      }
      virtual ~Widget() { EMP_TRACK_DESTRUCT(WebWidget); }

    public:
      const std::string & GetID() const { return id; }
      static std::string TypeName() { return "Web::WidgetCore"; }

      virtual std::string CSS(const std::string & setting) { return style.Get(setting); }
      bool HasCSS(const std::string & setting) { return style.Has(setting); }
      
    };

    // WidgetCore is a template that provides accessors into Widget with a derived return type.
    
    template <typename RETURN_TYPE>
    class WidgetCore : public Widget {
    protected: 
      // WidgetCore cannot be built unless within derived class, so constructors are protected
      WidgetCore(const std::string & in_id="") : Widget(in_id) { ; }
      WidgetCore(const WidgetCore & in) : Widget(in) { ; }
      virtual ~WidgetCore() { ; }

    public:
      template <typename SETTING_TYPE>
      RETURN_TYPE & CSS(const std::string & setting, SETTING_TYPE && value) {
        style.Set(setting, value);
        return (RETURN_TYPE &) *this;
      }

      // Allow multiple CSS settings to be grouped.
      template <typename T1, typename T2, typename... OTHER_SETTINGS>
      RETURN_TYPE & CSS(const std::string & setting1, T1 && val1,
                        const std::string & setting2, T2 && val2,
                        OTHER_SETTINGS... others) {
        CSS(setting1, val1);                      // Set the first CSS value.
        return CSS(setting2, val2, others...);    // Recurse to the others.
      }

      RETURN_TYPE & ID(const std::string & in_id) { id = in_id; return (RETURN_TYPE &) *this; }


      // Size Manipulation
      RETURN_TYPE & Width(double w, const std::string & unit="px") {
        return CSS("width", emp::to_string(w, unit) );
      }
      RETURN_TYPE & Height(double h, const std::string & unit="px") {
        return CSS("height", emp::to_string(h, unit) );
      }
      RETURN_TYPE & Size(double w, double h, const std::string & unit="px") {
        Width(w, unit); return Height(h, unit);
      }

      // Position Manipulation
      RETURN_TYPE & Center() { return CSS("margin", "auto"); }
      RETURN_TYPE & SetPosition(int x, int y, const std::String & unit="px") {
        return CSS("position", "fixed",
                   "left", emp::to_string(x, unit),
                   "top", emp::to_string(y, unit));
      }
      RETURN_TYPE & SetPositionRT(int x, int y, const std::String & unit="px") {
        return CSS("position", "fixed",
                   "right", emp::to_string(x, unit),
                   "top", emp::to_string(y, unit));
      }
      RETURN_TYPE & SetPositionRB(int x, int y, const std::String & unit="px") {
        return CSS("position", "fixed",
                   "right", emp::to_string(x, unit),
                   "bottom", emp::to_string(y, unit));
      }
      RETURN_TYPE & SetPositionLB(int x, int y, const std::String & unit="px") {
        return CSS("position", "fixed",
                   "left", emp::to_string(x, unit),
                   "bottom", emp::to_string(y, unit));
      }


      // Text Manipulation
      RETURN_TYPE & Font(const std::string & font) { return CSS("font-family", font); }
      RETURN_TYPE & FontSize(int s) { return CSS("font-size", emp::to_string(s, "px")); }
      RETURN_TYPE & FontSizeVW(double s) { return CSS("font-size", emp::to_string(s, "vw")); }
      RETURN_TYPE & CenterText() { return CSS("text-align", "center"); }

      // Color Manipulation
      RETURN_TYPE & Background(const std::string & v) { return CSS("background-color", v); }
      RETURN_TYPE & Color(const std::string & v) { return CSS("color", v); }
      RETURN_TYPE & Opacity(double v) { return CSS("opacity", v); }
    };
    
  };

};
};  


#endif

