#ifndef EMP_UI_BASE_H
#define EMP_UI_BASE_H

#include <functional>
#include <ostream>
#include <string>

namespace emp {
namespace UI {

  namespace internal {
  
    // CSS_Info contains information about a single CSS Setting.
    
    struct CSS_Info {
      std::string setting;
      std::string value;
      
      CSS_Info(const std::string & s, const std::string & v) : setting(s), value(v) { ; }
    };

    // Widget_base is a base class containing information needed by all GUI widget classes
    // (Buttons, Images, etc...).
    
    class Widget_base {
    protected: 
      std::string div_id;  // ID used for the div surrounding this element.
      std::string obj_ext; // Extension for internal object if eeds own id: div_id + '__but'
      int width;
      int height;
      
      std::vector<CSS_Info> css_mods;
      
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
        Widget_base::div_id = in_name;
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
      
      Widget_wrap & Background(const std::string & color) {
        Widget_base::css_mods.emplace_back("background-color", color);
        return *this;
      }

      void TriggerCSS() {
        for (auto css_mod : Widget_base::css_mods) {
          std::string obj_id = Widget_base::div_id + Widget_base::obj_ext;
          EM_ASM_ARGS({
              var id = Pointer_stringify($0);
              var name = Pointer_stringify($1);
              var value = Pointer_stringify($2);
              $( '#' + id ).css( name, value);
            }, obj_id.c_str(), css_mod.setting.c_str(), css_mod.value.c_str());
        };
      }
    };
    
  };


  // class Text : public UI_base {
  // protected:
  // public:
  //   Text(const std::string & in_name="") : UI_base(in_name) { ; }
  // };


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

};
};

#endif
