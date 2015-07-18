#include "../../tools/mem_track.h"

#include "../../geometry/Physics2D.h"
#include "../../tools/Random.h"
#include "../../UI/UI.h"
#include "../../UI/canvas_utils.h"
#include "../../UI/Animate.h"

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


void EvokeAnim_Basic(int colors_used, const std::string & canvas_name, Evoke & evoke)
{
  emp_assert(colors_used > 0);
  evoke.physics.Update();

  // Test which organisms should replicate.
  auto & body_set = evoke.physics.GetBodySet();
  double repro_prob = 0.008;
  if (body_set.size() < 10) repro_prob = 0.02;

  // Note the time any organisms are born.
  const double birth_time = evoke.anim.GetRunTime();
  const int cur_sec = birth_time / 1000;
  const int phase = ((cur_sec > 5) ? (cur_sec/5 - 1) : 0) % colors_used;

  if (body_set.size()) {
    for (auto * body : body_set) {
      // Bodies that are reproducing cannot produce a second offspring until they are done.
      // Bodies under pressure do not produce offspring.
      if (body->IsReproducing() || body->GetPressure() > 1.0) continue;
      if (evoke.random.P(repro_prob) || body_set.size() == 1) {
        emp::Angle repro_angle(evoke.random.GetDouble(2.0 * emp::PI));
        auto * new_body = body->BuildOffspring( repro_angle.GetPoint(0.1) );

        new_body->SetBirthTime(birth_time);
        new_body->SetColorID(phase);
        evoke.physics.AddBody( new_body );
      }
    }

    // If we have too many organisms, kill the oldest!
    const int num_kills = body_set.size()/100 - 1;
    for (int i = 0; i < num_kills; i++) evoke.physics.KillOldest();
  }

  auto & canvas = doc.Canvas(canvas_name);
  emp::UI::Draw(canvas, evoke.physics.GetSurface(), color_map);
  
  canvas.Refresh();
}

void EvokeAnim_Change() {
  EvokeAnim_Basic (2, "canvas_change", evoke_change);
}
void EvokeAnim_Novelty() {
  EvokeAnim_Basic (color_map.size(), "canvas_novel", evoke_novel);
}


int main() {
  UI::Initialize();

  evoke_change.anim.SetCallback(EvokeAnim_Change);
  evoke_novel.anim.SetCallback(EvokeAnim_Novelty);

  auto & canvas_change = doc.AddCanvas(cw, ch, "canvas_change");
  doc << "<br>";
  doc.AddAnimToggle(evoke_change.anim);
  emp::UI::Draw(canvas_change, evoke_change.physics.GetSurface(), color_map);

  doc << "<br>";

  auto & canvas_novel = doc.AddCanvas(cw, ch, "canvas_novel");
  doc << "<br>";
  doc.AddAnimToggle(evoke_novel.anim);
  emp::UI::Draw(canvas_novel, evoke_novel.physics.GetSurface(), color_map);

  // Draw the surface on a new canvas!

  doc.Update();

}
