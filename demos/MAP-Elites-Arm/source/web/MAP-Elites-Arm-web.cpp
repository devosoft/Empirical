/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file MAP-Elites-Arm-web.cpp
 *  @brief Controller for WEB version of MAP-Elites app.
 */

#include "emp/math/math.hpp"
#include "emp/web/web.hpp"

#include "../ArmWorld.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");
UI::Div div_pop("div_pop");
UI::Div div_stats("div_stats");
UI::Div div_controls("div_controls");
UI::Div div_vis("div_vis");

double layout_x1 = 10;
double layout_y1 = 70;
double layout_x2 = 650;
double layout_y2 = 300;
double layout_y3 = 700;

ArmWorld world;
const double world_size = 600;
size_t target_id = 0;
ArmOrg target_arm;

void LayoutDivs() {
  div_pop.SetPosition(layout_x1, layout_y1);
  div_controls.SetPosition(layout_x2, layout_y1);
  div_stats.SetPosition(layout_x2, layout_y2);
  div_vis.SetPosition(layout_x1, layout_y3);
}

void DrawWorldCanvas_Grid() {
  UI::Canvas canvas = doc.Canvas("world_canvas");
  canvas.Clear("gray");

  const size_t world_x = world.GetWidth();
  const size_t world_y = world.GetHeight();
  const double canvas_x = (double) canvas.GetWidth();
  const double canvas_y = (double) canvas.GetHeight();

  const double org_x = canvas_x / (double) world_x;
  const double org_y = canvas_y / (double) world_y;
  const double org_r = emp::Min(org_x, org_y) / 2.0;

  // Draw all of the organisms
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

void DrawWorldCanvas_Scatter() {
  LayoutDivs();

  UI::Canvas canvas = doc.Canvas("world_canvas");

  const size_t world_size = world.GetSize();
  const double total_length = world.CalcTotalLength();

  const double canvas_x = (double) canvas.GetWidth();
  const double canvas_y = (double) canvas.GetHeight();

  const double org_r = emp::Min(canvas_x, canvas_y) / 120.0;
  const emp::Point middle(canvas_x / 2.0, canvas_y / 2.0);
  const double arm_scale = (canvas_x / total_length) / 2.0;

  // Draw the background grid.
  UI::DrawGridBG(canvas, 40, 40, "#202020", "#606060");

  // Draw all of the organisms
  for (size_t org_id = 0; org_id < world_size; org_id++) {
    if (world.IsOccupied(org_id) == false) continue;

    const double fitness = world.CalcFitnessID(org_id);

    std::string circle_color;
    if (fitness == 0.0) { circle_color = "#444444"; }          // Dark Gray
    else if (fitness < 0.6) { circle_color = "#FFC0CB"; }      // Pink
    else if (fitness < 0.8) { circle_color = "#FFD899"; }      // Pale Orange
    else if (fitness < 0.95) { circle_color = "#EEEE33"; }     // Pale Yellow
    else if (fitness < 0.98) { circle_color = "#88FF88"; }     // Pale green
    else if (fitness < 0.995) { circle_color = "#00CC00"; }    // Mid green
    else { circle_color = "green"; }                           // Full green

    emp::Point org_pos = world.CalcEndPoint(org_id);
    org_pos.Scale(arm_scale) += middle;

    canvas.Circle(org_pos, org_r, circle_color, "black");

    if (!target_id && fitness > 0.0) {
      target_id = org_id;
      target_arm = world[org_id];
    }
  }

  // Add a plus sign in the middle.
  const double plus_bar = org_r * 3;
  canvas.Line(middle.GetX(), middle.GetY()-plus_bar, middle.GetX(), middle.GetY()+plus_bar, "#8888FF");
  canvas.Line(middle.GetX()-plus_bar, middle.GetY(), middle.GetX()+plus_bar, middle.GetY(), "#8888FF");

  // Setup the arm.
  const std::string arm_color = "white";
  const double dilation = canvas_x / (total_length * 2.0);
  emp::vector<emp::Point> draw_points = world.CalcPoints(target_arm, middle, dilation);
  canvas.MultiLine(middle, draw_points, arm_color, 3.0);

  // Add joints along arm.
  canvas.Circle(middle, 5, "blue", "black");
  for (emp::Point p : draw_points) {
    canvas.Circle(p, 3, "blue", "black");
  }

}

void DrawWorldCanvas() { DrawWorldCanvas_Scatter(); }

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

void CanvasClick2(int x, int y) {
  x -= layout_x1;
  y -= layout_y1;
  const size_t world_size = world.GetSize();
  const double total_length = world.CalcTotalLength();

  UI::Canvas canvas = doc.Canvas("world_canvas");
  const double canvas_x = (double) canvas.GetWidth();
  const double canvas_y = (double) canvas.GetHeight();

  const emp::Point middle(canvas_x / 2.0, canvas_y / 2.0);
  const double inv_arm_scale = 2.0 / (canvas_x / total_length);

  size_t best_id = 0;
  double best_dist = 1000000.0;

  emp::Point target(x,y);
  target -= middle;
  target.Scale(inv_arm_scale);

  // Determine which organism is closest to the click.
  for (size_t org_id = 0; org_id < world_size; org_id++) {
    if (world.IsOccupied(org_id) == false) continue;

    emp::Point org_pos = world.CalcEndPoint(org_id);
    double dist = org_pos.SquareDistance(target);
    if (dist < best_dist) {
      best_dist = dist;
      best_id = org_id;
    }
  }

  // Update the target and redraw.
  target_id = best_id;
  target_arm = world[best_id];
  DrawWorldCanvas();

}

int main()
{
  doc << "<h1>MAP-Elites: Arm Positioning Challenge</h1>";

  UI::Canvas world_canvas(world_size, world_size, "world_canvas");
  div_pop << world_canvas;

  // Setup Selector
  auto mode_select = UI::Selector("mode_select");
  mode_select.SetOption("Well Mixed", [](){ world.ResetMixed(); DrawWorldCanvas(); });
  mode_select.SetOption("MAP-Elites", [](){ world.ResetMAP(); DrawWorldCanvas(); });
  mode_select.SetOption("DiverseElites", [](){ world.ResetDiverse(); DrawWorldCanvas(); });
  mode_select.SelectID(1);

  // Add some Buttons
  div_controls << UI::Button( [](){ emp::RandomSelect(world, 1); DrawWorldCanvas(); }, "Do Birth", "birth_button");
  div_controls << UI::Button( [](){ emp::RandomSelect(world, 100); DrawWorldCanvas(); }, "Do Birth 100", "birth_100_button");
  div_controls << UI::Button( [](){ emp::RandomSelect(world, 10000); DrawWorldCanvas(); }, "Do Birth 10000", "birth_10000_button");
  div_controls << UI::Button( [](){ for (size_t i=0; i<100; i++) world.DoDeath(); DrawWorldCanvas(); }, "Clean", "clean_button");
  div_controls << "<br>";
  div_controls << "Mode: " << mode_select;
  div_controls << "<br>";

  // Add some stats
  div_stats << "<h3>Stats</h3>";

  // Setup canvas interactions
  world_canvas.On("click", CanvasClick2);

  doc << div_pop;
  doc << div_stats;
  doc << div_controls;
  doc << div_vis;

  DrawWorldCanvas();

  emp::web::OnDocumentReady( [](){ LayoutDivs(); } );

  EM_ASM({ $('#(x)'); });  // <-- What is this needed for?  Clicking on the canvas doesn't work without it.

}
