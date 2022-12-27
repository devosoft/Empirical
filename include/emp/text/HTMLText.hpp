/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022.
 *
 *  @file HTMLText.hpp
 *  @brief Builds on Text.hpp, but assumes all text input and output are in HTML encodings.
 *  @note Status: ALPHA
 * 
 */

#ifndef EMP_TOOLS_HTML_TEXT_HPP_INCLUDE
#define EMP_TOOLS_HTML_TEXT_HPP_INCLUDE

#include <set>
#include <string>
#include <unordered_map>

#include "../base/assert.hpp"
#include "../compiler/Lexer.hpp"

#include "Text.hpp"

namespace emp {

  class HTMLEncoding : public emp::TextEncoding {
  private:
    struct StyleInfo{
      std::string open;
      std::string close;
    };
    enum class TagType {
      UNKNOWN = 0,
      OPEN,     // Start a new style (e.g., '<b>')
      CLOSE,    // Stop using a style (e.g., '</b>')
      FORMAT,   // Indicate a structural element (e.g., '<p>')
      REPLACE,  // Indicate a direct replacement (e.g., '&lt;' means '<')
      TAG_COUNT // Track total number of tag types.
    };
    struct TagInfo {
      TagType type;
      std::string style;  // The style that this tag is associated with.
      std::string text;   // Basic text associated with this tag (e.g., for replacements)
    };

    std::unordered_map<std::string, StyleInfo> style_info;
    std::unordered_map<std::string, TagInfo> tag_info;
    std::string tag_regex;
    emp::Lexer lexer;

    int token_text = -1;
    int token_tag = -1;
    int token_char = -1;

    std::set<std::string> active_styles; // Styles to use on appended text.

    void BuildStyle(const std::string & style,
                   const std::string & open,
                   const std::string & close)
    {
      if (style_info.size()) tag_regex += '|';
      tag_regex += emp::to_literal(open) + '|' + emp::to_literal(close);

      style_info[style] = StyleInfo{open, close};
      tag_info[open] = TagInfo{TagType::OPEN, style};
      tag_info[close] = TagInfo{TagType::CLOSE, style};
    }

    void BuildReplacement(const std::string & html_v, const std::string & txt_v) {
      if (style_info.size()) tag_regex += '|';
      tag_regex += emp::to_literal(html_v);
      tag_info[html_v] = TagInfo{TagType::REPLACE, txt_v};
    }

    void SetupTags() {
      BuildStyle("bold", "<b>", "</b>");
      BuildStyle("code", "<code>", "</code>");
      BuildStyle("italic", "<i>", "</i>");
      BuildStyle("strike", "<del>", "</del>");
      BuildStyle("subscript", "<sub>", "</sub>");
      BuildStyle("superscript", "<sup>", "</sup>");
      BuildStyle("underline", "<u>", "</u>");

      BuildReplacement("&amp;", "&");
      BuildReplacement("&gt;", ">");
      BuildReplacement("&lt;", "<");
      BuildReplacement("&nbsp;", " ");

      // Now that all of the tags are loaded, put them into the lexer.
      token_text = lexer.AddToken("text","[^<&]+");    // Non-tag or special characters.
      token_tag = lexer.AddToken("tags", tag_regex);   // Tags
      token_char = lexer.AddToken("chars", "[<&]");    // Special char, but not as tag.
    }

    // Append a string that has already been otherwise processed.
    void Append_String(const std::string & in) {
      size_t start = text.size();
      size_t end = start + in.size();
      text.Append_Raw(in);
      for (const std::string & style : active_styles) {
        text.SetStyle(style, start, end);
      }
    }

    void Append_Tag(const std::string & tag) {
      const auto & info = tag_info[tag];
      switch (info.type) {
        case TagType::OPEN:
          active_styles.insert(info.style);
          break;
        case TagType::CLOSE:
          active_styles.erase(info.style);
          break;
        case TagType::REPLACE:
          Append_String(info.text);
          break;
        default:
          break;
      }
    }

  public:
    HTMLEncoding(Text & _text, const std::string _name="html")
      : TextEncoding(_text, _name) { SetupTags(); }
    ~HTMLEncoding() = default;

    // Add new HTML into this object.
    void Append(std::string in) override {
//      std::cout << "APPEND: " << in << std::endl;

      auto tokens = lexer.Tokenize(in);
      for (const auto & token : tokens) {
        if (token.id == token_text) Append_String(token.lexeme);
        else if (token.id == token_tag) Append_Tag(token.lexeme);
        else if (token.id == token_char) Append_String(token.lexeme);
        else {
          std::cerr << "Error, unknown tag: " << token.lexeme << std::endl;
        }
      }
    }

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
        if (output_pos < tag_pos) {
          out_string += text.GetText().substr(output_pos, tag_pos-output_pos);
          output_pos = tag_pos;
        }
        out_string += tags;
      }

      // Add any final text after the last tag.
      if (output_pos < text.size()) {
        out_string += text.GetText().substr(output_pos, text.size()-output_pos);
      }

      return out_string;
    }

  private:
    // ------------   Helper functions   ------------

    // A helper to add start and end tag info to tag map for insertion into
    // the output string as it's created.
    void AddOutputTags(
      std::map<size_t, std::string> & tag_map,
      std::string style,
      std::string start_tag,
      std::string end_tag) const
    {
      const BitVector & sites = text.GetStyle(style);

      // Test if this style should be opened at the beginning.
      if (sites.Has(0)) tag_map[0] += start_tag;

      // Loop through other site checking for shifts in style.
      for (size_t i = 1; i < sites.size(); ++i) {
        if (sites[i] != sites[i-1]) {
          if (sites[i]) tag_map[i] += start_tag;
          else tag_map[i] += end_tag;
        }
      }

      // Close any styles left open by the end.
      if (sites.back()) tag_map[sites.size()] += end_tag;
    }
  };

}

#endif