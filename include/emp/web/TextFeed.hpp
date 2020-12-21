/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  TextFeedFeed.hpp
 *  @brief A representation of text on a web page optimized for rapid appends.
 *
 */

#ifndef EMP_WEB_TEXTFEED_HPP
#define EMP_WEB_TEXTFEED_HPP

#include <cstddef>
#include <deque>
#include <limits>
#include <string>

#include "Widget.hpp"

namespace emp {
namespace web {

  /// A TextFeed widget handles putting text on a web page that can be controlled and modified.

  class TextFeed : public internal::WidgetFacet<TextFeed> {
    friend class TextFeedInfo;
  protected:

    class TextFeedInfo : public internal::WidgetInfo {
      friend TextFeed;
    protected:
      std::deque<std::string> strings;    ///< All strings in TextFeed widget.

      std::string separator; ///< Text to print between strings.

      size_t max_size; ///< How many strings can this TextFeed hold before old strings are expunged?

      bool append_ok{ true };           ///< Can this TextFeed widget be extended?

      TextFeedInfo(
        const std::string & in_id_="",
        const std::string & separator_="",
        const size_t max_size_=std::numeric_limits< size_t >::max()
      ) : internal::WidgetInfo( in_id_ )
      , separator( separator_ )
      , max_size( max_size_ )
      { ; }

      TextFeedInfo(const TextFeedInfo &) = delete;               // No copies of INFO allowed
      TextFeedInfo & operator=(const TextFeedInfo &) = delete;   // No copies of INFO allowed
      virtual ~TextFeedInfo() { ; }

      std::string GetTypeName() const override { return "TextFeedInfo"; }

      bool AppendOK() const override { return append_ok; }
      void PreventAppend() override { append_ok = false; }

      Widget Append(const std::string & in_text) override;

      // All derived widgets must suply a mechanism for providing associated HTML code.
      virtual void GetHTML(std::stringstream & HTML) override {
        HTML.str("");                         // Clear the current text.
        HTML << "<span id=\'" << id << "'>";  // Initial span tag to keep id.
        for (const auto& item : strings) {
          if ( &item != &strings.front() ) HTML << separator;
          HTML << item;
        }
        HTML << "</span>";                    // Close span tag.
      }

    public:
      virtual std::string GetType() override { return "web::TextFeedInfo"; }
    };  // End of TextFeedInfo


    // Get a properly cast version of info.
    TextFeedInfo * Info() { return (TextFeedInfo *) info; }
    const TextFeedInfo * Info() const { return (TextFeedInfo *) info; }

    TextFeed(TextFeedInfo * in_info) : WidgetFacet(in_info) { ; }
  public:
    TextFeed(
      const std::string & in_id="",
      const std::string & separator="",
      const size_t max_size=std::numeric_limits< size_t >::max()
    ) : WidgetFacet(in_id) {
      // When a name is provided, create an associated Widget info.
      info = new TextFeedInfo(in_id, separator, max_size);
    }
    TextFeed(const TextFeed & in) : WidgetFacet(in) { ; }
    TextFeed(const Widget & in) : WidgetFacet(in) { emp_assert(in.IsTextFeed()); }
    ~TextFeed() { ; }

    using INFO_TYPE = TextFeedInfo;

    /// How many text items are contained?
    size_t GetSize() const { return Info()->strings.size(); }

    /// Erase current text.
    TextFeed & Clear() { Info()->strings.clear(); return *this; }

    /// Remove last text item.
    TextFeed & PopBack() { Info()->strings.pop_back(); return *this; }
  };

  /// Add new text to this string.
  Widget TextFeed::TextFeedInfo::Append(const std::string & text) {
    // If text widget cannot append, forward to parent.
    if (!append_ok) return ForwardAppend(text);

    if (state == Widget::ACTIVE) {
      const std::string to_append = strings.empty() ? text : (separator + text);
      // ideally, we would use MAIN_THREAD_ASYNC_EM_ASM but that seems to
      // garble string arguments (as of emscripten 1.38.48)
      MAIN_THREAD_EM_ASM({
        var content = document.createElement('span');
        content.innerHTML = UTF8ToString($1);
        $( `#${UTF8ToString($0)}` ).append( content );
      }, id.c_str(), to_append.c_str() );
    }

    strings.push_back(text); // Record the new string being added.

    if ( strings.size() > max_size ) {

      strings.pop_front();
      MAIN_THREAD_ASYNC_EM_ASM({
        $(`#${UTF8ToString($0)}`).contents().first().remove();
      }, id.c_str() );

    }

    // prevent runaway async execution! do this every N?
    // MAIN_THREAD_EM_ASM({
    //   ()=>{}; // nop
    // });

    return web::TextFeed(this);

  }

} // namespace web
} // namespace emp

#endif// #ifndef EMP_WEB_TEXTFEED_HPP
