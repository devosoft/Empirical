/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022.
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

#include "../base/assert.hpp"
#include "../base/array.hpp"
#include "../base/notify.hpp"
#include "../compiler/Lexer.hpp"

#include "Text.hpp"

namespace emp {

  class EmphaticEncoding : public emp::TextEncoding {
  private:
    emp::array<std::string, 128> tag_map;
    emp::array<std::string, 128> escape_map;

    emp::Lexer lexer;
    enum EmphToken {
      text = 255,
      escape = 254,
      tag = 253,
      code = 252,
      comment = 251,
      link = 250,
      url = 249,
      image = 248,
      plain = 247,
      exe = 246,
      eval = 245,
      error = 244 };

    std::set<std::string> active_styles; // Styles to use on appended text.


    void SetupTags() {
      tag_map['*'] = "bold";
      tag_map['`'] = "code";
      tag_map['/'] = "italic";
      tag_map['~'] = "strike";
      tag_map['.'] = "subscript";
      tag_map['^'] = "superscript";
      tag_map['_'] = "underline";

      tag_map['#'] = "heading";
      tag_map['-'] = "bullet";
      tag_map['+'] = "ordered";
      tag_map['>'] = "indent";

      // AVAILABLE TAGS: @&;:'",?()]\

      // Start of more complex tags...
      // tag_map['|'] = "continue";  // Keep previous lines setup (for set of full-line tags)
      // tag_map['%'] = "comment";   // `% Should be removed by the lexer.
      // tag_map['['] = "link";      // `[Include a link name here](http://and.its.url.here)
      // tag_map['<'] = "URL";       // `<http://just.a.url.here>
      // tag_map['!'] = "image";     // `![Link/to/image/here.jpg](with an optional URL link)
      // tag_map['='] = "plaintext"; // `=No special `characters` should be \acknowledged.
      // tag_map['{'] = "execute";   // `{Code to run`}
      // tag_map['$'] = "eval";      // `$var_value_printed_here$

      escape_map['\\'] = '\\';
      escape_map['`'] = '`';
      escape_map[' '] = ' ';  // Plus a non-breaking style on the space.
      escape_map['\n'] = '\n';
      escape_map['\t'] = '\t';

      // Now that all of the tags are loaded, put them into the lexer.
      [[maybe_unused]] int token_id;
      token_id = lexer.AddToken("text","[^`\\]+");    // Non-tag or special characters.
      emp_assert(token_id == EmphToken::text);
      token_id = lexer.AddToken("escape", "\\\\[\\\\` \n\t]");
      emp_assert(token_id == EmphToken::escape);
      token_id = lexer.AddToken("tag", "`[*`/~.\\^_#+>|-]");   // Tags
      emp_assert(token_id == EmphToken::tag);

      // Specialty tags...
      token_id = lexer.AddToken("code", "`[a-zA-Z0-9 \n\t]"); // Single ` okay if followed by alphanumeric or whitespace.
      emp_assert(token_id == EmphToken::code);
      token_id = lexer.AddToken("comment", "`%.*");        // Clear whole line.
      emp_assert(token_id == EmphToken::comment);
      token_id = lexer.AddToken("link", "`\\[[^\\]\n]*\\]\\([^)\n]*\\)" );
      emp_assert(token_id == EmphToken::link);
      token_id = lexer.AddToken("url", "`<[^>\n]*>" );
      emp_assert(token_id == EmphToken::url);
      token_id = lexer.AddToken("image", "`!\\[[^\\]\n]*\\](\\([^)\n]*\\))?");
      emp_assert(token_id == EmphToken::image);
      token_id = lexer.AddToken("plain", "`=.*");
      emp_assert(token_id == EmphToken::plain);
      token_id = lexer.AddToken("exe", "`{[^`]*`}");
      emp_assert(token_id == EmphToken::exe);
      token_id = lexer.AddToken("eval", "`$[a-zA-Z0-1_]*$");
      emp_assert(token_id == EmphToken::eval);
    }

    // Append a string that has already been otherwise processed.
    void Append_String(const std::string & in) {
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
        case TagType::OPEN:
          active_styles.insert(info.style);
          break;
        case TagType::CLOSE:
          active_styles.erase(info.style);
          break;
        case TagType::REPLACE:
          Append_String(info.text);
          break;
        default:
          break;
      }
    }

  public:
    EmphaticEncoding(Text & _text, const std::string _name="emphatic")
      : TextEncoding(_text, _name) { SetupTags(); }
    ~EmphaticEncoding() = default;

    // Add new Emphatic encoded text into this object.
    void Append(std::string in) override {
//      std::cout << "APPEND: " << in << std::endl;

      auto tokens = lexer.Tokenize(in);
      for (const auto & token : tokens) {
        switch (token.id) {
          case EmphToken::text:
            break;
          case EmphToken::escape:
            break;
          case EmphToken::tag:
            break;
          case EmphToken::code:
            break;
          case EmphToken::comment:
            break;
          case EmphToken::link:
            break;
          case EmphToken::url:
            break;
          case EmphToken::image:
            break;
          case EmphToken::plain:
            break;
          case EmphToken::exe:
            break;
          case EmphToken::eval:
            break;
          default:
            notify::Error("Unknown token: ", token.id);
        }
      }
    }

    // Convert this to a string in Emphatic format.
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

  private:
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

      // Loop through other site checking for shifts in style.
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