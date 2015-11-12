// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_PANEL_H
#define EMP_PANEL_H

#include "Kinetic.h"

namespace emp {

  class Panel : public CustomShape {
  private:
  public:
    Panel(int _x, int _y, int _width, int _height)
      : CustomShape(_x, _y, _width, _height, this, &Panel::Draw)
    { ; }

    void Draw(Canvas & canvas) {
      // Setup the background black...
      canvas.SetFill("#AAAAAA");
      canvas.Rect(0, 0, GetWidth(), GetHeight(), true);
      canvas.SetStroke("#000000");
      canvas.Rect(0, 0, GetWidth(), GetHeight(), false);
    }

  };

};

#endif
