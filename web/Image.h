#ifndef EMP_WEB_IMAGE_H
#define EMP_WEB_IMAGE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Image widget.
//

#include "Widget.h"

namespace emp {
namespace web {

  class Image : public internal::WidgetFacet<Image> {
    friend class ImageInfo;
  protected:
    
    class ImageInfo : public internal::WidgetInfo {
      friend Image;
      
    protected:
      std::string url;
      std::string alt_text;
            
      ImageInfo(const std::string & in_id="") : internal::WidgetInfo(in_id) { ; }
      ImageInfo(const ImageInfo &) = delete;               // No copies of INFO allowed
      ImageInfo & operator=(const ImageInfo &) = delete;   // No copies of INFO allowed
      virtual ~ImageInfo() { ; }
      
      virtual bool IsImageInfo() const override { return true; }

      virtual void GetHTML(std::stringstream & HTML) override {
        HTML.str("");                                      // Clear the current text.
        HTML << "<img src=\"" << url << "\""
             << " alt=\"" << alt_text << "\""
             << " id=\"" << id << "\">";
      }
      
      void UpdateURL(const std::string & in_url) {
        url = in_url;
        if (state == Widget::ACTIVE) ReplaceHTML();
      }
      void UpdateAlt(const std::string & in_alt) {
        alt_text = in_alt;
        if (state == Widget::ACTIVE) ReplaceHTML();
      }
      
    public:
      virtual std::string GetType() override { return "web::ButtonInfo"; }
    };


    // Get a properly cast version of indo.  
    ImageInfo * Info() { return (ImageInfo *) info; }
    const ImageInfo * Info() const { return (ImageInfo *) info; }
 
    Image(ImageInfo * in_info) : WidgetFacet(in_info) { ; }

  public:
    Image(const std::string & in_url, const std::string & in_id="")
      : WidgetFacet(in_id)
    {
      info = new ImageInfo(in_id);
      Info()->url = in_url;
    }
    Image(const Image & in) : WidgetFacet(in) { ; }
    Image(const Widget & in) : WidgetFacet(in) { emp_assert(info->IsImageInfo()); }
    virtual ~Image() { ; }

    using INFO_TYPE = ImageInfo;


  public:
    Image & URL(const std::string & in_url) { Info()->UpdateURL(in_url); return *this; }
    Image & Alt(const std::string & in_alt) { Info()->UpdateAlt(in_alt); return *this; }
  };

}
}

#endif
