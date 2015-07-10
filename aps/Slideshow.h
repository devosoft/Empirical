#ifndef EMP_SLIDESHOW_H
#define EMP_SLIDESHOW_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This ap maintains a slideshow based on the UI tools.
//

#include <string>
#include <vector>

#include "../emtools/emfunctions.h"
#include "../UI/UI.h"
#include "../UI/keypress.h"

namespace emp {

  class Slideshow {
  private:
    std::string div_name;
    std::vector<UI::Document *> slides;
    int cur_pos;

    // UI Tracking
    UI::KeypressManager key_manager;
    int window_width;
    int window_height;

    // Formatting details.
    std::string default_font;
    
    // Helper functions
    void OnResize(int new_w, int new_h) {
      // emp::Alert("New size = ", new_w, ", ", new_h);
      window_width = new_w;
      window_height = new_h;
    }
  public:
    Slideshow(const std::string name = "emp_base") : div_name(name), cur_pos(0) {
      // Setup default captures.
      emp::OnResize( std::bind(&Slideshow::OnResize, this, UI::_1, UI::_2) );

      // Track page information
      window_width = GetWindowInnerWidth();
      window_height = GetWindowInnerHeight();

      // Setup default look.
      default_font = "\"Lucida Sans Unicode\", \"Lucida Grande\", sans-serif";

      NewSlide(); // Create the title slide.
    }
    ~Slideshow() {
      for (auto * slide : slides) delete slide;
    }

    int GetSlideNum() const { return cur_pos; }

    UI::Document & operator[](int slide_id) {
      emp_assert(slide_id >= 0 && slide_id < (int) slides.size());
      cur_pos = slide_id;
      return *(slides[slide_id]);
    }

    template <typename T>
    Slideshow & operator<<(T && input) {
      (*slides[cur_pos]) << input;
      return *this;
    }

    Slideshow & NewSlide(const std::string & slide_title = "") {
      cur_pos = (int) slides.size();
      auto * new_slide = new UI::Document(div_name);
      new_slide->Font(default_font);
      if (slide_title != "") {
        (*new_slide) << UI::Text("title").FontSize(50).Center() << slide_title;
        new_slide->Text("title").PreventAppend();  // Additional text in a new box!
      }
      slides.push_back(new_slide);

      (*new_slide) << UI::Button([this](){this->PrevSlide();}, "<b>Prev</b>", "prev")
                   << UI::Button([this](){this->NextSlide();}, "<b>Next</b>", "next");

      const int bw = 50;
      const int bh = 50;
      const int boffset = 10;
      new_slide->Button("next").SetPositionRB(boffset, boffset).Size(bw, bh).Opacity(1.0);
      new_slide->Button("prev").SetPositionRB(boffset+bw, boffset).Size(bw, bh).Opacity(1.0);
        
      return *this;
    }
    
    UI::Document & GetSlide() { return *(slides[cur_pos]); }

    void Update() { slides[cur_pos]->Update(); }

    // Presentation Navigation

    void Start(int first_slide=0) { 
      cur_pos = first_slide;
      Update();
    }
    void NextSlide() { if (++cur_pos >= slides.size()) --cur_pos; Update(); }
    void PrevSlide() { if (--cur_pos < 0) cur_pos = 0; Update(); }


    Slideshow & ActivateKeypress() {
      // key_manager.AddKeydownCallback('p', [this](){this->NextSlide();});
      key_manager.AddKeydownCallback("N ", std::bind(&Slideshow::NextSlide, this));
      key_manager.AddKeydownCallback("P\b", std::bind(&Slideshow::PrevSlide, this));

      key_manager.AddKeydownCallback(39, std::bind(&Slideshow::NextSlide, this));
      key_manager.AddKeydownCallback(37, std::bind(&Slideshow::PrevSlide, this));
      return *this;
    }
    
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
      if (cur_pos < 0 || cur_pos >= (int) slides.size()) {
        ss << "Error: Show has " << slides.size() << " slides.  Current slide = "
           << cur_pos << std::endl;
        ok = false;
      }

      // Check each of the individual slides.
      for (auto * slide : slides) if (!slide->OK(ss, verbose, prefix+"  ")) ok = false;

      return ok;
    }
  };


}

#endif
