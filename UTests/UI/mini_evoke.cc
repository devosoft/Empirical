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

  Evoke(int w, int h) : physics(w,h) { ; }
};

UI::Document doc("emp_base");
Evoke evoke(cw,ch);
UI::Animate anim;

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

void EvokeAnim_Change(const UI::Animate & anim)
{
  evoke.physics.Update();

  // Test which organisms should replicate.
  double repro_prob = 0.015;

  auto & body_set = evoke.physics.GetBodySet();

  const int cur_color = 0;
  const int alt_color = 230;

  if (body_set.size()) {
    for (auto * body : body_set) {
      // Bodies that are reproducing cannot produce a second offspring until they are done.
      // Bodies under pressure do not produce offspring.
      if (body->IsReproducing() || body->GetPressure() > 1.0) continue;
      if (evoke.random.P(repro_prob) || body_set.size() == 1) {
        emp::Angle repro_angle(evoke.random.GetDouble(2.0 * emp::PI));
        auto * new_body = body->BuildOffspring( repro_angle.GetPoint(0.1) );

        int cur_time = anim.GetRunTime() / 1000;
        if (anim.GetCurTime() > 20 && (cur_time/10 % 2) ) {
          new_body->SetColorID(alt_color);
        }
        else {
          new_body->SetColorID( cur_color );
        }
        evoke.physics.AddBody( new_body );
      }
    }
  }

  auto & canvas = doc.Canvas("can");
  emp::UI::Draw(canvas, evoke.physics.GetSurface(), emp::GetHueMap(360));
  
  canvas.Refresh();
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

  auto & canvas = doc.AddCanvas(cw, ch, "can");
  doc << "<br>";
  doc.AddAnimToggle(anim);
  doc.Update();

  // Draw the surface on a new canvas!
  emp::UI::Draw(canvas, evoke.physics.GetSurface(), emp::GetHueMap(360));

}
