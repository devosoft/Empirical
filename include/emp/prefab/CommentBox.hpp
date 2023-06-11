/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file CommentBox.hpp
 *  @brief Create a light grey "comment bubble."
 *
 * TODO: When prefab tools for adding mobile only and desktop only
 * content are created, remove AddMobileContent(), desktop_content
 * and mobile_content divs, and ConfigPanel as a friend class.
 * AddConent() should stream into all_content div.
 */

#ifndef EMP_PREFAB_COMMENTBOX_HPP_INCLUDE
#define EMP_PREFAB_COMMENTBOX_HPP_INCLUDE

#include "../tools/string_utils.hpp"
#include "../web/Div.hpp"

namespace emp {
namespace prefab {
  class ConfigPanel;
  /**
   * Use CommentBox class to create a light grey "comment bubble".
   * Optionally, it can contain text and other web elements.
   */
  class CommentBox: public web::Div {
    friend prefab::ConfigPanel;
    private:
      // ID for the comment box Div
      std::string box_base = this->GetID();
      // Provides stylized "carrot" for box
      web::Div triangle{emp::to_string(box_base, "_triangle")};
      // Contains Divs for mobile and desktop content
      web::Div all_content{emp::to_string(box_base, "_all_content")};
      // Content that shows on all screen sizes
      web::Div desktop_content{emp::to_string(box_base, "_desktop_content")};
      // Content that shows only on mobile devices
      web::Div mobile_content{emp::to_string(box_base, "_mobile_content")};

    protected:
      /// Add content only to be shown on small screens

      /*
       * TODO: In the future, remove this method and instead
       * have a prefab tool that can add mobile content to
       *  web element.
       */
      template <typename T>
      void AddMobileContent(T val) {
        mobile_content << val;
      }

    public:
      CommentBox(const std::string id="") : web::Div(id) {
        *this << triangle;
        *this << all_content;
        all_content << desktop_content;
        all_content << mobile_content;

        triangle.SetAttr("class", "commentbox_triangle");
        all_content.SetAttr("class", "commentbox_content");
        mobile_content.SetAttr("class", "mobile_commentbox");
      }

      /**
       * Take input of any type and add it to the comment box.
       * Content will show on all screen sizes.
       */

      /*
       * TODO: override the input operator to stream into the correct div
       * See issue #345 (https://github.com/devosoft/Empirical/issues/345)
       * for methods tried already.
       */
      template <typename T>
      void AddContent(T val) {
        desktop_content << val;
      }
  };
}
}

#endif // #ifndef EMP_PREFAB_COMMENTBOX_HPP_INCLUDE
