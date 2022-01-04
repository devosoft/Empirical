/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file mini_evoke.cpp
 */

#include <algorithm>

#include "../../tools/mem_track.h"

#include "../../geometry/Physics2D.h"
#include "../../tools/Random.h"
#include "../../UI/Animate.h"
#include "../../UI/canvas_utils.h"
#include "../../UI/UI.h"

namespace UI = emp::UI;

double cw = 300.0;
double ch = 300.0;

using dBRAIN = int;
using dBODY = emp::CircleBody2D<dBRAIN>;
using dPHYSICS = emp::Physics2D<dBODY, dBRAIN>;

const emp::vector<std::string> color_map =
  std::vector<std::string>( { "red", "blue",
        "#ffa280", "#992626", "#ff8800", "#ffcc00", "#5d8c00", "#269954", "#00ffee", "#0088ff",
        "#002ca6", "#a280ff", "#8c0070", "#331a24", "#ffbfbf", "#592400", "#ffc480", "#333226",
        "#556633", "#004d33", "#269199", "#4d7599", "#bfc8ff", "#754d99", "#59003c", "#994d61",
        "#400900", "#a64200", "#593c00", "#eeff00", "#00ff00", "#4d665e", "#003d4d", "#262d33",
        "#2200ff", "#cc00ff", "#ff80c4", "#ff2200", "#998273", "#8c7000", "#f2ffbf", "#7fffa1",
        "#bffff2", "#00ccff", "#163159", "#180059", "#4b394d", "#ff0066"
        } );


struct Evoke {
  dPHYSICS physics;
  emp::Random random;
  UI::Animate anim;

  Evoke(int w, int h) : physics(w,h) {
    // Initialize physics with a single circle body.
    auto new_circle = emp::Circle<>(cw/2.0, ch/2.0, 7.0);
    dBODY * new_body = new dBODY( new_circle );
    new_body->SetColorID( 0 );
    physics.AddBody( new_body );
  }
};

UI::Document doc("emp_base");
Evoke evoke_change(cw,ch);
Evoke evoke_novel(cw,ch);
Evoke evoke_eco(cw,ch);


void EvokeAnim_Basic(int colors_used, const std::string & canvas_name, Evoke & evoke,
                     bool use_eco=false)
{
  emp_assert(colors_used > 0);
  evoke.physics.Update();

  // Test which organisms should replicate.
  auto & body_set = evoke.physics.GetBodySet();
  const int num_bodies = (int) body_set.size();
  double repro_prob = 0.008;
  if (num_bodies < 10) repro_prob = 0.02;

  // Note the time any organisms are born.
  const double birth_time = evoke.anim.GetRunTime();
  const int cur_sec = birth_time / 1000;
  const int steps = ((cur_sec > 5) ? (cur_sec/5 - 1) : 0);
  const int phase = steps % colors_used;
  const int active_colors = std::min(steps+1, (int) color_map.size());

  // If we're in an eco mode, determine the repro prob of each color (base on abundance)
  emp::vector<int> color_count(active_colors, 0);
  emp::vector<int> new_ids;
  emp::vector<double> color_repro(active_colors);
  if (use_eco) {
    // Count each color.
    for (auto * body : body_set) {
      color_count[body->GetColorID()]++;
    }

    // How big should each color be?
    double target_size = ((double) num_bodies) / (double) active_colors;
    double target_min = target_size * 0.8;
    double target_max = target_size * 1.2;

    // Categorize each color.
    for (int i = 0; i < active_colors; i++) {
      const int cur_count = color_count[i];
      if (cur_count == 0) new_ids.push_back(i);
      if (cur_count < target_min) color_repro[i] = 0.02;
      else if (cur_count < target_max) color_repro[i] = 0.008;
      else color_repro[i] = 0.003;
    }
  }

  // Outside of eco mode, all colors have the same repro prob.
  else {
    for (int i = 0; i < active_colors; i++) {
      color_repro[i] = repro_prob;
    }
  }

  for (auto * body : body_set) {
    // Bodies that are reproducing cannot produce a second offspring until they are done.
    // Bodies under pressure do not produce offspring.
    if (body->IsReproducing() || body->GetPressure() > 1.0) continue;
    const double cur_repro_prob = color_repro[body->GetColorID()];
    if (evoke.random.P(cur_repro_prob) || num_bodies == 1) {
      emp::Angle repro_angle(evoke.random.GetDouble(2.0 * emp::PI));
      auto * new_body = body->BuildOffspring( repro_angle.GetPoint(0.1) );

      new_body->SetBirthTime(birth_time);
      if (use_eco) {
        if (new_ids.size() && evoke.random.P(0.05)) {
          new_body->SetColorID(new_ids.back());  // @CAO or random?
          new_ids.resize(0);
        }
        else {
          new_body->SetColorID( body->GetColorID() );
        }
      } else {
        new_body->SetColorID(phase);
      }
      evoke.physics.AddBody( new_body );
    }
  }

  // If we have too many organisms, kill the oldest!
  const int num_kills = num_bodies/100 - 1 - use_eco;
  for (int i = 0; i < num_kills; i++) evoke.physics.KillOldest();

  auto & canvas = doc.Canvas(canvas_name);
  emp::UI::Draw(canvas, evoke.physics.GetSurface(), color_map);

  // canvas.Refresh();
}

void EvokeAnim_Change() {
  EvokeAnim_Basic (2, "canvas_change", evoke_change);
}
void EvokeAnim_Novelty() {
  EvokeAnim_Basic (color_map.size(), "canvas_novel", evoke_novel);
}
void EvokeAnim_Eco() {
  EvokeAnim_Basic (color_map.size(), "canvas_eco", evoke_eco, true);
}


int main() {
  UI::Initialize();

  evoke_change.anim.SetCallback(EvokeAnim_Change);
  evoke_novel.anim.SetCallback(EvokeAnim_Novelty);
  evoke_eco.anim.SetCallback(EvokeAnim_Eco);

  auto & main_table = doc.AddTable(6, 2);

  main_table.GetCell(0,0).SetHeader();
  main_table << "High Change Potential";
  main_table.GetCell(1,0);
  main_table << UI::Canvas(cw, ch, "canvas_change");
  main_table.GetCell(2,0);
  main_table << UI::Button([](){
      evoke_change.anim.ToggleActive();                       // Toggle state of animation.
      auto & but = doc.Button("button_change");               // Lookup this button...
      if (evoke_change.anim.GetActive()) but.Label("Pause");  // Setup proper label on button.
      else but.Label("Play");
      but.Update();                                           // Redraw the button.
    }, "Play", "button_change").Size(50,30);
  emp::UI::Draw(doc.Canvas("canvas_change"), evoke_change.physics.GetSurface(), color_map);


  main_table.GetCell(0,1).SetHeader();
  main_table << "High Novelty Potential";
  main_table.GetCell(1,1);
  main_table << UI::Canvas(cw, ch, "canvas_novel");
  main_table.GetCell(2,1);
  main_table << UI::Button([](){
      evoke_novel.anim.ToggleActive();                       // Toggle state of animation.
      auto & but = doc.Button("button_novel");               // Lookup this button...
      if (evoke_novel.anim.GetActive()) but.Label("Pause");  // Setup proper label on button.
      else but.Label("Play");
      but.Update();                                           // Redraw the button.
    }, "Play", "button_novel").Size(50,30);
  emp::UI::Draw(doc.Canvas("canvas_novel"), evoke_novel.physics.GetSurface(), color_map);


  main_table.GetCell(3,0).SetHeader();
  main_table << "High Ecological Potential";
  main_table.GetCell(4,0);
  main_table << UI::Canvas(cw, ch, "canvas_eco");
  main_table.GetCell(5,0);
  main_table << UI::Button([](){
      evoke_eco.anim.ToggleActive();                       // Toggle state of animation.
      auto & but = doc.Button("button_eco");               // Lookup this button...
      if (evoke_eco.anim.GetActive()) but.Label("Pause");  // Setup proper label on button.
      else but.Label("Play");
      but.Update();                                           // Redraw the button.
    }, "Play", "button_eco").Size(50,30);
  emp::UI::Draw(doc.Canvas("canvas_eco"), evoke_eco.physics.GetSurface(), color_map);





  main_table.GetCell(3,1).SetHeader();
  main_table << "High Complexity Potential";



  // auto & canvas_novel = doc.AddCanvas(cw, ch, "canvas_novel");
  // doc << "<br>";
  // doc.AddAnimToggle(evoke_novel.anim);
  // emp::UI::Draw(canvas_novel, evoke_novel.physics.GetSurface(), color_map);

  // Draw the surface on a new canvas!

  doc.Update();

}
