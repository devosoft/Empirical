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

#include "../base/array.hpp"
#include "../base/assert.hpp"
#include "../base/notify.hpp"
#include "../compiler/Lexer.hpp"
#include "../tools/string_utils.hpp"

#include "Text.hpp"

namespace emp {

  class EmphaticEncoding : public emp::TextEncoding {
  private:
    emp::array<std::string, 128> tag_map;     // Maps tag characters to style names
    std::map<std::string, > style_map;    // Maps styles
    emp::array<std::string, 128> escape_map;

    emp::Lexer lexer;

    enum class EmphToken {
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
      EOL = 244,
      error = 243 };

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
      tag_map['"'] = "blockquote";
      tag_map['|'] = "continue";  // Keep previous lines setup (for set of full-line tags)

      // AVAILABLE TAGS: @&;:',?()]\

      // Start of more complex tags...
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
      emp_assert(token_id == (int) EmphToken::text);
      token_id = lexer.AddToken("escape", "\\\\[\\\\` \n\t]");
      emp_assert(token_id == (int) EmphToken::escape);
      token_id = lexer.AddToken("tag", "`[*`/~.\\^_#+>\"|-]");   // Tags
      emp_assert(token_id == (int) EmphToken::tag);

      // Specialty tags...
      token_id = lexer.AddToken("code", "`[a-zA-Z0-9 \n\t]"); // Single ` okay if followed by alphanumeric or whitespace.
      emp_assert(token_id == (int) EmphToken::code);
      token_id = lexer.AddToken("comment", "`%.*");        // Clear whole line.
      emp_assert(token_id == (int) EmphToken::comment);
      token_id = lexer.AddToken("link", "`\\[[^\\]\n]*\\]\\([^)\n]*\\)" );
      emp_assert(token_id == (int) EmphToken::link);
      token_id = lexer.AddToken("url", "`<[^>\n]*>" );
      emp_assert(token_id == (int) EmphToken::url);
      token_id = lexer.AddToken("image", "`!\\[[^\\]\n]*\\](\\([^)\n]*\\))?");
      emp_assert(token_id == (int) EmphToken::image);
      token_id = lexer.AddToken("plain", "`=.*");
      emp_assert(token_id == (int) EmphToken::plain);
      token_id = lexer.AddToken("exe", "`{[^`]*`}");
      emp_assert(token_id == (int) EmphToken::exe);
      token_id = lexer.AddToken("eval", "`$[a-zA-Z0-1_]*$");
      emp_assert(token_id == (int) EmphToken::eval);
      token_id = lexer.AddToken("EOL", "\n");
      emp_assert(token_id == (int) EmphToken::EOL);
    }

    // Append a string that has already been otherwise processed.
    void Append_Text(const std::string & in) {
      size_t start = text.size();
      size_t end = start + in.size();
      text.Append_Raw(in);
      for (const std::string & style : active_styles) {
        text.SetStyle(style, start, end);
      }
    }

    void Append_Tag(const std::string & in) {
      emp_assert(in.size() >= 2);
      emp_assert(in[0] == '`');
      const std::string & style = tag_map[in[1]];
      switch (in[1]) {
        // -- Tags that toggle --
        case '*': // Bold
        case '`': // Code
        case '/': // Italic
        case '~': // Strike
        case '.': // Subscript
        case '^': // Superscript
        case '_': // Underline
          if (active_styles.count(style)) {
            active_styles.erase(style);
          } else {
            active_styles.insert(style);
          }
          break;

        // -- Tags that go to the end of line --
        case '#': // Heading
        case '-': // Bullet
        case '+': // Number Bullet
        case '>': // Indent
        case '"': // Blockquote
          active_styles.insert(style);
          break;

        // -- Other special tags --
        case '|': // Continue previous format.
          break;
      }
    }

    void Append_Newline() {
      // Terminate any style that only goes to the end of the current line.
      active_styles.erase("heading");
      active_styles.erase("bullet");
      active_styles.erase("ordered");
      active_styles.erase("indent");
      active_styles.erase("blockquote");

      // Pass along the newline.
      Append_Text("\n");
    }

    void Append_Escape(const std::string & in) {
      emp_assert(in[0] == '\\');
      switch (in[1]) {
        case '`': Append_Text("`"); break;
        case '\\': Append_Text("\\"); break;
        case ' ': Append_Text(" "); text.back().SetStyle("nobreak"); break;
        case 'n': Append_Text("\n"); break;
        case 't': Append_Text("\t"); break;
      }
    }

  public:
    EmphaticEncoding(Text & _text, const std::string _name="emphatic")
      : TextEncoding(_text, _name) { SetupTags(); }
    ~EmphaticEncoding() override { };

    // Add new Emphatic encoded text into this object.
    void Append(std::string in) override {
//      std::cout << "APPEND: " << in << std::endl;

      auto tokens = lexer.Tokenize(in);
      for (const auto & token : tokens) {
        switch ((EmphToken) token.id) {
          case EmphToken::text:
            Append_Text(token.lexeme);
            break;
          case EmphToken::escape:
            Append_Escape(token.lexeme);
            break;
          case EmphToken::tag:
            Append_Tag(token.lexeme);
            break;
          case EmphToken::code:
            Append_Tag("``");
            Append_Text(emp::to_string(token.lexeme[1]));
            break;
          case EmphToken::comment:
            // Explicitly discard comment.
            break;
          case EmphToken::link:
            // TODO
            break;
          case EmphToken::url:
            // TODO
            break;
          case EmphToken::image:
            // TODO
            break;
          case EmphToken::plain:
            Append_Text(token.lexeme.substr(2,token.lexeme.size()-2));
            break;
          case EmphToken::exe:
            // TODO
            break;
          case EmphToken::eval:
            // TODO
            break;
          case EmphToken::EOL:
            Append_Newline();
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
