#ifndef EMP_WEB_STYLE_H
#define EMP_WEB_STYLE_H

///////////////////////////////////////////////////////////////////////////////////////////
//
//  A CSS class tracking font style, etc.
//

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#include "../tools/string_utils.h"

#include <map>
#include <string>

namespace emp {
namespace web {

  class Style {
  private:
    std::map<std::string, std::string> settings;
    
  public:
    Style() { ; }
    Style(const Style &) = default;
    Style & operator=(const Style &) = default;
    
    int GetSize() const { return (int) settings.size(); }
    
    Style & DoSet(const std::string & in_set, const std::string & in_val) {
      settings[in_set] = in_val;
      return *this;
    }
    
    template <typename SET_TYPE>
    Style & Set(const std::string & s, SET_TYPE v) {
      return DoSet(s, emp::to_string(v));
    }

    Style & Insert(Style & in_css) {
      settings.insert(in_css.settings.begin(), in_css.settings.end());
      return *this;
    }

    bool Has(const std::string setting) {
      return settings.find(setting) != settings.end();
    };

    const std::string & Get(const std::string setting) {
      // Note: if setting did not exist, this does create an empty entry.
      return settings[setting];
    };
    
    void Apply(const std::string & widget_id) {
      for (auto css_pair : settings) {
        if (css_pair.second == "") continue; // Ignore empy entries.
#ifdef EMSCRIPTEN
        EM_ASM_ARGS({
            var id = Pointer_stringify($0);
            var name = Pointer_stringify($1);
            var value = Pointer_stringify($2);
            $( '#' + id ).css( name, value);
          }, widget_id.c_str(), css_pair.first.c_str(), css_pair.second.c_str());
#else
        std::cout << "Setting '" << widget_id << "' attribute '" << css_pair.first
                  << "' to '" << css_pair.second << "'.";
#endif
      }
    }
    
  };


};
};


#endif
