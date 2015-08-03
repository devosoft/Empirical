#ifndef EMP_WEB_TEXT_H
#define EMP_WEB_TEXT_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Text widget.
//

#include "../tools/DynamicStringSet.h"

#include "Widget.h"

namespace emp {
namespace web {

  class Text;

  class TextInfo : public internal::WidgetInfo {
    friend Text;
  protected:
    DynamicStringSet strings;

    internal::Widget Append(const std::string & in_text) override;
    internal::Widget Append(const std::function<std::string()> & in_fun) override;

  public:
    TextInfo(const std::string & in_id="") : internal::WidgetInfo(in_id) { ; }
    TextInfo(const TextInfo &) = delete;               // No copies of INFO allowed
    TextInfo & operator=(const TextInfo &) = delete;   // No copies of INFO allowed
    virtual ~TextInfo() { ; }

    virtual std::string GetType() override { return "web::TextInfo"; }
  };


  class Text : public internal::WidgetFacet<Text> {
    friend TextInfo;
  protected:
    // Get a properly cast version of indo.
    TextInfo * Info() { return (TextInfo *) info; }

    Text(TextInfo * in_info) : WidgetFacet(in_info) { ; }
  public:
    Text(const std::string & in_name) : WidgetFacet(in_name) {
      // When a name is provided, create an associated Widget info.
      info = new TextInfo(in_name);
    }
    Text(const Text & in) : WidgetFacet(in) { ; }
    ~Text() { ; }

    virtual bool IsText() const { return true; }

    void Clear() { Info()->strings.Clear(); }

  };  

  internal::Widget TextInfo::Append(const std::string & in_text) {
    strings.Append(in_text);
    return web::Text(this);
  }

  internal::Widget TextInfo::Append(const std::function<std::string()> & in_fun) {
    strings.Append(in_fun);
    return web::Text(this);
  }

};
};

#endif
