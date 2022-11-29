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

#include <string>
#include <unordered_map>

#include "../base/assert.hpp"
#include "../bits/BitVector.hpp"

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

    // Simple formatting.
    FormattedText & Bold() { return SetBits( attr_map["bold"] ); }

    // Compatibility with std::string
    size_t size() const { return GetSize(); }


  private:
    // ----   Helper functions   ----

    FormattedText & SetBits(BitVector & cur_bits) {
      cur_bits.Resize(text.size());
      cur_bits.SetAll();
      return *this;
    }
  };

}


#endif