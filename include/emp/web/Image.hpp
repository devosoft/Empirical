/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file  Image.hpp
 *  @brief Easily load an image and place it in a document.
 */


#ifndef EMP_WEB_IMAGE_H
#define EMP_WEB_IMAGE_H

#include "Widget.hpp"

namespace emp {
namespace web {

  ///  The Image widget will load an image from a specified URL.  For example, you can use
  ///  emp::web::Image("http://kripken.github.io/emscripten-site/_static/Emscripten_logo_full.png")
  ///  to load the emscripten logo from its website.  These can be easily inserted into a web
  ///  document using << and all JS callbacks (after loading) will be handled automatically.
  ///
  ///  You can also set the Alt-text with the Alt() function.

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

      std::string GetTypeName() const override { return "ImageInfo"; }

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
    /// Create a new image, indicating the URL to load from.
    Image(const std::string & in_url, const std::string & in_id="")
      : WidgetFacet(in_id)
    {
      info = new ImageInfo(in_id);
      Info()->url = in_url;
    }

    /// Link to an existing Image widget.
    Image(const Image & in) : WidgetFacet(in) { ; }
    Image(const Widget & in) : WidgetFacet(in) { emp_assert(in.IsImage()); }
    virtual ~Image() { ; }

    using INFO_TYPE = ImageInfo;

    /// Update the URL for this image.
    Image & URL(const std::string & in_url) { Info()->UpdateURL(in_url); return *this; }

    /// Add Alt-text for this image.
    Image & Alt(const std::string & in_alt) { Info()->UpdateAlt(in_alt); return *this; }
  };

}
}

#endif
