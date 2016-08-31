// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#include <emscripten.h>
#include <iostream>

#include <list>
#include <string>
#include <vector>

#include "../../tools/assert.h"
#include "../../tools/callbacks.h"
#include "../../tools/const.h"
#include "../../geometry/Circle2D.h"
#include "../../geometry/Point2D.h"

#include "../../web/emfunctions.h"
#include "../../web/Color.h"
#include "../../web/Font.h"
#include "../../web/RawImage.h"

#include "../../kinetic/Kinetic.h"

extern "C" {
  extern int EMP_Tween_Build(int target_id, double seconds);
  extern int EMP_Tween_Clone(int orig_settings_id);

  extern int EMP_Rect_Build(int _x, int _y, int _w, int _h, const char * _fill, const char * _stroke, int _stroke_width, int _draggable);
  extern int EMP_RegularPolygon_Build(int _x, int _y, int _sides, int _radius,
                                      const char * _fill, const char * _stroke, int _stroke_width, int _draggable);

  extern int EMP_Animation_Build(int callback_ptr, int layer_id);
  extern int EMP_Animation_Build_NoFrame(int callback_ptr, int layer_id);

  extern int EMP_Custom_Shape_Build(int x, int y, int w, int h, int draw_callback);

  extern void my_js();
}

int main()
{
  my_js();
  emp::Alert("Hi", 2, '!');
}
