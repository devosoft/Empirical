/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2022
*/
/**
 *  @file
 *  @brief Simple functions and tools to manipulate individual characters.
 *  @note Status: BETA
 *
 *  Available Class
 *    CharSet - A collection of characters for use in other functions.
 *
 *  Available Functions
 *    bool is_whitespace(char test_char)
 *    bool is_upper_letter(char test_char)
 *    bool is_lower_letter(char test_char)
 *    bool is_letter(char test_char)
 *    bool is_digit(char test_char)
 *    bool is_alphanumeric(char test_char)
 *    bool is_idchar(char test_char)
 *    bool is_one_of(char test_char, const std::string & char_set)
 *    bool is_valid(char test_char )
 *    bool is_valid(char test_char, std::function<bool(char)> fun1, FUNS... funs)
 *
 */

#ifndef EMP_TOOLS_CHAR_UTILS_HPP_INCLUDE
#define EMP_TOOLS_CHAR_UTILS_HPP_INCLUDE


#include "../base/array.hpp"
#include "../base/assert.hpp"
#include "../base/Ptr.hpp"

namespace emp {

  /// A char_set is a fast true/false lookup table to identify which ASCII chars are in a set.
  template <typename CHAR_T=char, size_t MAX_CHAR=128>
  class CharSetBase {
  private:
    using this_t = CharSetBase<CHAR_T,MAX_CHAR>;
    using char_set_t = emp::array<CHAR_T, MAX_CHAR>;
    char_set_t char_set{};

  public:
    CharSetBase() = default;

    CharSetBase(CHAR_T c) { char_set[static_cast<size_t>(c)] = true; }
    CharSetBase(const std::string & in_chars) {
      for (CHAR_T x : in_chars) char_set[static_cast<size_t>(x)] = true;
    }
    CharSetBase(const char * in_chars) : CharSetBase(std::string(in_chars)) { }
    CharSetBase(const this_t &) = default;

    this_t & operator=(const this_t &) = default;
    this_t & operator=(char c) { Reset(); char_set[static_cast<size_t>(c)] = true; }
    this_t & operator=(const std::string & in_chars) {
      for (CHAR_T x : in_chars) char_set[static_cast<size_t>(x)] = true;
    }

    CharSetBase & Reset() { char_set.fill(0); }

    size_t GetMaxChar() const noexcept { return MAX_CHAR; }
    bool Has(CHAR_T index) const { return char_set[static_cast<size_t>(index)]; }
    bool Has(const std::string & str) const {
      for (CHAR_T c : str) if (!Has(c)) return false;
      return true;
    }
    bool HasAny(const std::string & str) const {
      for (CHAR_T c : str) if (Has(c)) return true;
      return false;
    }
    bool HasOnly(const std::string & str) const {
      for (CHAR_T c : str) if (!Has(c)) return false;
      return true;
    }
    bool HasAt(const std::string & str, size_t pos) const {
      return (pos < str.size()) && Has(str[pos]);
    }

    size_t FindIn(const std::string & str, size_t pos=0) const {
      while (pos < str.size()) {
        if (Has(str[pos])) return pos;
        ++pos;
      }
      return std::string::npos;
    }

    bool operator[](CHAR_T index) const { return char_set[static_cast<size_t>(index)]; }

    /// Count up the number of characters in the set.
    size_t GetSize() const noexcept {
      size_t size = 0;
      for (CHAR_T x : char_set) size += x;
      return size;
    }

    CharSetBase<CHAR_T,MAX_CHAR> & Set(CHAR_T c) {
      char_set[static_cast<size_t>(c)] = true;
      return *this;
    }
    CharSetBase<CHAR_T,MAX_CHAR> & Clear(CHAR_T c) {
      char_set[static_cast<size_t>(c)] = false;
      return *this;
    }

    /// Set a range of characters INCLUSIVE of c1 and c2.
    CharSetBase<CHAR_T,MAX_CHAR> & SetRange(CHAR_T c1, CHAR_T c2) {
      emp_assert(c1 <= c2);
      for (CHAR_T c = c1; c <= c2; ++c) Set(c);
      return *this;
    }
    CharSetBase<CHAR_T,MAX_CHAR> & ClearRange(CHAR_T c1, CHAR_T c2) {
      emp_assert(c1 <= c2);
      for (CHAR_T c = c1; c <= c2; ++c) Clear(c);
      return *this;
    }

    CharSetBase<CHAR_T,MAX_CHAR> operator+(const this_t & other) const {
      this_t out(*this);
      for (CHAR_T c : other) out.Set(c);
      return out;
    }

    CharSetBase<CHAR_T,MAX_CHAR> operator!() const {
      this_t out(*this);
      for (CHAR_T & c : out.char_set) c = !c;
      return out;
    }

    class iterator_t {
    private:
      emp::Ptr<const this_t> ptr;
      size_t index;
    public:
      iterator_t(emp::Ptr<const this_t> _ptr, size_t _index) : ptr(_ptr), index(_index) { MakeValid(); }
      iterator_t(const iterator_t &) = default;
      iterator_t & operator=(const iterator_t &) = default;

      iterator_t & MakeValid() {
        while (index < MAX_CHAR && !ptr->char_set[index]) ++index;
        return *this;
      }

      bool operator==(const iterator_t & in) const noexcept { return ptr==in.ptr && index==in.index; }
      bool operator!=(const iterator_t & in) const noexcept { return ptr!=in.ptr || index!=in.index; }
      bool operator<(const iterator_t & in) const noexcept {
        return (ptr==in.ptr) ? (index < in.index) : (ptr < in.ptr);
      }
      bool operator>(const iterator_t & in) const noexcept { return in < *this; }
      bool operator<=(const iterator_t & in) const noexcept { return !(*this > in); }
      bool operator>=(const iterator_t & in) const noexcept { return !(in > *this); }

      CHAR_T operator*() const { return static_cast<CHAR_T>(index); }
      iterator_t & operator++() { ++index; MakeValid(); return *this; }
    };

    iterator_t begin() const { return iterator_t(this,0); }
    iterator_t end() const { return iterator_t(this,MAX_CHAR); }

    /// Count the number of matches that occur in a string.
    size_t CountMatches(const std::string & str) const {
      size_t count = 0;
      for (char c : str) if (Has(c)) ++count;
      return count;
    }

    /// Count the number of matches that occur in a sub-string.
    size_t CountMatches(const std::string & str, size_t start, size_t end) const {
      size_t count = 0;
      for (size_t i = start; i < end; ++i) if (Has(str[i])) ++count;
      return count;
    }

    /// Count the number of matches that occur at the beginning of a string.
    size_t CountFrontMatches(const std::string & str, size_t start=0) const {
      size_t count = 0;
      while (count+start < str.size() && Has(str[count+start])) count++;
      return count;
    }

    size_t CountBackMatches(const std::string & str) const {
      size_t end_pos = str.size();
      while (end_pos > 0 && Has(str[end_pos-1])) end_pos--;
      return str.size() - end_pos;
    }

    /// Convert this set of characters into a regex-style character set.
    std::string AsString() const {
      std::string out("[");
      for (CHAR_T c : *this) out += c;
      return out += ']';
    }
  };

  using CharSet = CharSetBase<>;

  static CharSet CharSetRange(char c1, char c2) {
    CharSet cs;
    cs.SetRange(c1,c2);
    return cs;
  }

  static const CharSet & WhitespaceCharSet() {
    static CharSet cs(" \n\r\t");
    return cs;
  }

  static const CharSet & UpperCharSet() {
    static CharSet cs = CharSetRange('A', 'Z');
    return cs;
  }

  static const CharSet & LowerCharSet() {
    static CharSet cs = CharSetRange('a', 'z');
    return cs;
  }

  static const CharSet & LetterCharSet() {
    static CharSet cs = CharSetRange('A', 'Z') + CharSetRange('a', 'z');
    return cs;
  }

  static const CharSet & DigitCharSet() {
    static CharSet cs = CharSetRange('0', '9');
    return cs;
  }

  static const CharSet & AlphanumericCharSet() {
    static CharSet cs = CharSetRange('A', 'Z') + CharSetRange('a', 'z') + CharSetRange('0', '9');
    return cs;
  }

  static const CharSet & IDCharSet() {
    static CharSet cs = AlphanumericCharSet() + '_';
    return cs;
  }

  static const CharSet & PunctuationCharSet() {
    static CharSet cs = !(AlphanumericCharSet() + '_' + WhitespaceCharSet());
    return cs;
  }

  /// Which characters can come after a backslash in a string?
  static const CharSet & EscapeCodeCharSet() {
    static CharSet cs("bfnrtv0\\\"\'`");
    return cs;
  }

  inline bool is_whitespace(char test_char)   { return WhitespaceCharSet().Has(test_char); }
  inline bool is_upper_letter(char test_char) { return UpperCharSet().Has(test_char); }
  inline bool is_lower_letter(char test_char) { return LowerCharSet().Has(test_char); }
  inline bool is_letter(char test_char)       { return LetterCharSet().Has(test_char); }
  inline bool is_digit(char test_char)        { return DigitCharSet().Has(test_char); }
  inline bool is_alphanumeric(char test_char) { return AlphanumericCharSet().Has(test_char); }
  inline bool is_idchar(char test_char)       { return IDCharSet().Has(test_char); }
  inline bool is_punctuation(char test_char)   { return PunctuationCharSet().Has(test_char); }
  inline bool is_escape_code(char test_char)   { return EscapeCodeCharSet().Has(test_char); }

  /// Determine if a character is in a set of characters (represented as a string)
  static inline bool is_one_of(char test_char, const std::string & char_set) {
    for (char x : char_set) if (test_char == x) return true;
    return false;
  }

  /// If no functions are provided to is_valid(), always return false as base case.
  inline bool is_valid(char /* test_char */ ) { return false; }

  /// Determine if a character passes any of the test functions provided.
  template <typename... FUNS>
  inline bool is_valid(char test_char, std::function<bool(char)> fun1, FUNS... funs) {
    return fun1(test_char) || is_valid(test_char, funs...);
  }

  /// Convert a char after a backslash to its escaped version.
  inline char ToEscapeChar(char c) {
    switch (c) {
      case 'b': return '\b';   // Backspace
      case 'f': return '\f';   // Form feed
      case 'n': return '\n';   // Newline
      case 'r': return '\r';   // Return
      case 't': return '\t';   // Tab
      case 'v': return '\v';   // Vertical tab
      case '0': return '\0';   // Empty (character 0)
      case '\\': return '\\';  // Backslash
      case '"': return '"';    // Double quote
      case '\'': return '\'';  // Single quote
      case '`': return '`';    // Backquote
      default:
        emp_assert(false, "unknown escape char used; probably need to update converter!");
    }

    // @CAO: Need to add special types of numerical escapes here (e.g., ascii codes!)
    return '\0';
  }

}

#endif // #ifndef EMP_TOOLS_CHAR_UTILS_HPP_INCLUDE
