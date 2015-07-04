#ifndef EMP_SLIDESHOW_H
#define EMP_SLIDESHOW_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This ap maintains a slideshow based on the UI tools.
//

#include <string>
#include <vector>

#include "../UI/UI.h"

namespace emp {

  class Slideshow {
  private:
    std::string div_name;
    std::vector<UI::Document *> slides;
    int cur_slide;
    
  public:
    Slideshow(const std::string name = "emp_base") : div_name(name), cur_slide(0) {
      slides.push_back(new UI::Document(div_name));
    }
    ~Slideshow() {
      for (auto * slide : slides) delete slide;
    }

    int GetSlideNum() const { return cur_slide; }

    UI::Document & operator[](int id) {
      emp_assert(id >= 0 && id < (int) slides.size());
      cur_slide = id;
      return *(slides[id]);
    }

    template <typename T>
    Slideshow & operator<<(T && input) {
      (*slides[cur_slide]) << input;
      return *this;
    }

    void NewSlide() {
      cur_slide = (int) slides.size();
      slides.push_back(new UI::Document(div_name));
    }

    void NextSlide() { ++cur_slide; if (cur_slide == slides.size()) --cur_slide; }
    void PrevSlide() { --cur_slide; if (cur_slide < 0) cur_slide = 0; }
    void Update() { slides[cur_slide]->Update(); }

    virtual bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
      bool ok = true;

      if (verbose) {
        ss << prefix << "Scanning: emp::SlideShow" << std::endl;
      }

      if (div_name == "") {
        ss << "Warning: Must have a div name in SlideShow!" << std::endl;
        ok = false;
      }
      
      // Make sure the current slide position is valid.
      if (cur_slide < 0 || cur_slide >= (int) slides.size()) {
        ss << "Error: Show has " << slides.size() << " slides.  Current slide = "
           << cur_slide << std::endl;
        ok = false;
      }

      // Check each of the individual slides.
      for (auto * slide : slides) if (!slide->OK(ss, verbose, prefix+"  ")) ok = false;

      return ok;
    }
  };


}

#endif
