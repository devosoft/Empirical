/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file Slideshow.hpp
 *
 */

#ifndef DEMOS_SLIDESHOW_HPP_INCLUDE
#define DEMOS_SLIDESHOW_HPP_INCLUDE

#include <string>

#include "emp/base/vector.hpp"
#include "emp/web/emfunctions.hpp"
#include "emp/web/KeypressManager.hpp"
#include "emp/web/web.hpp"

namespace emp {

  class Slideshow {
  private:
    std::string div_name;
    emp::vector<UI::Document *> slides;
    int cur_pos;

    // UI Tracking
    UI::KeypressManager key_manager;
    int window_width;
    int window_height;

    // Formatting details.
    int base_width;
    int base_height;
    std::string default_font;
    double title_height;       // Font height of title in vw's (percent of viewport width)
    double text_height;        // Font height of main text (such as bullets) in vw's.

    // Show details
    std::string show_title;

    // Helper functions
    void OnResize(int new_w, int new_h) {
      // emp::Alert("New size = ", new_w, ", ", new_h);
      window_width = new_w;
      window_height = new_h;
    }

  public:
    Slideshow(const std::string & in_title="", const std::string name = "emp_base")
      : div_name(name), cur_pos(0)
      , base_width(1000), base_height(625)
      , title_height(5.0), text_height(3.0)
      , show_title(in_title)
    {
      (void) base_width;
      (void) base_height;

      // Setup default captures.
      emp::OnResize( std::bind(&Slideshow::OnResize, this, UI::_1, UI::_2) );

      // Track page information
      window_width = GetWindowInnerWidth();
      window_height = GetWindowInnerHeight();

      // Setup default look.
      default_font = "\"Lucida Sans Unicode\", \"Lucida Grande\", sans-serif";

      NewSlide(); // Create the title slide.
      *this << UI::Text("title").FontSizeVW(title_height*1.3) << show_title;
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

    Slideshow & NewSlide(const std::string & slide_title = "", double in_height=-1.0) {
      cur_pos = (int) slides.size();
      auto * new_slide = new UI::Document(div_name);
      new_slide->SizeVW(100, 62.5).Background("black");
      new_slide->Font(default_font);
      if (slide_title != "") {
        if (in_height < 0.0) in_height = title_height;
        (*new_slide) << UI::Text("title").FontSizeVW(in_height).Center()
                     << slide_title;
        new_slide->Text("title").PreventAppend();  // Additional text in a new box!
      }
      slides.push_back(new_slide);

      (*new_slide) << UI::Button([this](){this->PrevSlide();}, "<b>Prev</b>", "prev")
                   << UI::Button([this](){this->NextSlide();}, "<b>Next</b>", "next")
                   << UI::Button(std::string("document.querySelector('body').webkitRequestFullScreen();"),
                                 "<b>Full</b>", "full");

      const double bw = 5.0;  // Button width (in vw's)
      const double bh = 5.0;  // Button height
      const double bo = 1.0;  // Button offset from edge.
      new_slide->Button("next").SetPositionRBVW(bo, bo).SizeVW(bw, bh).Opacity(1.0);
      new_slide->Button("prev").SetPositionRBVW(bo+bw, bo).SizeVW(bw, bh).Opacity(1.0);
      new_slide->Button("full").SetPositionRBVW(bo+bw+bw, bo).SizeVW(bw, bh).Opacity(1.0);

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

#endif // #ifndef DEMOS_SLIDESHOW_HPP_INCLUDE
