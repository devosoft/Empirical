#ifndef EMP_SLIDE_H
#define EMP_SLIDE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This ap maintains a slideshow based on the UI tools.
//

#include <string>
#include <vector>

#include "../emtools/emfunctions.h"
#include "../UI/UI.h"
#include "../UI/keypress.h"
#include "Constants.h"


namespace emp {

  class Slide {
  private:
		std::string name;
		std::string name_mini;
		UI::ElementSlate slide;
		UI::ElementSlate slide_mini;

  public:
    Slide(const std::string &_name,
					const double width,
					const double height,
					const std::string & font,
					const std::string & title="", double title_height=-1.0,
					const std::string & background_color=defaults::SLIDE_BACKGROUND)
			: name(_name)
			, name_mini(name + "_mini")
			, slide(name)
			, slide_mini(name_mini)
		{
      slide.SizeVW(width, height).Background(background_color).Font(font);
    }
    ~Slideshow() { ; }

		/*
    int GetSlideNum() const { return cur_pos; }

    UI::ElementSlate & operator[](int slide_id) {
      emp_assert(slide_id >= 0 && slide_id < (int) slides.size());
      cur_pos = slide_id;
      return *(slides[slide_id]);
    }

    Slideshow & operator<<(UI::Text & input) {
      if (!input.HasCSS("font-size")) {
        input.FontSizeVW(text_height);
      }
      (*slides[cur_pos]) << input;
      return *this;
    }

    Slideshow & operator<<(UI::Text && input) {
      if (!input.HasCSS("font-size")) {
        input.FontSizeVW(text_height);
      }
      (*slides[cur_pos]) << std::forward<UI::Text>(input);
      return *this;
    }

    Slideshow & operator<<(const std::string & input) {
      (*slides[cur_pos]) << input;
      return *this;
    }

    template <typename T>
    Slideshow & operator<<(T && input) {
      (*slides[cur_pos]) << input;
      return *this;
    }

    Slideshow & NewSlide(const std::string & slide_title="", double in_height=-1.0) {
      cur_pos = (int) slides.size();
      auto * new_slide = new UI::ElementSlate(name);
      new_slide->SizeVW(100, 62.5).Background("black");
      new_slide->Font(default_font);
      if (slide_title != "") {
        if (in_height < 0.0) in_height = title_height;
        (*new_slide) << UI::Text("title").FontSizeVW(in_height).Center()
                     << slide_title;
        new_slide->Text("title").PreventAppend();  // Additional text in a new box!
      }
      slides.push_back(new_slide);

      return *this;
    }

    UI::ElementSlate & GetSlide() { return *(slides[cur_pos]); }

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

      if (name == "") {
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
		*/
  };

}

#endif
