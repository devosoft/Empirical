/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022.
 *
 *  @file HTMLText.hpp
 *  @brief Like emp::Text, but defaults to using HTMLEncoding.
 *  @note Status: ALPHA
 * 
 */

#ifndef EMP_TOOLS_HTML_TEXT_HPP_INCLUDE
#define EMP_TOOLS_HTML_TEXT_HPP_INCLUDE

#include "Text.hpp"
#include "HTMLEncoding.hpp"

namespace emp {

  class HTMLText : public emp::Text {
  public:
    /// @brief Create a new, default HTMLText object.
    HTMLText() {
      encodings["html"] = NewPtr<HTMLEncoding>(*this, "html");
      encoding_ptr = encodings["html"];
    };

    /// @brief Copy over another Text object, but make it use an HTML encoding.
    /// @param in Starting text to load in.
    HTMLText(const Text & in) : Text(in) {
      if (!HasEncoding("html")) {
        encodings["html"] = NewPtr<HTMLEncoding>(*this, "html");
      }
      // Set the html encoding to be the default.
      encoding_ptr = encodings["html"];
    }

    /// @brief Create a new HTMLText object and default it to the provided string.
    /// @param in Staring (html-encoded) string to use.
    HTMLText(const std::string & in) {      
      encodings["html"] = NewPtr<HTMLEncoding>(*this, "html");
      encoding_ptr = encodings["html"];
      Append(in);
    }
  };

}

#endif