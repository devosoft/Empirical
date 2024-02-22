/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2024
 *
 *  @file Text_Rule.hpp
 *  @brief Helper struct for managing rules text between a target encoding and Emphatic.
 *  @note Status: ALPHA
 *
 */

#ifndef EMP_TEXT_TEXT_RULE_HPP_INCLUDE
#define EMP_TEXT_TEXT_RULE_HPP_INCLUDE

#include <type_traits>
#include <unordered_map>

#include "../base/assert.hpp"
#include "../base/notify.hpp"
#include "../math/constants.hpp"
#include "../tools/String.hpp"

namespace emp {

  /// A text rule links an input encoding (like "<b>TEXT</b>" or "\bold{TEXT}") with the
  /// Emphatic conversion that it should become ("TEXT" with style "bold").
  ///
  /// The TARGET encoding is the non-Emphatic language being used.
  /// The assumed structure is:
  ///   OPEN_TAG_START  CONTROL OPEN_TAG_END  TEXT CLOSE_TAG
  ///                   ^^^^^^optional^^^^^^  ^^^optional^^^
  ///
  /// Where the TAGs are all fixed sequences and the CONTROL and TEXT vary by instance.
  ///   OPEN_TAG_END is empty if no CONTROL is allowed to be included
  ///   CLOSE_TAG is empty if no TEXT is allowed to be included
  ///
  /// For example: <a href="http://my.url">Go to My URL</a>
  /// Would have:
  ///   OPEN_TAG_START : <a
  ///   CONTROL        : href="http://my.url"
  ///   OPEN_TAG_END   : >
  ///   TEXT           : Go to My URL
  ///   CLOSE_TAG      : </a>
  ///
  /// Post-conversion, this would become the text "Go to My URL" with the associated style
  /// "link:http://my.url"
  ///
  /// When converting back to HTML, the start of the "link" style would trigger the open
  /// tag to be generated.  When the link ends, the close take will appear.
  /// If there is no style, the internal character will trigger this rule to apply.

  class Text_Rule {
  private:
    size_t id = emp::MAX_SIZE_T;  // Unique ID for this rule (index in tag_map)
    int open_token_id = -1;       // ID of open_tag_start in the lexer
    int close_token_id = -1;      // ID of close_tag in the lexer

    // - Target parsing info -
    String open_tag_start;    ///< Beginning of open tag. (E.g. "<a" or "\bold{")
    char open_tag_end = '\0'; ///< End of open tag (E.g., '>') or empty if no control used
    String close_tag;         ///< Tag to indicate close of text, or empty if no text used

    // - Styling info -
    // A base style would be "font"
    // Style argument could be "arial"
    // This would generate a full style: "font:arial"
    String base_style;          ///< What style is this rule associated with?
    char internal_char = '\0';  ///< What character is used to placehold internally in Text?

    std::function<String(String)> to_style_arg; ///< Convert CONTROL into STYLE arguments
    std::function<String(String)> to_control;   ///< Convert STYLE into CONTROL

  public:
    Text_Rule() { }
    Text_Rule(String open_tag_start, char open_tag_end, String close_tag, String base_style)
      : open_tag_start(open_tag_start), open_tag_end(open_tag_end), close_tag(close_tag)
      , base_style(base_style)
    { }

    // === Accessors ===
    size_t GetID() const { return id; }
    const String & GetOpenTagStart() const { return open_tag_start; }
    char GetOpenTagEnd() const { return open_tag_end; }
    const String & GetCloseTag() const { return close_tag; }
    const String & GetBaseStyle() const { return base_style; }
    char GetInternalChar() const { return internal_char; }

    void SetID(size_t in_id) {
      emp_assert(in_id < 1000000000, in_id, "Invalid rule ID");
      id = in_id;
    }
    void SetInternalChar(char in) { internal_char = in; }
    void SetConversions(auto _to_style_arg, auto _to_control) {
      to_style_arg = _to_style_arg;
      to_control = _to_control;
    }

    // === Helper functions ===
    bool IsValid() const { return (id < emp::MAX_SIZE_T); }
    bool UsesControl() const { return open_tag_end != '\0'; }
    bool UsesText() const { return close_tag.size(); }
    bool UsesStyle() const { return base_style.size(); }
    bool GeneratesText() const { return internal_char; }

    String MakeStyle(String control) const {
      if (UsesControl() && to_style_arg) {
        String style_arg = to_style_arg(control);
        if (style_arg.size()) return MakeString(base_style, ':', style_arg);
      }
      return base_style;
    }

    String MakeOpenTag(String style) const {
      if (UsesControl()) {
        return MakeString(open_tag_start, to_control(style), open_tag_end);
      }
      else return open_tag_start;
    }

    void PrintDebug(std::ostream & os = std::cout) const {
      os << "Rule " << id << ": " << open_tag_start;
      if (UsesControl()) os << "CONTROL" << open_tag_end;
      if (UsesText()) os << "TEXT" << close_tag;
      os << " : base_style='" << base_style
         << "' ; internal_char='" << internal_char
         << "' ; open_token_id=" << open_token_id
         << " ; close_token_id=" << close_token_id
         << std::endl;
    }
  };

}

#endif // #ifndef EMP_TEXT_TEXT_RULE_HPP_INCLUDE
