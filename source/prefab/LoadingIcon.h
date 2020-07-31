#ifndef EMP_LOADING_ICON_H
#define EMP_LOADING_ICON_H

#include "../web/Element.h"
#include "../web/Div.h"
#include "../web/Widget.h"
#include "../tools/string_utils.h"
#include "../base/errors.h"

namespace emp {
namespace prefab {
  /// Use LoadingIcon class to add a loading glyph from the
  /// FontAwesome library to your web app.
  /// (https://fontawesome.com/v4.7.0/icon/spinner)
  class LoadingIcon: public web::Element {
    private:
      web::Element icon{emp::to_string("span")}; // Spinning icon, necessary classes will be added
      web::Element text{emp::to_string("span")}; // Alternative text

    public:
      LoadingIcon(const std::string & in_name=""): web::Element("span", in_name) {
          *this << icon;
          *this << text;
          icon.SetAttr("class", "fa fa-spinner fa-pulse fa-3x fa-fw");
          text.SetAttr("class", "sr-only");
          text << "Loading...";
      }

      // TODO: Prevent user from streaming content into the icon, throw error
      // The method below throws an error when trying to stream
      // a code block into anything else (web element, html).
      // template <typename T>
      // void operator<<(T invalid) {
      //     emp::LibraryError("Not allowed to add code to the loading icon");
      // }
  };
}
}

#endif