/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023-2024.
 *
 *  @file TextEncoding.hpp
 *  @brief Plugs into emp::Text with the specification of a given encoding.
 *  @note Status: ALPHA
 *
 *  DEVELOPER NOTES:
 *  - Right now a close tag of "\n" is special; it will end ALL styles it is associated with.
 *    Perhaps we should have configurable rules for how common end tags work?  For example,
 *    there could be "no duplicates allowed", "close most recent", or "close all"
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
#include "Text_Rule.hpp"

namespace emp {

  class TextEncoding : public emp::TextEncoding_Interface {
  protected:

    struct Style {
      String name;                 // Unique name for this style.
      size_t id = emp::MAX_SIZE_T; // Unique ID number (vector position) for this style.
      std::set<size_t> rule_ids;   // Which rules manage this style?

      Style(String name, size_t id) : name(name), id(id) { }
    };

    // Track information about _active_ styles.
    struct StyleEntry {
      size_t style_id;
      String full_info;  // Style name and arguments, as should be stored.
      size_t rule_id=0;  // Which rule created this entry? (determines legal end)

      StyleEntry(size_t style_id, String full_info, size_t rule_id)
        : style_id(style_id), full_info(full_info), rule_id(rule_id) { }
      StyleEntry(const StyleEntry &) = default;
    };

    // Track info about each unique tag.
    class TagInfo {
    private:
      size_t open_id = 0;         // A tag may open only a single rule.
      std::set<size_t> close_ids; // A tag may close any number of rules.
      bool multi_close = false;   // Can a single instance of this tag close multiple rules?

    public:
      bool HasOpen() const { return open_id; }
      bool HasClose() const { return close_ids.size(); }
      bool HasClose(size_t id) const { return close_ids.count(id); }
      bool IsMultiClose() const { return multi_close; }
      size_t GetOpenID() const { return open_id; }
      std::set<size_t> GetCloseIDs() const { return close_ids; }
      void AddOpen(size_t rule_id) {
        emp_assert(open_id == 0, "Rule trying to use open tag already in use.", rule_id, open_id);
        open_id = rule_id;
      }
      void AddClose(size_t rule_id) { close_ids.insert(rule_id); }
    };

    emp::vector<Text_Rule> rule_set;
    emp::vector<Style> style_set;

    std::map<String, size_t> name_to_style_id; // Link style names to their ID
    std::map<String, TagInfo> tag_map;         // Map of tags to the rules they may affect

    // Map of special (Emphatic) strings to the rules that generate them
    std::map<String, size_t> special_strings;

    emp::Lexer lexer;                           // Lexer to process encoding.
    int text_token = -1;                        // Token to represent any non-tag text.
    std::unordered_map<int, TagInfo> token_map; // Map of token ids to possible tags.

    // Styles to use on appended text; handled as stack so a common close tag (e.g. '}') will
    // close only the most recently opened match.
    std::vector<StyleEntry> active_styles;

    // === Helper Functions ===

    // Get a style entry.  If it doesn't exist yet, add it if allowed.
    Style & GetStyle(String style_name, bool create_ok=true) {
      // If this style has control arguments, remove them for retrieval.
      style_name.ResizeTo(':');

      // If this style hasn't been used before, try to build its entry.
      if (!emp::Has(name_to_style_id, style_name)) {
        emp::notify::TestError(create_ok == false, "Requested style '", style_name,
                               "' does not exist; creation is disallowed.");
        style_set.emplace_back(style_name, style_set.size());
        name_to_style_id[style_name] = style_set.back().id;
        return style_set.back();
      }
      const size_t style_id = name_to_style_id[style_name];
      return style_set[style_id];
    }

    // Get a const version of an existing style entry.
    const Style & GetStyle(String style_name) const {
      // If this style has control arguments, remove them for retrieval.
      style_name.ResizeTo(':');

      // If this style hasn't been used before, error (cannot build it).
      notify::TestError(!emp::Has(name_to_style_id, style_name),
                        "Requested style '", style_name,
                        "' does not exist; creation is disallowed.");
      size_t style_id = name_to_style_id.find(style_name)->second;
      return style_set[style_id];
    }

    // Get a rule associated with a style that can be applied at the specific text position.
    const Text_Rule & GetRule(const Style & style, const Text & text, size_t pos) const {
      // Scan through possible rules taking the first that applies.
      for (size_t rule_id : style.rule_ids) {
        const Text_Rule & rule = rule_set[rule_id];
        // If this rule has associated internal text, make sure it's there.
        if (rule.internal_text.size() && !text.AsString().HasAt(rule.internal_text, pos)) {
          continue; // Does not have correct internal text.
        }
        return rule;
      }
      // No rule found.  Give a warning and return the default rule.
      notify::Warning("No appropriate rule to convert style '", style.name, "'. Skipping.");
      return rule_set[0];
    }

    // Build all of the regular expressions for the lexer and setup the tags.
    void SetupLexer() {
      if (lexer.GetNumTokens()) return;  // If the lexer is ready, stop here.

      // Loop through all tags, inserting each into the lexer.
      for (const auto & [tag, info] : tag_map) {
        int id = lexer.AddToken(MakeString("Tag:", tag), tag);
        token_map[id] = info;
      }

      // Finally, add an "everything else" token for regular text.
      text_token = lexer.AddToken("plain text", ".");
    }

    // Append a string that has already been otherwise processed.
    void Append_RawText(Text & text, const String & in) const {
      size_t start = text.size();
      size_t end = start + in.size();
      text.Append_Raw(in);
      // Apply all active styles.
      for (const auto & style : active_styles) {
        text.SetStyle(style.full_info, start, end);
      }
    }

    // Helper function to append text when we know we've hit a tag.
    void Append_Tag(Text & text, int token_id, const String & lexeme, std::stringstream & ss) {
      emp_assert(Has(token_map, token_id), "Unknown token id: ", token_id);

      const TagInfo & info = token_map[token_id];
      bool used = false;

      // If this token might CLOSE a style, search for options.
      if (info.HasClose()) {
        // Scan through active styles from most recent to figure out which one to remove.
        for (size_t id = active_styles.size()-1; id < active_styles.size(); --id) {
          // If we have found a tag to end, do so.
          if (info.HasClose(active_styles[id].rule_id)) {
            active_styles.erase(active_styles.begin()+id);
            used = true; // Indicate that this tag has been used at least once.
            if (!info.IsMultiClose()) break;
          }
        }
      }

      if (used) return;  // If the tag was used to close, don't ALSO open.

      // OPEN a new style if indicated by this tag.
      if (info.HasOpen()) {
        const Text_Rule & rule = rule_set[info.GetOpenID()];

        // Collect a control sequence if needed by this rule.
        String control;
        if (rule.UsesControl()) {
          char c = '\0';
          while (ss) {
            ss >> c;
            if (c == rule.open_tag_end) break; // Stop when we find the tag end.
            control += c;
          }
          notify::TestError(c != rule.open_tag_end,
                            "Open tag beginning with '", rule.open_tag_start,
                            "' did not have end ('", rule.open_tag_end, "')");
        }

        // Setup style opened by this rule, if any.
        String style_desc = rule.MakeStyle(control);
        if (style_desc.size()) {
          size_t style_id = name_to_style_id[rule.base_style];
          active_styles.emplace_back(style_id, style_desc, rule.id);
        }

        // Do any text insertion required by this rule.
        if (rule.GeneratesText()) {
          text.Append_Raw(rule.internal_text);
        }

        // If this rule does not use a close, end new styles here.
        if (rule.close_tag.size() == 0 && style_desc.size()) {
          active_styles.pop_back();
        }

        used = true; // This tag has been used.
      }
    }

    // === Tools for Constructing an Encoding ===

    // Add a pre-build rule to this encoding.
    void RegisterRule(const Text_Rule & rule) {
      rule_set.emplace_back(rule);
      rule_set.back().id = rule_set.size() - 1;

      // All rules must have a unique start to open_tag (for parse simplicity)
      String start_pattern = rule.open_tag_start;
      emp::notify::TestError(start_pattern.empty(),
        "Tag ", rule.id, " does not have a start pattern.");
      emp::notify::TestError(emp::Has(tag_map, start_pattern),
        "Start tag ", rule.id, ", '", start_pattern, "' cannot be used for more than one rule.");

      // Register the start pattern so we know when we hit it.
      tag_map[start_pattern].AddOpen(rule.id);

      // If this rule has an ending pattern, register it as well.
      // NOTE: A newline close tag (i.e., "\n") is special and will end ALL styles that use it.
      if (rule.close_tag.size()) tag_map[rule.close_tag].AddClose(rule.id);

      // If this rule uses a style, link it in.
      if (rule.base_style.size()) {
        Style & style = GetStyle(rule.base_style);
        style.rule_ids.insert(rule.id);
      }

      // If this rule generates internal text without a style, make it reversible.
      if (rule.internal_text.size() && !rule.base_style.size()) {
        special_strings[rule.internal_text] = rule.id;
      }
    }

    /// Specify both open and close tags that indicate a style.
    void AddStyleTags(String style_name, String open_tag, String close_tag) {
      Text_Rule rule(open_tag, '\0', close_tag, style_name);
      RegisterRule(rule);
    }

    /// Specify a single tag that will toggle a style when used.
    void AddStyleToggle(String style_name, String tag) {
      Text_Rule rule(tag, '\0', tag, style_name);
      RegisterRule(rule);
    }

    /// Specify a tag that will set a new style to the end of the current line only.
    void AddStyleLine(String style_name, String tag) {
      Text_Rule rule(tag, '\0', "\n", style_name);
      RegisterRule(rule);
    }

    /// Specify a new set of tags, with a control block.  The control sequence
    /// is directly moved back and forth with the style arguments.
    void AddStyleControl(String open_tag_start, char open_tag_end, String close_tag,
                         String base_style)
    {
      Text_Rule rule(open_tag_start, open_tag_end, close_tag, base_style);
      rule.to_style_arg = [](String arg){ return arg; };
      rule.to_control = [](String arg){ return arg; };
      RegisterRule(rule);
    }

    /// Specify a new set of tags, with a control block.  You must also specify HOW
    /// the control should be converted to style args and vice-versa.
    void AddStyleDynamic(String open_tag_start, String open_tag_end, String close_tag,
                         String base_style, auto to_style_arg, auto to_control)
    {
      Text_Rule rule(open_tag_start, open_tag_end, close_tag, base_style);
      rule.to_style_arg = to_style_arg;
      rule.to_control = to_control;
      RegisterRule(rule);
    }

    /// Add a new tag that gets replaced in plain-text. For example:
    ///   AddReplaceTag("&lt;", "<")                 for less-than in HTML -or-
    ///   AddReplaceTag("&nbsp;", " ", "no_break")   for non-breaking spaces.
    void AddReplaceTag(String tag, String text_str, String style_name="") {
      Text_Rule rule(tag, '\0', "", style_name);
      rule.internal_text = text_str;
      RegisterRule(rule);
    }

    /// Add a symbol that will be represented as a space, but specified in its style.
    void AddReplaceControl(String tag_start, char tag_end, String text_str, String style_name) {
      Text_Rule rule(tag_start, tag_end, "", style_name);
      rule.internal_text = text_str;
      RegisterRule(rule);
    }

/*
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




    // Get the style name with any associated args that should be used in Text.
    String GetStyleDesc(const Style & style, const Tag & start_tag, const String & lexeme) {
      if (!start_tag.get_style_args) return style.name;
      return emp::MakeString(style.name, ':', start_tag.get_style_args(lexeme));
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
*/

  public:
    TextEncoding() : TextEncoding_Interface() {
      // Setup a default rule that is always ID zero.
      Text_Rule default_rule("__default_Text_Rule__", '\0', "", "");
      default_rule.id = 0;
      rule_set.push_back(default_rule);

      // Setup a default style that is always ID zero.
      Style default_style("__default_style__", 0);
      style_set.push_back(default_style);
    }
    ~TextEncoding() = default;

    // Reset the current encoding to clear all tag knowledge for this encoding.
    void Reset() {
      rule_set.resize(1);       // Reduce to just the default rule.
      style_set.resize(1);      // Reduce to just the default style.
      name_to_style_id.clear();
      tag_map.clear();
      special_strings.clear();
      lexer.Reset();
      text_token = -1;
      active_styles.resize(0);
    }

    // Add new text into this object, translated as needed
    void Append(Text & text, const String & in) override {
      SetupLexer();
      String raw_text;  // Place to accumulate raw text for adding all at once.
      size_t line_num;  // What line number are we on in the input?

      std::stringstream ss;
      ss << in;

      while (const auto token = lexer.TokenizeNext(ss, line_num)) {
        if (token.id == text_token) raw_text += token.lexeme;
        else {
          if (raw_text.size()) Append_RawText(text, raw_text.PopAll());
          Append_Tag(text, token.id, token.lexeme, ss);
        }
      }
      if (raw_text.size()) Append_RawText(text, raw_text.PopAll());
    }

    String Encode(const Text & text) const override {
      // Create a map of "text positions" to style info that open (or close) at that position.
      // Each position should have:
      //  1: a pointer to the style change there, and
      //  2: a flag to indicate if it should be an "open" tag
      using pos_info_t = std::pair<emp::Ptr<const String>, bool>;
      std::map<size_t, emp::vector<pos_info_t>> style_pos_map;

      // Go through the styles in this Text and insert the associated rules into the maps.
      const std::unordered_map<String, BitVector> & style_map = text.GetStyleMap();
      for (const auto & [style_desc, sites] : style_map) {
        // Loop through marking every change in style.
        auto ranges = sites.GetRanges();
        for (const auto & range : ranges) {
          auto & open_v = style_pos_map[range.GetLower()];
          auto & close_v = style_pos_map[range.GetUpper()];
          // Insert opens at the end and closes at the beginning; thus we close all existing
          // styles before we open any new ones (and closes happen in reverse order.)
          open_v.emplace_back(&style_desc, true);
          close_v.insert(close_v.begin(), pos_info_t{&style_desc, false});
        }
      }

      // Export the string, encoding tags back in as we go.
      std::stringstream out_stream;
      size_t scan_pos = 0;
      for (auto [tag_pos, style_vec] : style_pos_map) {
        // Copy everything into the out_stream up to the tags we need to insert.
        if (scan_pos < tag_pos) {
          out_stream << text.AsString().ViewRange(scan_pos, tag_pos);
          scan_pos = tag_pos;
        }

        // Insert any needed tags.  They should already be in the right order to deal with.
        for (auto [style_ptr, is_open] : style_vec) {
          const Style & style = GetStyle(*style_ptr);
          const Text_Rule & rule = GetRule(style, text, scan_pos);
          if (rule.id == 0) continue; // No rule found; warning already sent.

          // If the rule has text associated with it, skip over text.
          if (rule.internal_text.size()) scan_pos += rule.internal_text.size();

          // Drop the appropriate tag here.
          if (is_open) out_stream << rule.MakeOpenTag(*style_ptr);
          else out_stream << rule.close_tag;
        }
      }

      // @CAO CONTINUE HERE



      for (auto [tag_pos, tags] : tag_map) {
        _EncodeTo(text, out_stream, scan_pos, tag_pos);
        out_stream += tags;
      }
      _EncodeTo(text, out_stream, scan_pos, text.size()); // Add final text after the last tag.

      return out_stream;






      const emp::vector<String> & style_list = text.GetStyles();
      for (String style_desc : style_list) {
        String style_name = style_desc.Pop(":");
        const Style & style = GetStyle(style_desc);
        emp_assert(style.rule_ids.size(), "Style has no associated rules!", style_name);
        size_t rule_id = *style.rule_ids.begin();

        if (!style.make_open_tag) continue; // If no tags are available, assume a replacement style.
        AddOutputTags(text, tag_map, style_name,
                      style.make_open_tag(style_desc), style.make_close_tag(style_desc));
      }

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
