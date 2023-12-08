/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2022-2023
*/
/**
 *  @file
 *  @brief Plugs into emp::Text, setting up inputs and output to be HTML encoded.
 *  @note Status: ALPHA
 *
 */

#ifndef EMP_TEXT_HTMLENCODING_HPP_INCLUDE
#define EMP_TEXT_HTMLENCODING_HPP_INCLUDE

#include <set>
#include <string>
#include <unordered_map>

#include "TextEncoding.hpp"
#include "Text.hpp"

namespace emp {

  class HTMLEncoding : public emp::TextEncoding {
  private:

    void SetupTags() {
      SetupStyleTags("bold", "<b>", "</b>");
      SetupStyleTags("code", "<code>", "</code>");
      SetupStyleTags("italic", "<i>", "</i>");
      SetupStyleTags("strike", "<del>", "</del>");
      SetupStyleTags("subscript", "<sub>", "</sub>");
      SetupStyleTags("superscript", "<sup>", "</sup>");
      SetupStyleTags("underline", "<u>", "</u>");
      SetupStyleTags("header1", "<h1>", "</h1>");
      SetupStyleTags("header2", "<h2>", "</h2>");
      SetupStyleTags("header3", "<h3>", "</h3>");
      SetupStyleTags("header4", "<h4>", "</h4>");
      SetupStyleTags("header5", "<h5>", "</h5>");
      SetupStyleTags("header6", "<h6>", "</h6>");

      SetupReplaceTag("&amp;", '&');
      SetupReplaceTag("&gt;", '>');
      SetupReplaceTag("&lt;", '<');
      SetupReplaceTag("&nbsp;", ' ', "no_break");
    }


  public:
    HTMLEncoding() { SetupTags(); }
    ~HTMLEncoding() = default;

    String GetName() const override { return "html"; }
    emp::Ptr<TextEncoding_Interface> Clone() const override {
      return emp::NewPtr<HTMLEncoding>();
    }
  };

  using HTMLText = EncodedText<HTMLEncoding>;

  template <typename... Ts>
  emp::Text MakeHTMLText(Ts &&... args) {
    return MakeEncodedText<HTMLEncoding>(std::forward<Ts>(args)...);
  }
}

#endif // #ifndef EMP_TEXT_HTMLENCODING_HPP_INCLUDE
