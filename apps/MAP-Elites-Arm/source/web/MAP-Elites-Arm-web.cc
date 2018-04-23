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
      if (fitness == 0.0) {
        canvas.Circle(cur_x, cur_y, org_r, "#444444");
      } else if (fitness < 0.6) {
        canvas.Circle(cur_x, cur_y, org_r, "pink");
      } else if (fitness < 0.8) {
        canvas.Circle(cur_x, cur_y, org_r, "#EEEE33");  // Pale Yellow
      } else if (fitness < 0.95) {
        canvas.Circle(cur_x, cur_y, org_r, "#88FF88");  // Pale green
      } else {
        canvas.Circle(cur_x, cur_y, org_r, "green");
      }

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
  //canvas.MultiLine(start_point, draw_points, arm_color);
  for (emp::Point p : draw_points) {
    canvas.Line(start_point, p, arm_color, 3.0);
    start_point = p;
  }

  // for (const Org & org : pop) {
  //   if (org.coop) {
  //     canvas.Circle(org.x*world_size, org.y*world_size, 2, "blue", "#8888FF");
  //   } else {
  //     canvas.Circle(org.x*world_size, org.y*world_size, 2, "#FF8888", "red");
  //   }
  // }

  doc.Text("ud_text").Redraw();
}

int main()
{
  doc << "<h1>MAP-Elites: Arm Positioning Challenge</h1>";
  auto world_canvas = doc.AddCanvas(world_size, world_size, "world_canvas");
  DrawWorldCanvas();
}
