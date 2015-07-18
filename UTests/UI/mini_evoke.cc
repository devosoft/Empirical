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

struct Evoke {
  emp::Physics2D<dBODY, dBRAIN> physics;
  emp::Random random;

  const emp::vector<std::string> color_map =
    { "red", "blue",
      "#ffa280", "#992626", "#ff8800", "#ffcc00", "#5d8c00", "#269954", "#00ffee", "#0088ff",
      "#002ca6", "#a280ff", "#8c0070", "#331a24", "#ffbfbf", "#592400", "#ffc480", "#333226",
      "#556633", "#004d33", "#269199", "#4d7599", "#bfc8ff", "#754d99", "#59003c", "#994d61",
      "#400900", "#a64200", "#593c00", "#eeff00", "#00ff00", "#4d665e", "#003d4d", "#262d33",
      "#2200ff", "#cc00ff", "#ff80c4", "#ff2200", "#998273", "#8c7000", "#f2ffbf", "#7fffa1",
      "#bffff2", "#00ccff", "#163159", "#180059", "#4b394d", "#ff0066"
    };
  
  
  Evoke(int w, int h) : physics(w,h) { ; }
};

UI::Document doc("emp_base");
Evoke evoke(cw,ch);
UI::Animate anim;
int pop_size = 0;

void EvokeAnim(const UI::Animate &)
{
  evoke.physics.Update();

  // Test which organisms should replicate.
  //double repro_prob = 0.003;
  double repro_prob = 0.01;

  auto & body_set = evoke.physics.GetBodySet();

  if (body_set.size()) {
    for (auto * body : body_set) {
      // Bodies that are reproducing cannot produce a second offspring until they are done.
      // Bodies under pressure do not produce offspring.
      if (body->IsReproducing() || body->GetPressure() > 1.0) continue;
      if (evoke.random.P(repro_prob) || body_set.size() == 1) {
        emp::Angle repro_angle(evoke.random.GetDouble(2.0 * emp::PI));
        auto * new_body = body->BuildOffspring( repro_angle.GetPoint(0.1) );
        if (evoke.random.P(0.95)) {
          new_body->SetColorID(body->GetColorID());
        }
        else {
          new_body->SetColorID( evoke.random.GetInt(360) );
        }
        evoke.physics.AddBody( new_body );
      }
    }
  }

  auto & canvas = doc.Canvas("can");
  emp::UI::Draw(canvas, evoke.physics.GetSurface(), emp::GetHueMap(360));
  
  canvas.Refresh();
}

// void EvokeAnim_Change(const UI::Animate & anim)
// {
//   evoke.physics.Update();

//   // Test which organisms should replicate.
//   double repro_prob = 0.008;

//   auto & body_set = evoke.physics.GetBodySet();

//   const int cur_color = 0;
//   const int alt_color = 1;

//   if (body_set.size()) {
//     for (auto * body : body_set) {
//       // Bodies that are reproducing cannot produce a second offspring until they are done.
//       // Bodies under pressure do not produce offspring.
//       if (body->IsReproducing() || body->GetPressure() > 1.0) continue;
//       if (evoke.random.P(repro_prob) || body_set.size() == 1) {
//         emp::Angle repro_angle(evoke.random.GetDouble(2.0 * emp::PI));
//         auto * new_body = body->BuildOffspring( repro_angle.GetPoint(0.1) );

//         const double birth_time = anim.GetRunTime();
//         new_body->SetBirthTime(birth_time);
//         const int cur_sec = birth_time / 1000;
//         if (anim.GetCurTime() > 20 && (cur_sec/5 % 2) ) {
//           new_body->SetColorID(alt_color);
//         } else {
//           new_body->SetColorID( cur_color );
//         }
//         evoke.physics.AddBody( new_body );
//       }
//     }

//     // If we have too many organisms, kill the oldest!
//     const int num_kills = body_set.size()/100 - 1;
//     for (int i = 0; i < num_kills; i++) evoke.physics.KillOldest();
//   }

//   auto & canvas = doc.Canvas("can");
//   emp::UI::Draw(canvas, evoke.physics.GetSurface(), evoke.color_map);
  

//   canvas.Refresh();

//   pop_size = evoke.physics.GetConstBodySet().size();
//   doc.Text("stats").Update();
// }

// void EvokeAnim_Novelty(const UI::Animate & anim)
// {
//   evoke.physics.Update();

//   // Test which organisms should replicate.
//   auto & body_set = evoke.physics.GetBodySet();
//   double repro_prob = 0.008;
//   if (body_set.size() < 10) repro_prob = 0.02;

//   // Note the time any organisms are born.
//   const double birth_time = anim.GetRunTime();
//   const int cur_sec = birth_time / 1000;
//   const int phase = ((cur_sec > 5) ? (cur_sec/5 - 1) : 0) % (int) evoke.color_map.size();

//   if (body_set.size()) {
//     for (auto * body : body_set) {
//       // Bodies that are reproducing cannot produce a second offspring until they are done.
//       // Bodies under pressure do not produce offspring.
//       if (body->IsReproducing() || body->GetPressure() > 1.0) continue;
//       if (evoke.random.P(repro_prob) || body_set.size() == 1) {
//         emp::Angle repro_angle(evoke.random.GetDouble(2.0 * emp::PI));
//         auto * new_body = body->BuildOffspring( repro_angle.GetPoint(0.1) );

//         new_body->SetBirthTime(birth_time);
//         new_body->SetColorID(phase);
//         evoke.physics.AddBody( new_body );
//       }
//     }

//     // If we have too many organisms, kill the oldest!
//     const int num_kills = body_set.size()/100 - 1;
//     for (int i = 0; i < num_kills; i++) evoke.physics.KillOldest();
//   }

//   auto & canvas = doc.Canvas("can");
//   emp::UI::Draw(canvas, evoke.physics.GetSurface(), evoke.color_map);
  

//   canvas.Refresh();

//   pop_size = evoke.physics.GetConstBodySet().size();
//   doc.Text("stats").Update();
// }

void EvokeAnim_Basic(const UI::Animate & anim, int colors_used=-1)
{
  if (colors_used < 0) colors_used = (int) evoke.color_map.size();
  evoke.physics.Update();

  // Test which organisms should replicate.
  auto & body_set = evoke.physics.GetBodySet();
  double repro_prob = 0.008;
  if (body_set.size() < 10) repro_prob = 0.02;

  // Note the time any organisms are born.
  const double birth_time = anim.GetRunTime();
  const int cur_sec = birth_time / 1000;
  const int phase = ((cur_sec > 5) ? (cur_sec/5 - 1) : 0) % 2;

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

  auto & canvas = doc.Canvas("can");
  emp::UI::Draw(canvas, evoke.physics.GetSurface(), evoke.color_map);
  

  canvas.Refresh();

  pop_size = evoke.physics.GetConstBodySet().size();
  doc.Text("stats").Update();
}

void EvokeAnim_Change(const UI::Animate & anim) {
  EvokeAnim_Basic (anim, 2);
}
void EvokeAnim_Novelty(const UI::Animate & anim) {
  EvokeAnim_Basic (anim, evoke.color_map.size());
}


int main() {
  UI::Initialize();

  // Add bodies to the physics...
  // for (int i = 0; i < 300; i++) {
  //   auto new_circle = emp::Circle<>(evoke.random.GetDouble(cw), evoke.random.GetDouble(ch), 7);
  //   dBODY * new_body = new dBODY( new_circle );
  //   new_body->SetColorID(evoke.random.GetInt(360));  // Set color to random hue.
  //   evoke.physics.AddBody( new_body );
  // }

  auto new_circle = emp::Circle<>(cw/2.0, ch/2.0, 7);
  dBODY * new_body = new dBODY( new_circle );
  new_body->SetColorID( 0 );  // Set color to random hue.
  evoke.physics.AddBody( new_body );

  anim.SetCallback(EvokeAnim_Change);
  // anim.SetCallback(EvokeAnim_Novelty);

  auto & canvas = doc.AddCanvas(cw, ch, "can");
  doc << "<br>";
  doc << UI::Text("stats")
      << UI::Live( pop_size )
      << "<br>";
  doc.AddAnimToggle(anim);

  doc.Update();

  // Draw the surface on a new canvas!
  emp::UI::Draw(canvas, evoke.physics.GetSurface(), emp::GetHueMap(360));

}
