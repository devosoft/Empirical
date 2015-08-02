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

    Element & Append(const std::string & in_text) override {
      strings.Append(in_text);
      return *this;
    }

    Element & Append(const std::function<std::string()> & in_fun) override {
      strings.Append(in_fun);
      return *this;
    }

  public:
    virtual std::string GetType() override { return "web::TextInfo"; }

  };


  class Text : public internal::WidgetFacet<Text> {
  protected:
    // Get a properly cast version of indo.
    TextInfo * Info() { return (TextInfo *) info; }

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

};
};

#endif
