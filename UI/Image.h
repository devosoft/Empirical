#ifndef EMP_UI_IMAGE_H
#define EMP_UI_IMAGE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Image widget.
//


#include "Widget.h"

namespace emp {
namespace UI {

  class Image : public internal::Widget<Image> {
  protected:
    std::string url;
    std::string alt_text;
      
    void WriteHTML(std::ostream & os) {
      os << "<img src=\"" << url << "\""
         << " alt=\"" << alt_text << "\""
         << " id=\"" << div_id << obj_ext << "\"";
      os << ">";
    }

  public:
    Image(const std::string & in_url, const std::string & in_name="")
      : Widget(in_name), url(in_url), alt_text("") { obj_ext = "__i"; }
      
    Image & URL(const std::string & in_url) { url = in_url; return *this; }
    Image & Alt(const std::string & in_alt) { alt_text = in_alt; return *this; }

    static std::string TypeName() { return "Image"; }
  };

};
};

#endif
