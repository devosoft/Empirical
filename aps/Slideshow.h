#ifndef EMP_SLIDESHOW_H
#define EMP_SLIDESHOW_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This ap maintains a slideshow based on the UI tools.
//

#include <sstream>
#include <string>
#include <vector>

#include "../emtools/emfunctions.h"
#include "../tools/string_utils.h"
#include "../UI/UI.h"
#include "../UI/keypress.h"
#include "Constants.h"
#include "Viewport.h"

namespace emp {

  class Slideshow {
  private:
    std::string name;
		UI::Document doc;
		Viewport viewport;
    UI::ElementSlate * nav_container;
    UI::ElementSlate * nav;
    UI::ElementSlate * viewer_container;
    UI::ElementSlate * viewer;
	  std::string view_mode;

    std::vector<UI::ElementSlate *> slides;     // Deleted by parent
    std::vector<UI::ElementSlate *> slides_nav; // Deleted by parent
    int cur_pos;

    // UI Tracking
    UI::KeypressManager key_manager;
    int window_width;
    int window_height;
		double aspect_ratio;
    double nav_width;
    std::string slide_transform;

    // Formatting details.
    std::string default_font;
    double title_height;       // Font height of title in vw's (percent of viewport width)
    double text_height;        // Font height of main text (such as bullets) in vw's.

    void ChangeAspectRatio(const double new_aspect_ratio) {
      aspect_ratio = new_aspect_ratio;
      OnResize(window_width, window_height);
    }

    void Transform(int new_w, int new_h) {
      window_width = new_w;
      window_height = new_h;
      
      int doc_width = window_width < defaults::DOC_MIN_WIDTH ? defaults::DOC_MIN_WIDTH :
                                                               window_width;
      int doc_height = window_height < defaults::DOC_MIN_HEIGHT ? defaults::DOC_MIN_HEIGHT :
                                                                  window_height;
      int view_width = doc_width - nav_width;
      int view_height = doc_height;

      // Prepare slide transformation
      // Estimate height and width for respective dimension based on aspect ratio.  If the
      // estimated dimension is less than or equal to the other view dimension than use the
      // sampled dimension as the baseline dimension for the view; i.e. 100 x 100 square and a
      // 1.6 aspect ratio:
      // width -->  100 / 1.6 -> 62.5 --- 62.5 <= 100 (height) *use width
      // height --> 100 * 1.6 -> 160  --- 160  <= 100 (width)  *not true, do not use height
      int slide_width  = view_width  - 2*defaults::VIEW_PADDING;
      int slide_height = view_height - 2*defaults::VIEW_PADDING;
      double est_h = slide_width  / aspect_ratio;
      double est_w = slide_height * aspect_ratio;
      if (est_h <= slide_height) { // use width
        est_w = slide_width;
      } else {
        est_h = slide_height;
      }
      double scale_x = est_w / defaults::SLIDE_WIDTH;
      double scale_y = est_h / defaults::SLIDE_HEIGHT;
      double left = (defaults::SLIDE_WIDTH  - est_w) / -2.0 + (slide_width  - est_w) / 2.0;
      double top  = (defaults::SLIDE_HEIGHT - est_h) / -2.0 + (slide_height - est_h) / 2.0;
      slide_transform = emp::to_string("translate(", left, "px,", top, "px) scale(", scale_x, ",",
                                       scale_y, ")");

			doc.Size(doc_width, doc_height, "px");
			(*nav_container).Size(nav_width, doc_height, "px");
			(*nav).Size(nav_width, doc_height, "px");
			(*viewer_container).Size(view_width, doc_height, "px")
                         .CSS("left", emp::to_string(nav_width, "px");
			(*viewer).Size(view_width, view_height, "px");
      for (auto * slide : slides) {
        (*slide).CSS("transform", slide_transform);
      }
    }

    // Helper functions
    void OnResize(int new_w, int new_h) {
      // emp::Alert("New size = ", new_w, ", ", new_h);
      Transform(new_w, new_h);
      doc.Update();
    }

  public:
    Slideshow(const std::string _name="emp_base", const double default_title_height=-1.0)
      : name(_name)
			, doc(name)
			, viewport(name,
    					   doc,
                 [this](){ this->PrevSlide(); },
                 [this](){ this->NextSlide(); },
               	 emp::to_string("document.querySelector('#", name, "-viewer-content')",
                                ".webkitRequestFullScreen();"))
			, view_mode("edit")
			, cur_pos(0)
			, aspect_ratio(defaults::ASPECT_RATIO)
      , nav_width(defaults::NAV_WIDTH)
			, default_font(defaults::FONT)
      , title_height(default_title_height < 0 ? 5.0 : default_title_height)
			, text_height(3.0)
    {
      (void) title_height;

      // Setup default captures.
      emp::OnResize( std::bind(&Slideshow::OnResize, this, UI::_1, UI::_2) );

			// Initialize doc
			doc.CSS("position", "fixed", "top", "0", "left", "0", "overflow", "hidden");

			// Left hand nav
      nav_container = new UI::ElementSlate(name + "-nav-container");
			(*nav_container).CSS("display", "inline-block", "overflow", "hidden");
      nav = new UI::ElementSlate(name + "-nav-content");
			(*nav).CSS("overflow-y", "auto", "position", "relative");
			(*nav).CSS("background-color", "blue");

			// Viewer
      viewer_container = new UI::ElementSlate(name + "-viewer-container");
      (*viewer_container).CSS("display", "inline-block", "overflow", "hidden",
                              "position", "absolute"));
      viewer = new UI::ElementSlate(name + "-viewer-content");
      (*viewer).CSS("position", "relative");
			(*viewer).CSS("background-color", "black");

			doc.AddChild(nav_container);
			doc.AddChild(viewer_container);
			nav_container->AddChild(nav);
			viewer_container->AddChild(viewer);

      // Initial transformation; calls Update.
      OnResize(GetWindowInnerWidth(), GetWindowInnerHeight());
    }
    ~Slideshow() { ; }

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

    UI::ElementSlate & NewSlide() {
      cur_pos = (int) slides.size();
      auto * new_slide = new UI::ElementSlate(emp::to_string(name, "-slide-", cur_pos));
      (*new_slide).Size(defaults::SLIDE_WIDTH, defaults::SLIDE_HEIGHT, "px")
                  .Background("white")
                  .Font(default_font)
                  .CSS("position", "relative", "top", "20px", "left", "20px",
                       "transform", slide_transform);
      slides.push_back(new_slide);
      (*viewer).AddChild(new_slide);

      return *new_slide;
    }
    

    UI::ElementSlate & GetSlide() { return *(slides[cur_pos]); }

    void Update() {
			doc.Update();
			//slides[cur_pos]->Update();
		}

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
  };


}

#endif
