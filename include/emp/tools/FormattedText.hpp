/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022.
 *
 *  @file FormattedText.hpp
 *  @brief Functionality similar to std::string, but tracks text formatting for easy conversion.
 *  @note Status: ALPHA
 * 
 *  FormattedText should be functionally interchangable with string, but can easily convert to
 *  HTML, Latex, RTF, or other formats that support bold, italic, super/sub-scripting, fonts,
 *  etc.
 */

#ifndef EMP_TOOLS_FORMATTED_TEXT_HPP_INCLUDE
#define EMP_TOOLS_FORMATTED_TEXT_HPP_INCLUDE

#include <map>
#include <string>
#include <unordered_map>

#include "../base/assert.hpp"
#include "../bits/BitVector.hpp"
#include "../datastructs/map_utils.hpp"

namespace emp {

  class FormattedText {
  private:
    std::string text = "";
    // Attributes are basic formatting for strings, including "bold", "italic", "underline",
    // "strike", "superscript", "subscript", and "code".  Fonts are described as font name,
    // a colon, and the font size.  E.g.: "TimesNewRoman:12"
    std::unordered_map<std::string, BitVector> attr_map;

    struct TagInfo{
      std::string open;
      std::string close;
    };    
    using tag_map_t = std::unordered_map<std::string, TagInfo>;
    std::unordered_map<std::string, tag_map_t> tag_maps;

  public:
    FormattedText() = default;
    FormattedText(const FormattedText &) = default;
    FormattedText(FormattedText &&) = default;
    FormattedText(const std::string & in) : text(in) { };
    FormattedText(std::string && in) : text(std::move(in)) { };
    ~FormattedText() = default;

    FormattedText & operator=(const FormattedText &) = default;
    FormattedText & operator=(FormattedText &&) = default;

    FormattedText & operator=(const std::string & in) {
      attr_map.clear(); // Clear out existing content.
      text=in;
      return *this;
    };
    FormattedText & operator=(std::string && in) {
      attr_map.clear(); // Clear out existing content.
      text = std::move(in);
      return *this;
    }

    // GetSize() returns the number of characters IGNORING all formatting.
    size_t GetSize() const { return text.size(); }

    // Return the current text as an unformatted string.
    const std::string & GetString() const { return text; }

    /// Automatic conversion back to a (unformatted) string
    operator const std::string &() const { return GetString(); }

    void Resize(size_t new_size) {
      text.resize(new_size);
      for (auto & [tag, bits] : attr_map) {
        if (bits.GetSize() > new_size) bits.Resize(new_size);
      }
    }

    char & operator[](size_t pos) {
      emp_assert(pos < GetSize(), pos, GetSize());
      return text[pos];
    }

    char operator[](size_t pos) const {
      emp_assert(pos < GetSize(), pos, GetSize());
      return text[pos];
    }

    // STL-like functions for perfect compatability with string.
    size_t size() const { return text.size(); }
    void resize(size_t new_size) { Resize(new_size); }

    template <typename... Ts>
    FormattedText & assign(Ts &&... in) { text.assign( std::forward<Ts>(in)... ); }
    char & front() { return text[0]; }
    char front() const { return text[0]; }
    char & back() { return text[size()-1]; }
    char back() const { return text[size()-1]; }
    bool empty() const { return text.empty(); }
    // void push_back(char c) { text.push_back(c); }
    // void pop_back() { text.pop_back(); }
    template <typename... Ts>
    bool starts_with(Ts &&... in) const { text.starts_with(std::forward<Ts>(in)... ); }
    template <typename... Ts>
    bool ends_with(Ts &&... in) const { text.ends_with(std::forward<Ts>(in)... ); }

    template <typename... Ts>
    size_t find(Ts &&.. in) const { return text.find(std::forward<Ts>(in)...); }
    template <typename... Ts>
    size_t rfind(Ts &&.. in) const { return text.rfind(std::forward<Ts>(in)...); }
    template <typename... Ts>
    size_t find_first_of(Ts &&.. in) const { return text.find_first_of(std::forward<Ts>(in)...); }
    template <typename... Ts>
    size_t find_first_not_of(Ts &&.. in) const { return text.find_first_not_of(std::forward<Ts>(in)...); }
    template <typename... Ts>
    size_t find_last_of(Ts &&.. in) const { return text.find_last_of(std::forward<Ts>(in)...); }
    template <typename... Ts>
    size_t find_last_not_of(Ts &&.. in) const { return text.find_last_not_of(std::forward<Ts>(in)...); }

    // ---------------- FORMATTING functions ----------------

    // Simple formatting: set all characters to a specified format.
    FormattedText & SetStyle(std::string style) {
      BitVector & cur_bits = attr_map[style];
      cur_bits.Resize(text.size());
      cur_bits.SetAll();
      return *this;
    }
    FormattedText & Bold() { return SetStyle("bold"); }
    FormattedText & Code() { return SetStyle("code"); }
    FormattedText & Italic() { return SetStyle("italic"); }
    FormattedText & Strike() { return SetStyle("strike"); }
    FormattedText & Subscript() { return SetStyle("subscript"); }
    FormattedText & Superscript() { return SetStyle("superscript"); }
    FormattedText & Underline() { return SetStyle("underline"); }

    // Simple formatting: set a single character to a specified format.
    FormattedText & SetStyle(std::string style, size_t pos) {
      BitVector & cur_bits = attr_map[style];
      if (cur_bits.size() <= pos) cur_bits.Resize(pos+1);
      cur_bits.Set(pos);
      return *this;
    }
    FormattedText & Bold(size_t pos) { return SetStyle("bold", pos); }
    FormattedText & Code(size_t pos) { return SetStyle("code", pos); }
    FormattedText & Italic(size_t pos) { return SetStyle("italic", pos); }
    FormattedText & Strike(size_t pos) { return SetStyle("strike", pos); }
    FormattedText & Subscript(size_t pos) { return SetStyle("subscript", pos); }
    FormattedText & Superscript(size_t pos) { return SetStyle("superscript", pos); }
    FormattedText & Underline(size_t pos) { return SetStyle("underline", pos); }

    // Simple formatting: set a range of characters to a specified format.
    FormattedText & SetStyle(std::string style, size_t start, size_t end) {
      BitVector & cur_bits = attr_map[style];
      emp_assert(start <= end && end <= text.size());
      if (cur_bits.size() <= end) cur_bits.Resize(end+1);
      cur_bits.SetRange(start, end);
      return *this;
    }
    FormattedText & Bold(size_t start, size_t end) { return SetStyle("bold", start, end ); }
    FormattedText & Code(size_t start, size_t end) { return SetStyle("code", start, end); }
    FormattedText & Italic(size_t start, size_t end) { return SetStyle("italic", start, end); }
    FormattedText & Strike(size_t start, size_t end) { return SetStyle("strike", start, end); }
    FormattedText & Subscript(size_t start, size_t end) { return SetStyle("subscript", start, end); }
    FormattedText & Superscript(size_t start, size_t end) { return SetStyle("superscript", start, end); }
    FormattedText & Underline(size_t start, size_t end) { return SetStyle("underline", start, end); }

    // Test if a particular style is present anywhere in the text
    bool HasStyle(const std::string & style) const {
      if (!emp::Has(attr_map, style)) return false;
      return GetConstRef(attr_map, style).Any();
    }
    bool HasBold() const { return HasStyle("bold"); }
    bool HasCode() const { return HasStyle("code"); }
    bool HasItalic() const { return HasStyle("italic"); }
    bool HasStrike() const { return HasStyle("strike"); }
    bool HasSubscript() const { return HasStyle("subscript"); }
    bool HasSuperscript() const { return HasStyle("superscript"); }
    bool HasUnderline() const { return HasStyle("underline"); }

    // Test if a particular style is present at a given position.
    bool HasStyle(const std::string & style, size_t pos) const {
      auto it = attr_map.find(style);
      if (it == attr_map.end()) return false; // Style is nowhere.
      return it->second.Has(pos);
    }
    bool HasBold(size_t pos) const { return HasStyle("bold", pos); }
    bool HasCode(size_t pos) const { return HasStyle("code", pos); }
    bool HasItalic(size_t pos) const { return HasStyle("italic", pos); }
    bool HasStrike(size_t pos) const { return HasStyle("strike", pos); }
    bool HasSubscript(size_t pos) const { return HasStyle("subscript", pos); }
    bool HasSuperscript(size_t pos) const { return HasStyle("superscript", pos); }
    bool HasUnderline(size_t pos) const { return HasStyle("underline", pos); }

    // Clear ALL formatting
    FormattedText & Clear() { attr_map.clear(); return *this; }

    // Clear specific formatting across all text
    FormattedText & Clear(const std::string & style) {
      attr_map.erase(style);
      return *this;
    }
    FormattedText & ClearBold() { return Clear("bold"); }
    FormattedText & ClearCode() { return Clear("code"); }
    FormattedText & ClearItalic() { return Clear("italic"); }
    FormattedText & ClearStrike() { return Clear("strike"); }
    FormattedText & ClearSubscript() { return Clear("subscript"); }
    FormattedText & ClearSuperscript() { return Clear("superscript"); }
    FormattedText & ClearUnderline() { return Clear("underline"); }
    
    // Simple formatting: clear a single character from a specified format.
    FormattedText & Clear(const std::string & style, size_t pos) {
      auto it = attr_map.find(style);
      if (it != attr_map.end() && it->second.size() > pos) {  // If style bit exists...
        it->second.Clear(pos);
      }
      return *this;
    }
    FormattedText & ClearBold(size_t pos) { return Clear("bold", pos); }
    FormattedText & ClearCode(size_t pos) { return Clear("code", pos); }
    FormattedText & ClearItalic(size_t pos) { return Clear("italic", pos); }
    FormattedText & ClearStrike(size_t pos) { return Clear("strike", pos); }
    FormattedText & ClearSubscript(size_t pos) { return Clear("subscript", pos); }
    FormattedText & ClearSuperscript(size_t pos) { return Clear("superscript", pos); }
    FormattedText & ClearUnderline(size_t pos) { return Clear("underline", pos); }

    // Simple formatting: clear a range of characters from a specified format.
    FormattedText & Clear(const std::string & style, size_t start, size_t end) {
      auto it = attr_map.find(style);
      if (it != attr_map.end() && it->second.size() > start) {  // If style bits exist...
        if (end > it->second.size()) end = it->second.size();   // ...don't pass text end
        it->second.Clear(start, end);
      }
      return *this;
    }
    FormattedText & ClearBold(size_t start, size_t end) { return Clear("bold", start, end); }
    FormattedText & ClearCode(size_t start, size_t end) { return Clear("code", start, end); }
    FormattedText & ClearItalic(size_t start, size_t end) { return Clear("italic", start, end); }
    FormattedText & ClearStrike(size_t start, size_t end) { return Clear("strike", start, end); }
    FormattedText & ClearSubscript(size_t start, size_t end) { return Clear("subscript", start, end); }
    FormattedText & ClearSuperscript(size_t start, size_t end) { return Clear("superscript", start, end); }
    FormattedText & ClearUnderline(size_t start, size_t end) { return Clear("underline", start, end); }

    // Compatibility with std::string
    size_t size() const { return GetSize(); }

    tag_map_t & GetHTMLMap() {
      tag_map_t & html_map = tag_maps["html"];
      if (html_map.size() == 0) {
      html_map["bold"] = TagInfo{"<b>", "</b>"};
      html_map["code"] = TagInfo{"<code>", "</code>"};
      html_map["italic"] = TagInfo{"<i>", "</i>"};
      html_map["strike"] = TagInfo{"<del>", "</del>"};
      html_map["subscript"] = TagInfo{"<sub>", "</sub>"};
      html_map["superscript"] = TagInfo{"<sup>", "</sup>"};
      html_map["underline"] = TagInfo{"<u>", "</u>"};
      }
      return html_map;
    }

    // Convert this to a string in HTML format.
    std::string AsHTML() {
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