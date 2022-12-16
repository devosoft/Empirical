/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022.
 *
 *  @file Text.hpp
 *  @brief Functionality similar to std::string, but tracks text formatting for easy conversion.
 *  @note Status: ALPHA
 * 
 *  Text should be functionally interchangable with string, but can easily convert to
 *  HTML, Latex, RTF, or other formats that support bold, italic, super/sub-scripting, fonts,
 *  etc.
 */

#ifndef EMP_TOOLS_TEXT_HPP_INCLUDE
#define EMP_TOOLS_TEXT_HPP_INCLUDE

#include <map>
#include <string>
#include <unordered_map>

#include "../base/assert.hpp"
#include "../bits/BitVector.hpp"
#include "../datastructs/map_utils.hpp"

namespace emp {

  class Text;

  // An individual proxy character from Text that is format aware.
  class TextCharRef {
  private:
    emp::Ptr<Text> text_ptr;
    size_t pos;
  public:
    TextCharRef(emp::Ptr<Text> _ptr, size_t _pos) : text_ptr(_ptr), pos(_pos) { }
    TextCharRef(const TextCharRef & in) = default;
    ~TextCharRef() = default;

    // Set this character equal (with same inputs) as in parameter; don't change reference.
    TextCharRef & operator=(const TextCharRef & in);

    // Set just this character; don't change style.
    TextCharRef & operator=(char in);

    // Convert to a normal C++ char.
    operator char();

  };

  class Text {
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
    Text() = default;
    Text(const Text &) = default;
    Text(Text &&) = default;
    Text(const std::string & in) : text(in) { };
    Text(std::string && in) : text(std::move(in)) { };
    ~Text() = default;

    Text & operator=(const Text &) = default;
    Text & operator=(Text &&) = default;

    Text & operator=(const std::string & in) {
      attr_map.clear(); // Clear out existing content.
      text=in;
      return *this;
    };
    Text & operator=(std::string && in) {
      attr_map.clear(); // Clear out existing content.
      text = std::move(in);
      return *this;
    }

    // GetSize() returns the number of characters IGNORING all formatting.
    size_t GetSize() const { return text.size(); }

    // Return the current text as an unformatted string.
    const std::string & GetString() const { return text; }

    /// Automatic conversion back to an unformatted string
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
    Text & assign(Ts &&... in) { text.assign( std::forward<Ts>(in)... ); }
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
    Text & SetStyle(std::string style) {
      BitVector & cur_bits = attr_map[style];
      cur_bits.Resize(text.size());
      cur_bits.SetAll();
      return *this;
    }
    Text & Bold() { return SetStyle("bold"); }
    Text & Code() { return SetStyle("code"); }
    Text & Italic() { return SetStyle("italic"); }
    Text & Strike() { return SetStyle("strike"); }
    Text & Subscript() { return SetStyle("subscript"); }
    Text & Superscript() { return SetStyle("superscript"); }
    Text & Underline() { return SetStyle("underline"); }

    // Simple formatting: set a single character to a specified format.
    Text & SetStyle(std::string style, size_t pos) {
      BitVector & cur_bits = attr_map[style];
      if (cur_bits.size() <= pos) cur_bits.Resize(pos+1);
      cur_bits.Set(pos);
      return *this;
    }
    Text & Bold(size_t pos) { return SetStyle("bold", pos); }
    Text & Code(size_t pos) { return SetStyle("code", pos); }
    Text & Italic(size_t pos) { return SetStyle("italic", pos); }
    Text & Strike(size_t pos) { return SetStyle("strike", pos); }
    Text & Subscript(size_t pos) { return SetStyle("subscript", pos); }
    Text & Superscript(size_t pos) { return SetStyle("superscript", pos); }
    Text & Underline(size_t pos) { return SetStyle("underline", pos); }

    // Simple formatting: set a range of characters to a specified format.
    Text & SetStyle(std::string style, size_t start, size_t end) {
      BitVector & cur_bits = attr_map[style];
      emp_assert(start <= end && end <= text.size());
      if (cur_bits.size() <= end) cur_bits.Resize(end+1);
      cur_bits.SetRange(start, end);
      return *this;
    }
    Text & Bold(size_t start, size_t end) { return SetStyle("bold", start, end ); }
    Text & Code(size_t start, size_t end) { return SetStyle("code", start, end); }
    Text & Italic(size_t start, size_t end) { return SetStyle("italic", start, end); }
    Text & Strike(size_t start, size_t end) { return SetStyle("strike", start, end); }
    Text & Subscript(size_t start, size_t end) { return SetStyle("subscript", start, end); }
    Text & Superscript(size_t start, size_t end) { return SetStyle("superscript", start, end); }
    Text & Underline(size_t start, size_t end) { return SetStyle("underline", start, end); }

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
    Text & Clear() { attr_map.clear(); return *this; }

    // Clear specific formatting across all text
    Text & Clear(const std::string & style) {
      attr_map.erase(style);
      return *this;
    }
    Text & ClearBold() { return Clear("bold"); }
    Text & ClearCode() { return Clear("code"); }
    Text & ClearItalic() { return Clear("italic"); }
    Text & ClearStrike() { return Clear("strike"); }
    Text & ClearSubscript() { return Clear("subscript"); }
    Text & ClearSuperscript() { return Clear("superscript"); }
    Text & ClearUnderline() { return Clear("underline"); }
    
    // Simple formatting: clear a single character from a specified format.
    Text & Clear(const std::string & style, size_t pos) {
      auto it = attr_map.find(style);
      if (it != attr_map.end() && it->second.size() > pos) {  // If style bit exists...
        it->second.Clear(pos);
      }
      return *this;
    }
    Text & ClearBold(size_t pos) { return Clear("bold", pos); }
    Text & ClearCode(size_t pos) { return Clear("code", pos); }
    Text & ClearItalic(size_t pos) { return Clear("italic", pos); }
    Text & ClearStrike(size_t pos) { return Clear("strike", pos); }
    Text & ClearSubscript(size_t pos) { return Clear("subscript", pos); }
    Text & ClearSuperscript(size_t pos) { return Clear("superscript", pos); }
    Text & ClearUnderline(size_t pos) { return Clear("underline", pos); }

    // Simple formatting: clear a range of characters from a specified format.
    Text & Clear(const std::string & style, size_t start, size_t end) {
      auto it = attr_map.find(style);
      if (it != attr_map.end() && it->second.size() > start) {  // If style bits exist...
        if (end > it->second.size()) end = it->second.size();   // ...don't pass text end
        it->second.Clear(start, end);
      }
      return *this;
    }
    Text & ClearBold(size_t start, size_t end) { return Clear("bold", start, end); }
    Text & ClearCode(size_t start, size_t end) { return Clear("code", start, end); }
    Text & ClearItalic(size_t start, size_t end) { return Clear("italic", start, end); }
    Text & ClearStrike(size_t start, size_t end) { return Clear("strike", start, end); }
    Text & ClearSubscript(size_t start, size_t end) { return Clear("subscript", start, end); }
    Text & ClearSuperscript(size_t start, size_t end) { return Clear("superscript", start, end); }
    Text & ClearUnderline(size_t start, size_t end) { return Clear("underline", start, end); }

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