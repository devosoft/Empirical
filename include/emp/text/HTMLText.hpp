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

#include <unordered_map>

#include "../base/assert.hpp"

#include "Text.hpp"

namespace emp {

  class HTMLText : public emp::Text {
  private:
    struct TagInfo{
      std::string open;
      std::string close;
    };    
    std::unordered_map<std::string, TagInfo> tag_map;

    void SetupTags() {
      tag_map["bold"] = TagInfo{"<b>", "</b>"};
      tag_map["code"] = TagInfo{"<code>", "</code>"};
      tag_map["italic"] = TagInfo{"<i>", "</i>"};
      tag_map["strike"] = TagInfo{"<del>", "</del>"};
      tag_map["subscript"] = TagInfo{"<sub>", "</sub>"};
      tag_map["superscript"] = TagInfo{"<sup>", "</sup>"};
      tag_map["underline"] = TagInfo{"<u>", "</u>"};
    }

    void Append(const std::string & in) {

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
      std::map<size_t, std::string> tag_map; // Where do tags go?
      if (HasBold()) AddOutputTags(tag_map, "bold", "<b>", "</b>");
      if (HasCode()) AddOutputTags(tag_map, "code", "<code>", "</code>");
      if (HasItalic()) AddOutputTags(tag_map, "italic", "<i>", "</i>");
      if (HasStrike()) AddOutputTags(tag_map, "strike", "<del>", "</del>");
      if (HasSubscript()) AddOutputTags(tag_map, "subscript", "<sub>", "</sub>");
      if (HasSuperscript()) AddOutputTags(tag_map, "superscript", "<sup>", "</sup>");
      if (HasUnderline()) AddOutputTags(tag_map, "underline", "<u>", "</u>");

      std::string out_string;
      size_t copy_pos = 0;
      for (auto [tag_pos, tags] : tag_map) {
        if (copy_pos < tag_pos) {
          out_string += text.substr(copy_pos, tag_pos-copy_pos);
          copy_pos = tag_pos;
        }
        out_string += tags;
      }
      if (copy_pos < text.size()) {
        out_string += text.substr(copy_pos, text.size()-copy_pos);
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

      if (sites.Has(0)) tag_map[0] += start_tag;
      for (size_t i = 1; i < sites.size(); ++i) {
        if (sites[i] != sites[i-1]) {
          if (sites[i]) tag_map[i] += start_tag;
          else tag_map[i] += end_tag;
        }
      }
      if (sites.back()) tag_map[sites.size()] += end_tag;
    }
  };

}

#endif