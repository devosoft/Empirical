/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023.
 *
 *  @file String.hpp
 *  @brief Simple class to facilitate string manipulations
 *  @note Status: ALPHA
 *
 *    -- CLASSIFICATION --
 *    size_t count(const std::string & str, char c)    - Count the occurrences of c in str.
 *    bool is_literal_char(const std::string & value)
 *    bool is_literal_string(const std::string & value, type="\"")
 *    bool is_composed_of(const std::string & test_str, const std::string & char_set)
 *    bool is_digits(const std::string & test_str)
 *    bool is_number(const std::string & test_str)
 *    bool is_alphanumeric(const std::string & test_str)
 *    bool is_whitespace(const std::string & test_str)
 *    bool has_whitespace(const std::string & test_str)
 *    bool has_nonwhitespace(const std::string & test_str)
 *    bool has_upper_letter(const std::string & test_str)
 *    bool has_lower_letter(const std::string & test_str)
 *    bool has_letter(const std::string & test_str)
 *    bool has_digit(const std::string & test_str)
 *    bool has_alphanumeric(const std::string & test_str)
 *    bool has_one_of(const std::string & test_str, const std::string & char_set)
 *    bool has_prefix(const std::string & in_string, const std::string & prefix)
 *    bool has_char_at(const std::string & str, char c, size_t pos)
 *    bool has_one_of_at(const std::string & str, const std::string & opts, size_t pos)
 *    bool has_digit_at(const std::string & str, size_t pos)
 *    bool has_letter_at(const std::string & str, size_t pos)
 *
 *    -- SEARCHING --
 *    size_t find_quote_match(std::string_view in_string, size_t start_pos=0)
 *    size_t find_paren_match(std::string_view in_string, size_t start_pos=0,
 *                            bool skip_quotes=true)
 *    size_t find_match(std::string_view in_string, size_t start_pos=0, bool skip_quotes=true)
 *    size_t find(std::string_view in_string, std::string target, size_t start_pos,
 *                bool skip_quotes=false, bool skip_parens=false, bool skip_braces=false,
 *                bool skip_brackets=false)
 *    void find_all(std::string_view in_string, char target, emp::vector<size_t> & results,
 *                bool skip_quotes=false, bool skip_parens=false, bool skip_braces=false,
 *                bool skip_brackets=false)
 *    emp::vector<size_t> find_all(std::string_view in_string, char target, bool skip_quotes=false
 *      bool skip_parens=false, bool skip_braces=false, bool skip_brackets=false)
 *    size_t find_any_of(const std::string & test_str, std::string... tests)
 *    size_t find_any_of(const std::string & test_str, size_t start_pos, std::string... tests)
 *    size_t find_id(std::string_view in_string, std::string target, size_t start_pos,
 *      bool skip_quotes=true, bool skip_parens=false, bool skip_braces=false, bool skip_brackets=false)
 *    size_t find_non_whitespace(std::string_view in_string, size_t start_pos)
 *
 *    -- FORMATTING --
 *    std::string to_escaped_string(char value)
 *    std::string to_escaped_string(const std::string & value)
 *    std::string to_web_safe_string(const std::string & value)
 *    template<bool encode_space=false> std::string url_encode(const std::string &value)
 *    template<bool decode_plus=false> std::string url_decode(const std::string& str)
 *    std::string to_literal(...)
 *    char from_literal_char(const std::string & value)
 *    std::string from_literal_string(const std::string & value)
 *    std::string to_upper(std::string value)
 *    std::string to_lower(std::string value)
 *    std::string to_titlecase(std::string value)
 *    std::string to_roman_numeral(int val, const std::string & prefix="")
 *    void trim_whitespace(std::string & in_string)
 *    void compress_whitespace(std::string & in_string)
 *    void remove_whitespace(std::string & in_string)
 *    void remove_punctuation(std::string & in_string)
 *    std::string slugify(const std::string & in_string)
 *    std::string combine_strings(const string_vec_t & strings, std::string spacer=" ")
 *    std::string to_english_list(const string_vec_t & strings)
 *    string_vec_t transform_strings(const string_vec_t & in_strings,
 *                                   std::function<std::string(const std::string &)> fun)
 *    string_vec_t quote_strings(const string_vec_t & in_strings, const std::string quote="'")
 *    string_vec_t quote_strings(const string_vec_t & in_strings, const std::string open_quote,
 *                               const std::string close_quote)
 *    to_quoted_list(const string_vec_t & in_strings, const std::string quote="'")
 *    std::string format_string( const std::string& format, Args... args )
 *    std::string replace_vars( const std::string& base, const MAP_T & map )
 *    std::string replace_macro( const std:string &in_string, std::string macro_name,
 *                               string(string) fun, bool skip_quotes=true)
 *
 *    -- EXTRACTIONS and CROPPING --
 *    void remove_chars(std::string & in_string, std::string chars)
 *    bool string_pop_if_char(std::string & in_string, char c)
 *    std::string string_pop_fixed(std::string & in_string, std::size_t end_pos,
 *                                 size_t delim_size=0)
 *    std::string string_get_range(const std::string & in_string, std::size_t start_pos,
 *                                 std::size_t end_pos)
 *    std::string string_pop(std::string & in_string, const char delim=' ')
 *    std::string string_get(const std::string & in_string, const char delim=' ',
 *                           size_t start_pos=0)
 *    std::string string_pop(std::string & in_string, const std::string & delim_set)
 *    std::string string_get_to(const std::string & in_string, const std::string & delim=" ")
 *    std::string string_pop_to(std::string & in_string, const std::string & delim=" ",
 *      size_t start_pos=0, bool skip_quotes=false, bool skip_parens=false,
 *      bool skip_braces=false, bool skip_brackets=false)
 *    std::string string_get(const std::string & in_string, const std::string & delim_set,
 *                           size_t start_pos=0)
 *    std::string string_pop_word(std::string & in_string)
 *    std::string string_get_word(const std::string & in_string, size_t start_pos=0)
 *    std::string string_pop_line(std::string & in_string)
 *    std::string string_get_line(const std::string & in_string, size_t start_pos=0)
 *    std::string string_pop_quote(std::string & in_string)
 *    size_t string_pop_uint(std::string & in_string)
 *    size_t string_get_uint(const std::string & in_string)
 *    std::string left_justify(std::string & in_string)
 *    void right_justify(std::string & in_string)
 *    void justify(std::string & in_string)
 *
 *    -- STRING_VIEW HELPERS --
 *    std::string_view view_string(const std::string_view & str)
 *    std::string_view view_string(const std::string_view & str, size_t start)
 *    std::string_view view_string(const std::string_view & str, size_t start, size_t npos)
 *    std::string_view view_string_front(const std::string_view & str, size_t npos)
 *    std::string_view view_string_back(const std::string_view & str, size_t npos)
 *    std::string_view view_string_range(const std::string_view & str, size_t start, size_t end)
 *    std::string_view view_string_to(const std::string_view & in_string, const char delim,
 *                                    size_t start_pos=0)
 *
 *    -- OTHER MANIPULATIONS --
 *    std::string pad_front(const std::string & in_string, char padding, size_t target_size)
 *    std::string pad_back(const std::string & in_string, char padding, size_t target_size)
 *    std::string repeat(const std::string& value, const size_t n)
 *    void slice(const std::string_view & in_string, emp::vector<std::string> & out_set,
 *               const char delim='\n', [size_t max_split], bool keep_quotes=false,
 *               bool keep_parens=false, bool keep_braces=false, bool kee_brackets=false)
 *    emp::vector<std::string>
 *      slice(const std::string_view & in_string, const char delim='\n', [size_t max_split],
 *            bool keep_quotes=false, bool keep_parens=false, bool keep_braces=false,
 *            bool keep_brackets=false)
 *    void view_slices(const std::string_view & in_string, emp::vector<std::string_view> & out_set,
 *                     char delim='\n', bool keep_quotes=false, bool keep_parens=false,
 *                     bool keep_braces=false, bool keep_brackets=false)
 *    emp::vector<std::string_view>
 *      view_slices(const std::string_view & in_string, char delim='\n',
 *                  bool keep_quotes=false, bool keep_parens=false,
 *                  bool keep_braces=false, bool keep_brackets=false)
 *    std::map<std::string, std::string>
 *      slice_assign(const std::string_view & in_string, const char delim=',',
 *                   std::string assign="=", [size_t max_split], bool trim_whitespace=true,
 *                   bool keep_quotes=true, bool keep_parens=true, bool keep_braces=true,
 *                   bool keep_brackets=true)
 *    emp::vector<std::string_view> ViewCSV(const std::string_view & csv_line)
 *    std::string_view
 *      ViewNestedBlock(std::string_view str, const std::string symbols="()", size_t start=0)
 *    std::string to_string(...)
 *    void from_string(const std::string & str, ...)
 *    std::string join(const emp::vector<T> & v, std::string join_str)
 *
 *    -- ANSI TOOLS --
 *    char ANSI_ESC()
 *    std::string ANSI_Reset()
 *    std::string ANSI_Bold()
 *    std::string ANSI_Faint()
 *    std::string ANSI_Italic()
 *    std::string ANSI_Underline()
 *    std::string ANSI_SlowBlink()
 *    std::string ANSI_Blink()
 *    std::string ANSI_Reverse()
 *    std::string ANSI_Strike()
 *    std::string ANSI_NoBold()
 *    std::string ANSI_NoItalic()
 *    std::string ANSI_NoUnderline()
 *    std::string ANSI_NoBlink()
 *    std::string ANSI_NoReverse()
 *    std::string ANSI_Black()
 *    std::string ANSI_Red()
 *    std::string ANSI_Green()
 *    std::string ANSI_Yellow()
 *    std::string ANSI_Blue()
 *    std::string ANSI_Magenta()
 *    std::string ANSI_Cyan()
 *    std::string ANSI_White()
 *    std::string ANSI_DefaultColor()
 *    std::string ANSI_BlackBG()
 *    std::string ANSI_RedBG()
 *    std::string ANSI_GreenBG()
 *    std::string ANSI_YellowBG()
 *    std::string ANSI_BlueBG()
 *    std::string ANSI_MagentaBG()
 *    std::string ANSI_CyanBG()
 *    std::string ANSI_WhiteBG()
 *    std::string ANSI_DefaultBGColor()
 *    std::string ANSI_BrightBlack()
 *    std::string ANSI_BrightRed()
 *    std::string ANSI_BrightGreen()
 *    std::string ANSI_BrightYellow()
 *    std::string ANSI_BrightBlue()
 *    std::string ANSI_BrightMagenta()
 *    std::string ANSI_BrightCyan()
 *    std::string ANSI_BrightWhite()
 *    std::string ANSI_BrightBlackBG()
 *    std::string ANSI_BrightRedBG()
 *    std::string ANSI_BrightGreenBG()
 *    std::string ANSI_BrightYellowBG()
 *    std::string ANSI_BrightBlueBG()
 *    std::string ANSI_BrightMagentaBG()
 *    std::string ANSI_BrightCyanBG()
 *    std::string ANSI_BrightWhiteBG()
 *
 *    to_ansi_bold(const std::string & _in)
 *    to_ansi_italic(const std::string & _in)
 *    to_ansi_underline(const std::string & _in)
 *    to_ansi_blink(const std::string & _in)
 *    to_ansi_reverse(const std::string & _in)
 *
 *
 *
 *  @todo Make constexpr
 *  @todo Make handle non-char strings (i.e., use CharT template parameter)
 *  @todo Make handle allocators
 *  @todo Make work with stringviews
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

  // Some stand-alone functions.
  [[nodiscard]] std::string MakeEscaped(char c);
  [[nodiscard]] std::string MakeEscaped(const std::string & in);
  [[nodiscard]] std::string MakeWebSafe(const std::string & in);
  [[nodiscard]] std::string MakeLiteral(char value);
  [[nodiscard]] std::string MakeLiteral(char value);
  template <typename T>
  [[nodiscard]] typename std::enable_if<!emp::IsIterable<T>::value, std::string>::type MakeLiteral(const T & value);
  template <typename T>
  [[nodiscard]] typename std::enable_if<emp::IsIterable<T>::value, std::string>::type MakeLiteral(const T & value);
  [[nodiscard]] std::string MakeUpper(const std::string & in);
  [[nodiscard]] std::string MakeLower(const std::string & in);
  [[nodiscard]] std::string MakeTitleCase(const std::string & in);
  [[nodiscard]] std::string MakeRoman(int val);

  class String {
  private:
    std::string str;   // The main string that we are manipulating.

    enum Mask {
      USE_QUOTE_SINGLE =  1,
      USE_QUOTE_DOUBLE =  2,
      USE_QUOTE_BACK   =  4,
      USE_PAREN_ROUND  =  8,    // Parentheses
      USE_PAREN_SQUARE = 0x10,  // Brackets
      USE_PAREN_CURLY  = 0x20,  // Braces
      USE_PAREN_ANGLE  = 0x40,  // Chevrons
      USE_PAREN_QUOTES = 0x80   // Forward/back single quote
    };

    struct Mode {
      uint8_t val = USE_QUOTE_SINGLE + USE_QUOTE_DOUBLE +
                    USE_PAREN_ROUND + USE_PAREN_SQUARE + USE_PAREN_CURLY;
    } mode;

    // ------ HELPER FUNCTIONS ------

    String & _ChangeMode(Mask mask, bool use) {
      if (use) mode.val |= mask;
      else mode.val &= ~mask;
      return *this;      
    }

    bool IsQuote(char c) const {
      switch (c) {
        case '\'': return mode.val & USE_QUOTE_SINGLE;
        case '"': return mode.val & USE_QUOTE_DOUBLE;
        case '`': return mode.val & USE_QUOTE_BACK;
      }
      return false;
    }

    bool IsParen(char c) const {
      switch (c) {
        case '(': return mode.val & USE_PAREN_ROUND;
        case '[': return mode.val & USE_PAREN_SQUARE;
        case '{': return mode.val & USE_PAREN_CURLY;
        case '<': return mode.val & USE_PAREN_ANGLE;
        case '`': return mode.val & USE_PAREN_QUOTES;
      }
      return false;
    }

    static char GetMatch(char c) {
      switch (c) {
        case '`': return '\'';
        case '(': return ')';
        case '[': return ']';
        case '{': return '}';
        case '<': return '>';
      }
      return '\0';
    }

    template <typename FUN_T>
    std::string _AsOutput(FUN_T fun) {
      std::string out_str;
      std::swap(str,out_str);  // Backup current string value.
      fun();                   // Run the internal function 
      std::swap(str,out_str);  // Swap result back into the output (restoring local backup)
      return out_str;
    }

    template <typename FUN_T>
    String & _StrAsInput(FUN_T fun) {
      std::string backup_str;
      std::swap(str,backup_str);  // Backup current string value.
      fun(backup_str);            // Run the internal function on original string.
      return *this;
    }

    inline void _AssertPos(size_t pos) const { emp_assert(pos < str.size(), pos, str.size()); }

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

    String() = default;
    String(const String &) = default;
    String(String &&) = default;
    String(const std::string & _in) : str(_in) { }
    String(std::string && _in) : str(std::move(_in)) { }
    String(const std::string & _in, Mode _mode) : str(_in), mode(_mode) { }
    String(std::string && _in, Mode _mode) : str(std::move(_in)), mode(_mode) { }
    String(const char * _in) : str(_in) { }
    String(size_t count, char _in) : str(count, _in) { }
    String(std::initializer_list<char> _in) : str(_in) { }
    String(const String & _in, size_t start, size_t count=npos)
      : str(_in.str, start, count), mode(_in.mode) { }
    String(const std::string & _in, size_t start, size_t count=npos)
      : str(_in, start, count) { }
    String(const char * _in, size_t count) : str(_in, count) { }
    template< class InputIt >
    String(InputIt first, InputIt last) : str(first, last) { }
    String(std::nullptr_t) = delete;

    // ------ IMPORT ALL std::string PROPERTIES AND EXPAND TO emp::String ------

    // Assignment operators
    String & operator=(const String &) = default;
    String & operator=(String &&) = default;
    String & operator=(const std::string & _in) { str = _in; }
    String & operator=(std::string && _in) { str = std::move(_in); }
    String & operator=(const char * _in) { str = _in; }
    String & operator=(char _in) { str = _in; }
    String & operator=(std::initializer_list<char> _in) { str = _in; }
    String & operator=( std::nullptr_t ) = delete;

    // Element Access
    char & operator[](size_t pos) { _AssertPos(pos); return str[pos]; }
    char operator[](size_t pos) const { _AssertPos(pos); return str[pos]; }
    char & front() { _AssertPos(0); return str.front(); }
    char front() const { _AssertPos(0); return str.front(); }
    char & back() { _AssertPos(0); return str.back(); }
    char back() const { _AssertPos(0); return str.back(); }
    char * data() { return str.data(); }
    const char * data() const { return str.data(); }
    const char * c_str() const { return str.c_str(); }
    const std::string & cpp_str() const { return str; }

    // Iterators
    iterator begin() { return str.begin(); }
    const_iterator begin() const { return str.begin(); }
    const_iterator cbegin() const { return str.cbegin(); }
    reverse_iterator rbegin() { return str.rbegin(); }
    const_reverse_iterator rbegin() const { return str.rbegin(); }
    const_reverse_iterator crbegin() const { return str.crbegin(); }

    iterator end() { return str.end(); }
    const_iterator end() const { return str.end(); }
    const_iterator cend() const { return str.cend(); }
    reverse_iterator rend() { return str.rend(); }
    const_reverse_iterator rend() const { return str.rend(); }
    const_reverse_iterator crend() const { return str.crend(); }

    // Capacity
    bool empty() const { return str.empty(); }
    size_t size() const { return str.size(); }
    size_t length() const { return str.length(); }
    size_t max_size() const { return str.max_size(); }
    void reserve(size_t new_cap) { str.reserve(new_cap); }
    void reserve() { str.reserve(); }
    size_t capacity() const { return str.capacity(); }
    void shrink_to_fit() { str.shrink_to_fit(); }
    
    // Operations
    void clear() noexcept { str.clear(); }

    String & insert(size_t index, const String & in) { str.insert(index, in.str); return *this; }
    String & insert(size_t index, const String & in, size_t pos, size_t count=npos)
      { str.insert(index, in.str, pos, count); return *this; }
    template <typename... ARG_Ts> String & insert(size_t index, ARG_Ts &&... args) {
      str.insert(index, std::forward<ARG_Ts>(args)...);
      return *this;
    }
    template <typename... ARG_Ts> String & insert(const_iterator pos, ARG_Ts &&... args) {
      return str.insert(pos, std::forward<ARG_Ts>(args)...);
    }

    String & erase(size_t index=0, size_t count=npos) { str.erase(index,count); return *this; }
    iterator erase(const_iterator pos) { return str.erase(pos); }
    iterator erase(const_iterator first, const_iterator last) { return str.erase(first, last); }

    void push_back(char c) { str.push_back(c); }
    void pop_back() { str.pop_back(); }

    String & append(const String & in) { str.append(in.str); return *this; }
    String & append(const String & in, size_t pos, size_t count)
      { str.append(in.str, pos, count); return *this; }
    template <typename... ARG_Ts> String & append(ARG_Ts &&... args) {
      str.append(std::forward<ARG_Ts>(args)...);
      return *this;
    }

    String & operator+=(const String & in) { str += in.str; return *this; }
    template <typename ARG_T> String & operator+=(ARG_T && arg) {
      str += std::forward<ARG_T>(arg);
      return *this;
    }

    int compare(const String & in) { return str.compare(in.str); }
    template <typename... ARG_Ts> int compare(ARG_Ts &&... args)
      { return str.compare(std::forward<ARG_Ts>(args)...); }

    bool starts_with(const String & in) const noexcept { return str.starts_with(in.str); }
    template <typename ARG_T> bool starts_with( ARG_T && in ) const noexcept
      { return str.starts_with(std::forward<ARG_T>(arg)); }

    bool ends_with(const String & in) const noexcept { return str.ends_with(in.str); }
    template <typename ARG_T> bool ends_with( ARG_T && in ) const noexcept
      { return str.ends_with(std::forward<ARG_T>(arg)); }

    bool contains(const String & in) const noexcept { return str.find(in.str) != npos; }
    template <typename ARG_T> bool contains( ARG_T && in ) const noexcept
      { return str.find(std::forward<ARG_T>(arg)) != npos; }

    template <typename... ARG_Ts> String & replace(ARG_Ts &&... args)
      { str.replace(std::forward<ARG_Ts>(args)...); return *this; }

    String substr(size_t pos=0, size_t count=npos ) const {
      return String(str.substr(pos, count), mode);
    }

    size_t copy(char * dest, size_t count, size_t pos=0) const { return str.copy(dest, count, pos); }

    void resize( size_t count, char c='\0') { str.resize(count, c); }

    void swap(String & other) { str.swap(other.str); std::swap(mode, other.mode); }

    // Searching
    size_t find(const std::string & str, size_t pos=0) const noexcept { return find(str,pos); }
    size_t find(const char* s, size_t pos=0) const { return str.find(s,pos); }
    size_t find(const char* s, size_t pos, size_t count) const { return str.find(s,pos,count); }
    size_t find(char c, size_t pos=0) const noexcept { return str.find(c,pos); }
    template < class SVIEW_T > size_t find(const SVIEW_T & sv, size_t pos=0) const
      { return str.find(sv,pos); }

    size_t rfind(const std::string & str, size_t pos=0) const noexcept { return rfind(str,pos); }
    size_t rfind(const char* s, size_t pos=0) const { return str.rfind(s,pos); }
    size_t rfind(const char* s, size_t pos, size_t count) const { return str.rfind(s,pos,count); }
    size_t rfind(char c, size_t pos=0) const noexcept { return str.rfind(c,pos); }
    template < class SVIEW_T > size_t rfind(const SVIEW_T & sv, size_t pos=0) const
      { return str.rfind(sv,pos); }

    size_t find_first_of(const std::string & str, size_t pos=0) const noexcept
      { return find_first_of(str,pos); }
    size_t find_first_of(const char* s, size_t pos=0) const { return str.find_first_of(s,pos); }
    size_t find_first_of(const char* s, size_t pos, size_t count) const
      { return str.find_first_of(s,pos,count); }
    size_t find_first_of(char c, size_t pos=0) const noexcept { return str.find_first_of(c,pos); }
    template < class SVIEW_T > size_t find_first_of(const SVIEW_T & sv, size_t pos=0) const
      { return str.find_first_of(sv,pos); }

    size_t find_first_not_of(const std::string & str, size_t pos=0) const noexcept
      { return find_first_not_of(str,pos); }
    size_t find_first_not_of(const char* s, size_t pos=0) const
      { return str.find_first_not_of(s,pos); }
    size_t find_first_not_of(const char* s, size_t pos, size_t count) const
      { return str.find_first_not_of(s,pos,count); }
    size_t find_first_not_of(char c, size_t pos=0) const noexcept
      { return str.find_first_not_of(c,pos); }
    template < class SVIEW_T > size_t find_first_not_of(const SVIEW_T & sv, size_t pos=0) const
      { return str.find_first_not_of(sv,pos); }

    size_t find_last_of(const std::string & str, size_t pos=0) const noexcept
      { return find_last_of(str,pos); }
    size_t find_last_of(const char* s, size_t pos=0) const { return str.find_last_of(s,pos); }
    size_t find_last_of(const char* s, size_t pos, size_t count) const
      { return str.find_last_of(s,pos,count); }
    size_t find_last_of(char c, size_t pos=0) const noexcept { return str.find_last_of(c,pos); }
    template < class SVIEW_T > size_t find_last_of(const SVIEW_T & sv, size_t pos=0) const
      { return str.find_last_of(sv,pos); }

    size_t find_last_not_of(const std::string & str, size_t pos=0) const noexcept
      { return find_last_not_of(str,pos); }
    size_t find_last_not_of(const char* s, size_t pos=0) const
      { return str.find_last_not_of(s,pos); }
    size_t find_last_not_of(const char* s, size_t pos, size_t count) const
      { return str.find_last_not_of(s,pos,count); }
    size_t find_last_not_of(char c, size_t pos=0) const noexcept
      { return str.find_last_not_of(c,pos); }
    template < class SVIEW_T > size_t find_last_not_of(const SVIEW_T & sv, size_t pos=0) const
      { return str.find_last_not_of(sv,pos); }

    // Operators

    template <typename T> String operator+(T && in)
      { return String(str + std::forward<T>(in), mode); }
    template <typename T> bool operator==(T && in) { return in.str == std::forward<T>(in); }
    template <typename T> bool operator<=>(T && in) { return in.str <=> std::forward<T>(in); }
    

    // ------ SPECIAL CONFIGURATION ------

    String & UseQuoteSingle(bool use=true) { return _ChangeMode(USE_QUOTE_SINGLE, use); }
    String & UseQuoteDouble(bool use=true) { return _ChangeMode(USE_QUOTE_DOUBLE, use); }
    String & UseQuoteBack  (bool use=true) { return _ChangeMode(USE_QUOTE_BACK,   use); }
    String & UseParenRound (bool use=true) { return _ChangeMode(USE_PAREN_ROUND,  use); }
    String & UseParenSquare(bool use=true) { return _ChangeMode(USE_PAREN_SQUARE, use); }
    String & UseParenCurly (bool use=true) { return _ChangeMode(USE_PAREN_CURLY,  use); }
    String & UseParenAngle (bool use=true) { return _ChangeMode(USE_PAREN_ANGLE,  use); }
    String & UseParenQuotes(bool use=true) { return _ChangeMode(USE_PAREN_QUOTES, use); }

    bool Get_UseQuoteSingle() const { return mode.val & USE_QUOTE_SINGLE; }
    bool Get_UseQuoteDouble() const { return mode.val & USE_QUOTE_DOUBLE; }
    bool Get_UseQuoteBack  () const { return mode.val & USE_QUOTE_BACK; }
    bool Get_UseParenRound () const { return mode.val & USE_PAREN_ROUND; }
    bool Get_UseParenSquare() const { return mode.val & USE_PAREN_SQUARE; }
    bool Get_UseParenCurly () const { return mode.val & USE_PAREN_CURLY; }
    bool Get_UseParenAngle () const { return mode.val & USE_PAREN_ANGLE; }
    bool Get_UseParenQuotes() const { return mode.val & USE_PAREN_QUOTES; }

    // ------ CLASSIFICATION ------

    // Count the number of occurrences of a specific character.
    size_t Count(char c) const { return (size_t) std::count(str.begin(), str.end(), c); }

    /// Test if an string is formatted as a literal character.
    bool IsLiteralChar() const;

    /// Test if an string is formatted as a literal string.
    bool IsLiteralString(const std::string & quote_marks="\"") const;

    /// Explain what string is NOT formatted as a literal string.
    std::string DiagnoseLiteralString(const std::string & quote_marks="\"") const;

    /// Determine a string is composed only of a set of characters (represented as a string)
    bool IsComposedOf(const std::string & char_set) const {
      for (char x : str) if (!is_one_of(x, char_set)) return false;
      return true;
    }

    /// Determine if string is a valid number.
    bool IsNumber() const;

    /// Determine if string is a valid identifier (in most languages).
    bool IsIdentifier() const {
      // At least one character; cannot begin with digit, only letters, digits and `_`
      return str.size() && !is_digit(str[0]) && IDCharSet().Has(str);
    }

    bool OnlyLower() const { return (str.size()) ? LowerCharSet().Has(str) : true; }
    bool OnlyUpper() const { return (str.size()) ? UpperCharSet().Has(str) : true; }
    bool OnlyDigits() const { return (str.size()) ? DigitCharSet().Has(str) : true; }
    bool OnlyAlphanumeric() const { return (str.size()) ? AlphanumericCharSet().Has(str) : true; }
    bool OnlyWhitespace() const { return (str.size()) ? WhitespaceCharSet().Has(str) : true; }

    bool HasOneOf(const std::string & char_set) const {
      for (char c : str) if (is_one_of(c, char_set)) return true;
      return false;
    }
    bool HasWhitespace() const { return WhitespaceCharSet().HasAny(str); }
    bool HasNonwhitespace() const { return !WhitespaceCharSet().HasOnly(str); }
    bool HasUpperLetter() const { return UpperCharSet().HasAny(str); }
    bool HasLowerLetter() const { return LowerCharSet().HasAny(str); }
    bool HasLetter() const { return LetterCharSet().HasAny(str); }
    bool HasDigit() const { return DigitCharSet().HasAny(str); }
    bool HasAlphanumeric() const { return AlphanumericCharSet().HasAny(str); }

    bool HasCharAt(char c, size_t pos) const { return (pos < str.size()) && (str[pos] == c); }
    bool HasOneOfAt(const std::string & opts, size_t pos) const {
      return (pos < str.size()) && is_one_of(str[pos], opts);
    }
    bool HasDigitAt(size_t pos) const { return DigitCharSet().HasAt(str, pos); }
    bool HasLetterAt(size_t pos) const { return LetterCharSet().HasAt(str, pos); }

    bool HasPrefix(const std::string & prefix) const { return str.rfind(prefix, 0) == 0; }

    // ------ SEARCHING ------

    size_t FindQuoteMatch(size_t pos) const;
    size_t FindParenMatch(size_t pos, bool skip_quotes=true) const;
    size_t FindMatch(size_t pos) const;
    size_t Find(std::string target, size_t start, bool skip_quotes=false, bool skip_parens=false) const;
    size_t Find(const CharSet & char_set, size_t start,
                bool skip_quotes=false, bool skip_parens=false) const;
    void FindAll(char target, emp::vector<size_t> & results,
                 const bool skip_quotes=false, bool skip_parens=false) const;
    emp::vector<size_t> FindAll(char target, bool skip_quotes=false, bool skip_parens=false) const;
    template <typename... Ts>
    size_t FindAnyOfFrom(size_t start, std::string test1, Ts... tests) const;
    template <typename T, typename... Ts>
    size_t FindAnyOf(T test1, Ts... tests) const;
    size_t FindID(std::string target, size_t start, bool skip_quotes=true, bool skip_parens=false) const;

    size_t FindWhitespace(size_t start=0, bool skip_q=false, bool skip_p=false) const 
      { return Find(WhitespaceCharSet(), start, skip_q, skip_p); }
    size_t FindNonWhitespace(size_t start=0, bool skip_q=false, bool skip_p=false) const 
      { return Find(!WhitespaceCharSet(), start, skip_q, skip_p); }
    size_t FindUpperChar(size_t start=0, bool skip_q=false, bool skip_p=false) const 
      { return Find(UpperCharSet(), start, skip_q, skip_p); }
    size_t FindNonUpperChar(size_t start=0, bool skip_q=false, bool skip_p=false) const 
      { return Find(!UpperCharSet(), start, skip_q, skip_p); }
    size_t FindLowerChar(size_t start=0, bool skip_q=false, bool skip_p=false) const 
      { return Find(LowerCharSet(), start, skip_q, skip_p); }
    size_t FindNonLowerChar(size_t start=0, bool skip_q=false, bool skip_p=false) const 
      { return Find(!LowerCharSet(), start, skip_q, skip_p); }
    size_t FindLetterChar(size_t start=0, bool skip_q=false, bool skip_p=false) const 
      { return Find(LetterCharSet(), start, skip_q, skip_p); }
    size_t FindNonLetterChar(size_t start=0, bool skip_q=false, bool skip_p=false) const 
      { return Find(!LetterCharSet(), start, skip_q, skip_p); }
    size_t FindDigitChar(size_t start=0, bool skip_q=false, bool skip_p=false) const 
      { return Find(DigitCharSet(), start, skip_q, skip_p); }
    size_t FindNonDigitChar(size_t start=0, bool skip_q=false, bool skip_p=false) const 
      { return Find(!DigitCharSet(), start, skip_q, skip_p); }
    size_t FindAlphanumericChar(size_t start=0, bool skip_q=false, bool skip_p=false) const 
      { return Find(AlphanumericCharSet(), start, skip_q, skip_p); }
    size_t FindNonAlphanumericChar(size_t start=0, bool skip_q=false, bool skip_p=false) const 
      { return Find(!AlphanumericCharSet(), start, skip_q, skip_p); }
    size_t FindIDChar(size_t start=0, bool skip_q=false, bool skip_p=false) const 
      { return Find(IDCharSet(), start, skip_q, skip_p); }
    size_t FindNonIDChar(size_t start=0, bool skip_q=false, bool skip_p=false) const 
      { return Find(!IDCharSet(), start, skip_q, skip_p); }

    //  ------ FORMATTING ------
    // Append* adds to the end of the current string.
    // Set* replaces the current string.
    // To* Converts the current string.
    // As* Returns a modified version of the current string, leaving original intact.
    // Most also have stand-along Make* versions where the core implementation is found.

    String & AppendEscaped(char c) { str += MakeEscaped(c); }
    String & SetEscaped(char c) { str = MakeEscaped(c); }

    String & AppendEscaped(const std::string & in) { str+=MakeEscaped(in); return *this; }
    String & SetEscaped(const std::string & in) { str = MakeEscaped(in); return *this; }
    String & ToEscaped() { str = MakeEscaped(str); }
    [[nodiscard]] std::string AsEscaped() { return MakeEscaped(str); }

    String & AppendWebSafe(std::string in) { str+=MakeWebSafe(in); return *this; }
    String & SetWebSafe(const std::string & in) { str = MakeWebSafe(in); return *this;; }
    String & ToWebSafe() { str = MakeWebSafe(str); }
    [[nodiscard]] std::string AsWebSafe() { return MakeWebSafe(str); }

    // <= Creating Literals =>
    template <typename T>
    String & AppendLiteral(const T & in) { str+=MakeLiteral(in); return *this; }
    template <typename T>
    String & SetLiteral(const T & in) { str = MakeLiteral(in); return *this;; }
    String & ToLiteral() { str = MakeLiteral(str); }
    [[nodiscard]] std::string AsLiteral() { return MakeLiteral(str); }

    String & AppendUpper(const std::string & in) { str+=MakeUpper(in); return this; }
    String & SetUpper(const std::string & in) { str = MakeUpper(in); return *this; }
    String & ToUpper() { str = MakeUpper(str); }
    [[nodiscard]] std::string AsUpper() { return MakeUpper(str); }

    String & AppendLower(const std::string & in) { str+=MakeLower(in); return this; }
    String & SetLower(const std::string & in) { str = MakeLower(in); return *this; }
    String & ToLower() { str = MakeLower(str); }
    [[nodiscard]] std::string AsLower() { return MakeLower(str); }

    String & AppendTitleCase(const std::string & in) { str+=MakeTitleCase(in); return this; }
    String & SetTitleCase(const std::string & in) { str = MakeTitleCase(in); return *this; }
    String & ToTitleCase() { str = MakeTitleCase(str); }
    [[nodiscard]] std::string AsTitleCase() { return MakeTitleCase(str); }

    String & AppendRoman(int val) { str+=MakeRoman(val); return this; }
    String & SetRoman(int val) { str = MakeRoman(val); return *this; }
  };


  /// Determine if this string represents a proper number.
  bool String::IsNumber() const {
    if (!str.size()) return false;           // If string is empty, not a number!
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
    return (pos == str.size()) && HasDigit();
  }

  // Given the start position of a quote, find where it ends; marks must be identical
  size_t String::FindQuoteMatch(size_t pos) const {
    while (++pos < str.size()) {
      const char mark = str[pos];
      if (str[pos] == '\\') { ++pos; continue; } // Skip escaped characters
      if (str[pos] == mark) { return pos; }      // Found match!
    }
    return npos; // Not found.
  }

  // Given an open parenthesis, find where it closes (including nesting).  Marks must be different.
  size_t String::FindParenMatch(size_t pos, bool skip_quotes) const {
    const char open = str[pos];
    const char close = GetMatch(open);
    size_t open_count = 1;
    while (++pos < str.size()) {
      if (str[pos] == open) ++open_count;
      else if (str[pos] == close) { if (--open_count == 0) return pos; }
      else if (skip_quotes && IsQuote(str[pos]) ) pos = FindQuoteMatch(pos);
    }

    return npos;
  }

  size_t String::FindMatch(size_t pos) const {
    if (IsQuote(str[pos])) return FindQuoteMatch(pos);
    if (IsParen(str[pos])) return FindParenMatch(pos);
    return npos;
  }

  // A version of string::find() that can skip over quotes.
  size_t String::Find(std::string target, size_t start, bool skip_quotes, bool skip_parens) const {
    size_t found_pos = str.find(target, start);
    if (!skip_quotes && !skip_parens) return found_pos;

    // Make sure found_pos is not in a quote and/or parens; adjust as needed!
    for (size_t scan_pos=0;
         scan_pos < found_pos && found_pos != npos;
         scan_pos++)
    {
      // Skip quotes, if needed...
      if (skip_quotes && IsQuote(str[scan_pos])) {
        scan_pos = FindQuoteMatch(scan_pos);
        if (found_pos < scan_pos) found_pos = str.find(target, scan_pos);
      }
      else if (skip_parens && IsParen(str[scan_pos])) {
        scan_pos = FindParenMatch(scan_pos);
        if (found_pos < scan_pos) found_pos = str.find(target, scan_pos);
      }
    }

    return found_pos;
  }

  // Find any of a set of characters.
  size_t String::Find(const CharSet & char_set, size_t start,
                           bool skip_quotes, bool skip_parens) const
  {
    // Make sure found_pos is not in a quote and/or parens; adjust as needed!
    for (size_t pos=start; pos < str.size(); ++pos) {
      if (char_set.Has(str[pos])) return pos;
      else if (skip_quotes && IsQuote(str[pos])) pos = FindQuoteMatch(pos);
      else if (skip_parens && IsParen(str[pos])) pos = FindParenMatch(pos);
    }

    return npos;
  }

  void String::FindAll(char target, emp::vector<size_t> & results,
                            const bool skip_quotes, bool skip_parens) const {
    results.resize(0);
    for (size_t pos=0; pos < str.size(); pos++) {
      if (str[pos] == target) results.push_back(pos);

      // Skip quotes, if needed...
      if (skip_quotes && IsQuote(str[pos])) pos = FindQuoteMatch(pos);
      else if (skip_parens && IsParen(str[pos])) pos = FindParenMatch(pos);
    }
  }

  emp::vector<size_t> String::FindAll(char target, bool skip_quotes, bool skip_parens) const {
    emp::vector<size_t> out;
    FindAll(target, out, skip_quotes, skip_parens);
    return out;
  }

  template <typename... Ts>
  size_t String::FindAnyOfFrom(size_t start, std::string test1, Ts... tests) const {
    if constexpr (sizeof...(Ts) == 0) return test_str.find(test1, start);
    else {
      size_t pos1 = test_str.find(test1, start);
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
  size_t String::FindID(std::string target, size_t start,
                             bool skip_quotes, bool skip_parens) const
  {
    size_t pos = Find(target, start, skip_quotes, skip_parens);
    while (pos != npos) {
      bool before_ok = (pos == 0) || !is_idchar(in_string[pos-1]);
      size_t after_pos = pos+target.size();
      bool after_ok = (after_pos == in_string.size()) || !is_idchar(in_string[after_pos]);
      if (before_ok && after_ok) return pos;

      pos = Find(target, pos+target.size(), skip_quotes, skip_parens);
    }

    return npos;
  }





/////// @CAO CONTINUE HERE!!!!!!!!


  /// Remove whitespace from the beginning or end of a string.
  static inline void trim_whitespace(std::string & in_str) {
    size_t start_count=0;
    while (start_count < in_str.size() && is_whitespace(in_str[start_count])) ++start_count;
    if (start_count) in_str.erase(0, start_count);

    size_t new_size = in_str.size();
    while (new_size > 0 && is_whitespace(in_str[new_size-1])) --new_size;
    in_str.resize(new_size);
  }

  /// Every time one or more whitespace characters appear replace them with a single space.
  static inline void compress_whitespace(std::string & in_string) {
    const size_t strlen = in_string.size();
    bool last_whitespace = true;          // Remove whitespace from beginning of line.
    size_t pos = 0;

    for (size_t i = 0; i < strlen; i++) {
      if (is_whitespace(in_string[i])) {  // This char is whitespace
        if (last_whitespace) continue;
        in_string[pos++] = ' ';
        last_whitespace = true;
      }
      else {  // Not whitespace
        in_string[pos++] = in_string[i];
        last_whitespace = false;
      }
    }

    if (pos && last_whitespace) pos--;   // If the end of the line is whitespace, remove it.

    in_string.resize(pos);
  }

  /// Remove all whitespace from anywhere within a string.
  static inline void remove_whitespace(std::string & in_string) {
    const size_t strlen = in_string.size();
    size_t pos = 0;

    for (size_t i = 0; i < strlen; i++) {
      if (is_whitespace(in_string[i])) continue;
      in_string[pos++] = in_string[i];
    }

    in_string.resize(pos);
  }

  /// Remove all characters from a string except letters, numbers, and whitespace.
  static inline void remove_punctuation(std::string & in_string) {
    const size_t strlen = in_string.size();
    size_t pos = 0;

    for (size_t i = 0; i < strlen; i++) {
      const char cur_char = in_string[i];
      if (is_alphanumeric(cur_char) || is_whitespace(cur_char)) {
        in_string[pos++] = cur_char;
      }
    }

    in_string.resize(pos);
  }

  /// Remove instances of characters from file.
  static inline void remove_chars(std::string & in_string, std::string chars) {
    size_t cur_pos = 0;
    for (size_t i = 0; i < in_string.size(); i++) {
      if (is_one_of(in_string[i], chars)) continue;
      in_string[cur_pos++] = in_string[i];
    }
    in_string.resize(cur_pos);
  }

  /// Make a string safe(r)
  [[nodiscard]] static inline std::string slugify(const std::string & in_string) {
    //TODO handle complicated unicode strings
    std::string res = to_lower(in_string);
    remove_punctuation(res);
    compress_whitespace(res);
    std::transform(res.begin(), res.end(), res.begin(), [](char ch) {
      return (ch == ' ') ? '-' : ch;
    });
    return res;
  }

  // -------- Functions that operate on VECTORS of strings --------

  using string_vec_t = emp::vector<std::string>;

  [[nodiscard]] static inline std::string
  combine_strings(const string_vec_t & strings, std::string spacer=" ") {
    // If there are no input strings, return an empty string.
    if (strings.size() == 0) { return ""; }

    // If there is one string provided, return it by itself.
    if (strings.size() == 1) { return strings[0]; }

    // If there is more than one, separate with spaces.
    std::string out_string = strings[0];
    for (size_t i = 1; i < strings.size(); ++i) {
      out_string += spacer;
      out_string += strings[i];
    }
    return out_string;
  }

 /// Convert a vector of strings to an English list, such as "one, two, three, and four."
  [[nodiscard]] static inline std::string to_english_list(const string_vec_t & strings) {
    // If there are no input strings, return an empty string.
    if (strings.size() == 0) { return ""; }

    // If there is one string provided, return it by itself.
    if (strings.size() == 1) { return strings[0]; }

    // If two strings are provided, link them by an "and".
    if (strings.size() == 2) { return strings[0] + " and " + strings[1]; }

    // If MORE than two strings are provided, list the first n-1 followed by commas, ending
    // with an "and" before the final one.
    std::string out_str;
    for (size_t i = 0; i < strings.size(); i++) {
      if (i) {
        out_str += ", ";
        if (i == strings.size()-1) out_str += "and ";
      }
      out_str += strings[i];
    }

    return out_str;
  }


  /// Transform all strings in a vector.
  [[nodiscard]] static inline string_vec_t
  transform_strings(const string_vec_t & in_strings,
                    std::function<std::string(const std::string &)> fun) {
    string_vec_t out_strings(in_strings.size());
    for (size_t i = 0; i < in_strings.size(); i++) {
      out_strings[i] = fun(in_strings[i]);
    }
    return out_strings;
  }

  /// Put all strings provided in quotes (Like 'this'), pre- and post-fixing another string if
  /// provided.
  [[nodiscard]] static inline string_vec_t
  quote_strings(const string_vec_t & in_strings, const std::string quote="'") {
    return transform_strings(in_strings, [quote](const std::string & str) {
      return quote + str + quote;
    });
  }

  /// Pre-pend and post-pend specified sequences to all strings provided.
  [[nodiscard]] static inline string_vec_t
  quote_strings(const string_vec_t & in_strings,
                const std::string open_quote,
                const std::string close_quote) {
    return transform_strings(in_strings, [open_quote, close_quote](const std::string & str) {
      return open_quote + str + close_quote;
    });
  }

  /// Take a vector of strings, put them in quotes, and then transform it into an English list.
  [[nodiscard]] static inline std::string to_quoted_list(const string_vec_t & in_strings,
                                                         const std::string quote="'") {
    return to_english_list(quote_strings(in_strings, quote));
  }

  static inline bool string_pop_if_char(std::string & in_string, char c)
  {
    if (in_string.size() && in_string[0] == c) {
      in_string.erase(0,1);
      return true;
    }
    return false;
  }

  /// Pop a segment from the beginning of a string as another string, shortening original.
  static inline std::string
  string_pop_fixed(std::string & in_string, std::size_t end_pos, size_t delim_size=0)
  {
    if (end_pos == 0) return "";                   // Not popping anything!

    std::string out_string = "";
    if (end_pos >= in_string.size()) {            // Popping whole string.
      out_string = in_string;
      in_string = "";
    }
    else {
      out_string = in_string.substr(0, end_pos);  // Copy up to the deliminator for ouput
      in_string.erase(0, end_pos + delim_size);   // Delete output string AND deliminator
    }

    return out_string;
  }

  /// Get a segment from the beginning of a string as another string, leaving original untouched.
  [[nodiscard]] static inline std::string
  string_get_range(const std::string & in_string, std::size_t start_pos, std::size_t end_pos) {
    emp_assert(start_pos <= in_string.size());
    if (end_pos == std::string::npos) end_pos = in_string.size();
    emp_assert(end_pos <= in_string.size());
    return in_string.substr(start_pos, end_pos - start_pos);
  }

  /// Remove a prefix of the input string (up to a specified delimeter) and return it.  If the
  /// delimeter is not found, return the entire input string and clear it.
  inline std::string string_pop(std::string & in_string, const char delim=' ') {
    return string_pop_fixed(in_string, in_string.find(delim), 1);
  }

  /// Return a prefix of the input string (up to a specified delimeter), but do not modify it.
  /// If the delimeter is not found, return the entire input string.
  [[nodiscard]] inline std::string
  string_get(const std::string & in_string, const char delim=' ', size_t start_pos=0) {
    return string_get_range(in_string, start_pos, in_string.find(delim, start_pos));
  }

  /// Remove a prefix of the input string (up to any of a specified set of delimeters) and
  /// return it.  If the delimeter is not found, return the entire input string and clear it.
  inline std::string string_pop(std::string & in_string, const std::string & delim_set) {
    return string_pop_fixed(in_string, in_string.find_first_of(delim_set), 1);
  }

  /// Return a prefix of the input string (up to any of a specified set of delimeters), but do not
  /// modify it. If the delimeter is not found, return the entire input string.
  [[nodiscard]] inline std::string
  string_get(const std::string & in_string, const std::string & delim_set, size_t start_pos=0) {
    emp_assert(start_pos <= in_string.size());
    return string_get_range(in_string, start_pos, in_string.find_first_of(delim_set, start_pos));
  }

  inline std::string
  string_pop_to(std::string & in_string, const std::string & delim=" ", size_t start_pos=0,
                bool skip_quotes=false, bool skip_parens=false,
                bool skip_braces=false, bool skip_brackets=false) {
    const size_t found_pos =
      emp::find(in_string, delim, start_pos, skip_quotes, skip_parens, skip_braces, skip_brackets);
    return string_pop_fixed(in_string, found_pos, delim.size());
  }

  [[nodiscard]] inline std::string
  string_get_to(const std::string & in_string, const std::string & delim=" ", size_t start_pos=0) {
    return string_get_range(in_string, start_pos, in_string.find(delim, start_pos));    
  }

  /// Remove a prefix of a string, up to the first whitespace, and return it.
  inline std::string string_pop_word(std::string & in_string) {
    // Whitespace = ' ' '\n' '\r' or '\t'
    return string_pop(in_string, " \n\r\t");
  }

  /// Return a prefix of a string, up to the first whitespace (do not modify the original string)
  [[nodiscard]] inline std::string
  string_get_word(const std::string & in_string, size_t start_pos=0) {
    // Whitespace = ' ' '\n' '\r' or '\t'
    return string_get(in_string, " \n\r\t", start_pos);
  }

  /// Remove a prefix of a string, up to the first newline, and return it.
  inline std::string string_pop_line(std::string & in_string) {
    return string_pop(in_string, '\n');
  }

  /// Return a prefix of a string, up to the first newline (do not modify the original string)
  [[nodiscard]] inline std::string
  string_get_line(const std::string & in_string, size_t start_pos=0) {
    return string_get(in_string, '\n', start_pos);
  }

  inline std::string string_pop_quote(std::string & in_string) {
    const size_t end_pos = emp::find_quote_match(in_string);
    return end_pos ? string_pop_fixed(in_string, end_pos+1) : "";
  }

  inline size_t string_pop_uint(std::string & in_string) {
    size_t uint_size = 0;
    for (char c : in_string) {
      if (is_digit(c)) uint_size++;
      else break;
    }
    std::string out_uint = string_pop_fixed(in_string, uint_size);
    return std::stoull(out_uint);
  }

  inline size_t string_get_uint(const std::string & in_string) {
    size_t uint_size = 0;
    for (char c : in_string) {
      if (is_digit(c)) uint_size++;
      else break;
    }
    std::string out_uint = string_get_range(in_string, 0, uint_size);
    return std::stoull(out_uint);
  }

  /// Remove all whitespace at the beginning of a string.  Return the whitespace removed.
  inline std::string left_justify(std::string & in_string) {
    return string_pop_fixed(in_string, in_string.find_first_not_of(" \n\r\t"));
  }

  /// Remove all whitespace at the end of a string.
  inline void right_justify(std::string & in_string) {
    // @CAO *very* inefficient at the moment.
    while (is_whitespace(in_string.back())) in_string.pop_back();
  }

  /// Remove all whitespace at both the beginning and the end of a string.
  inline void justify(std::string & in_string) {
    left_justify(in_string);
    right_justify(in_string);
  }

  /// Apply sprintf-like formatting to a string.
  /// See https://en.cppreference.com/w/cpp/io/c/fprintf.
  /// Adapted from https://stackoverflow.com/a/26221725.
  template<typename... Args>
  [[nodiscard]] std::string format_string( const std::string& format, Args... args ) {

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-security"

    // Extra space for '\0'
    const size_t size = static_cast<size_t>(std::snprintf(nullptr, 0, format.c_str(), args...) + 1);

    emp::vector<char> buf( size );
    std::snprintf( buf.data(), size, format.c_str(), args... );

     // We don't want the '\0' inside
    return std::string( buf.data(), buf.data() + size - 1 );

    #pragma GCC diagnostic pop

  }

  /// Find any instances of ${X} and replace with dictionary lookup of X.
  template <typename MAP_T>
  [[nodiscard]] std::string replace_vars( const std::string& in_string, const MAP_T & var_map );

  /// Find any instance of MACRO_NAME(ARGS) and call replace it with fun(ARGS).
  template <typename FUN_T>
  [[nodiscard]] std::string replace_macro( const std::string & str, std::string macro_name,
                                           FUN_T && fun, bool skip_quotes=true );

  /// Provide a string_view on a given string
  static inline std::string_view view_string(const std::string_view & str) {
    return std::string_view(str);
  }

  /// Provide a string_view on a string from a given starting point.
  static inline std::string_view view_string(const std::string_view & str, size_t start) {
    emp_assert(start <= str.size());
    return std::string_view(str.data()+start);
  }

  /// Provide a string_view on a string from a starting point with a given size.
  static inline std::string_view view_string(const std::string_view & str,
                                             size_t start,
                                             size_t npos) {
    emp_assert(start + npos <= str.size());
    return std::string_view(str.data()+start, npos);
  }

  /// Provide a string_view on a string from the beginning to a given size.
  static inline std::string_view view_string_front(const std::string_view & str,
                                                   size_t npos) {
    emp_assert(npos <= str.size());
    return std::string_view(str.data(), npos);
  }

  /// Provide a string_view on a string from a starting point with a given size.
  static inline std::string_view view_string_back(const std::string_view & str,
                                                  size_t npos) {
    emp_assert(npos <= str.size());
    return std::string_view(str.data()+(str.size()-npos), npos);
  }

  /// Provide a string_view on a string from a starting point to an ending point.
  static inline std::string_view view_string_range(const std::string_view & str,
                                                   size_t start,
                                                   size_t end) {
    emp_assert(start <= end);
    emp_assert(end <= str.size());
    return std::string_view(str.data()+start, end - start);
  }

  /// Return a view of the prefix of the input string up to a specified delimeter.
  /// If the delimeter is not found, return the entire input string.
  static inline std::string_view view_string_to(const std::string_view & in_string,
                                                const char delim,
                                                size_t start_pos=0) {
    const size_t in_size = in_string.size();
    size_t end_pos = start_pos;
    while (end_pos < in_size && in_string[end_pos] != delim) end_pos++;
    return view_string_range(in_string, start_pos, end_pos);
  }

  inline std::string pad_front(const std::string & in_str, char padding, size_t target_size) {
    if (in_str.size() >= target_size) return in_str;
    const size_t pad_size = target_size - in_str.size();
    return std::string(pad_size, padding) + in_str;
  }

  inline std::string pad_back(const std::string & in_str, char padding, size_t target_size) {
    if (in_str.size() >= target_size) return in_str;
    const size_t pad_size = target_size - in_str.size();
    return in_str + std::string(pad_size, padding);
  }

  /// Concatenate n copies of a string.
  inline std::string repeat( const std::string& value, const size_t n ) {
    const emp::vector<std::string> repeated( n, value );
    return std::accumulate(
      std::begin(repeated), std::end(repeated), std::string{}
    );
  }

  /// Cut up a string based on the provided delimiter; fill them in to the provided vector.
  /// @param in_string string to be sliced
  /// @param out_set destination
  /// @param delim delimiter to split on
  /// @param max_split defines the maximum number of splits
  /// @param keep_quotes Should quoted text be kept together?
  /// @param keep_parens Should parentheses ('(' and ')') be kept together?
  /// @param keep_braces Should braces ('{' and '}') be kept together?
  /// @param keep_brackets Should brackets ('[' and ']') be kept together?
  static inline void slice(
    const std::string_view & in_string,
    emp::vector<std::string> & out_set,
    const char delim='\n',
    const size_t max_split=std::numeric_limits<size_t>::max(),
    const bool keep_quotes=false,
    const bool keep_parens=false,
    const bool keep_braces=false,
    const bool keep_brackets=false
  );

  /// Slice a string without passing in result vector (may be less efficient).
  /// @param in_string string to be sliced
  /// @param delim delimiter to split on
  /// @param max_split defines the maximum number of splits
  /// @param keep_quotes Should quoted text be kept together?
  /// @param keep_parens Should parentheses ('(' and ')') be kept together?
  /// @param keep_braces Should braces ('{' and '}') be kept together?
  /// @param keep_brackets Should brackets ('[' and ']') be kept together?
  static inline emp::vector<std::string> slice(
    const std::string_view & in_string,
    const char delim='\n',
    const size_t max_split=std::numeric_limits<size_t>::max(),
    const bool keep_quotes=false,
    const bool keep_parens=false,
    const bool keep_braces=false,
    const bool keep_brackets=false
  ) {
    emp::vector<std::string> result;
    slice(in_string, result, delim, max_split, keep_quotes, keep_parens, keep_braces, keep_brackets);
    return result;
  }

  /// Create a set of string_views based on the provided delimiter; fill them in to the provided vector.
  /// @param in_string string to be sliced
  /// @param out_set destination vector
  /// @param delim delimiter to split on
  /// @param keep_quotes Should quoted text be kept together?
  /// @param keep_parens Should parentheses ('(' and ')') be kept together?
  /// @param keep_braces Should braces ('{' and '}') be kept together?
  /// @param keep_brackets Should brackets ('[' and ']') be kept together?
  static inline void view_slices(
    const std::string_view & in_string,
    emp::vector<std::string_view> & out_set,
    char delim='\n',
    const bool keep_quotes=false,
    const bool keep_parens=false,
    const bool keep_braces=false,
    const bool keep_brackets=false
  ) {
    out_set.resize(0);
    size_t start_pos = 0;
    for (size_t pos=0; pos < in_string.size(); pos++) {
      if (keep_quotes && (in_string[pos] == '"' || in_string[pos] == '\'')) {
        pos = find_quote_match(in_string, pos, in_string[pos]);
      }
      else if (keep_parens && in_string[pos] == '(') {
        pos = find_paren_match(in_string, pos, '(', ')', keep_quotes);
      }
      else if (keep_braces && in_string[pos] == '{') {
        pos = find_paren_match(in_string, pos, '{', '}', keep_quotes);
      }
      else if (keep_brackets && in_string[pos] == '[') {
        pos = find_paren_match(in_string, pos, '[', ']', keep_quotes);
      }
      else if (in_string[pos] == delim) {  // Hit an end point!
        out_set.push_back( view_string_range(in_string, start_pos, pos) );
        start_pos = pos+1;
      }
    }

    // Include the final segment.
    out_set.push_back( view_string_range(in_string, start_pos, in_string.size()) );
  }

  /// Slice a string without passing in result vector (may be less efficient).
  static inline emp::vector<std::string_view> view_slices(
    const std::string_view & in_string,
    char delim='\n',
    const bool keep_quotes=false,
    const bool keep_parens=false,
    const bool keep_braces=false,
    const bool keep_brackets=false
  ) {
    emp::vector<std::string_view> result;
    view_slices(in_string, result, delim, keep_quotes, keep_parens, keep_braces, keep_brackets);
    return result;
  }

  /// Slice a string without passing in result vector (may be less efficient).
  /// @param in_string string to be sliced
  /// @param delim delimiter to split on (default ',')
  /// @param assign separator for left and right side of assignment (default: "=")
  /// @param max_split defines the maximum number of splits (default, no max)
  /// @param keep_quotes Should quoted text be kept together? (default: no)
  /// @param trim_whitespace Should extra whitespace around delim or assign be ignored?
  static inline std::map<std::string,std::string> slice_assign(
    const std::string_view & in_string,
    const char delim=',',
    std::string assign_op="=",
    const size_t max_split=std::numeric_limits<size_t>::max(),
    const bool trim_whitespace=true,
    const bool keep_quotes=true,
    const bool keep_parens=true,
    const bool keep_braces=true,
    const bool keep_brackets=true
  ) {
    auto assign_set = emp::slice(in_string, delim, max_split, keep_quotes, keep_parens, keep_braces, keep_brackets);
    std::map<std::string,std::string> result_map;
    for (auto setting : assign_set) {
      // Skip blank settings (especially at the end).
      if (emp::is_whitespace(setting)) continue;

      // Remove any extra spaces around parsed values.
      std::string var_name = emp::string_pop_to(setting, assign_op);
      if (trim_whitespace) {
        emp::trim_whitespace(var_name);
        emp::trim_whitespace(setting);
      }
      if (setting.size() == 0) {
        std::stringstream msg;
        msg << "No assignment found in slice_assign(): " << in_string;
        abort();
        emp::notify::Exception("emp::string_utils::slice_assign::missing_assign",
                               msg.str(), setting);                               
      }
      result_map[var_name] = setting;
    }
    return result_map;
  }

  static inline emp::vector<std::string_view> ViewCSV( const std::string_view & in_string ) {
    return view_slices(in_string, ',', true);
  }

  /// View a section of a string with the properly matching nested blocks.
  /// For example if ((abc(de))f(ghi)) would return "(abc(de))f(ghi)" at 0, "de" at 5, or
  /// "" at 2 (since there is no start!)
  std::string_view ViewNestedBlock(std::string_view str,
                                   const std::string symbols="()",
                                   size_t start=0) {
    // Test if we are not starting at the beginning of a block, return empty.
    if (str[start] != symbols[0]) return emp::view_string(str, 0, 0);

    size_t depth = 0;
    size_t stop = start;
    while (++stop < str.size()) {
      if (str[stop] == symbols[0]) depth++;
      else if (str[stop] == symbols[1]) {
        if (depth == 0) break;
        depth--;
      }
    }

    return emp::view_string(str, start+1, stop-start-1);
  }

  #ifndef DOXYGEN_SHOULD_SKIP_THIS

  // The next functions are not efficient, but they will take any number of inputs and
  // dynamically convert them all into a single, concatenated string.

  /// Setup emp::ToString declarations for built-in types.
  template <typename T, size_t N> inline std::string ToString(const emp::array<T,N> & container);
  template <typename T, typename... Ts>
  inline std::string ToString(const emp::vector<T, Ts...> & container);


  namespace internal {
    // If the item passed in has a ToString(), always use it.
    template <typename T>
    decltype(std::declval<T>().ToString()) to_stream_item(const T & in, bool) {
      return in.ToString();
    }

    // Otherwise, if emp::ToString(x) is defined for x, use it.
    template <typename T>
    auto to_stream_item(const T & in, int) -> decltype(emp::ToString(in)) {
      return emp::ToString(in);
    }

    // If neither works, just assume stream operator will handle things...
    // @CAO: Technically we can detect this to give a more informative error...
    template <typename T> const T & to_stream_item(const T & in, ...) { return in; }

  }

  #endif // DOXYGEN_SHOULD_SKIP_THIS


  /// This function does its best to convert any type to a string. Accepts any number of
  /// arguments and returns a single concatenated string. Conversions attempted for an
  /// object 'x' include (in order):
  /// - Call a x.ToString()
  /// - Call appropriate emp::ToString(x) overload
  /// - Pass x through stringstream
  template <typename... Ts>
  inline std::string to_string(const Ts &... values) {
    std::stringstream ss;
    (ss << ... << internal::to_stream_item(values, true));
    return ss.str();
  }

  /// Overload of to_string() string arguments to be directly returned.
  inline const std::string & to_string(const std::string & value) {
    return value;
  }

  /// Setup emp::ToString to work on arrays.
  template <typename T, size_t N>
  inline std::string ToString(const emp::array<T,N> & container) {
    std::stringstream ss;
    ss << "[ ";
    for (const auto & el : container) {
      ss << to_string(el);
      ss << " ";
    }
    ss << "]";
    return ss.str();
  }

  /// Setup emp::ToString to work on vectors.
  template <typename T, typename... Ts>
  inline std::string ToString(const emp::vector<T, Ts...> & container) {
    std::stringstream ss;
    ss << "[ ";
    for (const auto & el : container) {
      ss << to_string(el);
      ss << " ";
    }
    ss << "]";
    return ss.str();
  }

  /// This function tries to convert a string into any type you're looking for...  You just
  /// need to specify the out type as the template argument.
  template <typename T>
  inline T from_string(const std::string & str) {
    std::stringstream ss;
    ss << str;
    T out_val;
    ss >> out_val;
    return out_val;
  }

  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {
    static inline void _from_string(std::stringstream &) { ; }

    template <typename T, typename... Ts>
    void _from_string(std::stringstream & ss, T & arg1, Ts... extra_args) {
      ss >> arg1;
      _from_string(ss, extra_args...);
    }
  }
  #endif // DOXYGEN_SHOULD_SKIP_THIS

  /// The from_string() function can also take multiple args instead of a return.
  template <typename... Ts>
  inline void from_string(const std::string & str, Ts &... args) {
    std::stringstream ss;
    ss << str;
    internal::_from_string(ss, args...);
  }

  /// The from_strings() function takes a vector of strings and converts them into a vector
  /// of the appropriate type.
  template <typename T>
  inline emp::vector<T> from_strings(const emp::vector<std::string> & string_v) {
    emp::vector<T> vals(string_v.size());
    for (size_t i = 0; i < string_v.size(); i++) {
      vals[i] = from_string<T>(string_v[i]);
    }
    return vals;
  }

  /// This function tries to convert a string_view into any other type...  You must
  /// need to specify the out type as the template argument.
  template <typename T>
  inline T from_string(std::string_view str) {
    std::stringstream ss;
    ss << str;
    T out_val;
    ss >> out_val;
    return out_val;
  }

  /// This function returns values from a container as a single string separated
  /// by a given delimeter.
  /// @param container is any standard-interface container holding objects to be joined.
  /// @param join_str optional delimeter
  /// @return merged string of all values
  template <typename CONTAINER_T>
  inline std::string join(const CONTAINER_T & container, std::string join_str="") {
    if (container.size() == 0) return "";
    if (container.size() == 1) return to_string(container.front());

    std::stringstream out;
    for (auto it = container.begin(); it != container.end(); ++it) {
      if (it != container.begin()) out << join_str;
      out << to_string(*it);
    }

    return out.str();
  }


  // Some ANSI helper functions.
  inline constexpr char ANSI_ESC() { return (char) 27; }
  inline std::string ANSI_Reset() { return "\033[0m"; }
  inline std::string ANSI_Bold() { return "\033[1m"; }
  inline std::string ANSI_Faint() { return "\033[2m"; }
  inline std::string ANSI_Italic() { return "\033[3m"; }
  inline std::string ANSI_Underline() { return "\033[4m"; }
  inline std::string ANSI_SlowBlink() { return "\033[5m"; }
  inline std::string ANSI_Blink() { return "\033[6m"; }
  inline std::string ANSI_Reverse() { return "\033[7m"; }
  inline std::string ANSI_Strike() { return "\033[9m"; }

  inline std::string ANSI_NoBold() { return "\033[22m"; }
  inline std::string ANSI_NoItalic() { return "\033[23m"; }
  inline std::string ANSI_NoUnderline() { return "\033[24m"; }
  inline std::string ANSI_NoBlink() { return "\033[25m"; }
  inline std::string ANSI_NoReverse() { return "\033[27m"; }

  inline std::string ANSI_Black() { return "\033[30m"; }
  inline std::string ANSI_Red() { return "\033[31m"; }
  inline std::string ANSI_Green() { return "\033[32m"; }
  inline std::string ANSI_Yellow() { return "\033[33m"; }
  inline std::string ANSI_Blue() { return "\033[34m"; }
  inline std::string ANSI_Magenta() { return "\033[35m"; }
  inline std::string ANSI_Cyan() { return "\033[36m"; }
  inline std::string ANSI_White() { return "\033[37m"; }
  inline std::string ANSI_DefaultColor() { return "\033[39m"; }

  inline std::string ANSI_BlackBG() { return "\033[40m"; }
  inline std::string ANSI_RedBG() { return "\033[41m"; }
  inline std::string ANSI_GreenBG() { return "\033[42m"; }
  inline std::string ANSI_YellowBG() { return "\033[43m"; }
  inline std::string ANSI_BlueBG() { return "\033[44m"; }
  inline std::string ANSI_MagentaBG() { return "\033[45m"; }
  inline std::string ANSI_CyanBG() { return "\033[46m"; }
  inline std::string ANSI_WhiteBG() { return "\033[47m"; }
  inline std::string ANSI_DefaultBGColor() { return "\033[49m"; }

  inline std::string ANSI_BrightBlack() { return "\033[30m"; }
  inline std::string ANSI_BrightRed() { return "\033[31m"; }
  inline std::string ANSI_BrightGreen() { return "\033[32m"; }
  inline std::string ANSI_BrightYellow() { return "\033[33m"; }
  inline std::string ANSI_BrightBlue() { return "\033[34m"; }
  inline std::string ANSI_BrightMagenta() { return "\033[35m"; }
  inline std::string ANSI_BrightCyan() { return "\033[36m"; }
  inline std::string ANSI_BrightWhite() { return "\033[37m"; }

  inline std::string ANSI_BrightBlackBG() { return "\033[40m"; }
  inline std::string ANSI_BrightRedBG() { return "\033[41m"; }
  inline std::string ANSI_BrightGreenBG() { return "\033[42m"; }
  inline std::string ANSI_BrightYellowBG() { return "\033[43m"; }
  inline std::string ANSI_BrightBlueBG() { return "\033[44m"; }
  inline std::string ANSI_BrightMagentaBG() { return "\033[45m"; }
  inline std::string ANSI_BrightCyanBG() { return "\033[46m"; }
  inline std::string ANSI_BrightWhiteBG() { return "\033[47m"; }

  /// Make a string appear bold when printed to the command line.
  inline std::string to_ansi_bold(const std::string & _in) {
    return ANSI_Bold() + _in + ANSI_NoBold();
  }

  /// Make a string appear italics when printed to the command line.
  inline std::string to_ansi_italic(const std::string & _in) {
    return ANSI_Italic() + _in + ANSI_NoItalic();
  }

  /// Make a string appear underline when printed to the command line.
  inline std::string to_ansi_underline(const std::string & _in) {
    return ANSI_Underline() + _in + ANSI_NoUnderline();
  }

  /// Make a string appear blink when printed to the command line.
  inline std::string to_ansi_blink(const std::string & _in) {
    return ANSI_Blink() + _in + ANSI_NoBlink();
  }

  /// Make a string appear reverse when printed to the command line.
  inline std::string to_ansi_reverse(const std::string & _in) {
    return ANSI_Reverse() + _in + ANSI_NoReverse();
  }


  //////////////////////////////////////////////////////
  //  Implementations of larger functions (>25 lines)
  //////////////////////////////////////////////////////

  /// Test if an input string is properly formatted as a literal character.
  static inline bool is_literal_char(const std::string & value) {
    // A literal char must begin with a single quote, contain a representation of a single
    // character, and end with a single quote.
    if (value.size() < 3) return false;
    if (value[0] != '\'' || value.back() != '\'') return false;

    // If there's only a single character in the quotes, it's USUALLY legal.
    if (value.size() == 3) {
      switch (value[1]) {
        case '\'':         // Can't be a single quote (must be escaped!)
        case '\\':         // Can't be a backslash (must be followed by something!)
          return false;
        default:
          return true;
      }
    }

    // If there are more characters, must be an escape sequence.
    if (value.size() == 4) {
      if (value[1] != '\\') return false;

      // Identify legal escape sequences.
      // @CAO Need more here!
      switch (value[2]) {
        case 'n':   // Newline
        case 'r':   // Return
        case 't':   // Tab
        case '0':   // Empty (character 0)
        case '\\':  // Backslash
        case '\'':  // Single quote
          return true;
        default:
          return false;
      }
    }

    // @CAO: Need to add special types of numerical escapes here (e.g., ascii codes!)

    // If we made it here without a problem, it must be correct!
    return true;
  }


  /// Test if an input string is properly formatted as a literal string.
  static inline bool is_literal_string(const std::string & value,
                                       const std::string & quote_marks) {
    if (value.size() < 2) return false;               // Two short to contain even quote marks!
    char quote = value[0];
    if (!is_one_of(quote, quote_marks)) return false; // Must be working with allowed quote mark.
    if (value.back() != quote) return false;          // Must use same quote at front and back.

    // Are all of the characters valid?
    for (size_t pos = 1; pos < value.size() - 1; pos++) {
      if (value[pos] == quote) return false;          // Cannot have a raw quote in the middle.
      if (value[pos] == '\\') {                       // Allow escaped characters...
        if (pos == value.size()-2) return false;      // Backslash must have char to escape.

        // Move to the next char and make sure it is legal to be escaped.
        // @CAO Expand on options!
        pos++;
        switch (value[pos]) {
          case 'b':   // Backspace
          case 'f':   // Form feed
          case 'n':   // Newline
          case 'r':   // Return
          case 't':   // Tab
          case 'v':   // Vertical tab.
          case '0':   // Empty (character 0)
          case '\\':  // Backslash
          case '"':   // Double quote
          case '\'':  // Single quote
          case '`':   // Back quote
            continue;
          default:
            return false;
        }
      }
    }

    // @CAO: Need to check special types of numerical escapes (e.g., ascii codes!)

    // If we made it here without a problem, it must be correct!
    return true;
  }


  /// Test if an input string is properly formatted as a literal string.
  static inline std::string diagnose_literal_string(const std::string & value,
                                                    const std::string & quote_marks) {
    // A literal string must begin and end with a double quote and contain only valid characters.
    if (value.size() < 2) return "Too short!";
    char quote = value[0];
    if (!is_one_of(quote, quote_marks)) return "Must begin an end in quotes.";
    if (value.back() != quote) return "Begin and end quotes must match.";

    // Are all of the characters valid?
    for (size_t pos = 1; pos < value.size() - 1; pos++) {
      if (value[pos] == quote) return "Has a floating quote.";
      if (value[pos] == '\\') {
        if (pos == value.size()-2) return "Cannot escape the final quote.";  // Backslash must have char to escape.

        // Move to the next char and make sure it's legal to be escaped.
        // @CAO Expand on options!
        pos++;
        switch (value[pos]) {
          case 'b':   // Backspace
          case 'f':   // Form feed
          case 'n':   // Newline
          case 'r':   // Return
          case 't':   // Tab
          case 'v':   // Vertical tab.
          case '0':   // Empty (character 0)
          case '\\':  // Backslash
          case '"':   // Double quote
          case '\'':  // Single quote
          case '`':   // Back quote
            continue;
          default:
            return "Unknown escape charater.";
        }
      }
    }

    // @CAO: Need to check special types of numerical escapes (e.g., ascii codes!)

    // If we made it here without a problem, it must be correct!
    return "Good!";
  }






  /// Find any instances of ${X} and replace with dictionary lookup of X.
  template <typename MAP_T>
  [[nodiscard]] std::string replace_vars( const std::string& in_string, const MAP_T & var_map ) {
    std::string result = in_string;

    // Seek out instances of "${" to indicate the start of pre-processing.
    for (size_t i = 0; i < result.size(); ++i) {
      if (result[i] != '$') continue;   // Replacement tag must start with a '$'.
      if (result.size() <= i+2) break;  // Not enough room for a replacement tag.
      if (result[i+1] == '$') {         // Compress two $$ into one $
        result.erase(i,1);
        continue;
      }
      if (result[i+1] != '{') continue; // Eval must be surrounded by braces.

      // If we made it this far, we have a starting match!
      size_t end_pos = emp::find_paren_match(result, i+1, '{', '}', false);
      if (end_pos == i+1) {
        emp::notify::Exception("emp::string_utils::replace_vars::missing_close",
                               "No close brace found in string_utils::replace_vars()",
                               result);
        return result; // Stop where we are... No end brace found!
      }

      std::string key = result.substr(i+2, end_pos-i-2);
      auto replacement_it = var_map.find(key);
      if (replacement_it == var_map.end()) {
        emp::notify::Exception("emp::string_utils::replace_vars::missing_var",
                               emp::to_string("Lookup variable not found in var_map (key=", key, ")"),
                               key);
        return result; // Stop here; variable could not be replaced.
      }
      result.replace(i, end_pos-i+1, replacement_it->second);   // Put into place.
      i += replacement_it->second.size();                       // Continue at the next position...
    }

    return result;
  }

  /// @brief Find any instance of MACRO_NAME(ARGS) and replace it with fun(ARGS).
  /// @param in_string String to perform macro replacement.
  /// @param macro_name Name of the macro to look for.
  /// @param macro_fun Function to call with contents of macro.  Params are macro_args (string), line_num (size_t), and hit_num (size_t)
  /// @param skip_quotes Should we skip quotes when looking for macro?
  /// @return Processed version of in_string with macros replaced.
  template <typename FUN_T>
  [[nodiscard]] std::string replace_macro(
    const std::string & in_string,
    std::string macro_name,
    FUN_T && macro_fun,
    bool skip_quotes
  ) {
    std::stringstream out;

    // We need to identify the comparator and divide up arguments in macro.
    size_t macro_count = 0;     // Count of the number of hits for this macro.
    size_t line_num = 0;        // Line number where current macro hit was found.
    size_t macro_end = 0;
    for (size_t macro_pos = emp::find_id(in_string, macro_name, 0, skip_quotes);
         macro_pos != std::string::npos;
         macro_pos = emp::find_id(in_string, macro_name, macro_end, skip_quotes))
    {
      // Output everything from the end of the last macro hit to the beginning of this one.
      std::string code_segment = in_string.substr(macro_end, macro_pos-macro_end);
      line_num += emp::count(code_segment, '\n');
      out << code_segment;

      // Make sure this macro is okay.
      macro_pos = emp::find_non_whitespace(in_string, macro_pos+macro_name.size());
      if (in_string[macro_pos] != '(') {
        emp::notify::Warning("Line ", line_num, ": Invalid MACRO instance of '", macro_name,
          "' - found ", in_string[macro_pos], "instead of '('.");
        macro_end = macro_pos;
        continue;
      }

      // Isolate this macro instance and call the conversion function.
      macro_end = emp::find_paren_match(in_string, macro_pos);
      const std::string macro_body = in_string.substr(macro_pos+1, macro_end-macro_pos-1);
      macro_end += 2;  // Advance the end past the ");" at the end of the macro.

      out << macro_fun(macro_body, line_num, macro_count);

      // Find the next macro instance and loop starting from the end of this one.
      macro_count++;
    }

    // Grab the rest of the in_string and output the processed string.
    out << in_string.substr(macro_end);
    return out.str();
  }

  /// Cut up a string based on the provided delimiter; fill them in to the provided vector.
  /// @param in_string string to be sliced
  /// @param out_set destination
  /// @param delim delimiter to split on
  /// @param max_split defines the maximum number of splits
  /// @param keep_quotes Should quoted text be kept together?
  static inline void slice (
    const std::string_view & in_string,
    emp::vector<std::string> & out_set,
    const char delim,
    const size_t max_split,
    const bool keep_quotes,
    const bool keep_parens,
    const bool keep_braces,
    const bool keep_brackets
  ) {
    const size_t test_size = in_string.size();
    if (test_size == 0) return; // Nothing to set!

    // Count produced strings
    size_t out_count = 0;
    size_t pos = 0;
    size_t start_pos = 0;
    while (pos < test_size && out_count <= max_split) {
      // Find the end of the current segment.
      while (pos < test_size && in_string[pos] != delim) {
        if (keep_quotes && (in_string[pos] == '"' || in_string[pos] == '\'')) {
          pos = find_quote_match(in_string, pos, in_string[pos]);
        }
        else if (keep_parens && in_string[pos] == '(') {
          pos = find_paren_match(in_string, pos, '(', ')', keep_quotes);
        }
        else if (keep_braces && in_string[pos] == '{') {
          pos = find_paren_match(in_string, pos, '{', '}', keep_quotes);
        }
        else if (keep_brackets && in_string[pos] == '[') {
          pos = find_paren_match(in_string, pos, '[', ']', keep_quotes);
        }
        pos++;
      }

      // Record the current segment.
      if (out_count >= out_set.size()) {
        out_set.emplace_back( in_string.substr(start_pos, pos-start_pos) );
      } else {
        out_set[out_count] = in_string.substr(start_pos, pos-start_pos);
      }

      // Move on to the next segment.
      pos++;              // Skip deliminator
      start_pos = pos;    // Record start of segment.
      out_count++;        // Keep count of segments.
    }

    out_set.resize(out_count); // Shrink out_set if needed.
  }














  //------------- Stand-along function definitions --------------

  std::string MakeEscaped(char c) {
    // If we just append as a normal character, do so!
    if ( (c >= 40 && c < 91) || (c > 96 && c < 127)) return std::string(c);
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
    };

    return std::string(c);
  }
  
  std::string MakeEscaped(const std::string & in) {
    std::string out;
    out.reserve(in.size()); // Reserve minimum needed size. 
    for (char c : in) out += MakeEscaped(c);
    return out;
  }

  /// Take a string and replace reserved HTML characters with character entities
  std::string MakeWebSafe(const std::string & in) {
    std::string out;
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

  /// Take a value and convert it to a C++-style literal.
  template <typename T>
  [[nodiscard]] typename std::enable_if<!emp::IsIterable<T>::value, std::string>::type
  MakeLiteral(const T & value) {
    return std::to_string(value);
  }

  /// Take a char and convert it to a C++-style literal.
  [[nodiscard]] std::string MakeLiteral(char value) {
    std::stringstream ss;
    ss << "'" << MakeEscaped(value) << "'";
    return ss.str();
  }

  /// Take a string or iterable and convert it to a C++-style literal.
  // This is the version for string. The version for an iterable is below.
  [[nodiscard]] std::string MakeLiteral(const std::string & value) {
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

  /// Take any iterable value and convert it to a C++-style literal.
  template <typename T>
  [[nodiscard]] typename std::enable_if<emp::IsIterable<T>::value, std::string>::type
  MakeLiteral(const T & value) {
    std::stringstream ss;
    ss << "{ ";
    for (auto iter = std::begin( value ); iter != std::end( value ); ++iter) {
      if (iter != std::begin( value )) ss << " ";
      ss << String::MakeLiteral< std::decay_t<decltype(*iter)> >( *iter );
    }
    ss << " }";

    return ss.str();
  }

  #endif

  [[nodiscard]] std::string MakeFromLiteral(const std::string & value) {
    if (value.size() == 0) return "";
    if (value[0] == '\'') return std::string(MakeFromLiteral_Char(value)); 
    if (value[0] == '"') return MakeFromLiteral_String(value);
    // @CAO Add conversion from numerical literals, and especially octal (0-), binary (0b-), and hex (0x-)
  }

  /// Convert a literal character representation to an actual string.
  /// (i.e., 'A', ';', or '\n')
  [[nodiscard]] char MakeFromLiteral_Char(const std::string & value) {
    emp_assert(is_literal_char(value));
    // Given the assert, we can assume the string DOES contain a literal representation,
    // and we just need to convert it.

    if (value.size() == 3) return value[1];
    if (value.size() == 4) return ToEscapeChar(value[2]);

    return '\0'; // Error!
  }


  /// Convert a literal string representation to an actual string.
  [[nodiscard]] std::string MakeFromLiteral_String(const std::string & value) {
  /// Convert a literal string representation to an actual string.
    emp_assert(is_literal_string(value),
               value, diagnose_literal_string(value));
    // Given the assert, we can assume string DOES contain a literal string representation.

    std::string out_string;
    out_string.reserve(value.size()-2);  // Make a guess on final size.

    for (size_t pos = 1; pos < value.size() - 1; pos++) {
      // If we don't have an escaped character, just move it over.
      if (value[pos] != '\\') out_string.push_back(value[pos]);
      else out_string.push_back(ToEscapeChar(value[++pos]));
    }

    return out_string;
  }

  /// Convert a string to all uppercase.
  [[nodiscard]] std::string MakeUpper(std::string value) {
    constexpr int char_shift = 'a' - 'A';
    for (auto & x : value) {
      if (x >= 'a' && x <= 'z') x = (char) (x - char_shift);
    }
    return value;
  }

  /// Convert a string to all lowercase.
  [[nodiscard]] std::string MakeLower(std::string value) {
    constexpr int char_shift = 'a' - 'A';
    for (auto & x : value) {
      if (x >= 'A' && x <= 'Z') x = (char) (x + char_shift);
    }
    return value;
  }

  /// Make first letter of each word upper case
  [[nodiscard]] std::string MakeTitleCase(std::string value) {
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
  [[nodiscard]] std::string MakeRoman(int val, const std::string & prefix="") {
    std::string ret_string(prefix);
    if (val < 0) ret_string += to_roman_numeral(-val, "-");
    else if (val > 3999) { ; } // Out of bounds; return a blank;
    else if (val >= 1000) ret_string += to_roman_numeral(val - 1000, "M");
    else if (val >= 900) ret_string += to_roman_numeral(val - 900, "CM");
    else if (val >= 500) ret_string += to_roman_numeral(val - 500, "D");
    else if (val >= 400) ret_string += to_roman_numeral(val - 400, "CD");
    else if (val >= 100) ret_string += to_roman_numeral(val - 100, "C");
    else if (val >= 90) ret_string += to_roman_numeral(val - 90, "XC");
    else if (val >= 50) ret_string += to_roman_numeral(val - 50, "L");
    else if (val >= 40) ret_string += to_roman_numeral(val - 40, "XL");
    else if (val >= 10) ret_string += to_roman_numeral(val - 10, "X");
    else if (val == 9) ret_string += "IX";
    else if (val >= 5) ret_string += to_roman_numeral(val - 5, "V");
    else if (val == 4) ret_string += "IV";
    else if (val > 0) ret_string += to_roman_numeral(val - 1, "I");

    // else we already have it exactly and don't need to return anything.
    return ret_string;
  }


  // ------ External function overrides ------

  std::ostream & operator<<(std::ostream & os, const emp::String & str) {
    return os << str;
  }

  std::istream & operator>>(std::istream & is, emp::String & str) {
    return is >> str;
  }

  template<typename STREAM_T>
  STREAM_T & getline(STREAM_T && input, emp::String str, char delim) {
    return getline(std::forward<STREAM_T>(input), str.str, delim);
  }

  template<typename STREAM_T>
  STREAM_T & getline(STREAM_T && input, emp::String str) {
    return getline(std::forward<STREAM_T>(input), str.str);
  }
}

#endif // #ifndef EMP_TOOLS_STRING_UTILS_HPP_INCLUDE
