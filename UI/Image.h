#ifndef EMP_UI_IMAGE_H
#define EMP_UI_IMAGE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Image widget.
//


#include "UI_base.h"

namespace emp {
namespace UI {

  // Forward-declare the internal Image_detail class
  namespace internal { class Image_detail; };

  // Specify the Image class for use in return values in the Image_detail definitions below.
  using Image = internal::Widget_wrap<internal::Image_detail, const std::string &>;

  namespace internal {
    class Image_detail : public Widget_base {
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
      Image_detail(const std::string & in_url)
        : url(in_url), alt_text("") { obj_ext = "__i"; }
      
      Image & URL(const std::string & in_url) { url = in_url; return (Image &) *this; }
      Image & Alt(const std::string & in_alt) { alt_text = in_alt; return (Image &) *this; }

      static std::string TypeName() { return "Image"; }
    };

  };

};
};

#endif
