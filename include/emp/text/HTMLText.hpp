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

#include <string>
#include <sstream>
#include <unordered_map>

#include "../base/assert.hpp"
#include "../compiler/Lexer.hpp"

#include "Text.hpp"

namespace emp {

  class HTMLText : public emp::Text {
  private:
    struct StyleInfo{
      std::string open;
      std::string close;
    };
    enum class TagType {
      UNKNOWN = 0,
      OPEN,
      CLOSE,
      STRUCTURE,
      CHAR
    };
    struct TagInfo {
      TagType type;
      std::string style;
    };

    std::unordered_map<std::string, StyleInfo> style_info;
    std::unordered_map<std::string, TagInfo> tag_info;
    std::stringstream tag_regex;
    emp::Lexer lexer;

    void SetupStyle(const std::string & style,
                   const std::string & open,
                   const std::string & close)
    {
      if (style_info.size()) tag_regex << '|';
      tag_regex << open << '|' << close;

      style_info[style] = StyleInfo{open, close};
      tag_info[open] = TagInfo{TagType::OPEN, style};
      tag_info[close] = TagInfo{TagType::CLOSE, style};
    }

    void SetupTags() {
      SetupStyle("bold", "<b>", "</b>");
      SetupStyle("code", "<code>", "</code>");
      SetupStyle("italic", "<i>", "</i>");
      SetupStyle("strike", "<del>", "</del>");
      SetupStyle("subscript", "<sub>", "</sub>");
      SetupStyle("superscript", "<sup>", "</sup>");
      SetupStyle("underline", "<u>", "</u>");

      // Now that all of the tags are loaded, put them into the lexer.
      lexer.AddToken("text","[^<&]+");         // Non-tag or special characters.
      lexer.AddToken("tags", tag_regex.str()); // Tags
      lexer.AddToken("chars", "[<&]");         // Special char, but not as tag.
      lexer.Generate();
    }

    void Append(const std::string & in) {
      for (size_t scan_id = in.find('<');
           scan_id != std::string::npos;
           scan_id = in.find('<', scan_id)
      {

      }
    }

  public:
    HTMLText() { SetupTags(); }
    HTMLText(const HTMLText &) = default;
    HTMLText(HTMLText &&) = default;
    HTMLText(const std::string & in) { SetupTags(); Append(in); }
    ~HTMLText() = default;

    HTMLText & operator=(const HTMLText &) = default;
    HTMLText & operator=(HTMLText &&) = default;

    Text & operator=(const std::string & in) {
      Resize(0); // Clear out existing content.
      Append(in);
      return *this;
    };

    // Stream operator.
    template <typename T>
    Text & operator<<(T && in) {
      using in_t = std::remove_reference_t< std::remove_const_t<T> >;
      if constexpr (std::is_same_v<in_t, std::string>) {
        Append(in);
      } else {
        Append(emp::to_string(in));
      }
      return *this;
    }

    // Convert this to a string in HTML format.
    std::string ToString() {
      // Determine where tags should be placed.
      std::map<size_t, std::string> tag_map;
      for (const auto & [style, info] : style_info) {
        if (HasStyle(style)) AddOutputTags(tag_map, style, info.open, info.close);
      }

      // Convert the string, adding tags back in as we go.
      std::string out_string;
      size_t output_pos = 0;
      for (auto [tag_pos, tags] : tag_map) {
        if (output_pos < tag_pos) {
          out_string += text.substr(output_pos, tag_pos-output_pos);
          output_pos = tag_pos;
        }
        out_string += tags;
      }

      // Add any final text after the last tag.
      if (output_pos < text.size()) {
        out_string += text.substr(output_pos, text.size()-output_pos);
      }

      return out_string;
    }

  private:
    // ------------   Helper functions   ------------

    // A helper to add start and end tag info to tag map for insertion into
    // the output string as it's created.
    void AddOutputTags(
      std::map<size_t, std::string> & tag_map,
      std::string attr,
      std::string start_tag,
      std::string end_tag)
    {
      const BitVector & sites = attr_map[attr];

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