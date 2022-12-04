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
    // "strikethrough", "superscript", and "subscript".
    std::unordered_map<std::string, BitVector> attr_map;
    // Fonts are described as font name, a colon, and the font size.  E.g.: "TimesNewRoman:12"
    std::unordered_map<std::string, BitVector> font_map;

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
      attr_map.clear(); font_map.clear(); // Clear out existing content.
      text=in;
      return *this;
    };
    FormattedText & operator=(std::string && in) {
      attr_map.clear(); font_map.clear(); // Clear out existing content.
      text = std::move(in);
      return *this;
    }

    // GetSize() returns the number of characters IGNORING all formatting.
    size_t GetSize() const { return text.size(); }

    char & operator[](size_t pos) {
      emp_assert(pos < GetSize(), pos, GetSize());
      return text[pos];
    }

    char operator[](size_t pos) const {
      emp_assert(pos < GetSize(), pos, GetSize());
      return text[pos];
    }

    // Simple formatting: set all characters to a specified format.
    FormattedText & Bold() { return SetBits( attr_map["bold"] ); }
    FormattedText & Italic() { return SetBits( attr_map["italic"] ); }
    FormattedText & Strikethrough() { return SetBits( attr_map["strikethrough"] ); }
    FormattedText & Subscript() { return SetBits( attr_map["subscript"] ); }
    FormattedText & Superscript() { return SetBits( attr_map["superscript"] ); }
    FormattedText & Underline() { return SetBits( attr_map["underline"] ); }

    // Simple formatting: set a single character to a specified format.
    FormattedText & Bold(size_t pos) { return SetBit( attr_map["bold"], pos ); }
    FormattedText & Italic(size_t pos) { return SetBit( attr_map["italic"], pos ); }
    FormattedText & Strikethrough(size_t pos) { return SetBit( attr_map["strikethrough"], pos ); }
    FormattedText & Subscript(size_t pos) { return SetBit( attr_map["subscript"], pos ); }
    FormattedText & Superscript(size_t pos) { return SetBit( attr_map["superscript"], pos ); }
    FormattedText & Underline(size_t pos) { return SetBit( attr_map["underline"], pos ); }

    // Simple formatting: set a range of characters to a specified format.
    FormattedText & Bold(size_t start, size_t end) {
      return SetBits( attr_map["bold"], start, end );
    }
    FormattedText & Italic(size_t start, size_t end) {
      return SetBits( attr_map["italic"], start, end );
    }
    FormattedText & Strikethrough(size_t start, size_t end) {
      return SetBits( attr_map["strikethrough"], start, end );
    }
    FormattedText & Subscript(size_t start, size_t end) {
      return SetBits( attr_map["subscript"], start, end );
    }
    FormattedText & Superscript(size_t start, size_t end) {
      return SetBits( attr_map["superscript"], start, end );
    }
    FormattedText & Underline(size_t start, size_t end) {
      return SetBits( attr_map["underline"], start, end );
    }

    // Test if a particular attribute is present.
    bool HasBold() const { return HasAttr("bold"); }
    bool HasItalic() const { return HasAttr("italic"); }
    bool HasStrikethrough() const { return HasAttr("strikethrough"); }
    bool HasSubscript() const { return HasAttr("subscript"); }
    bool HasSuperscript() const { return HasAttr("superscript"); }
    bool HasUnderline() const { return HasAttr("underline"); }

    // Compatibility with std::string
    size_t size() const { return GetSize(); }

    // Convert this to a string in HTML format.
    std::string AsHTML() {
      std::map<size_t, std::string> tag_map; // Where do tags go?
      if (HasBold()) AddOutputTags(tag_map, "bold", "<b>", "</b>");
      if (HasItalic()) AddOutputTags(tag_map, "italic", "<i>", "</i>");
      if (HasStrikethrough()) AddOutputTags(tag_map, "strikethrough", "<del>", "</del>");
      if (HasSubscript()) AddOutputTags(tag_map, "subscript", "<sub>", "</sub>");
      if (HasSuperscript()) AddOutputTags(tag_map, "superscript", "<sup>", "</sup>");
      if (HasUnderline()) AddOutputTags(tag_map, "underline", "<u>", "</u>");
    }
    
  private:
    // ----   Helper functions   ----

    FormattedText & SetBits(BitVector & cur_bits) {
      cur_bits.Resize(text.size());
      cur_bits.SetAll();
      return *this;
    }

    FormattedText & SetBit(BitVector & cur_bits, size_t pos) {
      if (cur_bits.size() <= pos) cur_bits.Resize(pos+1);
      cur_bits.Set(pos);
      return *this;
    }

    FormattedText & SetBits(BitVector & cur_bits, size_t start, size_t end) {
      emp_assert(end <= text.size());
      emp_assert(start <= end);
      if (cur_bits.size() <= end) cur_bits.Resize(end+1);
      cur_bits.SetRange(start, end);
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