/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022.
 *
 *  @file HTMLEncoding.hpp
 *  @brief Plugs into emp::Text, setting up inputs and output to be HTML encoded.
 *  @note Status: ALPHA
 * 
 */

#ifndef EMP_TOOLS_HTML_ENCODING_HPP_INCLUDE
#define EMP_TOOLS_HTML_ENCODING_HPP_INCLUDE

#include <set>
#include <string>
#include <unordered_map>

#include "../base/assert.hpp"
#include "../compiler/Lexer.hpp"

#include "Text.hpp"
#include "TextEncoding.hpp"

namespace emp {

  class HTMLEncoding : public emp::TextEncoding {
  private:

    void SetupTags() {
      BuildBaseStyle("bold", "<b>", "</b>");
      BuildBaseStyle("code", "<code>", "</code>");
      BuildBaseStyle("italic", "<i>", "</i>");
      BuildBaseStyle("strike", "<del>", "</del>");
      BuildBaseStyle("subscript", "<sub>", "</sub>");
      BuildBaseStyle("superscript", "<sup>", "</sup>");
      BuildBaseStyle("underline", "<u>", "</u>");
      BuildBaseStyle("header1", "<h1>", "</h1>");
      BuildBaseStyle("header2", "<h2>", "</h2>");
      BuildBaseStyle("header3", "<h3>", "</h3>");
      BuildBaseStyle("header4", "<h4>", "</h4>");
      BuildBaseStyle("header5", "<h5>", "</h5>");
      BuildBaseStyle("header6", "<h6>", "</h6>");

      BuildReplacement("&amp;", "&");
      BuildReplacement("&gt;", ">");
      BuildReplacement("&lt;", "<");
      BuildReplacement("&nbsp;", " ");

      // Now that all of the tags are loaded, put them into the lexer.
      token_text = lexer.AddToken("text","[^<&]+");    // Non-tag or special characters.
      token_tag = lexer.AddToken("tags", tag_regex);   // Tags
      token_char = lexer.AddToken("chars", "[<&]");    // Special char, but not as tag.
    }


  public:
    HTMLEncoding(Text & _text, const std::string _name="html")
      : TextEncoding(_text, _name) { SetupTags(); }
    ~HTMLEncoding() = default;


    // Convert this to a string in HTML format.
    std::string ToString() const override {
      // Determine where tags should be placed.
      std::map<size_t, std::string> tag_map;
      for (const auto & [style, info] : style_info) {
        if (text.HasStyle(style)) AddOutputTags(tag_map, style, info.open, info.close);
      }

      // Convert the string, adding tags back in as we go.
      std::string out_string;
      size_t output_pos = 0;
      for (auto [tag_pos, tags] : tag_map) {
        while (output_pos < tag_pos) {
          char next_char = text.GetChar(output_pos);
          switch (next_char) {
          case '<':  out_string += "&lt;";   break;
          case '>':  out_string += "&gt;";   break;
          case '&':  out_string += "&amp;";  break;
          default:
            out_string += next_char;
          }
          ++output_pos;
        }
        out_string += tags;
      }

      // Add any final text after the last tag.
      if (output_pos < text.size()) {
        out_string += text.GetText().substr(output_pos, text.size()-output_pos);
      }

      return out_string;
    }

  };

}

#endif