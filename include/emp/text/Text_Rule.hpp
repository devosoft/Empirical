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

  // == DEVELOPER NOTES ==
  // Types of rules we need to support:
  //   Style (e.g., bold, font, link, etc)
  //     Standard - open and close are individually specified
  //     Toggles  - open and close are the same
  //     Lines    - open is specified and close is always a newline
  //     Control  - open may have a control sequence, but close is still simple
  //
  //   Object (e.g., Symbol, emoji, image, page break, etc)
  //     Simple replacement - encoding tag becomes the object name (perhaps "type:name")
  //     Parametered replacement - where a control parameter in the tag must be processed.

  class Text_Rule {
  protected:

    //  ====================
    //  === HELPER TYPES ===
    //  ====================

    enum RuleType { UNKNOWN_RULE=0, STYLE_RULE, OBJECT_RULE };

    /// A helper tag type for tags with a control sequence to specify their operation.
    struct TagInfo {
      int token_id = -1;  ///< ID of this tag in the lexer
      String start;   ///< Unique beginning of the tag (e.g., "<img " or "<a ")
      char end='\0';  ///< End of tag (E.g., '>')

      std::function<String(String)> to_arg_fun;     ///< Convert CONTROL into internal ARG info
      std::function<String(String)> to_control_fun; ///< Convert ARG description into CONTROL

      // Can this tag forgo a control sequence?
      TagInfo() { }
      TagInfo(String start, char end='\0') : start(start), end(end) { }
      [[nodiscard]] bool IsUsed() const { return token_id >= 0; }
      [[nodiscard]] bool IsSimple() const { return end == '\0'; }
      [[nodiscard]] bool HasFuns() const { return to_arg_fun && to_control_fun; }
    };


    //  =====================
    //  ===  MEMBER VARS  ===
    //  =====================

    size_t id = emp::MAX_SIZE_T;       ///< Unique ID for this rule (index in tag_map)
    RuleType rule_type = UNKNOWN_RULE; ///< What type of rule is this?

    TagInfo main_tag; ///< Identifier tag for an object or open tag for a style
    String base_type; ///< Styles (eg, "bold", "font:...") or Objects (eg, "image", "symbol:theta")

    // Style-only data
    TagInfo close_tag; ///< Identifier tag to signal the end of a style.

    // Object-only data.
    char placeholder = ' ';  ///< Character to use internally in Text

  public:
    Text_Rule() { }
    Text_Rule(String open_tag_start, char open_tag_end, String close_tag,
              String base_type, char placeholder=' ')
      : main_tag(open_tag_start, open_tag_end), base_type(base_type), close_tag(close_tag)
      , placeholder(placeholder)
    {
      // Styles have close tags, objects do not.
      rule_type = close_tag.empty() ? RuleType::OBJECT_RULE : RuleType::STYLE_RULE;
    }

    // === Accessors ===
    [[nodiscard]] size_t GetID()           const { return id; }
    [[nodiscard]] String GetOpenTagStart() const { return main_tag.start; }
    [[nodiscard]] char   GetOpenTagEnd()   const { return main_tag.end; }
    [[nodiscard]] String GetCloseTag()     const { return close_tag.start; }
    [[nodiscard]] String GetBaseStyle()    const { return rule_type == STYLE_RULE ? base_type : ""; }
    [[nodiscard]] String GetSymbol()       const { return rule_type == OBJECT_RULE ? base_type : ""; }
    [[nodiscard]] char   GetInternalChar() const { return placeholder; }

    void SetID(size_t in_id) {
      emp_assert(in_id < 1000000000, in_id, "Invalid rule ID");
      id = in_id;
    }
    void SetConversions(auto _to_arg_fun, auto _to_control_fun) {
      main_tag.to_arg_fun = _to_arg_fun;
      main_tag.to_control_fun = _to_control_fun;
    }

    // === Helper functions ===
    [[nodiscard]] bool IsValid() const { return (id != emp::MAX_SIZE_T); }
    [[nodiscard]] bool UsesControl() const { return !main_tag.IsSimple(); }
    [[nodiscard]] bool UsesText() const { return close_tag.IsUsed(); }
    [[nodiscard]] bool SetsStyle() const { return rule_type == RuleType::STYLE_RULE; }
    [[nodiscard]] bool SetsObject() const { return rule_type == RuleType::OBJECT_RULE; }
    [[nodiscard]] bool IsSimpleReplacement() const { return base_type.size() == 0; }

    [[nodiscard]] String MakeStyle(String control) const {
      if (UsesControl()) {
        emp_assert(main_tag.HasFuns());
        String style_arg = main_tag.to_arg_fun(control);
        if (style_arg.size()) return MakeString(base_type, ':', style_arg);
      }
      return base_type;
    }

    [[nodiscard]] String MakeOpenTag(String style) const {
      if (UsesControl()) {
        return MakeString(main_tag.start, main_tag.to_control_fun(style), main_tag.end);
      }
      else return main_tag.start;
    }

    void PrintDebug(std::ostream & os = std::cout) const {
      os << "Rule " << id << ": " << main_tag.start;
      if (UsesControl()) os << "CONTROL" << main_tag.end;
      if (UsesText()) os << "TEXT" << close_tag.start;
      os << " : base_type='" << base_type
         << "' ; placeholder='" << placeholder
         << "' ; open_token_id=" << main_tag.token_id
         << " ; close_token_id=" << close_tag.token_id
         << std::endl;
    }
  };

}

#endif // #ifndef EMP_TEXT_TEXT_RULE_HPP_INCLUDE
