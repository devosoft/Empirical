/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2023
*/
/**
 *  @file
 *  @brief Plugs into emp::Text with the specification of a given encoding.
 *  @note Status: ALPHA
 *
 */

#ifndef EMP_TEXT_TEXTENCODING_HPP_INCLUDE
#define EMP_TEXT_TEXTENCODING_HPP_INCLUDE

#include <set>
#include <string>
#include <unordered_map>

#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../compiler/Lexer.hpp"
#include "../datastructs/map_utils.hpp"

#include "Text.hpp"

namespace emp {

  class TextEncoding : public emp::TextEncoding_Interface {
  protected:

    // Tags have three possible things they can do:
    //  1: Start a new style
    //  2: End an ongoing style, or
    //  3: Be replaced with text (which may have a style associated)
    struct Tag {
      String name;                  // Unique name for this tag; default encoding symbol.
      String pattern;               // Regular expression to identify tag.
      size_t id = emp::MAX_SIZE_T;  // Unique ID for this tag (index in tag_map)
      int token_id = -1;            // ID of this tag in the lexer.

      // -- Starting a Style --
      // Tags can indicate that they start a style.  Since the style may have arguments, the
      // lexeme that identified the tag will be passed in and can be processed.
      // Note that style zero (the default) is "no_style" and used to indicate no start style.
      size_t start_style_id = 0;                    // No style started by default.
      std::function<String(String)> get_style_args; // Function to convert lexeme to style arguments.

      // -- Ending a Style --
      // Tags may be used to end one or more styles.  The set of styles that can be ended is
      // tracked, and whether multiple styles can be ended at once.  For example, a newline might
      // end a whole set of styles in an encoding.  A "</b>" in HTML, on the other hand might only
      // end the bold style.  A "}" in latex would end the most recent style started.
      std::set<size_t> end_style_ids;  // Set of styles this tag can end.
      bool multi_end = false;          // Can this tag end more than one style at a time?

      // -- Text Replacement --
      // Some tags might be used to produce a special character in the output text. For example,
      // "&lt;" in HTML would be replaced by '<'.  A replacement character can also have a
      // style with it.  For example, "&nbsp;" would be replaced by ' ' with the style "no_break".
      char replace_char = '\0';     // Character to place in TEXT (Default is none).
      size_t replace_style_id = 0;  // Style ID to use if there is a replacement char.
      String out_encoding="";       // Use to replace char (and correct style) in encoding.
    };

    struct Style {
      String name;                      // Unique name for this style.
      size_t id = emp::MAX_SIZE_T;      // Unique ID number (vector position) for this style.
      std::set<size_t> open_tag_ids;    // Which tags start this style?
      std::set<size_t> close_tag_ids;   // Which tags close this style?
      std::set<size_t> replace_tag_ids; // Which replacements use this style?

      std::function<String(String)> make_open_tag;  // Generate an open tag based on style args.
      std::function<String(String)> make_close_tag; // Generate an close tag based on style args.
    };

    emp::vector<Tag> tag_set;
    emp::vector<Style> style_set;

    std::map<String, size_t> pattern_to_tag_id;  // Link tag pattens to tag IDs.
    std::map<int, size_t> token_to_tag_id;       // Link lexer IDs to the associate tag.
    std::map<String, size_t> name_to_style_id;   // Lookup a style to find its ID.
    emp::array<size_t, 128> char_tags;           // Track which tags are associated with each character.

    emp::Lexer lexer;      // Lexer to process encoding.
    int text_token = -1;   // Token to represent any non-tag text.

    // Track information about active styles.
    struct StyleEntry {
      size_t style_id;
      String full_info;  // Style name and arguments, as should be stored.
    };
    std::vector<StyleEntry> active_styles; // Styles to use on appended text; done as stack.

    size_t GetTagID(const String & tag_name, const String & tag_pattern) {
      // If we don't have this tag yet, build it.
      if (!emp::Has(pattern_to_tag_id, tag_pattern)) {
        Tag new_tag;
        new_tag.name = tag_name;
        new_tag.pattern = tag_pattern;
        new_tag.id = tag_set.size();
        tag_set.push_back(new_tag);
        pattern_to_tag_id[tag_pattern] = new_tag.id;
        return tag_set.back().id;
      }
      return pattern_to_tag_id[tag_pattern];
    }

    // If no pattern is provided in get, use the tag name to generate it.
    size_t GetTagID(const String & tag_name) {
      return GetTagID(tag_name, emp::MakeLiteral(tag_name));
    }

    // Get a style entry.  If it doesn't exist yet, add it.
    Style & GetStyle(const String & style_name) {
      // If this style hasn't been used before, build its entry.
      if (!emp::Has(name_to_style_id, style_name)) {
        Style new_style;
        new_style.name = style_name;
        new_style.id = style_set.size();
        style_set.push_back(new_style);
        name_to_style_id[style_name] = new_style.id;
        return style_set.back();
      }
      size_t style_id = name_to_style_id[style_name];
      return style_set[style_id];
    }

    // Add new tags that setup a specific style.  For example:
    //   SetupStyleTags("bold", "<b>", "</b>")    for HTML -or-
    //   SetupStyleTags("bold, "{\bf", "}")       for Latex
    void SetupStyleTags(String style_name, String open_name, String close_name) {
      Style & style = GetStyle(style_name);
      size_t open_id = GetTagID(open_name);
      size_t close_id = GetTagID(close_name);
      Tag & open_tag = tag_set[open_id];
      Tag & close_tag = tag_set[close_id];

      style.open_tag_ids.insert(open_tag.id);
      style.close_tag_ids.insert(close_tag.id);
      if (!style.make_open_tag) style.make_open_tag = [open_name](String){ return open_name; };
      if (!style.make_close_tag) style.make_close_tag = [close_name](String){ return close_name; };

      emp::notify::TestError(open_tag.start_style_id != 0,
        "Tag '", open_name, "' cannot be used to start more than one style.");
      open_tag.start_style_id = style.id;
      close_tag.end_style_ids.insert(style.id);
    }

    // Add a new tag that gets replaced by a single character in plain-text. For example:
    //   SetupReplaceTag("&nbsp;", '<')               for less-than in HTML -or-
    //   SetupReplaceTag("&nbsp;", ' ', "no_break")   for non-breaking spaces.
    void SetupReplaceTag(String tag_name, char replace_char, String style_name="") {
      Tag & replace_tag = tag_set[ GetTagID(tag_name) ];
      replace_tag.replace_char = replace_char;
      replace_tag.out_encoding = tag_name;

      // Track character / tag association.
      size_t char_id = static_cast<size_t>(replace_char);
      notify::TestError(char_tags[char_id],
        "Cannot associate character ", char_id, "('", replace_char, "') with multiple tags.");
      char_tags[char_id] = replace_tag.id;

      // If we were given a style, set it up too.
      if (style_name.size()) {
        Style & style = GetStyle(style_name);
        style.replace_tag_ids.insert(replace_tag.id);
        replace_tag.replace_style_id = style.id;
      }
    }

    // Get the style name with any associated args that should be used in Text.
    String GetStyleDesc(const Style & style, const Tag & start_tag, const String & lexeme) {
      if (!start_tag.get_style_args) return style.name;
      return emp::MakeString(style.name, ':', start_tag.get_style_args(lexeme));
    }


    // Append a string that has already been otherwise processed.
    void Append_RawText(Text & text, const String & in) {
      size_t start = text.size();
      size_t end = start + in.size();
      text.Append_Raw(in);
      for (const auto & style : active_styles) {
        text.SetStyle(style.full_info, start, end);
      }
    }

    void Append_Tag(Text & text, int token_id, const String & lexeme) {
      Tag & tag = tag_set[ token_to_tag_id[token_id] ];

      // If this token might END a style, search for options.
      if (tag.end_style_ids.size()) {
        // Scan through active styles from most recent to figure out which one to remove.
        for (size_t id = active_styles.size()-1; id < active_styles.size(); --id) {
          // If we have found a tag to end, do so.
          if (tag.end_style_ids.count(active_styles[id].style_id)) {
            active_styles.erase(active_styles.begin()+id);
            if (!tag.multi_end) return;
          }
        }
      }

      // START a new styles if indicated by this tag.
      if (tag.start_style_id) {
        Style & style = style_set[tag.start_style_id];
        active_styles.push_back({style.id, GetStyleDesc(style, tag, lexeme)});
        return;
      }

      // Otherwise see if we need to do a replacement.
      if (tag.replace_char) {
        text.Append_Raw(tag.replace_char);
        if (tag.replace_style_id) {
          Style & style = style_set[tag.replace_style_id];
          text.SetStyle(style.name, text.size() -1);
        }
      }
    }

    // Build all of the regular expressions for the lexer and setup the tags.
    void SetupLexer() {
      // If the lexer is already setup, skip this step.
      if (lexer.GetNumTokens()) return;

      // Loop through the tags to build the regular expressions.
      for (Tag & tag : tag_set) {
        tag.token_id = lexer.AddToken(tag.name, tag.pattern);
        token_to_tag_id[tag.token_id] = tag.id;
      }

      // Finally, add an "everything else" token for regular text.
      text_token = lexer.AddToken("plain text", ".");
    }

    void _EncodeChar(const Text & text, String & out_string, const size_t char_pos) const {
      const char c = text.GetChar(char_pos);
      // If there is a tag associated with this character AND the associated type (if any) use tag.
      if (char_tags[c]) {
        const Tag & tag = tag_set[char_tags[c]];
        const Style & style = style_set[tag.replace_style_id];
        if (!tag.replace_style_id || text.HasStyle(style.name, char_pos)) {
          out_string += tag.out_encoding; // Char match AND style match (or no style), so print tag
        } else out_string += c;   // Wrong style for tag; just print character.
      } else {
        out_string += c;
      }
    }

    void _EncodeTo(const Text & text, String & out_string, size_t & start, const size_t end) const {
      while (start < end) {
        _EncodeChar(text, out_string, start);
        ++start;
      }
    }

  public:
    TextEncoding() : TextEncoding_Interface() {
        Tag default_tag;
        default_tag.name = "__default_tag__";
        default_tag.id = 0;
        tag_set.push_back(default_tag);

        Style default_style;
        default_style.name = "__default_style__";
        default_style.id = 0;
        style_set.push_back(default_style);

        char_tags.fill(0);
      }
    ~TextEncoding() = default;

    // Reset the current encoding to clear all tag knowledge for this encoding.
    void Reset() {
      tag_set.resize(1);           // Reduce down to just the default tag.
      style_set.resize(0);         // Reduce down to just the default style.
      char_tags.fill(0);           // Clear all character tags.
      pattern_to_tag_id.clear();   // Remove all references to tags
      token_to_tag_id.clear();
      name_to_style_id.size();
      active_styles.resize(0);     // All active styles turn off on changing encodings.
      text_token = -1;             // Allow the lexer to reset.
      lexer.Reset();
    }

    // Add new text into this object, translated as needed
    void Append(Text & text, const String & in) override {
      SetupLexer();
      auto tokens = lexer.Tokenize(in.str());
      String raw_text;  // Place to accumulate raw text.
      for (const auto & token : tokens) {
        if (token.id == text_token) raw_text += token.lexeme;
        else {
          if (raw_text.size()) Append_RawText(text, raw_text.PopAll());
          Append_Tag(text, token.id, token.lexeme);
        }
      }
      if (raw_text.size()) Append_RawText(text, raw_text.PopAll());
    }

    String Encode(const Text & text) const override {
      // Determine where tags should be placed.
      std::map<size_t, String> tag_map;
      emp::vector<String> style_list = text.GetStyles();
      for (String style_desc : style_list) {
        String style_name = style_desc.Pop(":");
        const Style & style = style_set[ emp::Find(name_to_style_id, style_name, 0) ];
        if (!style.make_open_tag) continue; // If no tags are available, assume a replacement style.
        AddOutputTags(text, tag_map, style_name,
                      style.make_open_tag(style_desc), style.make_close_tag(style_desc));
      }

      // Convert the string, adding tags back in as we go.
      String out_string;
      size_t output_pos = 0;
      for (auto [tag_pos, tags] : tag_map) {
        _EncodeTo(text, out_string, output_pos, tag_pos);
        out_string += tags;
      }
      _EncodeTo(text, out_string, output_pos, text.size()); // Add final text after the last tag.

      return out_string;
    }

    void PrintDebug(std::ostream & os = std::cout) const override {
      os << "Tags:\n";
      for (const auto & tag : tag_set) {
        os << "  '" << tag.name << "' id=" << tag.id << "; start_style=" << tag.start_style_id << "\n";
      }
      os << "Styles:\n";
      for (const auto & style : style_set) {
        os << "  '" << style.name << "' id=" << style.id << "; open_count=" << style.open_tag_ids.size() << "\n";
      }
    }

  protected:
    // ------------   Helper functions   ------------

    // A helper to add start and end tag info to tag map for insertion into
    // the output string as it's created.
    void AddOutputTags(
      const Text & text,
      std::map<size_t, String> & tag_map,
      String style,
      String start_tag,
      String end_tag) const
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

  template <typename ENCODING_T, typename... Ts>
  emp::Text MakeEncodedText(Ts &&... args) {
    Text out;                              // Create the text object
    out.AddEncoding<ENCODING_T>();         // Setup the encoding
    out.Append(std::forward<Ts>(args)...); // Append the inputs, if any
    return out;                            // Return the finished product
  }

  template <typename ENCODING_T>
  class EncodedText : public emp::Text {
  private:
    using this_t = EncodedText<ENCODING_T>;
  public:
    /// @brief Create a new, default EncodedText object.
    EncodedText() { AddEncoding<ENCODING_T>(); };
    EncodedText(const this_t & in) = default;

    template <typename... Ts>
    EncodedText(Ts &&... in) : EncodedText() {
      Append(std::forward<Ts>(in)...);
    }

    this_t & operator=(const this_t &) = default;
    this_t & operator=(this_t &&) = default;
    template <typename T>
    this_t & operator=(T && in) {
      Text::operator=(std::forward<T>(in));
      return *this;
    };
  };

}

#endif // #ifndef EMP_TEXT_TEXTENCODING_HPP_INCLUDE
