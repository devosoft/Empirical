/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-2024.
 *
 *  @file EmphaticEncoding.hpp
 *  @brief Plugs into emp::Text, setting up inputs and output to be Emphatic encoded.
 *  @note Status: ALPHA
 *
 */

#ifndef EMP_TEXT_EMPHATICENCODING_HPP_INCLUDE
#define EMP_TEXT_EMPHATICENCODING_HPP_INCLUDE

#include <set>
#include <string>
#include <unordered_map>

#include "TextEncoding.hpp"
#include "Text.hpp"

namespace emp {

  class EmphaticEncoding : public emp::TextEncoding {
  private:

    void SetupTags() {
      // === Styles ===
      AddStyleToggle("bold",        "`*");
      AddStyleToggle("code",        "`");  // Note: Longer tags will match first.
      AddStyleToggle("code",        "``");
      AddStyleToggle("code_block",  "```");
      AddStyleToggle("italic",      "`/");
      AddStyleToggle("strike",      "`~");
      AddStyleToggle("subscript",   "`.");
      AddStyleToggle("superscript", "`^");
      AddStyleToggle("underline",   "`_");
      AddStyleToggle("center",      "`=");
      AddStyleToggle("justify",     "`|");
      AddStyleToggle("ljustify",    "`<");
      AddStyleToggle("rjustify",    "`>");

      AddStyleLine("blockquote", "`\"");
      AddStyleLine("comment",    "`%");
      AddStyleLine("header:1",   "`#");
      AddStyleLine("header:2",   "`##");
      AddStyleLine("header:3",   "`###");
      AddStyleLine("header:4",   "`####");
      AddStyleLine("header:5",   "`#####");
      AddStyleLine("no_format",  "`-");
      AddStyleLine("continue_format", "`:");

      AddStyleControl("`@{", '}', "`@", "link");
      AddStyleControl("`[", ']', "`#",  "color");

      // === Insertions ===

      AddReplaceTag("\\`",  '`');            // A regular backtick
      AddReplaceTag("\\\\", '\\');           // A regular backslash
      AddReplaceTag("\\n",  '\n');           // A line break
      AddReplaceTag("\\t",  '\t');           // A tab
      AddReplaceTag("\\.",  ' ', "empty");   // An empty tag

      AddReplaceTag("\\ ",  ' ', "no_break");          // A non-breaking space
      AddReplaceTag("\\b",  ' ', "page_break");        // A page break
      AddReplaceTag("\\p",  ' ', "para_break");        // A paragraph break
      AddReplaceTag("\\-",  '-', "horizontal_rule"); // A horizontal rule

      AddReplaceTag("\\*",      '*', "bullet:1");  // Simple bullet
      AddReplaceTag("\\**",     '*', "bullet:2");  // Simple bullet (indented)
      AddReplaceTag("\\***",    '*', "bullet:3");  // Simple bullet (indented)
      AddReplaceTag("\\****",   '*', "bullet:4");  // Simple bullet (indented)
      AddReplaceTag("\\*****",  '*', "bullet:5");  // Simple bullet (indented)

      AddReplaceTag("\\+",      '1', "bullet_ordered:1"); // Next numerical bullet
      AddReplaceTag("\\++",     '1', "bullet_ordered:2"); // Next numerical bullet
      AddReplaceTag("\\+++",    '1', "bullet_ordered:3"); // Next numerical bullet
      AddReplaceTag("\\++++",   '1', "bullet_ordered:4"); // Next numerical bullet
      AddReplaceTag("\\+++++",  '1', "bullet_ordered:5"); // Next numerical bullet

      AddReplaceTag("\\a",      'a', "bullet_alpha:1");  // Next lowercase bullet
      AddReplaceTag("\\aa",     'a', "bullet_alpha:2");  // Next lowercase bullet
      AddReplaceTag("\\aaa",    'a', "bullet_alpha:3");  // Next lowercase bullet
      AddReplaceTag("\\aaaa",   'a', "bullet_alpha:4");  // Next lowercase bullet
      AddReplaceTag("\\aaaaa",  'a', "bullet_alpha:5");  // Next lowercase bullet

      AddReplaceTag("\\A",      'A', "bullet_ALPHA:1");  // Next uppercase bullet
      AddReplaceTag("\\AA",     'A', "bullet_ALPHA:2");  // Next uppercase bullet
      AddReplaceTag("\\AAA",    'A', "bullet_ALPHA:3");  // Next uppercase bullet
      AddReplaceTag("\\AAAA",   'A', "bullet_ALPHA:4");  // Next uppercase bullet
      AddReplaceTag("\\AAAAA",  'A', "bullet_ALPHA:5");  // Next uppercase bullet

      AddReplaceTag("\\o",      'o', "bullet_task:1");   // Checkbox bullet
      AddReplaceTag("\\oo",     'o', "bullet_task:2");   // Checkbox bullet
      AddReplaceTag("\\ooo",    'o', "bullet_task:3");   // Checkbox bullet
      AddReplaceTag("\\oooo",   'o', "bullet_task:4");   // Checkbox bullet
      AddReplaceTag("\\ooooo",  'o', "bullet_task:5");   // Checkbox bullet

      AddReplaceTag("\\r",      'i', "bullet_roman:1");  // Next lowercase roman-numeral bullet
      AddReplaceTag("\\rr",     'i', "bullet_roman:2");  // Next lowercase roman-numeral bullet
      AddReplaceTag("\\rrr",    'i', "bullet_roman:3");  // Next lowercase roman-numeral bullet
      AddReplaceTag("\\rrrr",   'i', "bullet_roman:4");  // Next lowercase roman-numeral bullet
      AddReplaceTag("\\rrrrr",  'i', "bullet_roman:5");  // Next lowercase roman-numeral bullet

      AddReplaceTag("\\R",      'I', "bullet_ROMAN:1");  // Next uppercase roman-numeral bullet
      AddReplaceTag("\\RR",     'I', "bullet_ROMAN:2");  // Next uppercase roman-numeral bullet
      AddReplaceTag("\\RRR",    'I', "bullet_ROMAN:3");  // Next uppercase roman-numeral bullet
      AddReplaceTag("\\RRRR",   'I', "bullet_ROMAN:4");  // Next uppercase roman-numeral bullet
      AddReplaceTag("\\RRRRR",  'I', "bullet_ROMAN:5");  // Next uppercase roman-numeral bullet

      AddReplaceControl("\\#{", '}', ' ', "tag");
      // AddReplaceControl("\\={", '}', "custom_bullet");

      // AddControlTag("\\|", "\n", "table" );
      AddReplaceControl("\\^{", '}', ' ', "note");
      AddReplaceControl("\\[",  ']', ' ', "image");
      AddReplaceControl("\\:",  ':', ' ', "symbol");
      AddReplaceControl("\\:{", '}', ' ', "symbol");
      AddReplaceControl("\\d{", '}', ' ', "div");

      // Use arg as BOTH text and style arg.
      // AddReplaceDynamic("\\(", ")", "link", [](emp::String arg){ return arg; }, [](emp::String arg){ return emp::MakeString("link:", arg); });


      // For easier HTML conversion, we also have the ability to simply escape existing HTML text:
      // \<tag> tries to match the associated HTML tag
      // \&name; or \&{name} insert a character by a given HTML name (unicode works same as HTML)

    }

  public:
    EmphaticEncoding() { SetupTags(); }
    ~EmphaticEncoding() = default;

    String GetName() const override { return "emphatic"; }
    emp::Ptr<TextEncoding_Interface> Clone() const override {
      return emp::NewPtr<EmphaticEncoding>();
    }
  };

  using EmphaticText = EncodedText<EmphaticEncoding>;

  template <typename... Ts>
  emp::Text MakeEmphaticText(Ts &&... args) {
    return MakeEncodedText<EmphaticEncoding>(std::forward<Ts>(args)...);
  }
}

#endif // #ifndef EMP_TEXT_EMPHATICENCODING_HPP_INCLUDE
