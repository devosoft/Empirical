//  This file is part of Project Name
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "web/web.h"
#include "../SimplePDWorld.h"

namespace UI = emp::web;

const double world_size = 600;

UI::Document doc("emp_base");
SimplePDWorld world;

int cur_x = -1;
int cur_y = -1;

void DrawCanvas() {
  UI::Canvas canvas = doc.Canvas("canvas");
  canvas.Clear("black");

  const emp::vector<Org> & pop = world.GetPop();

  if (cur_x >= 0) {
    canvas.Circle(cur_x, cur_y, world_size*world.GetR(), "pink");
  }

  for (const Org & org : pop) {
    if (org.coop) {
      canvas.Circle(org.x*world_size, org.y*world_size, 2, "blue", "white");
    } else {
      canvas.Circle(org.x*world_size, org.y*world_size, 2, "red", "white");
    }
  }
}

void CanvasClick(int x, int y) {
  cur_x = x;
  cur_y = y;
  DrawCanvas();
}

int main()
{
  doc << "<h2>Spatial Prisoner's Dilema</h2>";
  auto canvas = doc.AddCanvas(world_size, world_size, "canvas");
  // canvas.On("click", CanvasClick);
  DrawCanvas();

  auto & anim = doc.AddAnimation("anim_world", [](){ world.Run(1); DrawCanvas(); } );

  doc << "<br>";
  doc.AddButton([&anim](){
    anim.ToggleActive();
    auto but = doc.Button("start_but");
    if (anim.GetActive()) but.Label("Pause");
    else but.Label("Start");
  }, "Play", "start_but");
  doc.AddButton([](){ world.Run(1); DrawCanvas(); }, "Step", "step_but");
  doc.AddButton([](){ world.Run(); DrawCanvas(); }, "Run!", "run_but");
  doc.AddButton([](){ world.Reset(); DrawCanvas(); }, "Randomize", "rand_but");

  doc << "<br>Radius (<i>r</i>) = ";
  doc.AddTextArea([](const std::string & str){
    double r = emp::from_string<double>(str);
    world.SetR(r);
  }, "r_set").SetText(emp::to_string(world.GetR()));

  doc << "<br>cost/benefit ratio (<i>u</i>) = ";
  doc.AddTextArea([](const std::string & str){
    double u = emp::from_string<double>(str);
    world.SetU(u);
  }, "u_set").SetText(emp::to_string(world.GetU()));


  doc << "<br>Population Size (<i>N</i>) = ";
  doc.AddTextArea([](const std::string & str){
    size_t N = emp::from_string<size_t>(str);
    world.SetN(N);
  }, "N_set").SetText(emp::to_string(world.GetN()));


  doc << "<br>Num Epochs on Run (<i>E</i>) = ";
  doc.AddTextArea([](const std::string & str){
    size_t E = emp::from_string<size_t>(str);
    world.SetE(E);
  }, "E_set").SetText(emp::to_string(world.GetE()));

  doc << "<br>"
      << "NOTE: You must hit 'Randomize' after changing any parameters for them to take effect."
      << "<hr>"
      << "<h3>Full Runs</h3>"
      << "You can perform many runs at once with the same configuration."
      << "<br>"
      << "How many runs? ";

  auto run_input = doc.AddTextArea("run_count");
  run_input.SetText("10");

  doc.AddButton([run_input](){
    size_t num_runs = emp::from_string<size_t>(run_input.GetText());
    emp::Alert(num_runs);
    // world.Run();
  }, "GO", "go_but");

}
