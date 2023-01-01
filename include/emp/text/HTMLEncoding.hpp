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

namespace emp {

  class HTMLEncoding : public emp::TextEncoding {
  private:
    // A categorical description of how a particular style tag should work.
    enum class TagType {
      OPEN,      // Start a new style that needs to be ended (e.g., '<b>' in HTML)
      CLOSE,     // Stop using a style (e.g., '</b>' in HTML)
      TOGGLE,    // Toggle a style on/off (e.g., '*' in markdown)
      LINE,      // Set a style until the end of line (e.g., <li> in HTML or # in markdown)
      STRUCTURE, // Indicate a structural element (e.g., '<div>' in HTML)
      REPLACE    // Indicate a simple replacement (e.g., '&lt;' means '<' in HTML)
    };

    // TagInfo provides detailed information for how tags should be handled.
    struct TagInfo {
      TagType type;
      std::string base_style; // The base style that this tag is associated with. (e.g., "bold")
      std::string text;       // Basic text associated with this tag (e.g., for simple replacements)
      std::string pattern;    // A regex pattern for identifying this tag.
    };

    // A struct to help translate a style back into relevant tags.
    struct StyleInfo {
      std::string name;   // What is the base name of this style? (e.g. "bold" or "font")
      std::string open;   // What tag should be used when this style starts?
      std::string close;  // What tag should be used when this style ends?
      bool has_args;      // Does this style have arguments associated with it?
    };

    std::unordered_map<std::string, StyleInfo> style_info;
    std::unordered_map<std::string, TagInfo> tag_info;
    std::string tag_regex;
    emp::Lexer lexer;

    int token_text = -1;
    int token_tag = -1;
    int token_char = -1;

    std::set<std::string> active_styles; // Styles to use on appended text.

    void BuildBaseStyle(
      const std::string & name,
      const std::string & open,
      const std::string & close)
    {
      
      const bool is_toggle = (open == close); // If open and close are the same, tag is a TOGGLE.
      const bool is_line = (close == "\n");   // If close is a newline, tag is a LINE type.

      // We always want to track the open tag.
      if (style_info.size()) tag_regex += '|';
      tag_regex += emp::to_literal(open);

      // Only track the close tag if meaningful.
      if (!is_toggle && !is_line) tag_regex += emp::to_string('|', emp::to_literal(close));

      style_info[name] = StyleInfo{name, open, close, false};

      if (is_toggle) {
        tag_info[open] = TagInfo{TagType::TOGGLE, name};
      } else if (is_line) {
        tag_info[open] = TagInfo{TagType::LINE, name};        
      } else {
        tag_info[open] = TagInfo{TagType::OPEN, name};      
        tag_info[close] = TagInfo{TagType::CLOSE, name};
      }
    }

    void BuildReplacement(const std::string & html_v, const std::string & txt_v) {
      if (style_info.size()) tag_regex += '|';
      tag_regex += emp::to_literal(html_v);
      tag_info[html_v] = TagInfo{TagType::REPLACE, txt_v};
    }

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
          active_styles.insert(info.base_style);
          break;
        case TagType::CLOSE:
          active_styles.erase(info.base_style);
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