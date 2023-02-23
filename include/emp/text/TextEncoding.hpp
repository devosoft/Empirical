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
#include "../base/Ptr.hpp"
#include "../compiler/Lexer.hpp"

#include "Text.hpp"

namespace emp {

  class TextEncoding : public emp::TextEncoding_Base {
  protected:

    struct Tag {
      String name;     // Unique name for this tag; default encoding symbol.
      String pattern;  // Regular expression to identify tag.
      size_t id;       // For recognizing tag in lexer output.

      virtual ~Tag() {}
      size_t GetID() const { return id; }
      virtual String GetName() const { return name; }
      virtual String GetPattern() const { return pattern; }
      virtual String GetCloseName() const { return ""; }
      virtual bool IsStyle() const { return false; }
      virtual bool IsReplace() const { return false; }
      virtual String GetEncoding(String /* style_used */) const { return name; }
      virtual String GetClosePattern() const { return ""; }
      virtual String GetCloseEncoding(String /* style_used */) const { return name; }
      virtual String GetStyle(String /*lexeme*/) const { return ""; }
      virtual String GetText(String /*lexeme*/) const { return ""; }
    };

    // Tag type that indicates a section of text needs to be formatted...
    struct StyleTag : public Tag {
      String range_style;  // Style to use between open and close tags.
      String close;        // Where should format end? Can be same as open tag for toggle.

      virtual bool IsStyle() const { return true; }
      String GetCloseName() const override { return name + "_close"; }
      String GetClosePattern() const override { return close; }
      String GetCloseEncoding(String /* style_used */) const override  { return close; }
      String GetStyle(String /*lexeme*/) const override { return range_style; }
    };

    // Tag type to indicate a direct replacement (e.g., '&lt;' means '<' in HTML)
    // Note: reversible will require both replace_text AND replace_style in place.
    struct ReplaceTag : public Tag {
      String replace_text;    // What should this tag be replaced with?
      String replace_style;   // What style should the replacement text have?
      bool reversible=true;   // Substitute in the other direction when generating encoded text?

      virtual bool IsReplace() const { return true; }
      String GetText(String /*lexeme*/) const override { return replace_text; }
      String GetStyle(String /*lexeme*/) const override { return replace_style; }
    };


    // A struct to help translate a style back into relevant tags.
    struct Style {
      String name;            // What is the base name of this style? (e.g. "bold" or "font")
      String open_tag;        // Name of tag to be used when this style starts.
      String close_tag;       // Name of tag to be used when this style ends.
      bool has_args=false;    // Does this style have arguments associated with it?
      bool has_replace=false; // Is this style used in replacement tags?
    };

    // Information linking a token to the tag that it is associated with.
    struct TokenInfo {
      int token_id = -1; 
      std::set<size_t> start_ids; // Which tags might this token start?
      std::set<size_t> end_ids;   // Which tags might this token end?
    };

    emp::vector<emp::Ptr<Tag>> tag_set;
    emp::vector<Style> style_set;
    std::map<String, TokenInfo> token_pattern_map;  // Link token pattens to the information about them.
    std::map<int, emp::Ptr<TokenInfo>> token_id_map; // Link lexer ids to the associate Token info.
    emp::Lexer lexer;

    // Track information about active styles.
    struct StyleEntry {
      String style_name;
      emp::Ptr<StyleTag> tag_ptr;  // Which tag created this style entry?
    };
    std::vector<StyleEntry> active_styles; // Styles to use on appended text; done as stack.

    virtual ~TextEncoding() {
      for (auto ptr : tag_set) ptr.Delete();
    }

    TokenInfo & GetTokenInfo(String pattern, String name) {
      TokenInfo & token_info = token_pattern_map[pattern];
      if (token_info.token_id == -1) { // If this is a new pattern, add it to the lexer.
        token_info.token_id = lexer.AddToken(name, pattern);
        token_id_map[token_info.token_id] = &token_info;
      }
      return token_info;
    }

    /// @brief Once a new tag has been built, add it to the encoding.
    /// Tags are placed in the tag set and their tokens are setup to recognize them as needed.
    /// @param tag_ptr A pointer to the fully-built tag.
    void AddTag(emp::Ptr<Tag> tag_ptr) {
      tag_ptr->id = tag_set.size();
      tag_set.push_back(tag_ptr);

      TokenInfo & open_info = GetTokenInfo(tag_ptr->GetPattern(), tag_ptr->GetName());
      open_info.start_ids.insert(tag_ptr->id);

      if (!tag_ptr->GetClosePattern().empty()) {
        TokenInfo & close_info = GetTokenInfo(tag_ptr->GetClosePattern(), tag_ptr->GetCloseName());
        close_info.end_ids.insert(tag_ptr->id);
      }
    }

    /// @brief Add a pair of tags associated with the begin and end of a text style.
    /// @param style The text style associated with the new tag.
    /// @param start The main tag to start this style.
    /// @param stop  The tags (or other indication) that should end this style.
    /// The specific behavior of these tags depends on what as provided as stop:
    /// * Another tag  : the tags indicate the beginning and end of a style (E.g., "<b>", "</b>")
    /// * The same tag : tag toggles the style (e.g., "*"" in Markdown)
    /// * newline (\n) : the style should apply to the rest of the line (e.g., blockquote)
    /// * empty ("")   : the style should only be applied to one character (e.g., indent)    
    void AddStyleTag ( String style, String open_tag, String close_tag )
    { 
      emp::Ptr<StyleTag> tag_ptr = emp::NewPtr<StyleTag>();
      tag_ptr->name = open_tag;
      tag_ptr->pattern = MakeLiteral(open_tag);
      tag_ptr->range_style = style;
      tag_ptr->close = close_tag;
      AddTag(tag_ptr);
    }

    /// @brief Add a simple tag that is always replaced in a specific way.
    /// @param encoding The tag from the encoding. (e.g., "&nbsp;" or "&lt;")
    /// @param plain_text The text it should be replaced with (e.g., " " or "<")
    /// @param style The style of the replacement text ((e.g., "no_break")
    void AddReplacementTag(String encoding, String plain_text, String style="")
    {
      emp::Ptr<ReplaceTag> tag_ptr = emp::NewPtr<ReplaceTag>();
      tag_ptr->name = encoding;
      tag_ptr->pattern = MakeLiteral(encoding);
      tag_ptr->replace_text = plain_text;
      tag_ptr->replace_style = style;
      AddTag(tag_ptr);
    }


    // Append a string that has already been otherwise processed.
    void Append_RawText(const String & in) {
      size_t start = text.size();
      size_t end = start + in.size();
      text.Append_Raw(in);
      for (const auto & style : active_styles) {
        text.SetStyle(style.style_name, start, end);
      }
    }

    void Append_Tag(int token_id, const String & lexeme) {
      const TokenInfo & token_info = *token_id_map[token_id];

      // If this token might end something, search for what.
      if (token_info.end_ids.size()) {
        // Scan through active styles from most recent to figure out which one to remove.
        for (auto it = active_styles.rbegin(); it != active_styles.rend(); ++it) {
          // If we have found a tag to end, do so.
          if (token_info.end_ids.count(it->tag_ptr->id)) {
            active_styles.erase(it);
            if (lexeme != "\n") return;  // Newlines can end multiple styles
          }
        }
      }

      // Start all of the styles indicated by this tag?
      if (token_info.start_ids.size()) {
        for (size_t tag_id : token_info.start_ids) {
          emp::Ptr<Tag> tag_ptr = tag_set[tag_id];

          if (token_info.start_ids.size()) {
            active_styles.insert({tag_ptr->GetStyle(lexeme),tag_ptr});
          }
        }
        return;
      }

      // Otherwise see if we need to do a replacement.
    }

    // Build all of the regular expressions for the lexer and setup the tags.
    void SetupLexer() {
      // If the lexer is already setup, skip this step.
      if (lexer.GetNumTokens()) return;

      // Loop through the tags to build the regular expressions.
      for (auto & [name, info] : tag_info) {
        if (info.type == TagType::UNKNOWN) {
          emp::notify::Error("Trying to lex unknown tag type for: ", name);
          return;
        }

        info.token_id = lexer.AddToken(name, info.pattern);
      }
    }

  public:
    TextEncoding(Text & _text, const std::string _name="html")
      : TextEncoding_Base(_text, _name) { }
    ~TextEncoding() = default;

    // Add new text into this object, translated as needed
    void Append(std::string in) override {

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