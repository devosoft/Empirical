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
    const std::string & GetString() { return text; }

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

    // Simple formatting: set all characters to a specified format.
    FormattedText & Bold() { return SetBits("bold"); }
    FormattedText & Code() { return SetBits("code"); }
    FormattedText & Italic() { return SetBits("italic"); }
    FormattedText & Strike() { return SetBits("strike"); }
    FormattedText & Subscript() { return SetBits("subscript"); }
    FormattedText & Superscript() { return SetBits("superscript"); }
    FormattedText & Underline() { return SetBits("underline"); }

    // Simple formatting: set a single character to a specified format.
    FormattedText & Bold(size_t pos) { return SetBit("bold", pos); }
    FormattedText & Code(size_t pos) { return SetBit("code", pos); }
    FormattedText & Italic(size_t pos) { return SetBit("italic", pos); }
    FormattedText & Strike(size_t pos) { return SetBit("strike", pos); }
    FormattedText & Subscript(size_t pos) { return SetBit("subscript", pos); }
    FormattedText & Superscript(size_t pos) { return SetBit("superscript", pos); }
    FormattedText & Underline(size_t pos) { return SetBit("underline", pos); }

    // Simple formatting: set a range of characters to a specified format.
    FormattedText & Bold(size_t start, size_t end) { return SetBits("bold", start, end ); }
    FormattedText & Code(size_t start, size_t end) { return SetBits("code", start, end); }
    FormattedText & Italic(size_t start, size_t end) { return SetBits("italic", start, end); }
    FormattedText & Strike(size_t start, size_t end) { return SetBits("strike", start, end); }
    FormattedText & Subscript(size_t start, size_t end) { return SetBits("subscript", start, end); }
    FormattedText & Superscript(size_t start, size_t end) { return SetBits("superscript", start, end); }
    FormattedText & Underline(size_t start, size_t end) { return SetBits("underline", start, end); }

    // Test if a particular attribute is present.
    bool HasBold() const { return HasAttr("bold"); }
    bool HasCode() const { return HasAttr("code"); }
    bool HasItalic() const { return HasAttr("italic"); }
    bool HasStrike() const { return HasAttr("strike"); }
    bool HasSubscript() const { return HasAttr("subscript"); }
    bool HasSuperscript() const { return HasAttr("superscript"); }
    bool HasUnderline() const { return HasAttr("underline"); }

    // Clear ALL formatting
    FormattedText & Clear() { attr_map.clear(); return *this; }
    FormattedText & ClearBold() { attr_map.erase("bold"); return *this; }
    FormattedText & ClearCode() { attr_map.erase("code"); return *this; }
    FormattedText & ClearItalic() { attr_map.erase("italic"); return *this; }
    FormattedText & ClearStrike() { attr_map.erase("strike"); return *this; }
    FormattedText & ClearSubscript() { attr_map.erase("subscript"); return *this; }
    FormattedText & ClearSuperscript() { attr_map.erase("superscript"); return *this; }
    FormattedText & ClearUnderline() { attr_map.erase("underline"); return *this; }
    
    // Simple formatting: clear a single character from a specified format.
    FormattedText & ClearBold(size_t pos) { return SetBit("bold", pos, false); }
    FormattedText & ClearCode(size_t pos) { return SetBit("code", pos, false); }
    FormattedText & ClearItalic(size_t pos) { return SetBit("italic", pos, false); }
    FormattedText & ClearStrike(size_t pos) { return SetBit("strike", pos, false); }
    FormattedText & ClearSubscript(size_t pos) { return SetBit("subscript", pos, false); }
    FormattedText & ClearSuperscript(size_t pos) { return SetBit("superscript", pos, false); }
    FormattedText & ClearUnderline(size_t pos) { return SetBit("underline", pos, false); }

    // Simple formatting: clear a range of characters from a specified format.
    FormattedText & ClearBold(size_t start, size_t end) { return SetBits("bold", start, end, false); }
    FormattedText & ClearCode(size_t start, size_t end) { return SetBits("code", start, end, false); }
    FormattedText & ClearItalic(size_t start, size_t end) { return SetBits("italic", start, end, false); }
    FormattedText & ClearStrike(size_t start, size_t end) { return SetBits("strike", start, end, false); }
    FormattedText & ClearSubscript(size_t start, size_t end) { return SetBits("subscript", start, end, false); }
    FormattedText & ClearSuperscript(size_t start, size_t end) { return SetBits("superscript", start, end, false); }
    FormattedText & ClearUnderline(size_t start, size_t end) { return SetBits("underline", start, end, false); }

    // Compatibility with std::string
    size_t size() const { return GetSize(); }

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

      return out_string;
    }

  private:
    // ----   Helper functions   ----

    FormattedText & SetBits(const std::string & attr) {
      BitVector & cur_bits = attr_map[attr];
      cur_bits.Resize(text.size());
      cur_bits.SetAll();
      return *this;
    }

    FormattedText & SetBit(const std::string & attr, size_t pos, bool value=true) {
      BitVector & cur_bits = attr_map[attr];
      if (cur_bits.size() <= pos) cur_bits.Resize(pos+1);
      cur_bits.Set(pos, value);
      return *this;
    }

    FormattedText & SetBits(const std::string & attr, size_t start, size_t end, bool value=true) {
      BitVector & cur_bits = attr_map[attr];
      emp_assert(end <= text.size());
      emp_assert(start <= end);
      if (cur_bits.size() <= end) cur_bits.Resize(end+1);
      cur_bits.SetRange(start, end, value);
      return *this;
    }

    bool HasAttr(const std::string & attr) const {
      if (!emp::Has(attr_map, attr)) return false;
      return GetConstRef(attr_map, attr).Any();
    }

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