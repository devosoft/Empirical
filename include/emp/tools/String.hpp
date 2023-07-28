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

#include <algorithm>              // std::count
#include <cctype>                 // std::toupper and std::tolower
#include <functional>             // std::function
#include <initializer_list>
#include <map>
#include <sstream>
#include <string>
#include <string_view>

#include "../base/assert.hpp"
#include "../base/notify.hpp"
#include "../base/vector.hpp"
#include "../meta/type_traits.hpp"  // emp::IsIterable()

#include "_StringSyntax.hpp"
#include "char_utils.hpp"           // emp::CharSet (+ functions)


namespace emp {

  class String;

  // Some stand-alone functions to generate String objects.
  template <typename... Ts> [[nodiscard]] inline String MakeString(Ts &&... args);
  [[nodiscard]] inline const String & MakeString(const String & in) { return in; };
  [[nodiscard]] inline String MakeEscaped(char c);
  [[nodiscard]] inline String MakeEscaped(const String & in);
  [[nodiscard]] inline String MakeCSVSafe(const String & in);
  [[nodiscard]] inline String MakeWebSafe(const String & in);
  [[nodiscard]] inline String MakeLiteral(char value);
  [[nodiscard]] inline String MakeLiteral(const std::string & value);
  template <typename T> [[nodiscard]] inline String MakeLiteral(const T & value);
  [[nodiscard]] inline char MakeFromLiteral_Char(const String & value);
  [[nodiscard]] inline String MakeFromLiteral_String(const String & value);
  template <typename T> [[nodiscard]] inline T MakeFromLiteral(const String & value);
  [[nodiscard]] inline String MakeUpper(const String & in);
  [[nodiscard]] inline String MakeLower(const String & in);
  [[nodiscard]] inline String MakeTitleCase(String in);
  [[nodiscard]] inline String MakeCount(int val, String item, const String & plural_suffix);
  [[nodiscard]] inline String MakeRoman(int val);
  template <typename CONTAINER_T>
  [[nodiscard]] inline String MakeEnglishList(const CONTAINER_T & container);
  template <typename... Args>
  [[nodiscard]] inline String MakeFormatted(const String& format, Args... args);
  [[nodiscard]] inline String MakeRepeat(String base, size_t n);

  [[nodiscard]] inline String MakeTrimFront(const String & in, const CharSet & chars=WhitespaceCharSet());
  [[nodiscard]] inline String MakeTrimBack(const String & in, const CharSet & chars=WhitespaceCharSet());
  [[nodiscard]] inline String MakeTrimmed(String in, const CharSet & chars=WhitespaceCharSet());
  [[nodiscard]] inline String MakeCompressed(
    String in, const CharSet & chars=WhitespaceCharSet(), char compress_to=' ',
    bool trim_start=true, bool trim_end=true);
  [[nodiscard]] inline String MakeRemoveChars(String in, const CharSet & chars);
  [[nodiscard]] inline String MakeSlugify(String in);

  template <typename CONTAINER_T>
  [[nodiscard]] inline String Join(const CONTAINER_T & container, std::string join_str="",
                                        std::string open="", std::string close="");

  class String : public std::string {
  private:
    using Syntax = StringSyntax;

    static char no_char;

    // _ToString specializations
    [[nodiscard]] static const String & _ToString(const String & in) { return in; }
    template <typename T> [[nodiscard]] static String _ToString(const std::vector<T> & in)
      { return String("{") + Join(in, ",") + '}'; }
    template <typename T> [[nodiscard]] static String _ToString(T && in)
      { std::stringstream ss; ss << std::forward<T>(in); return ss.str(); }

    // Convert objects into a streamable type using _Convert(obj)
    template <typename T> static decltype(std::declval<T>()._ToString()) _Convert(const T & in, bool) { return in._ToString(); }
    template <typename T> static auto _Convert(const T & in, int) -> decltype(_ToString(in)) { return _ToString(in); }

    // Quick check to ensure a position is legal.
    void _AssertPos([[maybe_unused]] size_t pos) const { emp_assert(pos < size(), pos, size()); }

  public:
    using std::string::value_type;
    using std::string::allocator_type;
    using std::string::size_type;
    using std::string::difference_type;
    using std::string::reference;
    using std::string::const_reference;
    using std::string::pointer;
    using std::string::const_pointer;
    using std::string::iterator;
    using std::string::const_iterator;
    using std::string::reverse_iterator;
    using std::string::const_reverse_iterator;

    // Constructors duplicating from std::string
    String() = default;
    String(const std::string & _str) : std::string(_str) { }
    String(std::string && _str) : std::string(std::move(_str)) { }
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
    String(const std::string & _str, std::function<String(char)> transform_fun) {
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
    String & operator=(std::string_view _in) { std::string::operator=(_in); return *this; }
    String & operator=(std::initializer_list<char> _in) { std::string::operator=(_in); return *this; }
    String & operator=(std::nullptr_t) = delete;

    template <typename... ARG_Ts>
    String & assign(ARG_Ts &&... args)
      { std::string::assign(std::forward<ARG_Ts>(args)...); return *this; }

    // ------ Static Values ------
    [[nodiscard]] static const String & Empty() { static String empty=""; return empty; }

    // ------ Element Access ------
    // Inherited functions from std::string:
    //  char * data();
    //  const char * data() const;
    //  const char * c_str() const;
    std::string & str() { return *this; }
    const std::string & str() const { return *this; }

    [[nodiscard]] char & operator[](size_t pos)
      { _AssertPos(pos); return std::string::operator[](pos); }
    [[nodiscard]] char operator[](size_t pos) const
      { _AssertPos(pos); return std::string::operator[](pos); }
    [[nodiscard]] char & front() { _AssertPos(0); return std::string::front(); }
    [[nodiscard]] char front() const { _AssertPos(0); return std::string::front(); }
    [[nodiscard]] char & back() { _AssertPos(0); return std::string::back(); }
    [[nodiscard]] char back() const { _AssertPos(0); return std::string::back(); }
    [[nodiscard]] char & Get(size_t pos) { return (pos < size()) ? operator[](pos) : (no_char='\0'); }
    [[nodiscard]] char Get(size_t pos) const { return (pos < size()) ? operator[](pos) : '\0'; }

    [[nodiscard]] String substr(size_t pos=0, size_t count=npos ) const
      { _AssertPos(pos); return std::string::substr(pos, count); }
    [[nodiscard]] String GetRange(std::size_t start_pos, std::size_t end_pos) const
      { return std::string::substr(start_pos, end_pos - start_pos); }

    [[nodiscard]] std::string_view View(size_t start=0, size_t out_size=npos) const {
      if (start == npos) start = size();                // Can occur w/failed find; return empty end.
      if (out_size == npos) out_size = size() - start;  // npos size => View to end.
      emp_assert(start + out_size <= size());
      return std::string_view(data()+start, out_size);
    }
    [[nodiscard]] std::string_view ViewFront(size_t out_size) const { return View(0, out_size); }
    [[nodiscard]] std::string_view ViewBack(size_t out_size) const
      { emp_assert(out_size <= size()); return View(size()-out_size, out_size); }
    [[nodiscard]] std::string_view ViewRange(size_t start, size_t end) const {
      emp_assert(start <= end && end <= size(), start, end, size());
      return View(start, end - start);
    }
    [[nodiscard]] std::string_view ViewTo(CharSet stop_chars, size_t start=0,
                                          const Syntax & syntax=Syntax::None()) const
      { return ViewFront(Find(stop_chars, start, syntax)); }
    [[nodiscard]] std::string_view ViewBackTo(CharSet stop_chars, size_t start=npos,
                                              const Syntax & syntax=Syntax::None()) const
      { return View(RFind(stop_chars, start, syntax)); }
    [[nodiscard]] std::string_view ViewWord(const Syntax & syntax=Syntax::Quotes(), size_t start=0) const
      { return ViewTo(" \n\t\r", start, syntax); }
    [[nodiscard]] std::string_view ViewLine(const Syntax & syntax=Syntax::Quotes(), size_t start=0) const
      { return ViewTo("\n", start, syntax); }


    // ------ Iterators ------
    // Inherited functions from std::string:
    //  iterator begin();
    //  const_iterator begin() const;
    //  const_iterator cbegin() const;
    //  reverse_iterator rbegin();
    //  const_reverse_iterator rbegin() const;
    //  const_reverse_iterator crbegin() const;
    //  iterator end();
    //  const_iterator end() const;
    //  const_iterator cend() const;
    //  reverse_iterator rend();
    //  const_reverse_iterator rend() const;
    //  const_reverse_iterator crend() const;


    // ------ Capacity ------
    // Inherited functions from std::string:
    //  bool empty() const;
    //  size_t size() const;
    //  size_t length() const;
    //  size_t max_size() const;
    //  size_t capacity() const;
    //  void reserve(size_t new_cap);
    //  void shrink_to_fit();
    

    // ------ Classification and Comparisons ------
    // Inherited functions from std::string:
    //  int compare(...);
    //  bool starts_with(...) const noexcept;
    //  bool ends_with(...) const noexcept;
    //  bool contains(...) const noexcept;

    [[nodiscard]] bool HasAt(const String test, size_t pos) const 
      { return (size() < test.size()+pos) && View(pos, test.size()) == test; }
    [[nodiscard]] bool HasPrefix(const String & prefix) const { return starts_with(prefix); }
    [[nodiscard]] bool HasSuffix(const String & suffix) const { return ends_with(suffix); }

    size_t Hash() const noexcept { return std::hash<std::string>{}(*this); }

    // ------ Simple Analysis ------

    // Count the number of occurrences of a specific character.
    [[nodiscard]] size_t Count(char c, size_t start=0) const
      { return (size_t) std::count(begin()+start, end(), c); }

    // Count the number of occurrences of a specific character within a range.
    [[nodiscard]] size_t Count(char c, size_t start, size_t end) const
      { return (size_t) std::count(begin()+start, begin()+end, c); }

    /// Test if an string is formatted as a literal character.
    [[nodiscard]] inline bool IsLiteralChar() const;

    /// Test if an string is formatted as a literal string.
    [[nodiscard]] inline bool IsLiteralString(const String & quote_marks="\"") const;

    /// Explain what string is NOT formatted as a literal string.
    [[nodiscard]] inline String DiagnoseLiteralString(const String & quote_marks="\"") const;

    /// Is string composed only of a set of characters (can be provided as a string)
    [[nodiscard]] bool IsComposedOf(CharSet char_set) const { return char_set.Has(*this); }

    /// Is string a valid number (int, floating point, or scientific notation all valid)
    [[nodiscard]] inline bool IsNumber() const;

    /// Is string a valid identifier? At least one char; cannot begin with digit, only letters, digits and `_`
    [[nodiscard]] bool IsIdentifier() const
      { return size() && !is_digit(Get(0)) && IDCharSet().Has(*this); }

    [[nodiscard]] bool OnlyLower() const { return LowerCharSet().Has(*this); }
    [[nodiscard]] bool OnlyUpper() const { return UpperCharSet().Has(*this); }
    [[nodiscard]] bool OnlyDigits() const { return DigitCharSet().Has(*this); }
    [[nodiscard]] bool OnlyAlphanumeric() const { return AlphanumericCharSet().Has(*this); }
    [[nodiscard]] bool OnlyWhitespace() const { return WhitespaceCharSet().Has(*this); }

    [[nodiscard]] bool HasOneOf(CharSet char_set) const { return char_set.HasAny(*this); }
    [[nodiscard]] bool Has(char c) const { return Find(c) != npos; }
    [[nodiscard]] bool HasWhitespace() const { return WhitespaceCharSet().HasAny(*this); }
    [[nodiscard]] bool HasNonwhitespace() const { return !WhitespaceCharSet().HasOnly(*this); }
    [[nodiscard]] bool HasUpper() const { return UpperCharSet().HasAny(*this); }
    [[nodiscard]] bool HasLower() const { return LowerCharSet().HasAny(*this); }
    [[nodiscard]] bool HasLetter() const { return LetterCharSet().HasAny(*this); }
    [[nodiscard]] bool HasDigit() const { return DigitCharSet().HasAny(*this); }
    [[nodiscard]] bool HasAlphanumeric() const { return AlphanumericCharSet().HasAny(*this); }

    [[nodiscard]] bool HasCharAt(char c, size_t pos) const
      { return (pos < size()) && (Get(pos) == c); }
    [[nodiscard]] bool HasOneOfAt(CharSet opts, size_t pos) const { return opts.HasAt(*this, pos); }
    [[nodiscard]] bool HasDigitAt(size_t pos) const { return DigitCharSet().HasAt(*this, pos); }
    [[nodiscard]] bool HasLetterAt(size_t pos) const { return LetterCharSet().HasAt(*this, pos); }
    [[nodiscard]] bool HasWhitespaceAt(size_t pos) const { return WhitespaceCharSet().HasAt(*this, pos); }

    [[nodiscard]] size_t CountWhitespace() const { return WhitespaceCharSet().CountMatches(*this); }
    [[nodiscard]] size_t CountNonwhitespace() const { return size() - CountWhitespace(); }
    [[nodiscard]] size_t CountUpper() const { return UpperCharSet().CountMatches(*this); }
    [[nodiscard]] size_t CountLower() const { return LowerCharSet().CountMatches(*this); }
    [[nodiscard]] size_t CountLetters() const { return LetterCharSet().CountMatches(*this); }
    [[nodiscard]] size_t CountDigits() const { return DigitCharSet().CountMatches(*this); }
    [[nodiscard]] size_t CountAlphanumeric() const { return AlphanumericCharSet().CountMatches(*this); }

    // ------ Removals and Extractions ------
    // Inherited functions from std::string:
    //  void pop_back();

    String & clear() noexcept { std::string::clear(); return *this; }

    String & erase(size_t index=0, size_t count=npos) { std::string::erase(index,count); return *this; }
    iterator erase(const_iterator pos) { return std::string::erase(pos); }
    iterator erase(const_iterator first, const_iterator last) { return std::string::erase(first, last); }

    String & RemoveWhitespace()  { return RemoveChars(WhitespaceCharSet()); }
    String & RemoveUpper()       { return RemoveChars(UpperCharSet()); }
    String & RemoveLower()       { return RemoveChars(LowerCharSet()); }
    String & RemoveLetters()     { return RemoveChars(LetterCharSet()); }
    String & RemoveDigits()      { return RemoveChars(DigitCharSet()); }
    String & RemovePunctuation() { return RemoveChars(PunctuationCharSet()); }

    // Return string_view starting at a specified position, and advance that position
    // Helper function for scanning.
    std::string_view ScanTo(size_t & pos, size_t stop_pos) const
      { const size_t start = pos; return ViewRange(start, pos=std::min(stop_pos, size())); }
    std::string_view ScanWhile(size_t & pos, std::function<bool(char)> test_fun) {
      const size_t start = pos;
      while (pos < size() && test_fun(Get(pos))) ++pos;
      return ViewRange(start, pos);
    }
    std::string_view ScanWord(size_t & pos) const { return ScanTo(pos, FindWhitespace(pos)); }
    char ScanChar(size_t & pos) const { return Get(pos++); }
    std::string_view ScanWhitespace(size_t & pos) const { return ScanTo(pos, FindNonWhitespace(pos)); }

    inline bool PopIf(char c);
    inline bool PopIf(String in);
    inline String PopAll();
    inline String PopFixed(std::size_t end_pos, size_t delim_size=0);
    inline String Pop(CharSet chars=" \n\t\r", const Syntax & syntax=Syntax::None());
    inline String PopTo(String delim, const Syntax & syntax=Syntax::None());
    String PopWord(const Syntax & syntax=Syntax::None()) { return Pop(" \n\t\r", syntax); }
    String PopLine(const Syntax & syntax=Syntax::None()) { return Pop("\n", syntax); }
    inline String PopQuote(const Syntax & syntax=Syntax::Quotes());
    inline String PopParen(const Syntax & syntax=Syntax::Parens());
    inline String PopLiteralSigned();
    long long PopSigned() { return std::stoll(PopLiteralSigned()); }
    inline String PopLiteralUnsigned();
    unsigned long long PopUnsigned() { return std::stoull(PopLiteralUnsigned()); }
    inline String PopLiteralFloat();
    double PopFloat() { return std::stod(PopLiteralFloat()); }
    String PopLiteralChar(const Syntax & syntax=Syntax::CharQuotes()) { return PopQuote(syntax); }
    char PopChar() { return PopFixed(1)[0]; }

    template <typename T> inline String PopLiteral(const Syntax & syntax=Syntax::Quotes());
    template <typename T> inline T PopFromLiteral(const Syntax & syntax=Syntax::Quotes());


    // ------ Insertions and Additions ------
    // Inherited functions from std::string:
    //  void push_back(char c);

    String & insert(size_t index, const String & in) { std::string::insert(index, in); return *this; }
    String & insert(size_t index, const String & in, size_t pos, size_t count=npos)
      { std::string::insert(index, in, pos, count); return *this; }
    template <typename... ARG_Ts> String & insert(size_t index, ARG_Ts &&... args)
      { std::string::insert(index, std::forward<ARG_Ts>(args)...); return *this; }
    template <typename... ARG_Ts> String & insert(const_iterator pos, ARG_Ts &&... args)
      { return std::string::insert(pos, std::forward<ARG_Ts>(args)...); }

    String & append(const String & in) { std::string::append(in); return *this; }
    String & append(const String & in, size_t pos, size_t count)
      { std::string::append(in, pos, count); return *this; }
    template <typename... ARG_Ts> String & append(ARG_Ts &&... args)
      { std::string::append(std::forward<ARG_Ts>(args)...); return *this; }

    template <typename ARG_T> String & operator+=(ARG_T && arg)
      { std::string::operator+=(std::forward<ARG_T>(arg)); return *this; }

    String & PadFront(char padding, size_t target_size) {
      if (size() < target_size) *this = std::string(target_size - size(), padding) + *this;
      return *this;
    }

    String & PadBack(char padding, size_t target_size) {
      if (size() < target_size) *this += std::string(target_size - size(), padding);
      return *this;
    }

    // ------ Direct Modifications ------
    // Inherited functions from std::string:
    //  size_t copy(char * dest, size_t count, size_t pos=0) const
    //  void swap(String & other)

    template <typename... ARG_Ts> String & replace(ARG_Ts &&... args)
      { std::string::replace(std::forward<ARG_Ts>(args)...); return *this; }

    String & resize( size_t count, char c='\0') { std::string::resize(count, c); return *this; }

    String & ReplaceChar(char from, char to, size_t start=0)
      { for (size_t i=start; i < size(); ++i) if (Get(i) == from) Get(i) = to; return *this; }
    String & ReplaceRange(size_t start, size_t end, String value)
      { return replace(start, end-start, value); }

    // Find any instances of ${X} and replace with dictionary lookup of X.
    template <typename MAP_T>
    String & ReplaceVars(const MAP_T & var_map, String symbol="$",
                         const Syntax & syntax=Syntax::Full());

    // Find any instance of MACRO_NAME(ARGS) and call replace it with return from fun(ARGS).
    template <typename FUN_T>
    String & ReplaceMacro(String start_str, String end_str, FUN_T && fun,
                          const Syntax & syntax=Syntax{"\"", "()"});


    // ------ Searching ------
    // Inherited functions from std::string:
    // size_t find(...) const
    // size_t rfind(...) const
    // size_t find_first_of(...) const
    // size_t find_first_not_of(...) const
    // size_t find_last_of(...) const
    // size_t find_last_not_of(...) const

    [[nodiscard]] inline size_t FindQuoteMatch(size_t pos=0) const;
    [[nodiscard]] inline size_t FindParenMatch(size_t pos=0, const Syntax & syntax=Syntax::Parens()) const;
    [[nodiscard]] inline size_t RFindQuoteMatch(size_t pos=npos) const;
    [[nodiscard]] inline size_t RFindParenMatch(size_t pos=npos, const Syntax & syntax=Syntax::RParens()) const;
    [[nodiscard]] inline size_t FindMatch(size_t pos=0, const Syntax & syntax=Syntax::Full()) const;
    [[nodiscard]] inline size_t RFindMatch(size_t pos=npos, const Syntax & syntax=Syntax::Full()) const;
    [[nodiscard]] inline size_t Find(char target, size_t start=0, const Syntax & syntax=Syntax::None()) const;
    [[nodiscard]] inline size_t Find(String target, size_t start=0, const Syntax & syntax=Syntax::None()) const;
    [[nodiscard]] inline size_t Find(const CharSet & char_set, size_t start=0, const Syntax & syntax=Syntax::None()) const;
    [[nodiscard]] inline size_t Find(const char * target, size_t start=0, const Syntax & syntax=Syntax::None()) const
       { return Find(String(target), start, syntax); }
    [[nodiscard]] inline size_t RFind(char target, size_t start=npos, const Syntax & syntax=Syntax::None()) const;
    [[nodiscard]] inline size_t RFind(String target, size_t start=npos, const Syntax & syntax=Syntax::None()) const;
    [[nodiscard]] inline size_t RFind(const CharSet & char_set, size_t start=npos, const Syntax & syntax=Syntax::None()) const;
    [[nodiscard]] inline size_t RFind(const char * target, size_t start=npos, const Syntax & syntax=Syntax::None()) const
       { return RFind(String(target), start, syntax); }
    inline void FindAll(char target, emp::vector<size_t> & results, const Syntax & syntax=Syntax::None()) const;
    [[nodiscard]] inline emp::vector<size_t> FindAll(char target, const Syntax & syntax=Syntax::None()) const;
    template <typename... Ts>
    [[nodiscard]] inline size_t FindAnyOfFrom(size_t start, String test1, Ts... tests) const;
    template <typename T, typename... Ts>
    [[nodiscard]] inline size_t FindAnyOf(T test1, Ts... tests) const;
    [[nodiscard]] inline size_t FindID(String target, size_t start, const Syntax & syntax=Syntax::Quotes()) const;

    [[nodiscard]] size_t FindWhitespace(size_t start=0, const Syntax & syntax=Syntax::None()) const 
      { return Find(WhitespaceCharSet(), start, syntax); }
    [[nodiscard]] size_t FindNonWhitespace(size_t start=0, const Syntax & syntax=Syntax::None()) const 
      { return Find(!WhitespaceCharSet(), start, syntax); }
    [[nodiscard]] size_t FindUpperChar(size_t start=0, const Syntax & syntax=Syntax::None()) const 
      { return Find(UpperCharSet(), start, syntax); }
    [[nodiscard]] size_t FindNonUpperChar(size_t start=0, const Syntax & syntax=Syntax::None()) const 
      { return Find(!UpperCharSet(), start, syntax); }
    [[nodiscard]] size_t FindLowerChar(size_t start=0, const Syntax & syntax=Syntax::None()) const 
      { return Find(LowerCharSet(), start, syntax); }
    [[nodiscard]] size_t FindNonLowerChar(size_t start=0, const Syntax & syntax=Syntax::None()) const 
      { return Find(!LowerCharSet(), start, syntax); }
    [[nodiscard]] size_t FindLetterChar(size_t start=0, const Syntax & syntax=Syntax::None()) const 
      { return Find(LetterCharSet(), start, syntax); }
    [[nodiscard]] size_t FindNonLetterChar(size_t start=0, const Syntax & syntax=Syntax::None()) const 
      { return Find(!LetterCharSet(), start, syntax); }
    [[nodiscard]] size_t FindDigitChar(size_t start=0, const Syntax & syntax=Syntax::None()) const 
      { return Find(DigitCharSet(), start, syntax); }
    [[nodiscard]] size_t FindNonDigitChar(size_t start=0, const Syntax & syntax=Syntax::None()) const 
      { return Find(!DigitCharSet(), start, syntax); }
    [[nodiscard]] size_t FindAlphanumericChar(size_t start=0, const Syntax & syntax=Syntax::None()) const 
      { return Find(AlphanumericCharSet(), start, syntax); }
    [[nodiscard]] size_t FindNonAlphanumericChar(size_t start=0, const Syntax & syntax=Syntax::None()) const 
      { return Find(!AlphanumericCharSet(), start, syntax); }
    [[nodiscard]] size_t FindIDChar(size_t start=0, const Syntax & syntax=Syntax::None()) const 
      { return Find(IDCharSet(), start, syntax); }
    [[nodiscard]] size_t FindNonIDChar(size_t start=0, const Syntax & syntax=Syntax::None()) const 
      { return Find(!IDCharSet(), start, syntax); }


    // ------ Other Views ------
    [[nodiscard]] std::string_view ViewNestedBlock(size_t start=0, const Syntax & syntax=Syntax::Quotes()) const
      { return ViewRange(start+1, FindParenMatch(start, syntax) - 1); }
    [[nodiscard]] std::string_view ViewQuote(size_t start=0, const Syntax & syntax=Syntax::Quotes()) const
      { return ViewRange(start, syntax.IsQuote(Get(start)) ? FindQuoteMatch(start) : start); }


    // ------ Transformations into non-Strings ------
    // Note: For efficiency there are two versions of most of these: one where the output
    // data structure is provided and one where it must be generated.

    inline void Slice(emp::vector<String> & out_set, String delim=",",
                      const Syntax & syntax=Syntax::Quotes(), bool trim_whitespace=false) const;

    [[nodiscard]]
    inline emp::vector<String> Slice(String delim=",", const Syntax & syntax=Syntax::Quotes(),
                                     bool trim_whitespace=false) const;

    inline void ViewSlices(
      emp::vector<std::string_view> & out_set,
      String delim=",",
      const Syntax & syntax=Syntax::Quotes()
    ) const;

    [[nodiscard]] inline emp::vector<std::string_view> ViewSlices(
      String delim=",",
      const Syntax & syntax=Syntax::Quotes()
    ) const;
  
    inline void SliceAssign(
      std::map<String,String> & out_map,
      String delim=",", String assign_op="=",
      const Syntax & syntax=Syntax::Quotes(), bool trim_whitespace=true
    ) const;

    [[nodiscard]] inline std::map<String,String> SliceAssign(
      String delim=",", String assign_op="=",
      const Syntax & syntax=Syntax::Quotes(), bool trim_whitespace=true
    ) const;


    // ------ Other Operators ------
    // Inherited functions from std::string:
    //  bool operator==(const String & in) const
    //  bool operator!=(const String & in) const
    //  bool operator<(const String & in) const
    //  bool operator<=(const String & in) const
    //  bool operator>(const String & in) const
    //  bool operator>=(const String & in) const
    //  bool operator<=>(const String & in) const
    //  std::string operator+(const std::string & in) const
    String operator*(size_t count) const {
      String out; out.reserve(size() * count);
      for (size_t i = 0; i < count; ++i) out += *this;
      return out;
    }
    

    //  ------ FORMATTING ------
    // Set* replaces the current string with argument OR converts the current string if no argument.
    // As* Returns a modified version of the current string, leaving original intact.
    // Append* adds to the end of the current string.
    // Most also have stand-alone Make* versions where the core implementation is found.

    template <typename... Ts>
    String & Append(Ts... args) { return *this += MakeString(std::forward<Ts>(args)...);}
    template <typename... Ts>
    String & Set(Ts... args) { return *this = MakeString(std::forward<Ts>(args)...); }
    template <typename T>
    T As() const { std::stringstream ss; ss << *this; T out; ss >> out; return out; }

    String & AppendEscaped(char c) { *this += MakeEscaped(c); return *this; }
    String & SetEscaped(char c) { *this = MakeEscaped(c); return *this; }

    String & AppendEscaped(const String & in) { *this+=MakeEscaped(in); return *this; }
    String & SetEscaped(const String & in) { *this = MakeEscaped(in); return *this; }
    String & SetEscaped() { *this = MakeEscaped(*this); return *this; }
    [[nodiscard]] String AsEscaped() const { return MakeEscaped(*this); }

    String & AppendCSVSafe(String in) { *this+=MakeCSVSafe(in); return *this; }
    String & SetCSVSafe(const String & in) { *this = MakeCSVSafe(in); return *this; }
    String & SetCSVSafe() { *this = MakeCSVSafe(*this); return *this; }
    [[nodiscard]] String AsCSVSafe() const { return MakeCSVSafe(*this); }

    String & AppendWebSafe(String in) { *this+=MakeWebSafe(in); return *this; }
    String & SetWebSafe(const String & in) { *this = MakeWebSafe(in); return *this; }
    String & SetWebSafe() { *this = MakeWebSafe(*this); return *this; }
    [[nodiscard]] String AsWebSafe() const { return MakeWebSafe(*this); }

    // <= Creating Literals =>
    template <typename T>
    String & AppendLiteral(const T & in) { *this+=MakeLiteral(in); return *this; }
    template <typename T>
    String & SetLiteral(const T & in) { *this = MakeLiteral(in); return *this;; }
    String & SetLiteral() { *this = MakeLiteral(*this); return *this; }
    [[nodiscard]] String AsLiteral() const { return MakeLiteral(*this); }

    String & AppendUpper(const String & in) { *this+=MakeUpper(in); return *this; }
    String & SetUpper(const String & in) { *this = MakeUpper(in); return *this; }
    String & SetUpper() { *this = MakeUpper(*this); return *this; }
    [[nodiscard]] String AsUpper() { return MakeUpper(*this); }

    String & AppendLower(const String & in) { *this+=MakeLower(in); return *this; }
    String & SetLower(const String & in) { *this = MakeLower(in); return *this; }
    String & SetLower() { *this = MakeLower(*this); return *this; }
    [[nodiscard]] String AsLower() { return MakeLower(*this); }

    String & AppendTitleCase(const String & in) { *this+=MakeTitleCase(in); return *this; }
    String & SetTitleCase(const String & in) { *this = MakeTitleCase(in); return *this; }
    String & SetTitleCase() { *this = MakeTitleCase(*this); return *this; }
    [[nodiscard]] String AsTitleCase() const { return MakeTitleCase(*this); }

    String & AppendCount(int val, String item, String suffix="s")
      { *this+=MakeCount(val, item, suffix); return *this; }
    String & SetCount(int val, String item, String suffix="s")
      { *this = MakeCount(val, item, suffix); return *this; }
    String & SetAsCount(int val, String suffix="s")
      { *this = MakeCount(val, *this, suffix); return *this; }

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


    String & AppendTrimFront(const String & in, const CharSet & chars=WhitespaceCharSet())
      { return *this += MakeTrimFront(in, chars); }
    String & SetTrimFront(const String & in, const CharSet & chars=WhitespaceCharSet())
      { return *this = MakeTrimFront(in, chars); }
    String & TrimFront(const CharSet & chars=WhitespaceCharSet())
      { return *this = MakeTrimFront(*this, chars); }
    [[nodiscard]] String AsTrimFront(const CharSet & chars=WhitespaceCharSet()) const
      { return MakeTrimFront(*this, chars); }

    String & AppendTrimBack(const String & in, const CharSet & chars=WhitespaceCharSet())
      { return *this += MakeTrimBack(in, chars); }
    String & SetTrimBack(const String & in, const CharSet & chars=WhitespaceCharSet())
      { return *this = MakeTrimBack(in, chars); }
    String & TrimBack(const CharSet & chars=WhitespaceCharSet())
      { return *this = MakeTrimBack(*this, chars); }
    [[nodiscard]] String AsTrimBack(const CharSet & chars=WhitespaceCharSet()) const
      { return MakeTrimBack(*this, chars); }

    String & AppendTrimmed(const String & in, const CharSet & chars=WhitespaceCharSet())
      { return *this += MakeTrimmed(in, chars); }
    String & SetTrimmed(const String & in, const CharSet & chars=WhitespaceCharSet())
      { return *this = MakeTrimmed(in, chars); }
    String & Trim(const CharSet & chars=WhitespaceCharSet())
      { return *this = MakeTrimmed(*this, chars); }
    [[nodiscard]] String AsTrimmed(const CharSet & chars=WhitespaceCharSet()) const
      { return MakeTrimmed(*this, chars); }

    String & AppendCompressed(const String & in, const CharSet & chars=WhitespaceCharSet(),
                              char compress_to=' ', bool trim_start=true, bool trim_end=true)
      { return *this += MakeCompressed(in, chars, compress_to, trim_start, trim_end); }
    String & SetCompressed(const String & in, const CharSet & chars=WhitespaceCharSet(),
                           char compress_to=' ', bool trim_start=true, bool trim_end=true)
      { return *this = MakeCompressed(in, chars, compress_to, trim_start, trim_end); }
    String & Compress(const CharSet & chars=WhitespaceCharSet(), char compress_to=' ',
                      bool trim_start=true, bool trim_end=true)
      { return *this = MakeCompressed(*this, chars, compress_to, trim_start, trim_end); }
    [[nodiscard]] String AsCompressed(const CharSet & chars=WhitespaceCharSet(), char compress_to=' ',
                                      bool trim_start=true, bool trim_end=true) const
      { return MakeCompressed(*this, chars, compress_to, trim_start, trim_end); }

    String & AppendRemoveChars(const String & in, const CharSet & chars)
      { return *this += MakeRemoveChars(in, chars); }
    String & SetRemoveChars(const String & in, const CharSet & chars)
      { return *this = MakeRemoveChars(in, chars); }
    String & RemoveChars(const CharSet & chars)
      { return *this = MakeRemoveChars(*this, chars); }
    [[nodiscard]] String AsRemoveChars(const CharSet & chars) const
      { return MakeRemoveChars(*this, chars); }

    String & AppendSlugify(String in) { return *this += std::move(in.Slugify()); }
    String & SetSlugify(String in) { return *this = std::move(in.Slugify()); }
    String & Slugify()
      { return SetLower().RemovePunctuation().Compress().ReplaceChar(' ', '-'); }
    [[nodiscard]] String AsSlugify() const { return MakeSlugify(*this); }

    template <typename CONTAINER_T>
    String & AppendJoin(const CONTAINER_T & container, String delim="",
                        String open="", String close="")
      { *this += Join(container, delim, open, close); return *this;}
    template <typename CONTAINER_T>
    String & SetJoin(const CONTAINER_T & container, String delim="",
                     String open="", String close="")
      { *this = Join(container, delim, open, close); return *this;}

    template <typename... Ts>
    [[nodiscard]] static String Make(Ts... args) {
      std::stringstream ss;
      (ss << ... << String::_Convert(std::forward<Ts>(args), true));
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
    if (!size()) return false;               // If string is empty, not a number!
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
    const char mark = Get(pos);
    while (++pos < size()) {
      if (Get(pos) == '\\') { ++pos; continue; } // Skip escaped characters in quotes
      if (Get(pos) == mark) { // Found match!
        return pos;
      }
    }
    return npos; // Not found.
  }

  // Given the start position of a quote, find where it ends; marks must be identical
  size_t String::RFindQuoteMatch(size_t pos) const {
    if (pos >= size()) pos = size() - 1;
    const char mark = Get(pos);
    while (--pos < size()) {
      if (Get(pos) == mark) { // Found possible match! See if it is escaped...
        size_t esc_count = 0;
        while ((pos-esc_count) && Get(pos-esc_count-1) == '\\') esc_count++;
        if (esc_count%2 == 0) return pos;
      }
    }
    return npos; // Not found.
  }


  // Given an open parenthesis, find where it closes (including nesting).  Marks must be different.
  size_t String::FindParenMatch(size_t pos, const Syntax & syntax) const {
    const char open = Get(pos);
    if (!syntax.IsParen(open)) return npos; // Not a paren that we know!
    const char close = syntax.GetMatch(open);
    size_t open_count = 1;
    while (++pos < size()) {
      const char c = Get(pos);
      if (c == open) ++open_count;
      else if (c == close) {
        if (--open_count == 0) return pos;
      }
      else if (syntax.IsQuote(c)) pos = FindQuoteMatch(pos);
    }

    return npos;
  }

  // Given an open parenthesis, find where it closes (including nesting).  Marks must be different.
  size_t String::RFindParenMatch(size_t pos, const Syntax & syntax) const {
    if (pos >= size()) pos = size() - 1;
    const char open = Get(pos);
    if (!syntax.IsParen(open)) return npos; // Not a paren that we know!
    const char close = syntax.GetMatch(open);
    size_t open_count = 1;
    while (--pos < size()) {
      const char c = Get(pos);
      if (c == open) ++open_count;
      else if (c == close) {
        if (--open_count == 0) return pos;
      }
      else if (syntax.IsQuote(c)) pos = FindQuoteMatch(pos);
    }

    return npos;
  }

  size_t String::FindMatch(size_t pos, const Syntax & syntax) const {
    if (syntax.IsQuote(Get(pos))) return FindQuoteMatch(pos);
    if (syntax.IsParen(Get(pos))) return FindParenMatch(pos, syntax);
    return npos;
  }

  size_t String::RFindMatch(size_t pos, const Syntax & syntax) const {
    if (pos >= size()) pos = size() - 1;
    if (syntax.IsQuote(Get(pos))) return RFindQuoteMatch(pos);
    if (syntax.IsParen(Get(pos))) return RFindParenMatch(pos, syntax);
    return npos;
  }

  // A version of string::find() with single chars that can skip over quotes.
  size_t String::Find(char target, size_t start, const Syntax & syntax) const {
    // Make sure found_pos is not in a quote and/or parens; adjust as needed!
    for (size_t pos=start; pos < size(); ++pos) {
      if (Get(pos) == target) return pos;
      else if (syntax.IsQuote(Get(pos))) pos = FindQuoteMatch(pos);
      else if (syntax.IsParen(Get(pos))) pos = FindParenMatch(pos, syntax);
    }

    return npos;
  }

  // A version of string::find() with single chars that can skip over quotes.
  size_t String::RFind(char target, size_t start, const Syntax & syntax) const {
    if (start >= size()) start = size() - 1;
    // Make sure found_pos is not in a quote and/or parens; adjust as needed!
    for (size_t pos=start; pos < size(); --pos) {
      if (Get(pos) == target) return pos;
      else if (syntax.IsQuote(Get(pos))) pos = RFindQuoteMatch(pos);
      else if (syntax.IsParen(Get(pos))) pos = RFindParenMatch(pos, syntax);
    }

    return npos;
  }

  // A version of string::find() that can skip over quotes.
  size_t String::Find(String target, size_t start, const Syntax & syntax) const {
    size_t found_pos = find(target, start);
    if (syntax.GetCount() == 0) return found_pos;

    // Make sure found_pos is not in a quote and/or parens; adjust as needed!
    for (size_t scan_pos=0;
         scan_pos < found_pos && found_pos != npos;
         scan_pos++)
    {
      // Skip quotes, if needed...
      if (syntax.IsQuote(Get(scan_pos))) {
        scan_pos = FindQuoteMatch(scan_pos);
        if (found_pos < scan_pos) found_pos = find(target, scan_pos);
      }
      else if (syntax.IsParen(Get(scan_pos))) {
        scan_pos = FindParenMatch(scan_pos, syntax);
        if (found_pos < scan_pos) found_pos = find(target, scan_pos);
      }
    }

    return found_pos;
  }

  // A version of string::find() that can skip over quotes.
  size_t String::RFind(String target, size_t start, const Syntax & syntax) const {
    if (start >= size()) start = size() - 1;
    size_t found_pos = rfind(target, start);
    if (syntax.GetCount() == 0) return found_pos;

    // Make sure found_pos is not in a quote and/or parens; adjust as needed!
    for (size_t scan_pos=size()-1;
         scan_pos > found_pos && found_pos != npos;
         scan_pos--)
    {
      // Skip quotes, if needed...
      if (syntax.IsQuote(Get(scan_pos))) {
        scan_pos = RFindQuoteMatch(scan_pos);
        if (found_pos > scan_pos) found_pos = rfind(target, scan_pos);
      }
      else if (syntax.IsParen(Get(scan_pos))) {
        scan_pos = RFindParenMatch(scan_pos);
        if (found_pos > scan_pos) found_pos = rfind(target, scan_pos);
      }
    }

    return found_pos;
  }

  // Find any of a set of characters.
  size_t String::Find(const CharSet & char_set, size_t start, const Syntax & syntax) const
  {
    // Make sure found_pos is not in a quote and/or parens; adjust as needed!
    for (size_t pos=start; pos < size(); ++pos) {
      if (char_set.Has(Get(pos))) return pos;
      else if (syntax.IsQuote(Get(pos))) pos = FindQuoteMatch(pos);
      else if (syntax.IsParen(Get(pos))) pos = FindParenMatch(pos, syntax);
    }

    return npos;
  }

  // Find any of a set of characters.
  size_t String::RFind(const CharSet & char_set, size_t start, const Syntax & syntax) const
  {
    if (start >= size()) start = size() - 1;
    // Make sure found_pos is not in a quote and/or parens; adjust as needed!
    for (size_t pos=start; pos < size(); --pos) {
      if (char_set.Has(Get(pos))) return pos;
      else if (syntax.IsQuote(Get(pos))) pos = RFindQuoteMatch(pos);
      else if (syntax.IsParen(Get(pos))) pos = RFindParenMatch(pos, syntax);
    }

    return npos;
  }


  void String::FindAll(char target, emp::vector<size_t> & results, const Syntax & syntax) const {
    results.resize(0);
    for (size_t pos=0; pos < size(); ++pos) {
      if (Get(pos) == target) results.push_back(pos);

      // Skip quotes, if needed...
      if (syntax.IsQuote(Get(pos))) pos = FindQuoteMatch(pos);
      else if (syntax.IsParen(Get(pos))) pos = FindParenMatch(pos, syntax);
    }
  }

  emp::vector<size_t> String::FindAll(char target, const Syntax & syntax) const {
    emp::vector<size_t> out;
    FindAll(target, out, syntax);
    return out;
  }

  template <typename... Ts>
  size_t String::FindAnyOfFrom(size_t start, String test1, Ts... tests) const {
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
  size_t String::FindID(String target, size_t start, const Syntax & syntax) const
  {
    size_t pos = Find(target, start, syntax);
    while (pos != npos) {
      bool before_ok = (pos == 0) || !is_idchar(Get(pos-1));
      size_t after_pos = pos+target.size();
      bool after_ok = (after_pos == size()) || !is_idchar(Get(after_pos));
      if (before_ok && after_ok) return pos;

      pos = Find(target, pos+target.size(), syntax);
    }

    return npos;
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
  String String::PopAll() {
    String out;
    swap(out);
    return out;
  }

  /// Pop a segment from the beginning of a string as another string, shortening original.
  String String::PopFixed(std::size_t end_pos, size_t delim_size)
  {
    if (!end_pos) return "";                  // Not popping anything!
    if (end_pos >= size()) return PopAll();   // Popping everything!

    String out = substr(0, end_pos);     // Copy up to the deliminator for ouput
    erase(0, end_pos + delim_size);       // Delete output string AND delimiter
    return out;
  }

  /// Remove a prefix of the string (up to a specified delimeter) and return it.  If the
  /// delimeter is not found, return the entire string and clear it.
  String String::Pop(CharSet chars, const Syntax & syntax) {
    size_t pop_end = Find(chars, 0, syntax);
    size_t delim_size = pop_end == npos ? 0 : 1;
    return PopFixed(pop_end, delim_size);
  }

  /// Remove a prefix of the string (up to a specified delimeter) and return it.  If the
  /// delimeter is not found, return the entire string and clear it.
  String String::PopTo(String delim, const Syntax & syntax) {
    return PopFixed(Find(delim, 0, syntax), delim.size());
  }

  String String::PopQuote(const Syntax & syntax) {
    if (!syntax.IsQuote(Get(0))) return String();
    const size_t end_pos = FindQuoteMatch(0);
    return (end_pos==std::string::npos) ? "" : PopFixed(end_pos+1);
  }

  String String::PopParen(const Syntax & syntax) {
    const size_t end_pos = FindParenMatch(0, syntax);
    return (end_pos==std::string::npos) ? "" : PopFixed(end_pos+1);
  }

  String String::PopLiteralSigned() {
    size_t int_size = 0;
    if (HasCharAt(int_size, '-') || HasCharAt(int_size, '+')) int_size++;
    while (int_size < size() && isdigit(Get(int_size))) ++int_size;
    return PopFixed(int_size);
  }

  String String::PopLiteralUnsigned() {
    size_t uint_size = 0;
    while (uint_size < size() && isdigit(Get(uint_size))) ++uint_size;
    return PopFixed(uint_size);
  }

  String String::PopLiteralFloat() {
    if (!size()) return "";                  // If string is empty, not a number!
    size_t pos = 0;
    if (HasOneOfAt("+-", pos)) ++pos;        // Allow leading +/-
    while (HasDigitAt(pos)) ++pos;           // Any number of digits (none is okay)
    if (HasCharAt('.', pos)) {               // If there's a DECIMAL PLACE, look for more digits.
      ++pos;                                 // Skip over the dot.
      if (!HasDigitAt(pos++)) return "";     // Must have at least one digit after '.'
      while (HasDigitAt(pos)) ++pos;         // Any number of digits is okay.
    }
    if (HasOneOfAt("eE", pos)) {             // If there's an e... SCIENTIFIC NOTATION!
      ++pos;                                 // Skip over the e.
      if (HasOneOfAt("+-", pos)) ++pos;      // skip leading +/-
      if (!HasDigitAt(pos++)) return "";     // Must have at least one digit after 'e'
      while (HasDigitAt(pos)) ++pos;         // Allow for MORE digits.
    }
    return PopFixed(pos);
  }

  template <typename T>
  String String::PopLiteral(const Syntax & syntax) {
    if constexpr (std::is_same_v<char, T>) return PopLiteralChar(syntax);
    else if constexpr (std::is_base_of_v<std::string, T>) PopQuote(syntax);
    else if constexpr (std::is_floating_point_v<T>) return PopLiteralFloat();
    else if constexpr (std::is_unsigned_v<T>) return PopLiteralUnsigned();
    else if constexpr (std::is_signed_v<T>) return PopLiteralSigned();
    else static_assert(emp::dependent_false<T>(), "Invalid conversion for PopLiteral()");

    return "";
  }

  template <typename T>
  T String::PopFromLiteral(const Syntax & syntax) {
    return MakeFromLiteral<T>(PopLiteral<T>(syntax));    
  }

  /// @brief Cut up a string based on the provided delimiter; fill them in to the provided vector.
  /// @param out_set destination vector
  /// @param delim delimiter to split on (default: ",")
  /// @param syntax identify quotes and parens that should be kept together.
  /// @param trim_whitespace Should whitespace around delim or assign be ignored? (default: true)
  void String::Slice(
    emp::vector<String> & out_set, String delim, const Syntax & syntax, bool trim_whitespace
  ) const {
    if (empty()) return; // Nothing to set!

    size_t start_pos = 0;
    size_t found_pos = Find(delim, 0, syntax);
    while (found_pos < size()) {
      out_set.push_back( GetRange(start_pos, found_pos) );
      if (trim_whitespace) out_set.back().Trim();
      start_pos = found_pos+delim.size();
      found_pos = Find(delim, found_pos+1, syntax);
    }
    out_set.push_back( GetRange(start_pos, found_pos) );
  }


  /// @brief Slice a String on a delimiter; return a vector of results.
  /// @note May be less efficient, but easier than other version of Slice()
  /// @param delim delimiter to split on (default: ",")
  /// @param syntax identify quotes and parens that should be kept together.
  /// @param trim_whitespace Should whitespace around delim or assign be ignored? (default: true)
  emp::vector<String> String::Slice(
    String delim, const Syntax & syntax, bool trim_whitespace
  ) const {
    emp::vector<String> result;
    Slice(result, delim, syntax, trim_whitespace);
    return result;
  }

  /// @brief Fill vector out_set of string_views based on the provided delimiter.
  /// @param out_set destination vector
  /// @param delim delimiter to split on (default: ",")
  /// @param syntax identify quotes and parens that should be kept together.
  void String::ViewSlices(
    emp::vector<std::string_view> & out_set, String delim, const Syntax & syntax
  ) const {
    out_set.resize(0);
    if (empty()) return; // Nothing to set!

    size_t start_pos = 0;
    size_t found_pos = Find(delim, 0, syntax);
    while (found_pos < size()) {
      out_set.push_back( ViewRange(start_pos, found_pos) );
      start_pos = found_pos+delim.size();
      found_pos = Find(delim, found_pos+1, syntax);
    }
    out_set.push_back( View(start_pos) );
  }

  /// @brief Generate vector of string_views based on the provided delimiter.
  /// @param delim delimiter to split on (default: ",")
  /// @param syntax identify quotes and parens that should be kept together.
  [[nodiscard]] emp::vector<std::string_view> String::ViewSlices(
    String delim, const Syntax & syntax
  ) const {
    emp::vector<std::string_view> result;
    ViewSlices(result, delim, syntax);
    return result;
  }
  

  /// @brief Slice a string and treat each section as an assignment; place results in provided map.
  /// @param delim delimiter to split on (default ',')
  /// @param assign_op separator for left and right side of assignment (default: "=")
  /// @param syntax identify quotes and parens that should be kept together.
  /// @param trim_whitespace Should whitespace around delim or assign be ignored? (default: true)
  void String::SliceAssign(
    std::map<String,String> & result_map,
    String delim, String assign_op,
    const Syntax & syntax, bool trim_whitespace
  ) const {
    auto assign_set = Slice(delim, syntax, false);
    for (auto setting : assign_set) {
      if (setting.OnlyWhitespace()) continue; // Skip blank settings (especially at the end).

      // Remove any extra spaces around parsed values.
      String var_name = setting.PopTo(assign_op);
      if (trim_whitespace) {
        var_name.Trim();
        setting.Trim();
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
  /// @param syntax identify quotes and parens that should be kept together.
  /// @param trim_whitespace Should whitespace around delim or assign be ignored? (default: true)
  std::map<String,String> String::SliceAssign(
    String delim, String assign_op,
    const Syntax & syntax, bool trim_whitespace
  ) const {
    std::map<String,String> result_map;
    SliceAssign(result_map, delim, assign_op, syntax, trim_whitespace);
    return result_map;
  }


  /// Find any instances of ${X} and replace with dictionary lookup of X.
  template <typename MAP_T>
  String & String::ReplaceVars(const MAP_T & var_map, String symbol, const Syntax & syntax) {
    for (size_t pos = Find(symbol, 0, syntax);
         pos < size()-3;             // Need room for a replacement tag.
         pos = Find(symbol, pos+symbol.size(), syntax)) {
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

      String key = GetRange(symbol_end+1, end_pos);
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
  /// @param syntax What values should we skip as quotes or parens?
  /// @return This string object, post processing.
  /// @todo Separate syntax into find_start syntax and find_end (inside macro) syntax.
  template <typename FUN_T>
  String & String::ReplaceMacro(
    String start_str,
    String end_str,
    FUN_T && macro_fun,
    const String::Syntax & syntax
  ) {
    // We need to identify the comparator and divide up arguments in macro.
    size_t macro_count = 0;     // Count of the number of hits for this macro.
    size_t line_num = 0;        // Line number where current macro hit was found.
    size_t prev_pos = 0;
    for (size_t macro_pos = Find(start_str, 0, syntax);
         macro_pos != npos;
         macro_pos = Find(start_str, macro_pos+1, syntax))
    {
      // Make sure we're not just extending a previous identifier.
      if (macro_pos && is_idchar(Get(macro_pos-1))) continue;

      line_num += Count('\n', prev_pos, macro_pos);  // Count lines leading up to this macro.

      // Isolate this macro instance and call the conversion function.
      size_t end_pos = Find(end_str, macro_pos+start_str.size(), syntax);
      const String macro_body = GetRange(macro_pos+start_str.size(), end_pos);

      String new_str = macro_fun(macro_body, line_num, macro_count);
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
  bool String::IsLiteralString(const String & quote_marks) const {
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
  String String::DiagnoseLiteralString(const String & quote_marks) const {
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


  template <typename... Ts>
  String MakeString(Ts &&... args) {
    return String::Make(std::forward<Ts>(args)...);
  }

  String MakeEscaped(char c) {
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
  
  String MakeEscaped(const String & in) {
    return String(in, [](char c){ return MakeEscaped(c); });
  }

  String MakeCSVSafe(const String & in) {
    bool needsEscape = in.find_first_of(",\"\n\r") != String::npos;
    if (!needsEscape) return in;

    String out;
    out.reserve(in.size() + 2); // Reserve space for quotes and the string content

    out.push_back('\"');
    for (const char c : in) {
      if (c == '\"') out.push_back('\"'); // Add a backslash before quotes.
      out.push_back(c);
    }
    out.push_back('\"');

    return out;
  }

  /// Take a string and replace reserved HTML characters with character entities
  String MakeWebSafe(const String & in) {
    String out;
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
  [[nodiscard]] String MakeLiteral(char value) {
    std::stringstream ss;
    ss << "'" << MakeEscaped(value) << "'";
    return ss.str();
  }

  /// Take a string or iterable and convert it to a C++-style literal.
  // This is the version for string. The version for an iterable is below.
  [[nodiscard]] String MakeLiteral(const std::string & value) {
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
  template <typename T> [[nodiscard]] String MakeLiteral(const T & value) {
    std::stringstream ss;
    if constexpr (std::is_convertible_v<T,std::string>) {
      std::string str(value);
      return MakeLiteral(str);
    }
    else if constexpr (emp::IsIterable<T>::value) {
      ss << "{ ";
      for (auto it = std::begin( value ); it != std::end( value ); ++it) {
        if (it != std::begin( value )) ss << ",";
        ss << MakeLiteral( *it );
      }
      ss << " }";
    }
    else if constexpr (std::is_arithmetic_v<T>) {
      ss << value;
      String out = ss.str();
      // If there is a decimal point, remove extra zeros at back (and point if needed)
      if (out.Has('.')) out.TrimBack('0').TrimBack('.');
      return out;
    }
    else ss << value;
    return ss.str();
  }

  #endif

  /// Convert a literal character representation to an actual string.
  /// (i.e., 'A', ';', or '\n')
  [[nodiscard]] char MakeFromLiteral_Char(const String & value) {
    emp_assert(value.IsLiteralChar());
    // Given the assert, we can assume the string DOES contain a literal representation,
    // and we just need to convert it.

    if (value.size() == 3) return value[1];
    if (value.size() == 4) return emp::ToEscapeChar(value[2]);

    return '\0'; // Error!
  }


  /// Convert a literal string representation to an actual string.
  [[nodiscard]] String MakeFromLiteral_String(const String & value) {
  /// Convert a literal string representation to an actual string.
    emp_assert(value.IsLiteralString(), value, value.DiagnoseLiteralString());
    // Given the assert, we can assume string DOES contain a literal string representation.

    String out_string;
    out_string.reserve(value.size()-2);  // Make a guess on final size.

    for (size_t pos = 1; pos < value.size() - 1; pos++) {
      // If we don't have an escaped character, just move it over.
      if (value[pos] != '\\') out_string.push_back(value[pos]);
      else out_string.push_back(emp::ToEscapeChar(value[++pos]));
    }

    return out_string;
  }

  template <typename T> 
  [[nodiscard]] T MakeFromLiteral(const String & value) {
    if (value.size() == 0) return T{};

    if constexpr (std::is_same_v<char, T>) return MakeFromLiteral_Char(value);
    else if constexpr (std::is_base_of_v<std::string, T>) return MakeFromLiteral_String(value);
    else if constexpr (std::is_floating_point_v<T>) return static_cast<T>(std::stold(value));
    else if constexpr (std::is_unsigned_v<T>) return static_cast<T>(std::stoull(value));
    else if constexpr (std::is_signed_v<T>) return static_cast<T>(std::stoll(value));
    else {
      static_assert(emp::dependent_false<T>(), "Invalid conversion for MakeFromLiteral()");
      return T{};
    }
  }

  /// Convert a string to all uppercase.
  [[nodiscard]] String MakeUpper(const String & value) {
    return String(value, [](char c){ return std::toupper(c); });
  }

  /// Convert a string to all lowercase.
  [[nodiscard]] String MakeLower(const String & value) {
    return String(value, [](char c){ return std::tolower(c); });
  }

  /// Make first letter of each word upper case
  [[nodiscard]] String MakeTitleCase(String value) {
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

   /// Make a string with the correct pluralization of the item being counted.  For example,
   /// MakeCount(1, "cow") would produce "1 cow", but MakeCount(2, "cow") would produce "2 cows".
  [[nodiscard]] String MakeCount(int val, String item, const String & plural_suffix) {
    if (std::abs(val) == 1) return MakeString(val, ' ', item);
    return MakeString(val, " ", item, plural_suffix);
  }

  [[nodiscard]] inline String MakeCount(int val, String item) { return MakeCount(val, item, "s"); }

  /// Convert an integer to a roman numeral string.
  [[nodiscard]] String MakeRoman(int val) {
    String out;
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
  String MakeEnglishList(const CONTAINER_T & container) {
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
  String MakeFormatted(const std::string& format, Args... args) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-security"

    // Extra space for '\0'
    const size_t size = static_cast<size_t>(std::snprintf(nullptr, 0, format.c_str(), args...) + 1);

    emp::vector<char> buf(size);
    std::snprintf(buf.data(), size, format.c_str(), args...);

     // We don't want the '\0' inside
    return String( buf.data(), buf.data() + size - 1 );

    #pragma GCC diagnostic pop
  }

  /// Concatenate n copies of a string.
  String MakeRepeat(String base, size_t n ) {
    String out;
    out.reserve(n * base.size());
    for (size_t i=0; i < n; ++i) out += base;
    return out;
  }

  String MakeTrimFront(const String & in, const CharSet & chars)
    { return in.substr( chars.CountFrontMatches(in) ); }

  String MakeTrimBack(const String & in, const CharSet & chars)
    { return in.substr(0, in.size() - chars.CountBackMatches(in)); }

  /// Remove chars from the beginning AND end of a string.
  String MakeTrimmed(String in, const CharSet & chars)
    { return in.TrimFront(chars).TrimBack(chars); }

  // Take a set of characters and compress sequences of them down to a single character.
  String MakeCompressed(String in, const CharSet & chars, char compress_to,
                        bool trim_start, bool trim_end)
  {
    bool skip_next = trim_start;     // Remove characters from beginning of line?
    size_t pos = 0;
    for (const auto c : in) {
      if (chars.Has(c)) {            // This char should be compressed.
        if (skip_next) continue;     // Already skipping...
        in.Get(pos++) = compress_to; // Convert any block of chars to a single replace char.
        skip_next = true;
      } else {                       // Not a compress char.
        in.Get(pos++) = c;
        skip_next = false;
      }
    }

    if (trim_end && pos && skip_next) pos--;   // Remove chars from end if needed.
    in.resize(pos);

    return in;
  }

  /// Remove all instances of specified characters from file.
  String MakeRemoveChars(String in, const CharSet & chars) {
    size_t cur_pos = 0;
    for (const auto c : in) {
      if (!chars.Has(c)) in.Get(cur_pos++) = c;
    }
    in.resize(cur_pos);
    return in;
  }

  /// Make a string safe(r)
  String MakeSlugify(String in) {
    return in.SetLower().RemovePunctuation().Compress().ReplaceChar(' ', '-');
  }

  /// This function returns values from a container as a single string separated
  /// by a given delimeter and with optional surrounding strings.
  /// @param container is any standard-interface container holding objects to be joined.
  /// @param join_str optional delimeter
  /// @param open string to place before each string (e.g., "[" or "'")
  /// @param close string to place after each string (e.g., "]" or "'")
  /// @return merged string of all values
  template <typename CONTAINER_T>
  String Join(const CONTAINER_T & container, std::string join_str,
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

template <>
struct std::hash<emp::String> {
  size_t operator()(const emp::String & str) const noexcept { return str.Hash(); }
};


#endif // #ifndef EMP_TOOLS_STRING_HPP_INCLUDE
