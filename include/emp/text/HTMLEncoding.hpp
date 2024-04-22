/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2022-2024
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
#include "Text_utils.hpp"

namespace emp {

  class HTMLEncoding : public emp::TextEncoding {
  private:

    void SetupTags() {
      const auto & code_map = GetTextStyleMap_FromHTML();
      for (auto & [html_tag, style] : code_map) {
        AddStyleTags(style, MakeString("<",html_tag,">"), MakeString("</",html_tag,">"));
      }

      AddReplaceTag("&amp;", '&');
      AddReplaceTag("&gt;",  '>');
      AddReplaceTag("&lt;",  '<');
      AddReplaceTag("&tab;", '\t');
      AddReplaceTag("&nbsp;",    ' ', "no_break");

      // For generic symbols
      AddReplaceControl("&", ';', ' ', "symbol");


      AddReplaceTag("<br>",  '\n');           // A line break
      // AddReplaceTag("\\b",  ' ', "page_break");       // A page break
      AddReplaceTag("<p>",  ' ', "para_break");       // A paragraph break
      AddReplaceTag("<hr>",  '-', "horizontal_rule");  // A horizontal rule
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
