#ifndef EMP_UI_CSS_CLASS_H
#define EMP_UI_CSS_CLASS_H

///////////////////////////////////////////////////////////////////////////////////////////
//
//  A CSS class tracking font style, etc.
//

#include <string>

namespace emp {
namespace UI {

  class CSS_Class {
  private:
    std::map<std::string, std::string> settings;
    
  public:
    CSS_Class() { ; }
    CSS_Class(const CSS_Class &) = default;
    CSS_Class & operator=(const CSS_Class &) = default;
    
    int GetSize() const { return (int) settings.size(); }
    
    CSS_Class & DoSet(const std::string & in_set, const std::string & in_val) {
      settings[in_set] = in_val;
      return *this;
    }
    
    template <typename SET_TYPE>
    CSS_Class & Set(const std::string & s, SET_TYPE v) {
      return DoSet(s, emp::to_string(v));
    }

    CSS_Class & Insert(CSS_Class & in_css) {
      settings.insert(in_css.settings.begin(), in_css.settings.end());
      return *this;
    }

    const std::string & Get(const std::string setting) {
      // Note: if setting did not exist, this does create an empty entry.
      return settings[setting];
    };
    
    void Apply(const std::string & widget_id) {
      for (auto css_pair : settings) {
        if (css_pair.second == "") continue; // Ignore empy entries.
        EM_ASM_ARGS({
            var id = Pointer_stringify($0);
            var name = Pointer_stringify($1);
            var value = Pointer_stringify($2);
            $( '#' + id ).css( name, value);
          }, widget_id.c_str(), css_pair.first.c_str(), css_pair.second.c_str());
      };
    }
    
  };


};
};


#endif
