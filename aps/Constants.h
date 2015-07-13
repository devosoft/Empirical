#ifndef EMP_CONSTANTS_H
#define EMP_CONSTANTS_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Slideshow constants
//

#include <string>


namespace emp {
namespace defaults {

	const double TITLE_HEIGHT = 5.0;
  const double SLIDE_WIDTH = 2560.0;
	const double SLIDE_HEIGHT = 1600.0;
	const double ASPECT_RATIO = SLIDE_WIDTH / SLIDE_HEIGHT; // 1.6
  const double NAV_WIDTH = 300;
  const double VIEW_PADDING = 20;
  const double DOC_MIN_WIDTH = 600;
  const double DOC_MIN_HEIGHT = 200;
	const std::string SLIDE_BACKGROUND = "black";
	const std::string FONT = "\"Lucida Sans Unicode\", \"Lucida Grande\", sans-serif";

}
}


#endif
