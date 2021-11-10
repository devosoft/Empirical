/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file LoadingIcon.hpp
 *  @brief Wraps Font Awesome's loading glyph.
 */

#ifndef EMP_PREFAB_LOADINGICON_HPP_INCLUDE
#define EMP_PREFAB_LOADINGICON_HPP_INCLUDE

#include "../base/errors.hpp"
#include "../tools/string_utils.hpp"
#include "../web/Div.hpp"
#include "../web/Element.hpp"
#include "../web/Widget.hpp"

namespace emp {
namespace prefab {
  /**
   * Use LoadingIcon class to add a loading glyph from the
   * FontAwesome library to your web app.
   * (https://fontawesome.com/v4.7.0/icon/spinner)
   */
  class LoadingIcon: public web::Element {
    private:
      web::Element icon{emp::to_string("span")}; // Spinning icon, necessary classes will be added
      web::Element text{emp::to_string("span")}; // Alternative text

    public:
      LoadingIcon(const std::string & id=""): web::Element("span", id) {
          *this << icon;
          *this << text;
          icon.SetAttr("class", "fa fa-spinner fa-pulse fa-3x fa-fw");
          text.SetAttr("class", "sr-only");
          text << "Loading...";
      }

      /*
       * TODO: Prevent user from streaming content into the icon, throw error
       * The method below throws an error when trying to stream
       * a code block into anything else (web element, html).
       *
       * template <typename T>
       * void operator<<(T invalid) {
       *      emp::LibraryError("Not allowed to add code to the loading icon");
       * }
       */
  };
}
}

#endif // #ifndef EMP_PREFAB_LOADINGICON_HPP_INCLUDE
