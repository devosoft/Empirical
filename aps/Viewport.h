#ifndef VIEWPORT_H
#define VIEWPORT_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Slideshow viewport
//


#include <functional>

#include "../UI/UI.h"
#include "Constants.h"

namespace emp {

  class Viewport {
  private:
		UI::ElementSlate * viewport;  // Deleted by managing object.

  public:
    Viewport(const std::string & name,
						 UI::ElementSlate & parent,
						 std::function<void()> prevFn,
						 std::function<void()> nextFn,
             const std::string & fullScreenFn)
		{
      viewport = new UI::ElementSlate(name + "-viewport");
			(*viewport).Size(100, 100, "%")
				         .CSS("position", "absolute", "top", "0", "left", "0", "z-index", "1000",
                      "display", "block", "min-width", "600px");
      (*viewport) << UI::Button(prevFn, "<b>Prev</b>", "prev")
							    << UI::Button(nextFn, "<b>Next</b>", "next")
							    << UI::Button(fullScreenFn, "<b>Full</b>", "full");

			const double bw = 65; // Button width;  units: px
			const double bh = 65; // Button height; units: px
      const double bo = 20; // Button offset from edge; units: px
      (*viewport).Button("next").SetPositionRB(bo, bo).Size(bw, bh).Opacity(1.0);
      (*viewport).Button("prev").SetPositionRB(bo+bw+2, bo).Size(bw, bh).Opacity(1.0);
      (*viewport).Button("full").SetPositionRB(bo+bw+bw+4, bo).Size(bw, bh).Opacity(1.0);

			parent.AddChild(viewport);
    }
    ~Viewport() { ; }

		//void Update() { viewport.Update(); }
	};

}


#endif
