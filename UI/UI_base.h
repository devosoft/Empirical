#ifndef EMP_UI_BASE_H
#define EMP_UI_BASE_H

#include <functional>
#include <map>
#include <ostream>
#include <string>

#include "CSS_Class.h"

namespace emp {
namespace UI {

  namespace internal {
  
    // Provide a quick method for generating unique IDs when not otherwise specified.
    static std::string CalcNextID() {
      static int next_id = 0;
      return std::string("emp__") + std::to_string(next_id++);
    }
    
    // Widget_base is a base class containing information needed by all GUI widget classes
    // (Buttons, Images, etc...).
    
    class Widget_base {
    protected: 
      std::string div_id;  // ID used for the div surrounding this element.
      std::string obj_ext; // Extension for internal object if eeds own id: div_id + '__but'
      int width;
      int height;
      
      CSS_Class css_info;
      
      Widget_base() : width(-1), height(-1) { ; }

    public:
      const std::string & GetDivID() const { return div_id; }      
    };
    
    // Widget_wrap is a template wrapper to make sure all derived widgets return the
    // correct types when setting values.
    
    template <typename DETAIL_TYPE, typename... ARG_TYPES>
    class Widget_wrap : public DETAIL_TYPE {
    public:
      Widget_wrap(ARG_TYPES... args, const std::string & in_name="") : DETAIL_TYPE(args...)
      {
        // Setup the div_id to a non-empty and unique default if not specified.
        Widget_base::div_id = (in_name == "") ? CalcNextID() : in_name;
      }

      Widget_wrap & DivID(const std::string & in_name) {
        Widget_base::div_id = in_name;
        return *this;
      }
      Widget_wrap & Width(int w) { Widget_base::width = w; return *this; }
      Widget_wrap & Height(int h) { Widget_base::height = h; return *this; }
      Widget_wrap & Size(int w, int h) {
        Widget_base::width = w;
        Widget_base::height = h;
        return *this;
      }
      
      template <typename SETTING_TYPE>
      Widget_wrap & CSS(const std::string & setting, SETTING_TYPE && value) {
        Widget_base::css_info.Set(setting, value);
        return *this;
      }

      Widget_wrap & Background(const std::string & v) { return CSS("background-color", v); }
      Widget_wrap & Color(const std::string & v) { return CSS("color", v); }
      Widget_wrap & Opacity(double v) { return CSS("opacity", v); }

      void TriggerCSS() {
        std::string obj_id = Widget_base::div_id + Widget_base::obj_ext;
        Widget_base::css_info.Apply(obj_id);
      }
    };
    
  };


  // Specialty functions...

  std::string Link(const std::string & url, const std::string & text="") {
    std::string out_string = "<a href=\"";
    out_string += url;
    out_string += "\">";
    out_string += (text == "") ? url : text;
    out_string += "</a>";
    return out_string;
  }

  template <typename VAR_TYPE>
  std::function<std::string()> Var(VAR_TYPE & var) {
    return [&var](){ return emp::to_string(var); };
  }

  // Live keyword means that whatever is passed in needs to be re-evaluated every update.

  template <typename VAR_TYPE>
  std::function<std::string()> Live(VAR_TYPE & var) {
    return [&var](){ return emp::to_string(var); };
  }

  template <typename RET_TYPE>
  std::function<std::string()> Live(const std::function<RET_TYPE()> & fun) {
    return [fun](){ return emp::to_string(fun()); };
  }

  struct Close {
    int levels;
    Close(int l=1) : levels(l) { ; }

    virtual bool IsGeneral() const { return true; }
    virtual bool SlateOK() const { return true; }
    virtual bool TextOK() const { return true; }
  };

  struct CloseSlate : public Close {
    CloseSlate(int l=1) : Close(l) { ; }
    virtual bool IsGeneral() const { return false; }
    virtual bool SlateOK() const { return true; }
    virtual bool TextOK() const { return false; }
  };

  struct CloseText : public Close {
    CloseText(int l=1) : Close(l) { ; }
    virtual bool IsGeneral() const { return false; }
    virtual bool SlateOK() const { return false; }
    virtual bool TextOK() const { return true; }
  };

};
};

#endif
