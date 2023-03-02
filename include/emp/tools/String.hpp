/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023.
 *
 *  @file String.hpp
 *  @brief Simple class to facilitate string manipulations
 *  @note Status: ALPHA
 *
 *
 *  @todo Make constexpr
 *  @todo Make handle non-'char' strings (i.e., use CharT template parameter)
 *  @todo Make handle allocators
 *  @todo Make work more broadly with stringviews
 *  @todo Maybe add special construct types like RESERVE, REPEAT, and TO_STRING for special builds?
 *
 */

#ifndef EMP_TOOLS_STRING_HPP_INCLUDE
#define EMP_TOOLS_STRING_HPP_INCLUDE

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <functional>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>

#include "../base/array.hpp"
#include "../base/assert.hpp"
#include "../base/notify.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../meta/reflection.hpp"
#include "../meta/type_traits.hpp"

#include "char_utils.hpp"

namespace emp {

  class String;

  // Some stand-alone functions to generate String objects.
  template <typename... Ts> [[nodiscard]] emp::String MakeString(Ts... args);
  [[nodiscard]] emp::String MakeEscaped(char c);
  [[nodiscard]] emp::String MakeEscaped(const emp::String & in);
  [[nodiscard]] emp::String MakeWebSafe(const emp::String & in);
  [[nodiscard]] emp::String MakeLiteral(char value);
  [[nodiscard]] emp::String MakeLiteral(char value);
  template <typename T> [[nodiscard]] emp::String MakeLiteral(const T & value);
  [[nodiscard]] emp::String MakeUpper(const emp::String & in);
  [[nodiscard]] emp::String MakeLower(const emp::String & in);
  [[nodiscard]] emp::String MakeTitleCase(emp::String in);
  [[nodiscard]] emp::String MakeRoman(int val);
  template <typename CONTAINER_T>
  [[nodiscard]] emp::String MakeEnglishList(const CONTAINER_T & container);
  template <typename... Args>
  [[nodiscard]] emp::String MakeFormatted(const emp::String& format, Args... args);

  template <typename CONTAINER_T>
  [[nodiscard]] emp::String Join(const CONTAINER_T & container, std::string join_str="",
                                 std::string open="", std::string close="");

  // ToString specialization for emp::String
  const emp::String & ToString(const emp::String & in) { return in; }

  class String : public std::string {
  private:
    // Special class for tracking character interactions.
    struct Mode {
      emp::array<char, 128> char_matches;
      uint8_t count = 0;
      Mode() { char_matches.fill('\0'); }
      Mode(std::string quotes, std::string parens="") {
        emp_assert(parens.size() % 2 == 0, "String::Mode must have odd number of paren chars.");
        char_matches.fill('\0');
        for (char c : quotes) char_matches[c] = c;
        for (size_t i=0; i < parens.size(); i+=2) char_matches[quotes[i]] = quotes[i+1];
        count = static_cast<uint8_t>(quotes.size() + parens.size()/2);
      }
      Mode & operator=(const Mode &) = default;
      Mode & operator=(Mode &&) = default;
      bool IsParen(char c) const { return char_matches[c] && char_matches[c] == c; }
      bool IsQuote(char c) const { return char_matches[c] && char_matches[c] != c; }
      char GetMatch(char c) const { return char_matches[c]; }
      uint8_t GetCount() const { return count; }
    };

    static Mode EmptyMode() { return Mode(); }
    static Mode QuotesMode() { return Mode("\"'"); }
    static Mode ParensMode() { return Mode("", "()[]{}"); }
    static Mode DefaultMode() { return Mode("\"'", "()[]{}"); }
    static Mode FullMode() { return Mode("\"'`", "()[]{}<>"); }


    // Convert objects into a streamable type using _Convert(obj)
    template <typename T> static decltype(std::declval<T>().ToString()) _Convert(const T & in, bool) { return in.ToString(); }
    template <typename T> static auto _Convert(const T & in, int) -> decltype(emp::ToString(in)) { return emp::ToString(in); }
    template <typename T> static const T & _Convert(const T & in, ...) { return in; }

    void _AssertPos([[maybe_unused]] size_t pos) const { emp_assert(pos < size(), pos, size()); }

  public:
    using value_type = std::string::value_type;
    using allocator_type = std::string::allocator_type;
    using size_type = std::string::size_type;
    using difference_type = std::string::difference_type;
    using reference = std::string::reference;
    using const_reference = std::string::const_reference;
    using pointer = std::string::pointer;
    using const_pointer = std::string::const_pointer;
    using iterator = std::string::iterator;
    using const_iterator = std::string::const_iterator;
    using reverse_iterator = std::string::reverse_iterator;
    using const_reverse_iterator = std::string::const_reverse_iterator;

    static constexpr size_t npos = std::string::npos;

    // Constructors duplicating from std::string
    String() = default;
    String(const std::string & _str) : std::string(_str) { }
    String(std::string && _str) : std::string(std::move(_str)) { }
    // String(const std::string & _str, Mode _mode) : std::string(_str), mode(_mode) { }
    // String(std::string && _str, Mode _mode) : std::string(std::move(_str)), mode(_mode) { }
    String(const char * _str) : std::string(_str) { }
    String(size_t count, char _str) : std::string(count, _str) { }
    String(std::initializer_list<char> _str) : std::string(_str) { }
    String(const String & _str, size_t start, size_t count=npos)
      : std::string(_str, start, count) { }
    String(const std::string & _str, size_t start, size_t count=npos)
      : std::string(_str, start, count) { }
    String(const char * _str, size_t count) : std::string(_str, count) { }
    template< class InputIt >
    String(InputIt first, InputIt last) : std::string(first, last) { }
    String(std::nullptr_t) = delete;

    // ------ New constructors ------

    String(const String &) = default;
    String(String &&) = default;

    // Allow a string to be transformed during construction, 1-to-1
    String(const std::string & _str, std::function<char(char)> transform_fun) {
      reserve(_str.size());  // Setup expected size.
      for (auto & c : _str) { push_back(transform_fun(c)); }
    }

    // Allow a string to be transformed during construction, 1-to-any
    String(const std::string & _str, std::function<emp::String(char)> transform_fun) {
      reserve(_str.size());  // Setup expected size; assume size will be 1-to-1 by default.
      for (auto & c : _str) { append(transform_fun(c)); }
    }


    // ------ Assignment operators ------

    String & operator=(const String &) = default;
    String & operator=(String &&) = default;
    String & operator=(const std::string & _in) { std::string::operator=(_in); return *this; }
    String & operator=(std::string && _in) { std::string::operator=(std::move(_in)); return *this; }
    String & operator=(const char * _in) { std::string::operator=(_in); return *this; }
    String & operator=(char _in) { std::string::operator=(_in); return *this; }
    String & operator=(std::initializer_list<char> _in) { std::string::operator=(_in); return *this; }
    String & operator=(std::nullptr_t) = delete;

    template <typename... ARG_Ts>
    String & assign(ARG_Ts &&... args)
      { std::string::assign(std::forward<ARG_Ts>(args)...); return *this; }

    // ------ Element Access ------

    [[nodiscard]] char & operator[](size_t pos)
      { _AssertPos(pos); return std::string::operator[](pos); }
    [[nodiscard]] char operator[](size_t pos) const
      { _AssertPos(pos); return std::string::operator[](pos); }
    [[nodiscard]] char & front() { _AssertPos(0); return std::string::front(); }
    [[nodiscard]] char front() const { _AssertPos(0); return std::string::front(); }
    [[nodiscard]] char & back() { _AssertPos(0); return std::string::back(); }
    [[nodiscard]] char back() const { _AssertPos(0); return std::string::back(); }
    // [[nodiscard]] char * data() { return std::string::data(); }
    // [[nodiscard]] const char * data() const { return std::string::data(); }
    // [[nodiscard]] const char * c_str() const { return std::string::c_str(); }
    [[nodiscard]] char & Get(size_t pos) { return operator[](pos); }
    [[nodiscard]] char Get(size_t pos) const { return operator[](pos); }


    [[nodiscard]] String substr(size_t pos=0, size_t count=npos ) const
      { _AssertPos(pos); return std::string::substr(pos, count); }
    [[nodiscard]] String GetRange(std::size_t start_pos, std::size_t end_pos) const
      { return std::string::substr(start_pos, end_pos - start_pos); }

    [[nodiscard]] std::string_view View(size_t start=0, size_t out_size=npos) const {
      emp_assert(start + out_size <= size());
      return std::string_view(data()+start, out_size);
    }
    [[nodiscard]] std::string_view ViewFront(size_t out_size) const { return View(0, out_size); }
    [[nodiscard]] std::string_view ViewBack(size_t out_size) const
      { emp_assert(out_size <= size()); return View(size()-out_size, out_size); }
    [[nodiscard]] std::string_view ViewRange(size_t start, size_t end) const {
      emp_assert(start <= end && end <= size());
      return View(start, end - start);
    }


    // ------ Iterators ------

    // iterator begin() { return std::string::begin(); }
    // const_iterator begin() const { return std::string::begin(); }
    // const_iterator cbegin() const { return std::string::cbegin(); }
    // reverse_iterator rbegin() { return std::string::rbegin(); }
    // const_reverse_iterator rbegin() const { return std::string::rbegin(); }
    // const_reverse_iterator crbegin() const { return std::string::crbegin(); }

    // iterator end() { return std::string::end(); }
    // const_iterator end() const { return std::string::end(); }
    // const_iterator cend() const { return std::string::cend(); }
    // reverse_iterator rend() { return std::string::rend(); }
    // const_reverse_iterator rend() const { return std::string::rend(); }
    // const_reverse_iterator crend() const { return std::string::crend(); }


    // ------ Capacity ------

    // [[nodiscard]] bool empty() const { return std::string::empty(); }
    // [[nodiscard]] size_t size() const { return size(); }
    // [[nodiscard]] size_t length() const { return std::string::length(); }
    // [[nodiscard]] size_t max_size() const { return std::string::max_size(); }
    // [[nodiscard]] size_t capacity() const { return std::string::capacity(); }
    // void reserve(size_t new_cap) { std::string::reserve(new_cap); }
    // void shrink_to_fit() { std::string::shrink_to_fit(); }
    

    // ------ Classification and Comparisons ------
    
    // [[nodiscard]] int compare(const String & in) { return std::string::compare(in); }
    // template <typename... ARG_Ts> [[nodiscard]] int compare(ARG_Ts &&... args)
    //   { return std::string::compare(std::forward<ARG_Ts>(args)...); }

    // [[nodiscard]] bool starts_with(const String & in) const noexcept { return std::string::starts_with(in); }
    // template <typename ARG_T> [[nodiscard]] bool starts_with( ARG_T && in ) const noexcept
    //   { return std::string::starts_with(std::forward<ARG_T>(in)); }

    // [[nodiscard]] bool ends_with(const String & in) const noexcept { return std::string::ends_with(in); }
    // template <typename ARG_T> [[nodiscard]] bool ends_with( ARG_T && in ) const noexcept
    //   { return std::string::ends_with(std::forward<ARG_T>(in)); }

    // [[nodiscard]] bool contains(const String & in) const noexcept { return std::string::find(in) != npos; }
    // template <typename ARG_T> [[nodiscard]] bool contains( ARG_T && in ) const noexcept
    //   { return std::string::find(std::forward<ARG_T>(in)) != npos; }

    [[nodiscard]] bool HasAt(const emp::String test, size_t pos) const 
      { return (size() < test.size()+pos) && View(pos, test.size()) == test; }
    [[nodiscard]] bool HasPrefix(const emp::String & prefix) const { return starts_with(prefix); }
    [[nodiscard]] bool HasSuffix(const emp::String & suffix) const { return ends_with(suffix); }

    // ------ Simple Analysis ------

    // Count the number of occurrences of a specific character.
    [[nodiscard]] size_t Count(char c, size_t start=0) const
      { return (size_t) std::count(begin()+start, end(), c); }

    // Count the number of occurrences of a specific character within a range.
    [[nodiscard]] size_t Count(char c, size_t start, size_t end) const
      { return (size_t) std::count(begin()+start, begin()+end, c); }

    /// Test if an string is formatted as a literal character.
    [[nodiscard]] bool IsLiteralChar() const;

    /// Test if an string is formatted as a literal string.
    [[nodiscard]] bool IsLiteralString(const std::string & quote_marks="\"") const;

    /// Explain what string is NOT formatted as a literal string.
    [[nodiscard]] std::string DiagnoseLiteralString(const std::string & quote_marks="\"") const;

    /// Is string composed only of a set of characters (can be provided as a string)
    [[nodiscard]] bool IsComposedOf(emp::CharSet char_set) const { return char_set.Has(*this); }

    /// Is string a valid number (int, floating point, or scientific notation all valid)
    [[nodiscard]] bool IsNumber() const;

    /// Is string a valid identifier? At least one char; cannot begin with digit, only letters, digits and `_`
    [[nodiscard]] bool IsIdentifier() const
      { return size() && !is_digit(Get(0)) && IDCharSet().Has(*this); }

    [[nodiscard]] bool OnlyLower() const { return LowerCharSet().Has(*this); }
    [[nodiscard]] bool OnlyUpper() const { return UpperCharSet().Has(*this); }
    [[nodiscard]] bool OnlyDigits() const { return DigitCharSet().Has(*this); }
    [[nodiscard]] bool OnlyAlphanumeric() const { return AlphanumericCharSet().Has(*this); }
    [[nodiscard]] bool OnlyWhitespace() const { return WhitespaceCharSet().Has(*this); }

    [[nodiscard]] bool HasOneOf(emp::CharSet char_set) const { return char_set.HasAny(*this); }
    [[nodiscard]] bool HasWhitespace() const { return WhitespaceCharSet().HasAny(*this); }
    [[nodiscard]] bool HasNonwhitespace() const { return !WhitespaceCharSet().HasOnly(*this); }
    [[nodiscard]] bool HasUpper() const { return UpperCharSet().HasAny(*this); }
    [[nodiscard]] bool HasLower() const { return LowerCharSet().HasAny(*this); }
    [[nodiscard]] bool HasLetter() const { return LetterCharSet().HasAny(*this); }
    [[nodiscard]] bool HasDigit() const { return DigitCharSet().HasAny(*this); }
    [[nodiscard]] bool HasAlphanumeric() const { return AlphanumericCharSet().HasAny(*this); }

    [[nodiscard]] bool HasCharAt(char c, size_t pos) const
      { return (pos < size()) && (Get(pos) == c); }
    [[nodiscard]] bool HasOneOfAt(emp::CharSet opts, size_t pos) const { return opts.HasAt(*this, pos); }
    [[nodiscard]] bool HasDigitAt(size_t pos) const { return DigitCharSet().HasAt(*this, pos); }
    [[nodiscard]] bool HasLetterAt(size_t pos) const { return LetterCharSet().HasAt(*this, pos); }


    // ------ Removals and Extractions ------

    String & clear() noexcept { std::string::clear(); return *this; }

    String & erase(size_t index=0, size_t count=npos) { std::string::erase(index,count); return *this; }
    iterator erase(const_iterator pos) { return std::string::erase(pos); }
    iterator erase(const_iterator first, const_iterator last) { return std::string::erase(first, last); }

    // void pop_back() { std::string::pop_back(); }

    emp::String & RemoveChars(const CharSet & chars);
    emp::String & RemoveWhitespace()  { return RemoveChars(WhitespaceCharSet()); }
    emp::String & RemoveUpper()       { return RemoveChars(UpperCharSet()); }
    emp::String & RemoveLower()       { return RemoveChars(LowerCharSet()); }
    emp::String & RemoveLetters()     { return RemoveChars(LetterCharSet()); }
    emp::String & RemoveDigits()      { return RemoveChars(DigitCharSet()); }
    emp::String & RemovePunctuation() { return RemoveChars(PunctuationCharSet()); }

    bool PopIf(char c);
    bool PopIf(String in);
    emp::String PopAll();
    emp::String PopFixed(std::size_t end_pos, size_t delim_size=0);
    emp::String Pop(CharSet chars=" \n\t\r", const Mode & mode=EmptyMode());
    emp::String PopTo(std::string delim, const Mode & mode=EmptyMode());
    emp::String PopWord(const Mode & mode=EmptyMode());
    emp::String PopLine(const Mode & mode=EmptyMode());
    emp::String PopQuote(const Mode & mode=QuotesMode());
    emp::String PopParen(const Mode & mode=ParensMode());
    size_t PopUInt();

    // ------ Insertions and Additions ------

    String & insert(size_t index, const String & in) { std::string::insert(index, in); return *this; }
    String & insert(size_t index, const String & in, size_t pos, size_t count=npos)
      { std::string::insert(index, in, pos, count); return *this; }
    template <typename... ARG_Ts> String & insert(size_t index, ARG_Ts &&... args)
      { std::string::insert(index, std::forward<ARG_Ts>(args)...); return *this; }
    template <typename... ARG_Ts> String & insert(const_iterator pos, ARG_Ts &&... args)
      { return std::string::insert(pos, std::forward<ARG_Ts>(args)...); }

    // void push_back(char c) { std::string::push_back(c); }

    String & append(const String & in) { std::string::append(in); return *this; }
    String & append(const String & in, size_t pos, size_t count)
      { std::string::append(in, pos, count); return *this; }
    template <typename... ARG_Ts> String & append(ARG_Ts &&... args)
      { std::string::append(std::forward<ARG_Ts>(args)...); return *this; }

//    String & operator+=(const String & in) { return append(in); }
    template <typename ARG_T> String & operator+=(ARG_T && arg)
      { std::string::operator+=(std::forward<ARG_T>(arg)); return *this; }

    String & PadFront(char padding, size_t target_size) {
      if (size() < target_size) *this = std::string(target_size - size(), padding) + *this;
      return *this;
    }

    String & PadBack(char padding, size_t target_size) {
      if (size() < target_size) *this = *this + std::string(target_size - size(), padding);
      return *this;
    }

    // ------ Direct Modifications ------

    template <typename... ARG_Ts> String & replace(ARG_Ts &&... args)
      { std::string::replace(std::forward<ARG_Ts>(args)...); return *this; }

    // size_t copy(char * dest, size_t count, size_t pos=0) const { return std::string::copy(dest, count, pos); }

    String & resize( size_t count, char c='\0') { std::string::resize(count, c); return *this; }

    // void swap(String & other) { std::string::swap(other); }

    emp::String & ReplaceChar(char from, char to, size_t start=0)
      { for (size_t i=start; i < size(); ++i) if (Get(i) == from) Get(i) = to; return *this; }
    emp::String & ReplaceRange(size_t start, size_t end, std::string value)
      { return replace(start, end-start, value); }
    emp::String & TrimWhitespace();
    emp::String & CompressWhitespace();
    emp::String & Slugify();

    // Find any instances of ${X} and replace with dictionary lookup of X.
    template <typename MAP_T>
    emp::String & ReplaceVars(const MAP_T & var_map, emp::String symbol="$",
                              const Mode & mode=QuotesMode());

    // Find any instance of MACRO_NAME(ARGS) and call replace it with return from fun(ARGS).
    template <typename FUN_T>
    String & ReplaceMacro(std::string start_str, std::string end_str, FUN_T && fun,
                          const Mode & mode=QuotesMode());


    // ------ Searching ------

    // [[nodiscard]] size_t find(const std::string & s, size_t pos=0) const noexcept { return std::string::find(s,pos); }
    // [[nodiscard]] size_t find(const char* s, size_t pos=0) const { return std::string::find(s,pos); }
    // [[nodiscard]] size_t find(const char* s, size_t pos, size_t count) const { return std::string::find(s,pos,count); }
    // [[nodiscard]] size_t find(char c, size_t pos=0) const noexcept { return std::string::find(c,pos); }
    // template < class SVIEW_T > [[nodiscard]] size_t find(const SVIEW_T & sv, size_t pos=0) const
    //   { return std::string::find(sv,pos); }

    // [[nodiscard]] size_t rfind(const std::string & s, size_t pos=0) const noexcept { return std::string::rfind(s,pos); }
    // [[nodiscard]] size_t rfind(const char* s, size_t pos=0) const { return std::string::rfind(s,pos); }
    // [[nodiscard]] size_t rfind(const char* s, size_t pos, size_t count) const { return std::string::rfind(s,pos,count); }
    // [[nodiscard]] size_t rfind(char c, size_t pos=0) const noexcept { return std::string::rfind(c,pos); }
    // template < class SVIEW_T > [[nodiscard]] size_t rfind(const SVIEW_T & sv, size_t pos=0) const
    //   { return std::string::rfind(sv,pos); }

    // [[nodiscard]] size_t find_first_of(const std::string & s, size_t pos=0) const noexcept
    //   { return std::string::find_first_of(s,pos); }
    // [[nodiscard]] size_t find_first_of(const char* s, size_t pos=0) const { return std::string::find_first_of(s,pos); }
    // [[nodiscard]] size_t find_first_of(const char* s, size_t pos, size_t count) const
    //   { return std::string::find_first_of(s,pos,count); }
    // [[nodiscard]] size_t find_first_of(char c, size_t pos=0) const noexcept { return std::string::find_first_of(c,pos); }
    // template < class SVIEW_T > [[nodiscard]] size_t find_first_of(const SVIEW_T & sv, size_t pos=0) const
    //   { return std::string::find_first_of(sv,pos); }

    // [[nodiscard]] size_t find_first_not_of(const std::string & s, size_t pos=0) const noexcept
    //   { return std::string::find_first_not_of(s,pos); }
    // [[nodiscard]] size_t find_first_not_of(const char* s, size_t pos=0) const
    //   { return std::string::find_first_not_of(s,pos); }
    // [[nodiscard]] size_t find_first_not_of(const char* s, size_t pos, size_t count) const
    //   { return std::string::find_first_not_of(s,pos,count); }
    // [[nodiscard]] size_t find_first_not_of(char c, size_t pos=0) const noexcept
    //   { return std::string::find_first_not_of(c,pos); }
    // template < class SVIEW_T > [[nodiscard]] size_t find_first_not_of(const SVIEW_T & sv, size_t pos=0) const
    //   { return std::string::find_first_not_of(sv,pos); }

    // [[nodiscard]] size_t find_last_of(const std::string & s, size_t pos=0) const noexcept
    //   { return std::string::find_last_of(s,pos); }
    // [[nodiscard]] size_t find_last_of(const char* s, size_t pos=0) const { return std::string::find_last_of(s,pos); }
    // [[nodiscard]] size_t find_last_of(const char* s, size_t pos, size_t count) const
    //   { return std::string::find_last_of(s,pos,count); }
    // [[nodiscard]] size_t find_last_of(char c, size_t pos=0) const noexcept { return std::string::find_last_of(c,pos); }
    // template < class SVIEW_T > [[nodiscard]] size_t find_last_of(const SVIEW_T & sv, size_t pos=0) const
    //   { return std::string::find_last_of(sv,pos); }

    // [[nodiscard]] size_t find_last_not_of(const std::string & s, size_t pos=0) const noexcept
    //   { return std::string::find_last_not_of(s,pos); }
    // [[nodiscard]] size_t find_last_not_of(const char* s, size_t pos=0) const
    //   { return std::string::find_last_not_of(s,pos); }
    // [[nodiscard]] size_t find_last_not_of(const char* s, size_t pos, size_t count) const
    //   { return std::string::find_last_not_of(s,pos,count); }
    // [[nodiscard]] size_t find_last_not_of(char c, size_t pos=0) const noexcept
    //   { return std::string::find_last_not_of(c,pos); }
    // template < class SVIEW_T > [[nodiscard]] size_t find_last_not_of(const SVIEW_T & sv, size_t pos=0) const
    //   { return std::string::find_last_not_of(sv,pos); }

    [[nodiscard]] size_t FindQuoteMatch(size_t pos) const;
    [[nodiscard]] size_t FindParenMatch(size_t pos, const Mode & mode=ParensMode()) const;
    [[nodiscard]] size_t FindMatch(size_t pos, const Mode & mode=DefaultMode()) const;
    [[nodiscard]] size_t Find(char target, size_t start=0, const Mode & mode=EmptyMode()) const;
    [[nodiscard]] size_t Find(std::string target, size_t start=0, const Mode & mode=EmptyMode()) const;
    [[nodiscard]] size_t Find(const CharSet & char_set, size_t start, const Mode & mode=EmptyMode()) const;
    void FindAll(char target, emp::vector<size_t> & results, const Mode & mode=EmptyMode()) const;
    [[nodiscard]] emp::vector<size_t> FindAll(char target, const Mode & mode=EmptyMode()) const;
    template <typename... Ts>
    [[nodiscard]] size_t FindAnyOfFrom(size_t start, std::string test1, Ts... tests) const;
    template <typename T, typename... Ts>
    [[nodiscard]] size_t FindAnyOf(T test1, Ts... tests) const;
    [[nodiscard]] size_t FindID(std::string target, size_t start, const Mode & mode=QuotesMode()) const;

    [[nodiscard]] size_t FindWhitespace(size_t start=0, const Mode & mode=EmptyMode()) const 
      { return Find(WhitespaceCharSet(), start, mode); }
    [[nodiscard]] size_t FindNonWhitespace(size_t start=0, const Mode & mode=EmptyMode()) const 
      { return Find(!WhitespaceCharSet(), start, mode); }
    [[nodiscard]] size_t FindUpperChar(size_t start=0, const Mode & mode=EmptyMode()) const 
      { return Find(UpperCharSet(), start, mode); }
    [[nodiscard]] size_t FindNonUpperChar(size_t start=0, const Mode & mode=EmptyMode()) const 
      { return Find(!UpperCharSet(), start, mode); }
    [[nodiscard]] size_t FindLowerChar(size_t start=0, const Mode & mode=EmptyMode()) const 
      { return Find(LowerCharSet(), start, mode); }
    [[nodiscard]] size_t FindNonLowerChar(size_t start=0, const Mode & mode=EmptyMode()) const 
      { return Find(!LowerCharSet(), start, mode); }
    [[nodiscard]] size_t FindLetterChar(size_t start=0, const Mode & mode=EmptyMode()) const 
      { return Find(LetterCharSet(), start, mode); }
    [[nodiscard]] size_t FindNonLetterChar(size_t start=0, const Mode & mode=EmptyMode()) const 
      { return Find(!LetterCharSet(), start, mode); }
    [[nodiscard]] size_t FindDigitChar(size_t start=0, const Mode & mode=EmptyMode()) const 
      { return Find(DigitCharSet(), start, mode); }
    [[nodiscard]] size_t FindNonDigitChar(size_t start=0, const Mode & mode=EmptyMode()) const 
      { return Find(!DigitCharSet(), start, mode); }
    [[nodiscard]] size_t FindAlphanumericChar(size_t start=0, const Mode & mode=EmptyMode()) const 
      { return Find(AlphanumericCharSet(), start, mode); }
    [[nodiscard]] size_t FindNonAlphanumericChar(size_t start=0, const Mode & mode=EmptyMode()) const 
      { return Find(!AlphanumericCharSet(), start, mode); }
    [[nodiscard]] size_t FindIDChar(size_t start=0, const Mode & mode=EmptyMode()) const 
      { return Find(IDCharSet(), start, mode); }
    [[nodiscard]] size_t FindNonIDChar(size_t start=0, const Mode & mode=EmptyMode()) const 
      { return Find(!IDCharSet(), start, mode); }


    // ------ Other Views ------
    std::string_view ViewNestedBlock(size_t start=0, const Mode & mode=QuotesMode())
      { return ViewRange(start+1, FindParenMatch(start, mode) - 1); }
    std::string_view ViewQuote(size_t start=0, const Mode & mode=QuotesMode())
      { return ViewRange(start, mode.IsQuote(Get(start)) ? FindQuoteMatch(start) : start); }


    // ------ Transformations into non-Strings ------
    // Note: For efficiency there are two versions of most of these: one where the output
    // data structure is provided and one where it must be generated.

    void Slice(emp::vector<emp::String> & out_set, std::string delim=",",
               const Mode & mode=DefaultMode(), bool trim_whitespace=true) const;

    [[nodiscard]]
    emp::vector<emp::String> Slice(std::string delim=",", const Mode & mode=DefaultMode(),
                                   bool trim_whitespace=true) const;

    void ViewSlices(
      emp::vector<std::string_view> & out_set,
      std::string delim=",",
      const Mode & mode=DefaultMode()
    ) const;

    [[nodiscard]] emp::vector<std::string_view> ViewSlices(
      std::string delim=",",
      const Mode & mode=DefaultMode()
    ) const;
  
    void SliceAssign(
      std::map<emp::String,emp::String> & out_map,
      std::string delim=",", std::string assign_op="=",
      const Mode & mode=QuotesMode(), bool trim_whitespace=true
    ) const;

    [[nodiscard]] std::map<emp::String,emp::String> SliceAssign(
      std::string delim=",", std::string assign_op="=",
      const Mode & mode=QuotesMode(), bool trim_whitespace=true
    ) const;


    // ------ Other Operators ------

    // template <typename T> [[nodiscard]] String operator+(T && in) const
    //   { return std::string::operator+(std::forward<T>(in)); }
    // [[nodiscard]] bool operator==(const emp::String & in) const { return std::string::operator==(in); }
    // [[nodiscard]] bool operator==(const std::string & in) const { return std::string::operator==(in); }
    // [[nodiscard]] bool operator==(const char * in) const { return std::string::operator==(in); }

    // [[nodiscard]] bool operator!=(const emp::String & in) const { return std::string::operator!=(in); }
    // [[nodiscard]] bool operator!=(const std::string & in) const { return std::string::operator!=(in); }
    // [[nodiscard]] bool operator!=(const char * in) const { return std::string::operator!=(in); }

    // [[nodiscard]] bool operator<(const emp::String & in) const { return std::string::operator<(in); }
    // [[nodiscard]] bool operator<(const std::string & in) const { return std::string::operator<(in) in; }
    // [[nodiscard]] bool operator<(const char * in) const { return std::string::operator<(in) in; }

    // [[nodiscard]] bool operator<=(const emp::String & in) const { return std::string::operator<=(in); }
    // [[nodiscard]] bool operator<=(const std::string & in) const { return std::string::operator<=(in); }
    // [[nodiscard]] bool operator<=(const char * in) const { return std::string::operator<=(in); }

    // [[nodiscard]] bool operator>(const emp::String & in) const { return std::string::operator>(in); }
    // [[nodiscard]] bool operator>(const std::string & in) const { return std::string::operator>(in) in; }
    // [[nodiscard]] bool operator>(const char * in) const { return std::string::operator>(in) in; }

    // [[nodiscard]] bool operator>=(const emp::String & in) const { return std::string::operator>=(in); }
    // [[nodiscard]] bool operator>=(const std::string & in) const { return std::string::operator>=(in); }
    // [[nodiscard]] bool operator>=(const char * in) const { return std::string::operator>=(in); }

    // auto operator<=>(const emp::String & in) const = default;
    // auto operator<=>(const std::string & in) const { return *this <=> in; }
    // auto operator<=>(const char * in) const { return *this <=> in; }
    

    //  ------ FORMATTING ------
    // Append* adds to the end of the current string.
    // Set* replaces the current string.
    // To* Converts the current string.
    // As* Returns a modified version of the current string, leaving original intact.
    // Most also have stand-along Make* versions where the core implementation is found.

    template <typename... Ts>
    String & Append(Ts... args) { return *this += MakeString(std::forward<Ts>(args)...);}
    template <typename... Ts>
    String & Set(Ts... args) { return *this = MakeString(std::forward<Ts>(args)...); }
    template <typename T>
    T As() { std::stringstream ss; ss << *this; T out; ss >> out; return out; }

    String & AppendEscaped(char c) { *this += MakeEscaped(c); return *this; }
    String & SetEscaped(char c) { *this = MakeEscaped(c); return *this; }

    String & AppendEscaped(const std::string & in) { *this+=MakeEscaped(in); return *this; }
    String & SetEscaped(const std::string & in) { *this = MakeEscaped(in); return *this; }
    String & ToEscaped() { *this = MakeEscaped(*this); return *this; }
    [[nodiscard]] emp::String AsEscaped() { return MakeEscaped(*this); }

    String & AppendWebSafe(std::string in) { *this+=MakeWebSafe(in); return *this; }
    String & SetWebSafe(const std::string & in) { *this = MakeWebSafe(in); return *this; }
    String & ToWebSafe() { *this = MakeWebSafe(*this); return *this; }
    [[nodiscard]] emp::String AsWebSafe() { return MakeWebSafe(*this); }

    // <= Creating Literals =>
    template <typename T>
    String & AppendLiteral(const T & in) { *this+=MakeLiteral(in); return *this; }
    template <typename T>
    String & SetLiteral(const T & in) { *this = MakeLiteral(in); return *this;; }
    String & ToLiteral() { *this = MakeLiteral(*this); return *this; }
    [[nodiscard]] emp::String AsLiteral() { return MakeLiteral(*this); }

    String & AppendUpper(const std::string & in) { *this+=MakeUpper(in); return *this; }
    String & SetUpper(const std::string & in) { *this = MakeUpper(in); return *this; }
    String & ToUpper() { *this = MakeUpper(*this); return *this; }
    [[nodiscard]] emp::String AsUpper() { return MakeUpper(*this); }

    String & AppendLower(const std::string & in) { *this+=MakeLower(in); return *this; }
    String & SetLower(const std::string & in) { *this = MakeLower(in); return *this; }
    String & ToLower() { *this = MakeLower(*this); return *this; }
    [[nodiscard]] emp::String AsLower() { return MakeLower(*this); }

    String & AppendTitleCase(const std::string & in) { *this+=MakeTitleCase(in); return *this; }
    String & SetTitleCase(const std::string & in) { *this = MakeTitleCase(in); return *this; }
    String & ToTitleCase() { *this = MakeTitleCase(*this); return *this; }
    [[nodiscard]] emp::String AsTitleCase() { return MakeTitleCase(*this); }

    String & AppendRoman(int val) { *this+=MakeRoman(val); return *this; }
    String & SetRoman(int val) { *this = MakeRoman(val); return *this; }

    template <typename CONTAINER_T> String & AppendEnglishList(const CONTAINER_T & container)
      { *this += MakeEnglishList(container); return *this;}
    template <typename CONTAINER_T> String & SetEnglishList(const CONTAINER_T & container)
      { *this = MakeEnglishList(container); return *this;}
      
    template<typename... ARG_Ts> String & AppendFormatted(const std::string& format, ARG_Ts... args)
      { return *this += MakeFormatted(format, std::forward<ARG_Ts>(args)...); }
    template<typename... ARG_Ts> String & SetFormatted(const std::string& format, ARG_Ts... args)
      { return *this = MakeFormatted(format, std::forward<ARG_Ts>(args)...); }

    template <typename CONTAINER_T>
    String & AppendJoin(const CONTAINER_T & container, std::string delim="",
                        std::string open="", std::string close="")
      { *this += Join(container, delim, open, close); return *this;}
    template <typename CONTAINER_T>
    String & SetJoin(const CONTAINER_T & container, std::string delim="",
                     std::string open="", std::string close="")
      { *this = Join(container, delim, open, close); return *this;}

    template <typename... Ts>
    static emp::String Make(Ts... args) {
      std::stringstream ss;
      (ss << ... << String::_Convert(std::forward<Ts>(args)));
      return ss.str();
    }
  };


  ////////////////////////////////////////////////////////////////////////////
  //
  //    FUNCTION DEFINITIONS
  //
  ////////////////////////////////////////////////////////////////////////////
  
  /// Determine if this string represents a proper number.
  bool String::IsNumber() const {
    if (!size()) return false;           // If string is empty, not a number!
    size_t pos = 0;
    if (HasOneOfAt("+-", pos)) ++pos;        // Allow leading +/-
    while (HasDigitAt(pos)) ++pos;           // Any number of digits (none is okay)
    if (HasCharAt('.', pos)) {               // If there's a DECIMAL PLACE, look for more digits.
      ++pos;                                 // Skip over the dot.
      if (!HasDigitAt(pos++)) return false;  // Must have at least one digit after '.'
      while (HasDigitAt(pos)) ++pos;         // Any number of digits is okay.
    }
    if (HasOneOfAt("eE", pos)) {             // If there's an e... SCIENTIFIC NOTATION!
      ++pos;                                 // Skip over the e.
      if (HasOneOfAt("+-", pos)) ++pos;      // skip leading +/-
      if (!HasDigitAt(pos++)) return false;  // Must have at least one digit after 'e'
      while (HasDigitAt(pos)) ++pos;         // Allow for MORE digits.
    }
    // If we've made it to the end of the string AND there was at least one digit, success!
    return (pos == size()) && HasDigit();
  }

  // Given the start position of a quote, find where it ends; marks must be identical
  size_t String::FindQuoteMatch(size_t pos) const {
    while (++pos < size()) {
      const char mark = Get(pos);
      if (Get(pos) == '\\') { ++pos; continue; } // Skip escaped characters in quotes
      if (Get(pos) == mark) { return pos; }      // Found match!
    }
    return npos; // Not found.
  }

  // Given an open parenthesis, find where it closes (including nesting).  Marks must be different.
  size_t String::FindParenMatch(size_t pos, const Mode & mode) const {
    const char open = Get(pos);
    const char close = mode.GetMatch(open);
    size_t open_count = 1;
    while (++pos < size()) {
      if (Get(pos) == open) ++open_count;
      else if (Get(pos) == close) { if (--open_count == 0) return pos; }
      else if (mode.IsQuote(Get(pos)) ) pos = FindQuoteMatch(pos);
    }

    return npos;
  }

  size_t String::FindMatch(size_t pos, const Mode & mode) const {
    if (mode.IsQuote(Get(pos))) return FindQuoteMatch(pos);
    if (mode.IsParen(Get(pos))) return FindParenMatch(pos, mode);
    return npos;
  }

  // A version of string::find() with single chars that can skip over quotes.
  size_t String::Find(char target, size_t start, const Mode & mode) const {
    // Make sure found_pos is not in a quote and/or parens; adjust as needed!
    for (size_t pos=start; pos < size(); ++pos) {
      if (Get(pos) == target) return pos;
      else if (mode.IsQuote(Get(pos))) pos = FindQuoteMatch(pos);
      else if (mode.IsParen(Get(pos))) pos = FindParenMatch(pos, mode);
    }

    return npos;
  }

  // A version of string::find() that can skip over quotes.
  size_t String::Find(std::string target, size_t start, const Mode & mode) const {
    size_t found_pos = find(target, start);
    if (mode.GetCount() == 0) return found_pos;

    // Make sure found_pos is not in a quote and/or parens; adjust as needed!
    for (size_t scan_pos=0;
         scan_pos < found_pos && found_pos != npos;
         scan_pos++)
    {
      // Skip quotes, if needed...
      if (mode.IsQuote(Get(scan_pos))) {
        scan_pos = FindQuoteMatch(scan_pos);
        if (found_pos < scan_pos) found_pos = find(target, scan_pos);
      }
      else if (mode.IsParen(Get(scan_pos))) {
        scan_pos = FindParenMatch(scan_pos);
        if (found_pos < scan_pos) found_pos = find(target, scan_pos);
      }
    }

    return found_pos;
  }
  // Find any of a set of characters.
  size_t String::Find(const CharSet & char_set, size_t start, const Mode & mode) const
  {
    // Make sure found_pos is not in a quote and/or parens; adjust as needed!
    for (size_t pos=start; pos < size(); ++pos) {
      if (char_set.Has(Get(pos))) return pos;
      else if (mode.IsQuote(Get(pos))) pos = FindQuoteMatch(pos);
      else if (mode.IsParen(Get(pos))) pos = FindParenMatch(pos, mode);
    }

    return npos;
  }

  void String::FindAll(char target, emp::vector<size_t> & results, const Mode & mode) const {
    results.resize(0);
    for (size_t pos=0; pos < size(); pos++) {
      if (Get(pos) == target) results.push_back(pos);

      // Skip quotes, if needed...
      if (mode.IsQuote(Get(pos))) pos = FindQuoteMatch(pos);
      else if (mode.IsParen(Get(pos))) pos = FindParenMatch(pos, mode);
    }
  }

  emp::vector<size_t> String::FindAll(char target, const Mode & mode) const {
    emp::vector<size_t> out;
    FindAll(target, out, mode);
    return out;
  }

  template <typename... Ts>
  size_t String::FindAnyOfFrom(size_t start, std::string test1, Ts... tests) const {
    if constexpr (sizeof...(Ts) == 0) return find(test1, start);
    else {
      size_t pos1 = find(test1, start);
      size_t pos2 = FindAnyOfFrom(start, tests...);
      return std::min(pos1, pos2);
    }
  }

  template <typename T, typename... Ts>
  size_t String::FindAnyOf(T test1, Ts... tests) const {
    // If an offset is provided, use it.
    if constexpr (std::is_arithmetic_v<T>) {
      return FindAnyOfFrom(test1, std::forward<Ts>(tests)...);
    } else {
      return FindAnyOfFrom(0, test1, std::forward<Ts>(tests)...);
    }
  }

  // Find an whole identifier (same as find, but cannot have letter, digit or '_' before or after.) 
  size_t String::FindID(std::string target, size_t start, const Mode & mode) const
  {
    size_t pos = Find(target, start, mode);
    while (pos != npos) {
      bool before_ok = (pos == 0) || !is_idchar(Get(pos-1));
      size_t after_pos = pos+target.size();
      bool after_ok = (after_pos == size()) || !is_idchar(Get(after_pos));
      if (before_ok && after_ok) return pos;

      pos = Find(target, pos+target.size(), mode);
    }

    return npos;
  }


  /// Remove whitespace from the beginning or end of a string.
  emp::String & String::TrimWhitespace() {
    size_t start_count=0;
    while (start_count < size() && is_whitespace(Get(start_count))) ++start_count;
    if (start_count) erase(0, start_count);

    size_t new_size = size();
    while (new_size > 0 && is_whitespace(Get(new_size-1))) --new_size;
    resize(new_size);

    return *this;
  }

  /// Every time one or more whitespace characters appear replace them with a single space.
  emp::String & String::CompressWhitespace() {
    bool skip_whitespace = true;          // Remove whitespace from beginning of line.
    size_t pos = 0;

    for (const auto c : *this) {
      if (is_whitespace(c)) {          // This char is whitespace
        if (skip_whitespace) continue; // Already skipping...
        Get(pos++) = ' ';
        skip_whitespace = true;
      } else {  // Not whitespace
        Get(pos++) = c;
        skip_whitespace = false;
      }
    }

    if (pos && skip_whitespace) pos--;   // If the end of the line is whitespace, remove it.
    resize(pos);

    return *this;
  }

  /// Remove all instances of specified characters from file.
  emp::String & String::RemoveChars(const CharSet & chars) {
    size_t cur_pos = 0;
    for (const auto c : *this) {
      if (!chars.Has(c)) Get(cur_pos++) = c;
    }
    resize(cur_pos);
    return *this;
  }

  /// Make a string safe(r)
  emp::String & String::Slugify() {
    return ToLower().RemovePunctuation().CompressWhitespace().ReplaceChar(' ', '-');
  }


  // Pop functions...

  bool String::PopIf(char c) {
    if (size() && Get(0) == c) { erase(0,1); return true; }
    return false;
  }

  bool String::PopIf(String in) {
    if (HasPrefix(in)) { PopFixed(in.size()); return true; }
    return false;
  }

  /// Pop the entire string.
  emp::String String::PopAll() {
    std::string out;
    swap(out);
    return out;
  }

  /// Pop a segment from the beginning of a string as another string, shortening original.
  emp::String String::PopFixed(std::size_t end_pos, size_t delim_size)
  {
    if (!end_pos) return "";                  // Not popping anything!
    if (end_pos >= size()) return PopAll();   // Popping everything!

    emp::String out = substr(0, end_pos);     // Copy up to the deliminator for ouput
    erase(0, end_pos + delim_size);       // Delete output string AND delimiter
    return out;
  }

  /// Remove a prefix of the string (up to a specified delimeter) and return it.  If the
  /// delimeter is not found, return the entire string and clear it.
  emp::String String::Pop(CharSet chars, const Mode & mode) {
    size_t pop_end = Find(chars, 0, mode);
    size_t delim_size = pop_end == npos ? 0 : 1;
    return PopFixed(pop_end, delim_size);
  }

  /// Remove a prefix of the string (up to a specified delimeter) and return it.  If the
  /// delimeter is not found, return the entire string and clear it.
  emp::String String::PopTo(std::string delim, const Mode & mode) {
    return PopFixed(Find(delim, 0, mode), delim.size());
  }

  emp::String String::PopWord(const Mode & mode) { return Pop(" \n\t\r", mode); }
  emp::String String::PopLine(const Mode & mode) { return Pop("\n", mode); }

  emp::String String::PopQuote(const Mode & mode) {
    if (!mode.IsQuote(Get(0))) return emp::String();
    const size_t end_pos = FindQuoteMatch(0);
    return (end_pos==std::string::npos) ? "" : PopFixed(end_pos+1);
  }

  emp::String String::PopParen(const Mode & mode) {
    const size_t end_pos = FindParenMatch(0, mode);
    return (end_pos==std::string::npos) ? "" : PopFixed(end_pos+1);
  }

  size_t String::PopUInt() {
    size_t uint_size = 0;
    while (uint_size < size() && isdigit(Get(uint_size))) ++uint_size;
    std::string out_uint = PopFixed(uint_size);
    return std::stoull(out_uint);
  }

  /// @brief Cut up a string based on the provided delimiter; fill them in to the provided vector.
  /// @param out_set destination vector
  /// @param delim delimiter to split on (default: ",")
  /// @param mode identify quotes and parens that should be kept together.
  /// @param trim_whitespace Should whitespace around delim or assign be ignored? (default: true)
  void String::Slice(
    emp::vector<emp::String> & out_set, std::string delim, const Mode & mode, bool trim_whitespace
  ) const {
    if (empty()) return; // Nothing to set!

    size_t start_pos = 0;
    size_t found_pos = Find(delim, 0, mode);
    while (found_pos < size()) {
      out_set.push_back( GetRange(start_pos, found_pos) );
      if (trim_whitespace) out_set.back().TrimWhitespace();
      start_pos = found_pos+delim.size();
      found_pos = Find(delim, found_pos+1, mode);
    }
    out_set.push_back( GetRange(start_pos, found_pos) );
  }


  /// @brief Slice a String on a delimiter; return a vector of results.
  /// @note May be less efficient, but easier than other version of Slice()
  /// @param delim delimiter to split on (default: ",")
  /// @param mode identify quotes and parens that should be kept together.
  /// @param trim_whitespace Should whitespace around delim or assign be ignored? (default: true)
  emp::vector<emp::String> String::Slice(
    std::string delim, const Mode & mode, bool trim_whitespace
  ) const {
    emp::vector<emp::String> result;
    Slice(result, delim, mode, trim_whitespace);
    return result;
  }

  /// @brief Fill vector out_set of string_views based on the provided delimiter.
  /// @param out_set destination vector
  /// @param delim delimiter to split on (default: ",")
  /// @param mode identify quotes and parens that should be kept together.
  void String::ViewSlices(
    emp::vector<std::string_view> & out_set, std::string delim, const Mode & mode
  ) const {
    out_set.resize(0);
    if (empty()) return; // Nothing to set!

    size_t start_pos = 0;
    size_t found_pos = Find(delim, 0, mode);
    while (found_pos < size()) {
      out_set.push_back( ViewRange(start_pos, found_pos) );
      start_pos = found_pos+delim.size();
      found_pos = Find(delim, found_pos+1, mode);
    }
    out_set.push_back( ViewRange(start_pos, found_pos) );
  }

  /// @brief Generate vector of string_views based on the provided delimiter.
  /// @param delim delimiter to split on (default: ",")
  /// @param mode identify quotes and parens that should be kept together.
  [[nodiscard]] emp::vector<std::string_view> String::ViewSlices(
    std::string delim, const Mode & mode
  ) const {
    emp::vector<std::string_view> result;
    ViewSlices(result, delim, mode);
    return result;
  }
  

  /// @brief Slice a string and treat each section as an assignment; place results in provided map.
  /// @param delim delimiter to split on (default ',')
  /// @param assign_op separator for left and right side of assignment (default: "=")
  /// @param mode identify quotes and parens that should be kept together.
  /// @param trim_whitespace Should whitespace around delim or assign be ignored? (default: true)
  void String::SliceAssign(
    std::map<emp::String,emp::String> & result_map,
    std::string delim, std::string assign_op,
    const Mode & mode, bool trim_whitespace
  ) const {
    auto assign_set = Slice(delim, mode, false);
    for (auto setting : assign_set) {
      if (setting.OnlyWhitespace()) continue; // Skip blank settings (especially at the end).

      // Remove any extra spaces around parsed values.
      emp::String var_name = setting.PopTo(assign_op);
      if (trim_whitespace) {
        var_name.TrimWhitespace();
        setting.TrimWhitespace();
      }
      if (setting.empty()) {
        std::stringstream msg;
        msg << "No assignment found in slice_assign() for: " << var_name;
        emp::notify::Exception("emp::string_utils::slice_assign::missing_assign",
                               msg.str(), var_name);                               
      }
      result_map[var_name] = setting;
    }
  }


  /// @brief Slice a string and treat each section as an assignment; fill out a map and return it.
  /// @param delim delimiter to split on (default ',')
  /// @param assign_op separator for left and right side of assignment (default: "=")
  /// @param mode identify quotes and parens that should be kept together.
  /// @param trim_whitespace Should whitespace around delim or assign be ignored? (default: true)
  std::map<emp::String,emp::String> String::SliceAssign(
    std::string delim, std::string assign_op,
    const Mode & mode, bool trim_whitespace
  ) const {
    std::map<emp::String,emp::String> result_map;
    SliceAssign(result_map, delim, assign_op, mode, trim_whitespace);
    return result_map;
  }


  /// Find any instances of ${X} and replace with dictionary lookup of X.
  template <typename MAP_T>
  emp::String & String::ReplaceVars(const MAP_T & var_map, emp::String symbol, const Mode & mode) {
    for (size_t pos = Find(symbol, 0, mode);
         pos < size()-3;             // Need room for a replacement tag.
         pos = Find(symbol, pos+symbol.size(), mode)) {
      const size_t symbol_end = pos+symbol.size();
      if (HasAt(symbol, symbol_end)) {   // Compress two symbols (e.g., "$$") into one (e.g. "$")
        erase(pos, symbol.size());
        continue;
      }
      if (Get(symbol_end) != '{') continue; // Eval must be surrounded by braces.

      // If we made it this far, we have a starting match!
      size_t end_pos = FindParenMatch(symbol_end);
      if (end_pos == npos) {
        emp::notify::Exception("emp::string_utils::replace_vars::missing_close",
                               "No close brace found in string_utils::replace_vars()",
                               *this);
        break;
      }

      std::string key = GetRange(symbol_end+1, end_pos);
      auto replacement_it = var_map.find(key);
      if (replacement_it == var_map.end()) {
        emp::notify::Exception("emp::string_utils::replace_vars::missing_var",
                               MakeString("Lookup variable not found in var_map (key=", key, ")"),
                               key);
        break;
      }
      ReplaceRange(pos, end_pos+1, replacement_it->second);   // Put into place.
      pos += replacement_it->second.size();                   // Continue at the next position...
    }

    return *this;
  }

  /// @brief Find any instance of MACRO_NAME(ARGS) and replace it with fun(ARGS).
  /// @param start_str Initial sequence of macro to look for; for example "REPLACE("
  /// @param end_str   Sequence that ends the macro; for example ")"
  /// @param macro_fun Function to call with contents of macro.  Params are macro_args (string), line_num (size_t), and hit_num (size_t)
  /// @param mode What values should we skip as quotes or parens?
  /// @return This string object, post processing.
  template <typename FUN_T>
  emp::String & String::ReplaceMacro(
    std::string start_str,
    std::string end_str,
    FUN_T && macro_fun,
    const String::Mode & mode
  ) {
    // We need to identify the comparator and divide up arguments in macro.
    size_t macro_count = 0;     // Count of the number of hits for this macro.
    size_t line_num = 0;        // Line number where current macro hit was found.
    size_t prev_pos = 0;
    for (size_t macro_pos = Find(start_str, 0, mode);
         macro_pos != npos;
         macro_pos = Find(start_str, macro_pos+1, mode))
    {
      // Make sure we're not just extending a previous identifier.
      if (macro_pos && is_idchar(Get(macro_pos-1))) continue;

      line_num += Count('\n', prev_pos, macro_pos);  // Count lines leading up to this macro.

      // Isolate this macro instance and call the conversion function.
      size_t end_pos = Find(end_str, macro_pos+start_str.size(), mode);
      const std::string macro_body = GetRange(macro_pos+start_str.size(), end_pos);

      std::string new_str = macro_fun(macro_body, line_num, macro_count);
      ReplaceRange(macro_pos, end_pos+end_str.size(), new_str);
      prev_pos = macro_pos;
      macro_count++;
    }

    return *this;
  }

  /// Test if an input string is properly formatted as a literal character.
  bool String::IsLiteralChar() const {
    // @CAO: Need to add special types of numerical escapes here (e.g., ascii codes!)
    // Must contain a representation of a character, surrounded by single quotes.
    if (size() < 3 || size() > 4) return false;
    if (Get(0) != '\'' || back() != '\'') return false;

    // If there's only a single character in the quotes, it's USUALLY legal.
    if (size() == 3) return Get(1) != '\'' && Get(1) != '\\';

    // Multiple chars must begin with a backslash.
    if (Get(1) != '\\') return false;

    CharSet chars("nrt0\\\'");
    return chars.Has(Get(2));
  }

  /// Test if an input string is properly formatted as a literal string.
  bool String::IsLiteralString(const std::string & quote_marks) const {
    // Must begin and end with proper quote marks.
    if (size() < 2 || !is_one_of(Get(0), quote_marks) || back() != Get(0)) return false;

    // Are all of the characters valid?
    for (size_t pos = 1; pos < size() - 1; pos++) {
      if (Get(pos) == Get(0)) return false;          // Cannot have a raw quote in the middle.
      if (Get(pos) == '\\') {                        // Allow escaped characters...
        if (pos == size()-2) return false;       // Backslash must have char to escape.
        pos++;                                       // Skip past escaped character.
        if (!is_escape_code(Get(pos))) return false; // Illegal escaped character.
      }
    }

    // @CAO: Need to check special types of numerical escapes (e.g., ascii codes!)

    return true; // No issues found; mark as correct.
  }

  /// Test if an input string is properly formatted as a literal string.
  std::string String::DiagnoseLiteralString(const std::string & quote_marks) const {
    // A literal string must begin and end with a double quote and contain only valid characters.
    if (size() < 2) return "Too short!";
    if (!is_one_of(Get(0), quote_marks)) return "Must begin an end in quotes.";
    if (back() != Get(0)) return "Must have begin and end quotes that match.";

    // Are all of the characters valid?
    for (size_t pos = 1; pos < size() - 1; pos++) {
      if (Get(pos) == Get(0)) return "Has a floating quote.";
      if (Get(pos) == '\\') {
        if (pos == size()-2) return "Cannot escape the final quote.";  // Backslash must have char to escape.
        pos++;
        if (!is_escape_code(Get(pos))) return "Unknown escape charater.";
      }
    }

    // @CAO: Need to check special types of numerical escapes (e.g., ascii codes!)

    return "Good!";
  }




  // -------------------------------------------------------------
  //
  // ------------- Stand-alone function definitions --------------
  //
  // -------------------------------------------------------------

  // ------ External function overrides ------

  // std::ostream & operator<<(std::ostream & os, const emp::String & in) {
  //   return os << in.cpp_str();
  // }

  // std::istream & operator>>(std::istream & is, emp::String & in) {
  //   return is >> in.cpp_str();
  // }


  template <typename... Ts>
  emp::String MakeString(Ts... args) {
    return String::Make(std::forward<Ts>(args)...);
  }

  emp::String MakeEscaped(char c) {
    // If we just append as a normal character, do so!
    if ( (c >= 40 && c < 91) || (c > 96 && c < 127)) return emp::MakeString(c);
    switch (c) {
    case '\0': return "\\0";
    case 1: return "\\001";
    case 2: return "\\002";
    case 3: return "\\003";
    case 4: return "\\004";
    case 5: return "\\005";
    case 6: return "\\006";
    case '\a': return "\\a";  // case  7 (audible bell)
    case '\b': return "\\b";  // case  8 (backspace)
    case '\t': return "\\t";  // case  9 (tab)
    case '\n': return "\\n";  // case 10 (newline)
    case '\v': return "\\v";  // case 11 (vertical tab)
    case '\f': return "\\f";  // case 12 (form feed - new page)
    case '\r': return "\\r";  // case 13 (carriage return)
    case 14: return "\\016";
    case 15: return "\\017";
    case 16: return "\\020";
    case 17: return "\\021";
    case 18: return "\\022";
    case 19: return "\\023";
    case 20: return "\\024";
    case 21: return "\\025";
    case 22: return "\\026";
    case 23: return "\\027";
    case 24: return "\\030";
    case 25: return "\\031";
    case 26: return "\\032";
    case 27: return "\\033";  // case 27 (ESC), sometimes \e
    case 28: return "\\034";
    case 29: return "\\035";
    case 30: return "\\036";
    case 31: return "\\037";

    case '\"': return "\\\"";  // case 34
    case '\'': return "\\\'";  // case 39
    case '\\': return "\\\\";  // case 92
    case 127: return "\\177";  // (delete)
    }

    return emp::MakeString(c);
  }
  
  emp::String MakeEscaped(const emp::String & in) {
    return emp::String(in, [](char c){ return MakeEscaped(c); });
  }

  /// Take a string and replace reserved HTML characters with character entities
  emp::String MakeWebSafe(const emp::String & in) {
    emp::String out;
    out.reserve(in.size());
    for (char c : in) {
      switch (c) {
      case '&': out += "&amp"; break;
      case '<': out += "&lt"; break;
      case '>': out += "&gt"; break;
      case '\'': out += "&apos"; break;
      case '"': out += "&quot"; break;
      default: out += c;
      }
    }
    return out;
  }

  /// Take a char and convert it to a C++-style literal.
  [[nodiscard]] emp::String MakeLiteral(char value) {
    std::stringstream ss;
    ss << "'" << MakeEscaped(value) << "'";
    return ss.str();
  }

  /// Take a string or iterable and convert it to a C++-style literal.
  // This is the version for string. The version for an iterable is below.
  [[nodiscard]] emp::String MakeLiteral(const emp::String & value) {
    // Add quotes to the ends and convert each character.
    std::stringstream ss;
    ss << "\"";
    for (char c : value) {
      ss << MakeEscaped(c);
    }
    ss << "\"";
    return ss.str();
  }

  #ifndef DOXYGEN_SHOULD_SKIP_THIS

  /// Take a value and convert it to a C++-style literal.
  template <typename T> [[nodiscard]] emp::String MakeLiteral(const T & value) {
    std::stringstream ss;
    if constexpr (emp::IsIterable<T>::value) {
      ss << "{ ";
      for (auto it = std::begin( value ); it != std::end( value ); ++it) {
        if (it != std::begin( value )) ss << ",";
        ss << MakeLiteral< std::decay_t<decltype(*it)> >( *it );
      }
      ss << " }";
    }
    else ss << value;
    return ss.str();
  }

  #endif

  /// Convert a literal character representation to an actual string.
  /// (i.e., 'A', ';', or '\n')
  [[nodiscard]] char MakeFromLiteral_Char(const emp::String & value) {
    emp_assert(value.IsLiteralChar());
    // Given the assert, we can assume the string DOES contain a literal representation,
    // and we just need to convert it.

    if (value.size() == 3) return value[1];
    if (value.size() == 4) return ToEscapeChar(value[2]);

    return '\0'; // Error!
  }


  /// Convert a literal string representation to an actual string.
  [[nodiscard]] emp::String MakeFromLiteral_String(const emp::String & value) {
  /// Convert a literal string representation to an actual string.
    emp_assert(value.IsLiteralString(), value, value.DiagnoseLiteralString());
    // Given the assert, we can assume string DOES contain a literal string representation.

    emp::String out_string;
    out_string.reserve(value.size()-2);  // Make a guess on final size.

    for (size_t pos = 1; pos < value.size() - 1; pos++) {
      // If we don't have an escaped character, just move it over.
      if (value[pos] != '\\') out_string.push_back(value[pos]);
      else out_string.push_back(ToEscapeChar(value[++pos]));
    }

    return out_string;
  }

  [[nodiscard]] emp::String MakeFromLiteral(const emp::String & value) {
    if (value.size() == 0) return "";
    if (value[0] == '\'') return MakeString(MakeFromLiteral_Char(value));
    if (value[0] == '"') return MakeFromLiteral_String(value);
    // @CAO Add conversion from numerical literals, and especially octal (0-), binary (0b-), and hex (0x-)
    return value;
  }

  /// Convert a string to all uppercase.
  [[nodiscard]] emp::String MakeUpper(const emp::String & value) {
    return emp::String(value, [](char c){ return std::toupper(c); });
  }

  /// Convert a string to all lowercase.
  [[nodiscard]] emp::String MakeLower(const emp::String & value) {
    return emp::String(value, [](char c){ return std::tolower(c); });
  }

  /// Make first letter of each word upper case
  [[nodiscard]] emp::String MakeTitleCase(emp::String value) {
    constexpr int char_shift = 'a' - 'A';
    bool next_upper = true;
    for (size_t i = 0; i < value.size(); i++) {
      if (next_upper && value[i] >= 'a' && value[i] <= 'z') {
        value[i] = (char) (value[i] - char_shift);
      } else if (!next_upper && value[i] >= 'A' && value[i] <= 'Z') {
        value[i] = (char) (value[i] + char_shift);
      }

      next_upper = (value[i] == ' ');
    }
    return value;
  }

  /// Convert an integer to a roman numeral string.
  [[nodiscard]] emp::String MakeRoman(int val) {
    emp::String out;
    if (val < 0) { out += "-"; val *= -1; }

    // If out of bounds, divide up into sections of 1000 each.
    if (val > 3999) { out += MakeRoman(val/1000); val %= 1000; out += '|'; }

    // Loop through dealing with the rest of the number.
    while (val > 0) {
      if (val >= 1000) { out += "M";  val -= 1000; }
      else if (val >= 900)  { out += "CM"; val -= 900; }
      else if (val >= 500)  { out += "D";  val -= 500; }
      else if (val >= 400)  { out += "CD"; val -= 400; }
      else if (val >= 100)  { out += "C";  val -= 100; }
      else if (val >= 90)   { out += "XC"; val -= 90; }
      else if (val >= 50)   { out += "L";  val -= 50; }
      else if (val >= 40)   { out += "XL"; val -= 40; }
      else if (val >= 10)   { out += "X";  val -= 10; }
      else if (val == 9)    { out += "IX"; val -= 9; }
      else if (val >= 5)    { out += "V";  val -= 5; }
      else if (val == 4)    { out += "IV"; val -= 4; }
      else                  { out += "I";  val -= 1; }
    }

    return out;
  }

  template <typename CONTAINER_T>
  emp::String MakeEnglishList(const CONTAINER_T & container) {
    if (container.size() == 0) return "";
    if (container.size() == 1) return to_string(container.front());

    auto it = container.begin();
    if (container.size() == 2) return to_string(*it, " and ", *(it+1));

    auto last_it = container.end() - 1;
    String out = MakeString(*it);
    ++it;
    while (it != last_it) {
      out += ',';
      out += MakeString(*it);
      ++it;
    }
    out += MakeString(", and ", *it);

    return out;
  }

  /// Apply sprintf-like formatting to a string.
  /// See https://en.cppreference.com/w/cpp/io/c/fprintf.
  /// Adapted from https://stackoverflow.com/a/26221725.
  template<typename... Args>
  emp::String MakeFormatted(const std::string& format, Args... args) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-security"

    // Extra space for '\0'
    const size_t size = static_cast<size_t>(std::snprintf(nullptr, 0, format.c_str(), args...) + 1);

    emp::vector<char> buf(size);
    std::snprintf(buf.data(), size, format.c_str(), args...);

     // We don't want the '\0' inside
    return emp::String( buf.data(), buf.data() + size - 1 );

    #pragma GCC diagnostic pop
  }

  /// Concatenate n copies of a string.
  emp::String MakeRepeat(emp::String base, size_t n ) {
    emp::String out;
    out.reserve(n * base.size());
    for (size_t i=0; i < n; ++i) out += base;
    return out;
  }

  /// This function returns values from a container as a single string separated
  /// by a given delimeter and with optional surrounding strings.
  /// @param container is any standard-interface container holding objects to be joined.
  /// @param join_str optional delimeter
  /// @param open string to place before each string (e.g., "[" or "'")
  /// @param close string to place after each string (e.g., "]" or "'")
  /// @return merged string of all values
  template <typename CONTAINER_T>
  emp::String Join(const CONTAINER_T & container, std::string join_str,
                      std::string open, std::string close) {
    if (container.size() == 0) return "";
    if (container.size() == 1) return MakeString(open, container.front(), close);

    std::stringstream out;
    for (auto it = container.begin(); it != container.end(); ++it) {
      if (it != container.begin()) out << join_str;
      out << open << MakeString(*it) << close;
    }

    return out.str();
  }

}

// ---------------------- Implementations to work with standard library ----------------------

// namespace std {
//   /// Hash function to allow String to be used with unordered maps and sets (must be in std).
//   template <>
//   struct hash<emp::String> {
//     std::size_t operator()(const emp::String & str) const {
//       return std::hash<std::string>()(str.cpp_str()); 
//     }
//   };
// }


#endif // #ifndef EMP_TOOLS_STRING_HPP_INCLUDE
