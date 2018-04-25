/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  MAP-Elites-Arm-web.cc
 *  @brief Controller for WEB version of MAP-Elites app.
 */

#include "tools/math.h"
#include "web/web.h"
#include "../ArmWorld.h"

namespace UI = emp::web;

UI::Document doc("emp_base");
ArmWorld world;
const double world_size = 600;
size_t target_id = 0;
ArmOrg target_arm;

void DrawWorldCanvas() {
  UI::Canvas canvas = doc.Canvas("world_canvas");
  canvas.Clear("gray");

  const size_t world_x = world.GetWidth();
  const size_t world_y = world.GetHeight();
  const double canvas_x = (double) canvas.GetWidth();
  const double canvas_y = (double) canvas.GetHeight();

  const double org_x = canvas_x / (double) world_x;
  const double org_y = canvas_y / (double) world_y;
  const double org_r = emp::Min(org_x, org_y) / 2.0;

  for (size_t y = 0; y < world_y; y++) {
    for (size_t x = 0; x < world_x; x++) {
      const size_t org_id = y * world_x + x;
      const size_t cur_x = org_x * (0.5 + (double) x);
      const size_t cur_y = org_y * (0.5 + (double) y);
      const double fitness = world.CalcFitnessID(org_id);

      std::string circle_color;
      if (fitness == 0.0) { circle_color = "#444444"; }          // Dark Gray
      else if (fitness < 0.6) { circle_color = "#FFC0CB"; }      // Pink
      else if (fitness < 0.8) { circle_color = "#FFD899"; }      // Pale Orange
      else if (fitness < 0.95) { circle_color = "#EEEE33"; }     // Pale Yellow
      else if (fitness < 0.98) { circle_color = "#88FF88"; }     // Pale green
      else if (fitness < 0.995) { circle_color = "#00CC00"; }    // Mid green
      else { circle_color = "green"; }                           // Full green
      canvas.Circle(cur_x, cur_y, org_r, circle_color, "black");

      if (!target_id && fitness > 0.0) {
        target_id = org_id;
        target_arm = world[org_id];
      }
    }
  }

  // Add a plus sign in the middle.
  const double mid_x = org_x * world_x / 2.0;
  const double mid_y = org_y * world_y / 2.0;
  const double plus_bar = org_r * world_x;
  canvas.Line(mid_x, mid_y-plus_bar, mid_x, mid_y+plus_bar, "#8888FF");
  canvas.Line(mid_x-plus_bar, mid_y, mid_x+plus_bar, mid_y, "#8888FF");

  // Setup the arm.
  const std::string arm_color = "white";
  const double total_length = world.CalcTotalLength();
  const double dilation = canvas_x / (total_length * 2.0);
  emp::Point start_point(mid_x, mid_y);
  emp::vector<emp::Point> draw_points = world.CalcPoints(target_arm, start_point, dilation);
  canvas.MultiLine(start_point, draw_points, arm_color, 3.0);

  // Add joints along arm.
  canvas.Circle(start_point, 5, "blue", "black");
  for (emp::Point p : draw_points) {
    canvas.Circle(p, 3, "blue", "black");
  }

}

void CanvasClick(int x, int y) {
  UI::Canvas canvas = doc.Canvas("world_canvas");
  const double canvas_x = (double) canvas.GetWidth();
  const double canvas_y = (double) canvas.GetHeight();
  double px = ((double) x) / canvas_x;
  double py = ((double) y) / canvas_y;

  const size_t world_x = world.GetWidth();
  const size_t world_y = world.GetHeight();
  size_t pos_x = (size_t) (world_x * px);
  size_t pos_y = (size_t) (world_y * py);

  size_t org_id = pos_y * world_x + pos_x;
  if (world.CalcFitnessID(org_id) > 0.0) {
    target_id = org_id;
    target_arm = world[org_id];
    DrawWorldCanvas();
  }
}

int main()
{
  doc << "<h1>MAP-Elites: Arm Positioning Challenge</h1>";

  // Add some Buttons
  doc << UI::Button( [](){ emp::RandomSelect(world, 1); DrawWorldCanvas(); }, "Do Birth", "birth_button");
  doc << UI::Button( [](){ emp::RandomSelect(world, 100); DrawWorldCanvas(); }, "Do Birth 100", "birth_100_button");
  doc << UI::Button( [](){ emp::RandomSelect(world, 10000); DrawWorldCanvas(); }, "Do Birth 10000", "birth_10000_button");
  doc << "<br>";

  // Add the Canvas
  auto world_canvas = doc.AddCanvas(world_size, world_size, "world_canvas");
  world_canvas.On("click", CanvasClick);
  DrawWorldCanvas();


  EM_ASM({ $('#(x)'); });  // <-- What is this needed for?  Clicking on the canvas doesn't work without it.

}
