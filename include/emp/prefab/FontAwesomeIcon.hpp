/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file FontAwesomeIcon.hpp
 *  @brief Wraps Font Awesome's icons.
 */

#ifndef EMP_PREFAB_FONTAWESOMEICON_HPP_INCLUDE
#define EMP_PREFAB_FONTAWESOMEICON_HPP_INCLUDE

#include "../tools/string_utils.hpp"
#include "../web/Div.hpp"
#include "../web/Element.hpp"
#include "../web/Widget.hpp"

namespace emp {
namespace prefab {
  /**
   * Use FontAwesomeIcon class to add a glyph from the
   * FontAwesome library to your web app.
   * (https://fontawesome.com/v4.7.0/icons/)
   */
  class FontAwesomeIcon: public web::Element {
    public:
      /**
       * @param fa_name the font awesome class of the desired icon
       * @param id optional unique id the icon can be referenced by
       */

      FontAwesomeIcon(const std::string fa_name, const std::string & id="")
        : web::Element("span", id) {
        std::string full_class = emp::to_string("fa ", fa_name);
        this->SetAttr("class", full_class);
      }

      /*
       * TODO: Prevent user from streaming content into the icon, throw error
       * The method below throws an error when trying to stream
       * a code block into anything else (web element, html).
       *
       * template <typename T>
       * void operator<<(T invalid) {
       *      emp::LibraryError("Not allowed to add code to the FontAwesome icon");
       * }
       */
  };
}
}

#endif // #ifndef EMP_PREFAB_FONTAWESOMEICON_HPP_INCLUDE
