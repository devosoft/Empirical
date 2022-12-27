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
 * 
 *  The main Text class tracks a string of text (called simply "text") and any special attributes
 *  associated with each text position (in "attr_map").  The current TextEncoding class helps
 *  guide the conversion from one text encoding to another.
 *
 *  Internally, attributes that encodings should be able to handle are:
 *    BASIC FORMATS :
 *     "bold" (alias: "b" or "strong")
 *     "code" (alias: "tt" or "teletype")
 *     "italic" (alias: "i", "em", or "emphasis")
 *     "strike" (alias "strikethrough")
 *     "subscript" (alias "sub")
 *     "superscript" (alias "sup")
 *     "underline"
 *     "color:NAME"
 *     "font:NAME"
 *     "size:NAME" (in point size)
 * 
 *    STRUCTURAL :
 *     "heading:1" through "heading:6" different levels of headings.
 *     "blockquote"
 *     "bullet:1" through "bullet:6" (only on character after bulleted; stops an newline)
 *     "indent:1" through "indent:6" (same as bullet, but no bullet)
 *     "ordered:1" through "ordered:6" (numbers; reset less-tabbed structure is used)
 *     "link:URL" (create a link to the specified URL)
 *     "image:URL" (include in the specified image)
 */

#ifndef EMP_TOOLS_TEXT_HPP_INCLUDE
#define EMP_TOOLS_TEXT_HPP_INCLUDE

#include <map>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "../base/assert.hpp"
#include "../bits/BitVector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../tools/string_utils.hpp"

namespace emp {

  class Text;

  // An individual proxy character from Text that is format aware.
  template <bool IS_CONST=false>
  class TextCharRef {
  private:
    using text_t = typename std::conditional<IS_CONST, const Text, Text>::type;
    text_t & text_ref;
    size_t pos;
  public:
    TextCharRef(text_t & _ref, size_t _pos) : text_ref(_ref), pos(_pos) { }
    TextCharRef(const TextCharRef<false> & in) : text_ref(in.text_ref), pos(in.pos) { }
    TextCharRef(const TextCharRef<true> & in) : text_ref(in.text_ref), pos(in.pos) { }
    ~TextCharRef() = default;

    // Set this character equal (with same inputs) as in parameter; don't change reference.
    TextCharRef & operator=(const TextCharRef<false> & in);
    TextCharRef & operator=(const TextCharRef<true> & in);

    // Set just this character; don't change style.
    TextCharRef & operator=(char in);

    // Convert to a normal C++ char.
    char AsChar() const;
    operator char() const { return AsChar(); }

    // Comparison operators
    auto operator<=>(const TextCharRef & in) const;
    auto operator<=>(char in) const;

    text_t & GetText() const { return text_ref; }
    size_t GetPos() const { return pos; }
    emp::vector<std::string> GetStyles() const;

    bool HasStyle(const std::string & style) const;
    bool IsBold()        { return HasStyle("bold"); }
    bool IsCode()        { return HasStyle("code"); }
    bool IsItalic()      { return HasStyle("italic"); }
    bool IsStrike()      { return HasStyle("strike"); }
    bool IsSubscript()   { return HasStyle("subscript"); }
    bool IsSuperscript() { return HasStyle("superscript"); }
    bool IsUnderline()   { return HasStyle("underline"); }

    TextCharRef & SetStyle(const std::string & style);
    TextCharRef & Bold()        { return SetStyle("bold"); }
    TextCharRef & Code()        { return SetStyle("code"); }
    TextCharRef & Italic()      { return SetStyle("italic"); }
    TextCharRef & Strike()      { return SetStyle("strike"); }
    TextCharRef & Subscript()   { return SetStyle("subscript"); }
    TextCharRef & Superscript() { return SetStyle("superscript"); }
    TextCharRef & Underline()   { return SetStyle("underline"); }
  };

  // A base class for any special encodings that should work with Text objects.
  class TextEncoding {
  protected:
    Text & text;      // The emp::Text this encoding is associated with.
    std::string name; // The name by which this encoding should be called.
  public:
    TextEncoding(Text & _text, const std::string _name) : text(_text), name(_name) { }
    virtual ~TextEncoding() { }

    const std::string & GetName() const { return name; }

    // By default, append text assuming that there is no special formatting in it.
    virtual void Append(std::string in);

    // By default, return text and ignore all formatting.
    virtual std::string ToString() const;

    // Make a copy of this TextEncoding, including proper derived class.
    virtual emp::Ptr<TextEncoding> Clone(Text & _text) const {
      return emp::NewPtr<TextEncoding>(_text, name);
    }
  };

  class Text {
  protected:
    // Current state of the text, minus all formatting.
    std::string text = "";

    // Attributes are basic formatting for strings, including "bold", "italic", "underline",
    // "strike", "superscript", "subscript", and "code".  Fonts are described as font name,
    // a colon, and the font size.  E.g.: "TimesNewRoman:12"
    std::unordered_map<std::string, BitVector> attr_map;

    // A set of encodings that this Text object can handle.
    std::map< std::string, emp::Ptr<TextEncoding> > encodings;
    emp::Ptr<TextEncoding> encoding_ptr = nullptr;

    // Internal function to remove unused styles.
    void Cleanup() {
      // Scan for styles that are no longer unused.
      emp::vector<std::string> unused_styles;
      for (auto & [tag, bits] : attr_map) {
        if (bits.None()) unused_styles.push_back(tag);
      }
      // If any styles need to be deleted, do so.
      for (const std::string & style : unused_styles) {
        attr_map.erase(style);
      }
    }

  public:
    Text() {
      encodings["txt"] = NewPtr<TextEncoding>(*this, "txt");
      encoding_ptr = encodings["txt"];
    };
    Text(const Text & in) {
      for (const auto & [e_name, ptr] : encodings) {
        encodings[e_name] = ptr->Clone(*this);
      }
      encoding_ptr = encodings[in.encoding_ptr->GetName()];
    }
    Text(const std::string & in) {      
      encodings["txt"] = NewPtr<TextEncoding>(*this, "txt");
      encoding_ptr = encodings["txt"];
      Append(in);
    }
    ~Text() {
      if (encoding_ptr != nullptr) {
        for (auto & [e_name, ptr] : encodings) {
          ptr.Delete();
        }
      }
    }

    Text & operator=(const Text & in) {
      Text new_text(in);
      std::swap(*this, new_text);
      return *this;
    }

    Text & operator=(const std::string & in) {
      attr_map.clear(); // Clear out existing content.
      text.resize(0);
      Append(in);
      return *this;
    };

    // GetSize() returns the number of characters IGNORING all formatting.
    size_t GetSize() const { return text.size(); }

    // Return the current text as an unformatted string.
    const std::string & GetText() const { return text; }

    /// Automatic conversion back to an unformatted string
    operator const std::string &() const { return GetText(); }

    /// Append potentially-formatted text through the current encoding.
    Text & Append(const std::string & in) {
      encoding_ptr->Append(in);
      return *this;
    }

    // Append raw text; assume no formatting.
    Text & Append_Raw(const std::string & in) {
      text += in;
      return *this;
    }

    // Stream operator.
    template <typename T>
    Text & operator<<(T && in) {
      Append(emp::to_string(in));
      return *this;
    }

    void Resize(size_t new_size) {
      text.resize(new_size);
      for (auto & [tag, bits] : attr_map) {
        if (bits.GetSize() > new_size) {
          bits.Resize(new_size);
        }
      }
      Cleanup(); // Remove any styles that are no longer used.
    }

    // Direct Get accessors
    char GetChar(size_t pos) const {
      emp_assert(pos < text.size());
      return text[pos];
    }

    // Direct Set accessors
    Text & Set(size_t pos, char in) {
      text[pos] = in;
      return *this;
    }

    template <bool IS_CONST>
    Text & Set(size_t pos, TextCharRef<IS_CONST> in) {
      text[pos] = in.AsChar();

      // Match style of in.
      emp::vector<std::string> styles = in.GetStyles();
      Clear(pos);  // Clear old style.
      for (const std::string & style : styles) {
        SetStyle(style, pos);
      }
      return *this;
    }    

    TextCharRef<false> operator[](size_t pos) {
      emp_assert(pos < GetSize(), pos, GetSize());
      return TextCharRef<false>(*this, pos);
    }

    TextCharRef<true> operator[](size_t pos) const {
      emp_assert(pos < GetSize(), pos, GetSize());
      return TextCharRef<true>(*this, pos);
    }

    // STL-like functions for perfect compatability with string.
    size_t size() const { return text.size(); }
    void resize(size_t new_size) { Resize(new_size); }

    template <typename... Ts>
    Text & assign(Ts &&... in) { text.assign( std::forward<Ts>(in)... ); }
    char & front() { return text.front(); }
    char front() const { return text.front(); }
    char & back() { return text.back(); }
    char back() const { return text.back(); }
    bool empty() const { return text.empty(); }
    // void push_back(char c) { text.push_back(c); }
    // void pop_back() { text.pop_back(); }
    template <typename... Ts>
    bool starts_with(Ts &&... in) const { text.starts_with(std::forward<Ts>(in)... ); }
    template <typename... Ts>
    bool ends_with(Ts &&... in) const { text.ends_with(std::forward<Ts>(in)... ); }

    template <typename... Ts>
    size_t find(Ts &&... in) const { return text.find(std::forward<Ts>(in)...); }
    template <typename... Ts>
    size_t rfind(Ts &&... in) const { return text.rfind(std::forward<Ts>(in)...); }
    template <typename... Ts>
    size_t find_first_of(Ts &&... in) const { return text.find_first_of(std::forward<Ts>(in)...); }
    template <typename... Ts>
    size_t find_first_not_of(Ts &&... in) const { return text.find_first_not_of(std::forward<Ts>(in)...); }
    template <typename... Ts>
    size_t find_last_of(Ts &&... in) const { return text.find_last_of(std::forward<Ts>(in)...); }
    template <typename... Ts>
    size_t find_last_not_of(Ts &&... in) const { return text.find_last_not_of(std::forward<Ts>(in)...); }

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

    /// Return the set of active styles in this text.
    /// @param pos optional position to specify only styles used at position.
    emp::vector<std::string> GetStyles(size_t pos=MAX_SIZE_T) {
      emp::vector<std::string> styles;
      emp::vector<std::string> to_clear;
      for (const auto & [name, bits] : attr_map) {
        if (bits.None()) to_clear.push_back(name);
        else if (pos == MAX_SIZE_T || bits.Has(pos)) {
          styles.push_back(name);
        }
      }
      for (const auto & name : to_clear) Clear(name);

      return styles;
    }

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

    // Clear ALL formatting at a specified position.
    Text & Clear(size_t pos) {
      for (auto & [style,bits] : attr_map) {
        if (bits.Has(pos)) bits.Clear(pos);
      }
      return *this;
    }

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

  };

  // Set this character equal (with same inputs) as in parameter; don't change reference.
  template <bool IS_CONST>
  TextCharRef<IS_CONST> & TextCharRef<IS_CONST>::operator=(const TextCharRef<false> & in) {
    text_ref.Set(pos, in);
    return *this;
  }

  template <bool IS_CONST>
  TextCharRef<IS_CONST> & TextCharRef<IS_CONST>::operator=(const TextCharRef<true> & in) {
    static_assert(IS_CONST == false,
      "Cannot assign a const TextCharRef to a mutatble version.");
    text_ref.Set(pos, in);
    return *this;
  }

  // Set just this character; don't change style.
  template <bool IS_CONST>
  TextCharRef<IS_CONST> & TextCharRef<IS_CONST>::operator=(char in) {
    text_ref.Set(pos, in);
    return *this;
  }

  // Convert to a normal C++ char.
  template <bool IS_CONST>
  char TextCharRef<IS_CONST>::AsChar() const {
    return text_ref.GetChar(pos);
  }

  template <bool IS_CONST>
  auto TextCharRef<IS_CONST>::operator<=>(const TextCharRef & in) const {
    return text_ref.GetChar(pos) <=> in.text_ref.GetChar(in.pos);
  }
  template <bool IS_CONST>
  auto TextCharRef<IS_CONST>::operator<=>(char in) const {
    return text_ref.GetChar(pos) <=> in;
  }

  template <bool IS_CONST>
  emp::vector<std::string> TextCharRef<IS_CONST>::GetStyles() const {
    return text_ref.GetStyles(pos);
  }

  template <bool IS_CONST>
  bool TextCharRef<IS_CONST>::HasStyle(const std::string & style) const {
    return text_ref.HasStyle(style, pos);
  }

  template <bool IS_CONST>
  TextCharRef<IS_CONST> & TextCharRef<IS_CONST>::SetStyle(const std::string & style) {
    text_ref.HasStyle(style, pos);
    return *this;
  }


  // ------- TextEncoding --------

    // By default, append text assuming that there is no special formatting in it.
  void TextEncoding::Append(std::string in) {
    text.Append_Raw(in);
  }

  // By default, return text and ignore all formatting.
  std::string TextEncoding::ToString() const {
    return text;
  }


}


#endif