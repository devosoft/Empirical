/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-2023.
 *
 *  @file EmphaticEncoding.hpp
 *  @brief Plugs into emp::Text, setting up inputs and output to be Emphatic encoded.
 *  @note Status: ALPHA
 * 
 */

#ifndef EMP_TOOLS_EMPHATIC_ENCODING_HPP_INCLUDE
#define EMP_TOOLS_EMPHATIC_ENCODING_HPP_INCLUDE

#include <set>
#include <string>
#include <unordered_map>

#include "Text.hpp"
#include "TextEncoding.hpp"

namespace emp {

  class EmphaticEncoding : public emp::TextEncoding {
  private:

    void SetupTags() {
      SetupStyleTags("bold",        "`*", "`*");
      SetupStyleTags("code",        "``", "``");
      SetupStyleTags("italic",      "`/", "`/");
      SetupStyleTags("strike",      "`~", "`~");
      SetupStyleTags("subscript",   "~.", "~.");
      SetupStyleTags("superscript", "`^", "`^");
      SetupStyleTags("underline",   "`_", "`_");
      SetupStyleTags("header:1",    "`#", "`#");
      SetupStyleTags("header:2",    "`##", "`##");
      SetupStyleTags("header:3",    "`###", "`###");
      SetupStyleTags("header:4",    "`####", "`####");
      SetupStyleTags("header:5",    "`#####", "`#####");
      SetupStyleTags("header:6",    "`######", "`######");

      SetupReplaceTag("\\`",  '`');
      SetupReplaceTag("\\\\", '\\');
      SetupReplaceTag("\\ ",  ' ', "no_break");

      SetupReplaceTag("`-",  ' ', "bullet");
      SetupReplaceTag("`+",  ' ', "ordered");
      SetupReplaceTag("`>",  ' ', "indent");
      SetupReplaceTag("`\"",  ' ', "blockquote");

      // tag_map['|'] = "continue";  // Keep previous lines setup (for set of full-line tags)

      // AVAILABLE TAGS: @&;:',?()]

      // Start of more complex tags...
      // tag_map['%'] = "comment";   // `% Should be removed by the lexer.
      // tag_map['['] = "link";      // `[Include a link name here](http://and.its.url.here)
      // tag_map['<'] = "URL";       // `<http://just.a.url.here>
      // tag_map['!'] = "image";     // `![Link/to/image/here.jpg](with an optional URL link)
      // tag_map['='] = "plaintext"; // `=No special `characters` should be \acknowledged.
      // tag_map['{'] = "execute";   // `{Code to run`}
      // tag_map['$'] = "eval";      // `$var_value_printed_here$
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

#endif