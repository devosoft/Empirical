/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-23.
 *
 *  @file HTMLText.hpp
 *  @brief Like emp::Text, but defaults to using HTMLEncoding.
 *  @note Status: ALPHA
 */

#ifndef EMP_TOOLS_HTML_TEXT_HPP_INCLUDE
#define EMP_TOOLS_HTML_TEXT_HPP_INCLUDE

#include "Text.hpp"
#include "HTMLEncoding.hpp"

namespace emp {

  class HTMLText : public emp::Text {
  public:
    /// @brief Create a new, default HTMLText object.
    HTMLText() { AddEncoding<HTMLEncoding>(); };

    /// @brief Copy over another Text object, but make it uses an HTML encoding.
    /// @param in Already formatted Text object to load in.
    HTMLText(const Text & in) : Text(in) { ActivateEncoding<HTMLEncoding>("html"); }

    /// @brief Create a new HTMLText object and default it to the provided string.
    /// @param in Starting (html-encoded) string to use.
    HTMLText(const String & in)      { AppendAs<HTMLEncoding>("html", in); }
    HTMLText(const std::string & in) { AppendAs<HTMLEncoding>("html", in); }
    HTMLText(const char * in)        { AppendAs<HTMLEncoding>("html", in); }

    HTMLText & operator=(const HTMLText &) = default;
    HTMLText & operator=(HTMLText &&) = default;
    template <typename T>
    HTMLText & operator=(T && in) { Text::operator=(std::forward<T>(in)); return *this; };
  };

}

#endif