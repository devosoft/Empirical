/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-23.
 *
 *  @file Text.hpp
 *  @brief Functionality similar to emp::String, but tracks text formatting for easy conversion.
 *  @note Status: ALPHA
 * 
 *  Text should be functionally interchangable with string, but can easily convert to
 *  HTML, Latex, RTF, or other formats that support bold, italic, super/sub-scripting, fonts,
 *  etc.
 * 
 *  The main Text class tracks a string of text (called simply "text") and any special styles
 *  associated with each text position (in "style_map").  The current TextEncoding class helps
 *  guide the conversion from one text encoding to another.
 *
 *  Internally, styles that all encodings should be able to handle (or at least be aware of) are:
 *    BASIC FORMATS :
 *     "bold"
 *     "code"
 *     "italic"
 *     "strike"
 *     "subscript"
 *     "superscript"
 *     "underline"
 *     "no_break"
 *     "color:NAME"
 *     "font:NAME"
 *     "size:POINT_SIZE"
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
#include <type_traits>
#include <unordered_map>

#include "../base/assert.hpp"
#include "../base/notify.hpp"
#include "../bits/BitVector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../tools/String.hpp"

#include "TextCharRef.hpp"

namespace emp {

  class Text;

  // A base class for any special encodings that should work with Text objects.
  class TextEncoding_Interface {
  protected:
    Text & text;  // The emp::Text this encoding is associated with.
    using this_t = TextEncoding_Interface;
  public:
    TextEncoding_Interface(Text & _text) : text(_text) { }
    virtual ~TextEncoding_Interface() { }

    virtual void Append(const String & in) = 0;             // Add new text.
    virtual String Encode() const = 0;                      // Produce text formatted in encoding.
    virtual emp::Ptr<this_t> Clone(Text & _text) const = 0; // Copy encoding.
    virtual void PrintDebug(std::ostream &) const = 0;
  };

  class TextEncoding_None : public TextEncoding_Interface {
  public:
    TextEncoding_None(Text & _text) : TextEncoding_Interface(_text) { }
    void Append(const String & in) override;
    String Encode() const override;
    emp::Ptr<TextEncoding_Interface> Clone(Text & _text) const override;
    void PrintDebug(std::ostream & os) const override;
  };

  class Text {
  protected:
    // Current state of the text, minus all formatting.
    String text = "";

    // Styles are basic formatting for strings, including "bold", "italic", "underline",
    // "strike", "superscript", "subscript", and "code".  Fonts are described as font name,
    // a colon, and the font size.  E.g.: "TimesNewRoman:12"
    std::unordered_map<String, BitVector> style_map;

    // A set of encodings that this Text object can handle.
    std::map< String, emp::Ptr<TextEncoding_Interface> > encodings;
    emp::Ptr<TextEncoding_Interface> encoding_ptr = nullptr;

    // Internal function to remove unused styles.
    void Cleanup() {
      // Scan for styles that are no longer unused.
      emp::vector<String> unused_styles;
      for (auto & [tag, bits] : style_map) {
        if (bits.None()) unused_styles.push_back(tag);
      }
      // If any styles need to be deleted, do so.
      for (const String & style : unused_styles) {
        style_map.erase(style);
      }
    }

  public:
    Text() {
      encodings["txt"] = NewPtr<TextEncoding_None>(*this);
      encoding_ptr = encodings["txt"];
    };
    Text(const Text & in) : text(in.text), style_map(in.style_map) {
      for (const auto & [e_name, ptr] : encodings) {
        encodings[e_name] = ptr->Clone(*this);
        if (in.encoding_ptr == ptr) encoding_ptr = encodings[e_name];
      }
    }
    Text(const String & in) {      
      encodings["txt"] = NewPtr<TextEncoding_None>(*this);
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

    Text & operator=(Text && in) {
      std::swap(text, in.text);
      std::swap(style_map, in.style_map);
      std::swap(encodings, in.encodings);
      std::swap(encoding_ptr, in.encoding_ptr);
      return *this;
    }

    template <typename T>
    Text & operator=(const T & in) {
      style_map.clear(); // Clear out existing content.
      text.clear();
      Append(in);
      return *this;
    };


    // GetSize() returns the number of characters IGNORING all formatting.
    size_t GetSize() const { return text.size(); }

    // Return the current text as an unformatted string.
    const String & GetText() const { return text; }

    // Return the current bit pattern for a specified style.
    const BitVector & GetStyle(const String & style) const {
      return emp::GetConstRef(style_map, style);
    }

    /// Automatic conversion back to an unformatted string
    operator const String &() const { return GetText(); }

    /// @brief Test if this Text object is aware of how to use a specified encoding.
    /// @param name Name of the encoding to test for.
    /// @return A true/false indicate if the named encoding is known.
    bool HasEncoding(const String & name) const {
      return emp::Has(encodings, name);
    }

    /// @brief Get the name of the current encoding being applied.
    /// @return Name of the current encoding.
    String GetEncoding() const {
      for (const auto & [e_name, ptr] : encodings) {
        if (encoding_ptr == ptr) return e_name;
      }
      return "Unknown";
    }

    /// @brief Change the current encoding being used to another known encoding type.
    /// @param name Name of the encoding type to be used.
    /// @return A reference to this object itself.
    Text & SetEncoding(const String & name) {
      if (!emp::Has(encodings, name)) {
        notify::Error("Trying to set unknown encoding '", name, "'; No change made.");
      } else {
        encoding_ptr = encodings[name];
      }
      return *this;
    }

    /// Add a new encoding to this Text object.  Newly added encodings automatically become
    /// active (use SetEncoding() to choose a different encoding option).  Will give a warning
    /// if encoding already exists, and then replace it.

    /// @brief Add an encoding to this Text object; new encodings automatically become active.
    /// @tparam ENCODING_T The type of the new encoding to use
    /// @tparam ...EXTRA_Ts Automatically set by variadic arguments.
    /// @param name Name to be used for this new encoding.
    /// @param ...args Any extra arguments to configure this new encoding (passed to constructor)
    template <typename ENCODING_T, typename... EXTRA_Ts>
    void AddEncoding(const String & name, EXTRA_Ts &&... args) {
      emp_assert(!HasEncoding(name), name, "Trying to add a TextEncoding that already exists. To replace, use RemoveEncoding() first.");
      encoding_ptr = NewPtr<ENCODING_T>(*this, std::forward<EXTRA_Ts>(args)...);
      encodings[name] = encoding_ptr;
    }

    void RemoveEncoding(const String & name) {
      emp_assert(HasEncoding(name), name, "Trying to remove a TextEncoding that does not exist.");
      if (HasEncoding(name)) {
        if (encoding_ptr == encodings[name]) encoding_ptr = nullptr;
        encodings[name].Delete();
        encodings.erase(name);
      }
    }

    /// ActivateEncoding will add an encoding if (and only if) it doesn't exist already.
    template <typename ENCODING_T, typename... EXTRA_Ts>
    void ActivateEncoding(const String & name, EXTRA_Ts &&... args) {
      if (!HasEncoding(name)) {
        AddEncoding<ENCODING_T>(name, std::forward<EXTRA_Ts>(args)...);
      } else {
        SetEncoding(name);
      }
    }


    /// Append potentially-formatted text through the current encoding.
    Text & Append(const String & in) {
      encoding_ptr->Append(in);
      return *this;
    }

    /// Specify the encoding of a value being appended.
    template <typename ENCODING_T, typename IN_T>
    Text & AppendAs(const String & encode_name, IN_T && in) {
      ActivateEncoding<ENCODING_T>(encode_name);
      Append(std::forward<IN_T>(in));
      return *this;
    }

    // Append raw text; assume no formatting.
    template <typename T>
    Text & Append_Raw(T && in) {
      text += std::forward<T>(in);
      return *this;
    }

    // Stream operator.
    template <typename T>
    Text & operator<<(T && in) {
      return Append(emp::MakeString(in));
    }

    template <typename T>
    Text & operator+=(T && in) {
      return Append(emp::MakeString(in));
    }

    /// @brief Convert text to a string using the current encoding.
    /// @return The resulting string.
    String Encode() const { return encoding_ptr->Encode(); }

    void Resize(size_t new_size) {
      text.resize(new_size);
      for (auto & [tag, bits] : style_map) {
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
      emp::vector<String> styles = in.GetStyles();
      Clear(pos);  // Clear old style.
      for (const String & style : styles) {
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
    TextCharRef<false> front()      { return operator[](0); }
    TextCharRef<true> front() const { return operator[](0); }
    TextCharRef<false> back()       { return operator[](text.size()-1); }
    TextCharRef<true> back() const  { return operator[](text.size()-1); }

    bool empty() const { return text.empty(); }
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
    Text & SetStyle(String style) {
      BitVector & cur_bits = style_map[style];
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
    Text & SetStyle(String style, size_t pos) {
      BitVector & cur_bits = style_map[style];
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
    Text & SetStyle(String style, size_t start, size_t end) {
      BitVector & cur_bits = style_map[style];
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
    emp::vector<String> GetStyles(size_t pos=MAX_SIZE_T) {
      emp::vector<String> styles;
      emp::vector<String> to_clear;
      for (const auto & [name, bits] : style_map) {
        if (bits.None()) to_clear.push_back(name);
        else if (pos == MAX_SIZE_T || bits.Has(pos)) {
          styles.push_back(name);
        }
      }
      for (const auto & name : to_clear) Clear(name);

      return styles;
    }

    // Test if a particular style is present anywhere in the text
    bool HasStyle(const String & style) const {
      if (!emp::Has(style_map, style)) return false;
      return GetConstRef(style_map, style).Any();
    }
    bool HasBold() const { return HasStyle("bold"); }
    bool HasCode() const { return HasStyle("code"); }
    bool HasItalic() const { return HasStyle("italic"); }
    bool HasStrike() const { return HasStyle("strike"); }
    bool HasSubscript() const { return HasStyle("subscript"); }
    bool HasSuperscript() const { return HasStyle("superscript"); }
    bool HasUnderline() const { return HasStyle("underline"); }

    // Test if a particular style is present at a given position.
    bool HasStyle(const String & style, size_t pos) const {
      auto it = style_map.find(style);
      if (it == style_map.end()) return false; // Style is nowhere.
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
    Text & Clear() { style_map.clear(); return *this; }

    // Clear ALL formatting at a specified position.
    Text & Clear(size_t pos) {
      for (auto & [style,bits] : style_map) {
        if (bits.Has(pos)) bits.Clear(pos);
      }
      return *this;
    }

    // Clear specific formatting across all text
    Text & Clear(const String & style) {
      style_map.erase(style);
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
    Text & Clear(const String & style, size_t pos) {
      auto it = style_map.find(style);
      if (it != style_map.end() && it->second.size() > pos) {  // If style bit exists...
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
    Text & Clear(const String & style, size_t start, size_t end) {
      auto it = style_map.find(style);
      if (it != style_map.end() && it->second.size() > start) {  // If style bits exist...
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

    String ToDebugString() {
      String out;
      out += MakeString("Text: ", text, "\n");
      for (auto [name, bits] : style_map) {
        out += MakeString("      ", bits, " : ", name, "\n");
      }
      return out;
    }

    void PrintDebug(std::ostream & os = std::cout) {
      os << ToDebugString();
      encoding_ptr->PrintDebug(os);
    }
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
  emp::vector<String> TextCharRef<IS_CONST>::GetStyles() const {
    return text_ref.GetStyles(pos);
  }

  template <bool IS_CONST>
  bool TextCharRef<IS_CONST>::HasStyle(const String & style) const {
    return text_ref.HasStyle(style, pos);
  }

  template <bool IS_CONST>
  TextCharRef<IS_CONST> & TextCharRef<IS_CONST>::SetStyle(const String & style) {
    text_ref.HasStyle(style, pos);
    return *this;
  }

  void TextEncoding_None::Append(const String & in) { text.Append_Raw(in); }
  String TextEncoding_None::Encode() const { return text; };
  emp::Ptr<TextEncoding_Interface> TextEncoding_None::Clone(Text & _text) const {
    return emp::NewPtr<TextEncoding_None>(_text);
  }
  void TextEncoding_None::PrintDebug(std::ostream & os) const {
    os << "TextEncoding None.";
  };
}


#endif