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

    class Style {
    private:
      String name = "No Name";     // Unique name for this style.
      size_t id = emp::MAX_SIZE_T; // Unique ID number (vector position) for this style.
      std::set<size_t> rule_ids;   // Which rules manage this style?

    public:
      Style() { }
      Style(String name, size_t id) : name(name), id(id) { }

      const String & GetName() const { return name; }
      size_t GetID() const { return id; }
      const std::set<size_t> & GetRules() const { return rule_ids; }

      void SetID(size_t new_id) {
        emp_assert(new_id < 1000000000, new_id, "Setting a style to have an invalid ID");
        id = new_id;
      }
      void AddRuleID(size_t rule_id) {
        emp_assert(rule_id < 1000000000, rule_id, "Adding an invalid rule ID to a style");
        rule_ids.insert(rule_id);
      }

      void PrintDebug(std::ostream & os = std::cout) const {
        os << "Style " << id << " (" << name << "): Rule(s) "
           << emp::MakeEnglishList(rule_ids) << std::endl;
      }
    };

    // Track information about _active_ styles.
    struct StyleEntry {
      size_t style_id=0;
      String full_info;  // Style name and arguments, as should be stored.
      size_t rule_id=0;  // Which rule created this entry? (determines legal end)

      StyleEntry() { }
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
      void SetMultiClose(bool in_mc=true) { multi_close = in_mc; }

      void PrintDebug(std::ostream & os=std::cout) const {
        os << "open_id=" << open_id << " ; close_ids=";
        for (size_t id : close_ids) os << id << " ";
        os << "; multiclose=" << multi_close << std::endl;
      }
    };

    emp::vector<Text_Rule> rule_set;
    emp::vector<Style> style_set;

    std::map<String, size_t> name_to_style_id; // Link style names to their ID
    std::map<String, TagInfo> tag_map;         // Map of tags to the rules they may affect

    // Chart of characters to the rule by which they must be encoded (with no associated special style)
    std::array<size_t, 128> special_chars;

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
        name_to_style_id[style_name] = style_set.back().GetID();
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
      for (size_t rule_id : style.GetRules()) {
        const Text_Rule & rule = rule_set[rule_id];
        // If this rule uses an internal character, make sure it's there.
        if (rule.GeneratesText() && text.AsString()[pos] != rule.GetInternalChar()) {
          // std::cout << "Failed on style '" << style.GetName() << "' because wrong internal text; expected '" << rule.GetInternalChar() << "' but found '" << text.AsString()[pos] << "'." << std::endl;
          continue; // Does not have correct internal char.
        }
        return rule;
      }
      // No rule found.  Give a warning and return the default rule.
      notify::Warning("No appropriate rule to convert style '", style.GetName(), "'. Skipping.");
      return rule_set[0];
    }

    // Build all of the regular expressions for the lexer and setup the tags.
    void SetupLexer() {
      if (lexer.GetNumTokens()) return;  // If the lexer is ready, stop here.

      // Loop through all tags, inserting each into the lexer.
      for (const auto & [tag, info] : tag_map) {
        int id = lexer.AddToken(MakeString("Tag:", tag), tag.AsLiteral());
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
    void Append_Tag(Text & text, int token_id, std::istream & input_stream) {
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
          while (input_stream) {
            input_stream >> c;
            if (c == rule.GetOpenTagEnd()) break; // Stop when we find the tag end.
            control += c;
          }
          notify::TestError(c != rule.GetOpenTagEnd(),
                            "Open tag beginning with '", rule.GetOpenTagStart(),
                            "' did not have end ('", rule.GetOpenTagEnd(), "')");
        }

        // Setup style opened by this rule, if any.
        String style_desc = rule.MakeStyle(control);
        if (style_desc.size()) {
          size_t style_id = name_to_style_id[rule.GetBaseStyle()];
          active_styles.emplace_back(style_id, style_desc, rule.GetID());
        }

        // Do any text insertion required by this rule.
        if (rule.GeneratesText()) {
          Append_RawText(text, MakeString(rule.GetInternalChar()));
        }

        // If this rule does not use a close, end new styles here.
        if (!rule.UsesText() && style_desc.size()) {
          active_styles.pop_back();
        }

        used = true; // This tag has been used.
      }
    }

    // === Tools for Constructing an Encoding ===

    // Add a pre-built rule to this encoding.
    void RegisterRule(const Text_Rule & in_rule) {
      rule_set.emplace_back(in_rule);
      Text_Rule & rule = rule_set.back();
      rule.SetID(rule_set.size() - 1);

      // All rules must have a unique start to open_tag (for parse simplicity)
      String start_pattern = rule.GetOpenTagStart();
      emp::notify::TestError(start_pattern.empty(),
        "Tag ", rule.GetID(), " does not have a start pattern.");
      emp::notify::TestError(emp::Has(tag_map, start_pattern),
        "Start tag ", rule.GetID(), ", '", start_pattern, "' cannot be used for more than one rule.");

      // Register the start pattern so we know when we hit it.
      tag_map[start_pattern].AddOpen(rule.GetID());

      // If this rule has an ending pattern, register it as well.
      // NOTE: A newline close tag (i.e., "\n") is special and will end ALL styles that use it.
      if (rule.UsesText()) tag_map[rule.GetCloseTag()].AddClose(rule.GetID());

      // If this rule uses a style, link it in.
      if (rule.UsesStyle()) {
        Style & style = GetStyle(rule.GetBaseStyle());
        style.AddRuleID(rule.GetID());
      }

      // If this rule generates an internal char without a style, make it reversible.
      if (rule.GeneratesText() && !rule.UsesStyle()) {
        emp::notify::TestError(rule.GeneratesText() != 1,
          "Rules that convert text with no emphatic style must be limited to one character.\n",
          "Text = '", rule.GetInternalChar(), "'.");
        special_chars[rule.GetInternalChar()] = rule.GetID();
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
      rule.SetConversions( [](String arg){ return arg; }, [](String arg){ return arg; } );
      RegisterRule(rule);
    }

    /// Specify a new set of tags, with a control block.  You must also specify HOW
    /// the control should be converted to style args and vice-versa.
    void AddStyleDynamic(String open_tag_start, char open_tag_end, String close_tag,
                         String base_style, auto to_style_arg, auto to_control)
    {
      Text_Rule rule(open_tag_start, open_tag_end, close_tag, base_style);
      rule.SetConversions(to_style_arg, to_control);
      RegisterRule(rule);
    }

    /// Add a new tag that gets replaced in plain-text. For example:
    ///   AddReplaceTag("&lt;", "<")                 for less-than in HTML -or-
    ///   AddReplaceTag("&nbsp;", " ", "no_break")   for non-breaking spaces.
    void AddReplaceTag(String tag, char internal_char, String style_name="") {
      Text_Rule rule(tag, '\0', "", style_name);
      rule.SetInternalChar(internal_char);
      RegisterRule(rule);
    }

    /// Add a symbol pair (tag start and end) that will use its CONTROL argument as a STYLE argument.
    void AddReplaceControl(String open_start, char open_end, char internal_char, String style_name) {
      Text_Rule rule(open_start, open_end, "", style_name);
      rule.SetInternalChar(internal_char);
      RegisterRule(rule);
    }

  private: // Pure helper functions...
    void _Encode_NoStyle(std::ostream & out_stream, std::string_view text_block) const {
      for (char c : text_block) {
        if (c > 0 && special_chars[c]) {
          const Text_Rule & rule = rule_set[special_chars[c]];
          out_stream << rule.GetOpenTagStart();
        }
        else out_stream << c;
      }
    }

  public:
    TextEncoding() : TextEncoding_Interface() {
      // Setup a default rule that is always ID zero.
      Text_Rule default_rule("__default_Text_Rule__", '\0', "", "");
      default_rule.SetID(0);
      rule_set.push_back(default_rule);

      // Setup a default style that is always ID zero.
      Style default_style("__default_style__", 0);
      style_set.push_back(default_style);

      // Initially there should be no rules for special characters.
      special_chars.fill(0);
    }
    ~TextEncoding() = default;

    // Reset the current encoding to clear all tag knowledge for this encoding.
    void Reset() {
      rule_set.resize(1);       // Reduce to just the default rule.
      style_set.resize(1);      // Reduce to just the default style.
      name_to_style_id.clear();
      tag_map.clear();
      special_chars.fill(0);
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
          Append_Tag(text, token.id, ss);
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
          auto & open_vec = style_pos_map[range.GetLower()];
          auto & close_vec = style_pos_map[range.GetUpper()+1];
          // Insert opens at the end and closes at the beginning; thus we close all existing
          // styles before we open any new ones (and closes happen in reverse order.)
          open_vec.emplace_back(&style_desc, true);
          close_vec.insert(close_vec.begin(), pos_info_t{&style_desc, false});
        }
      }

      // Export the string, encoding tags back in as we go.
      std::stringstream out_stream;
      size_t scan_pos = 0;
      for (auto [tag_pos, style_vec] : style_pos_map) {
        // Copy everything into the out_stream up to the tags we need to insert.
        if (scan_pos < tag_pos) {
          auto text_block = text.AsString().ViewRange(scan_pos, tag_pos);
          _Encode_NoStyle(out_stream, text_block);
          scan_pos = tag_pos;
        }

        // Insert any needed tags.  They should already be in the right order to deal with.
        for (auto [style_ptr, is_open] : style_vec) {
          const Style & style = GetStyle(*style_ptr);
          const Text_Rule & rule = GetRule(style, text, scan_pos);
          if (rule.GetID() == 0) continue; // No rule found; warning already sent.

          // If the rule has text associated with it, skip over text.
          if (rule.GeneratesText()) scan_pos += rule.GeneratesText();

          // Drop the appropriate tag here.
          if (is_open) out_stream << rule.MakeOpenTag(*style_ptr);
          else out_stream << rule.GetCloseTag();
        }
      }

      // Encode any remaining text after the final tag.
      if (scan_pos < text.size()) {
        _Encode_NoStyle(out_stream, text.AsString().View(scan_pos));
      }

      return out_stream.str();
    }

    void PrintDebug_Rules(std::ostream & os = std::cout) const {
      for (const Text_Rule & rule : rule_set) rule.PrintDebug(os);
    }

    void PrintDebug_Styles(std::ostream & os = std::cout) const {
      for (const Style & style : style_set) style.PrintDebug(os);
    }

    void PrintDebug(std::ostream & os = std::cout) const override {
      PrintDebug_Rules(os);
      PrintDebug_Styles(os);
      // @CAO Should print other info.
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
