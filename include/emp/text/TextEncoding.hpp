/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023.
 *
 *  @file TextEncoding.hpp
 *  @brief Plugs into emp::Text with the specification of a given encoding.
 *  @note Status: ALPHA
 * 
 */

#ifndef EMP_TOOLS_TEXT_ENCODING_HPP_INCLUDE
#define EMP_TOOLS_TEXT_ENCODING_HPP_INCLUDE

#include <set>
#include <string>
#include <unordered_map>

#include "../base/assert.hpp"
#include "../compiler/Lexer.hpp"

#include "Text.hpp"

namespace emp {

  class TextEncoding : public emp::TextEncoding_Base {
  protected:
    // A categorical description of how a particular style tag should work.
    enum class TagType {
      UNKNOWN = 0, // Default state; indicates a tag unknown to this encoding.
      OPEN,        // Start a new style that needs to be ended (e.g., '<b>' in HTML)
      CLOSE,       // Stop using a style (e.g., '</b>' in HTML)
      TOGGLE,      // Toggle a style on/off (e.g., '*' in markdown)
      LINE,        // Set a style until the end of line (e.g., # in markdown)
      CHAR,        // Set a style for only a single character (e.g., <li> in HTML)
      STRUCTURE,   // Indicate a structural element (e.g., '<div>' in HTML)
      REPLACE      // Indicate a simple replacement (e.g., '&lt;' means '<' in HTML)
    };

    // TagInfo provides detailed information for how tags should be handled.
    struct TagInfo {
      TagType type = TagType::UNKNOWN;
      std::string base_style; // The base style that this tag is associated with. (e.g., "bold")
      std::string text;       // Basic text associated with this tag (e.g., for simple replacements)
      std::string pattern;    // A regex pattern for identifying this tag.
    };

    // A struct to help translate a style back into relevant tags.
    struct StyleInfo {
      std::string name;   // What is the base name of this style? (e.g. "bold" or "font")
      std::string open;   // What tag should be used when this style starts?
      std::string close;  // What tag should be used when this style ends?
      bool has_args;      // Does this style have arguments associated with it?
    };

    std::unordered_map<std::string, StyleInfo> style_info;
    std::unordered_map<std::string, TagInfo> tag_info;
    std::string tag_regex;
    emp::Lexer lexer;

    int token_text = -1;
    int token_tag = -1;
    int token_char = -1;

    std::set<std::string> active_styles; // Styles to use on appended text.

    /// @brief Add a simple tag associated with a text style.
    /// @param style The text style associated with the new tag.
    /// @param start The main tag to start this style.
    /// @param stop  The tags (or other indication) that should end this style.
    /// The specific behavior of these tags depends on what as provided as stop:
    /// * Another tag  : the tags indicate the beginning and end of a style (E.g., "<b>", "</b>")
    /// * The same tag : tag toggles the style (e.g., "*"" in Markdown)
    /// * newline (\n) : the style should apply to the rest of the line (e.g., blockquote)
    /// * empty ("")   : the style should only be applied to one character (e.g., indent)    
    void AddBasicTag (
      const std::string & style,
      const std::string & start,
      const std::string & stop="")
    { 
      TagType tag_type = TagType::OPEN;
      if (stop == "") tag_type = TagType::CHAR;
      else if (stop == "\n") tag_type = TagType::LINE;
      else if (stop == start) tag_type = TagType::TOGGLE;

      // We always want to track the start tag.
      if (style_info.size()) tag_regex += '|';
      tag_regex += emp::to_literal(start);

      style_info[style] = StyleInfo{style, start, stop, false};
      tag_info[start] = TagInfo{tag_type, style};

      // Only track the stop tag if meaningful.
      if (tag_type == TagType::OPEN) {
        tag_regex += emp::to_string('|', emp::to_literal(stop));
        tag_info[stop] = TagInfo{TagType::CLOSE, style};
      }
    }

    /// @brief Add a simple tag that is always replaced in a specific way.
    /// @param encode_v The tag from the encoding.
    /// @param txt_v The text it should be replaced with.
    /// @param style The style of the replacement text (if any)
    void AddReplacementTag(
      const std::string & encode_v, // Encoding for this symbol (e.g., "&nbsp;"")
      const std::string & txt_v,    // Text to convert this symbol to (e.g., " ")
      const std::string & style="") // Special style for this symbol, if any (e.g., "no_break")
    {
      if (style_info.size()) tag_regex += '|';
      tag_regex += emp::to_literal(encode_v);
      tag_info[encode_v] = TagInfo{ TagType::REPLACE, style, txt_v, encode_v };
    }

    // Take a tag, return associate style.
    using style_fun_t = std::function<std::string(const std::string &)>;
    // Take a tag, return associated text.
    using text_fun_t = std::function<std::string(const std::string &)>;
    // Take a style and text and return the associated tag.
    using tag_fun_t = std::function<std::string(const std::string &, const std::string &)>;

    /// @brief Create a more complex tag/style that uses arguments.
    /// @param style A unique style name used as the base style (E.g., "anchor").
    /// @param regex A regular expression that will uniquely identify this tag.
    /// @param tag_fun A function that takes style and text to generate this tag.
    /// @param style_fun A function that takes this tag and returns the style to use for it.
    /// @param text_fun A function that takes this tag and returns the text to use for it.
    void AddComplexTag(
      const std::string & style, // The base style name (without extra arguments)
      const std::string & regex, // Regular expression to identify this tag.
      tag_fun_t tag_fun,         // Function to covert style and text to associated tag.
      style_fun_t style_fun,     // Function to convert this tag to a style with details.
      text_fun_t text_fun)       // Function to convert this tag to associated text.
    {      
    }

    /// @brief Create a more complex tag/style that uses arguments.
    /// @param style A unique style name used as the base style (E.g., "anchor").
    /// @param regex A regular expression that will uniquely identify this tag.
    /// @param tag_fun A function that takes style and text to generate this tag.
    /// @param style_fun A function that takes this tag and returns the style to use for it.
    /// @param stop A string that represents the close tag for this entry (e.g., "</a>")
    void AddComplexTag(
      const std::string & style,   // The base style name (without extra arguments)
      const std::string & regex,   // Regular expression to identify this tag.
      tag_fun_t tag_fun,           // Function to covert style and text to associated tag.
      style_fun_t style_fun,       // Function to convert this tag to a style with details.
      const std::string & stop="") // Tag that should stop this style.
    {
    }

    // Append a string that has already been otherwise processed.
    void Append_RawText(const std::string & in) {
      size_t start = text.size();
      size_t end = start + in.size();
      text.Append_Raw(in);
      for (const std::string & style : active_styles) {
        text.SetStyle(style, start, end);
      }
    }

    void Append_Tag(const std::string & tag) {
      const auto & info = tag_info[tag];
      switch (info.type) {
        case TagType::UNKNOWN:
          emp::notify::Error("Unknown Encoding tag: ", tag);
        case TagType::OPEN:
        case TagType::LINE:
          active_styles.insert(info.base_style);
          break;
        case TagType::CLOSE:
          active_styles.erase(info.base_style);
          break;
        case TagType::TOGGLE:
          if (active_styles.count(info.base_style)) {
            active_styles.erase(info.base_style);
          } else {
            active_styles.insert(info.base_style);
          }
          break;
        case TagType::REPLACE:
          Append_RawText(info.text);
          break;
        default:
          break;
      }
    }

  public:
    TextEncoding(Text & _text, const std::string _name="html")
      : TextEncoding_Base(_text, _name) { }
    ~TextEncoding() = default;

    // Add new text into this object, translated as needed
    void Append(std::string in) override {
//      std::cout << "APPEND: " << in << std::endl;

      auto tokens = lexer.Tokenize(in);
      for (const auto & token : tokens) {
        if (token.id == token_text) Append_RawText(token.lexeme);
        else if (token.id == token_tag) Append_Tag(token.lexeme);
        else if (token.id == token_char) Append_RawText(token.lexeme);
        else {
          std::cerr << "Error, unknown tag: " << token.lexeme << std::endl;
        }
      }
    }

    // @CAO MAKE THIS FUNCTION GENERIC!
    // Convert this to a string in HTML format.
    std::string ToString() const override {
      // Determine where tags should be placed.
      std::map<size_t, std::string> tag_map;
      for (const auto & [style, info] : style_info) {
        if (text.HasStyle(style)) AddOutputTags(tag_map, style, info.open, info.close);
      }

      // Convert the string, adding tags back in as we go.
      std::string out_string;
      size_t output_pos = 0;
      for (auto [tag_pos, tags] : tag_map) {
        while (output_pos < tag_pos) {
          char next_char = text.GetChar(output_pos);
          switch (next_char) {
          case '<':  out_string += "&lt;";   break;
          case '>':  out_string += "&gt;";   break;
          case '&':  out_string += "&amp;";  break;
          default:
            out_string += next_char;
          }
          ++output_pos;
        }
        out_string += tags;
      }

      // Add any final text after the last tag.
      if (output_pos < text.size()) {
        out_string += text.GetText().substr(output_pos, text.size()-output_pos);
      }

      return out_string;
    }

  protected:
    // ------------   Helper functions   ------------

    // A helper to add start and end tag info to tag map for insertion into
    // the output string as it's created.
    void AddOutputTags(
      std::map<size_t, std::string> & tag_map,
      std::string style,
      std::string start_tag,
      std::string end_tag) const
    {
      const BitVector & sites = text.GetStyle(style);

      // Test if this style should be opened at the beginning.
      if (sites.Has(0)) tag_map[0] += start_tag;

      // Loop through other sites checking for shifts in style.
      for (size_t i = 1; i < sites.size(); ++i) {
        if (sites[i] != sites[i-1]) {
          if (sites[i]) tag_map[i] += start_tag;
          else tag_map[i] += end_tag;
        }
      }

      // Close any styles left open by the end.
      if (sites.back()) tag_map[sites.size()] += end_tag;
    }
  };

}

#endif