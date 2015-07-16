#include "../../tools/mem_track.h"

#include "../../geometry/Physics2D.h"
#include "../../tools/Random.h"
#include "../../UI/UI.h"
#include "../../UI/canvas_utils.h"
#include "../../UI/Animate.h"

namespace UI = emp::UI;

int cw = 300;
int ch = 300;

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

void EvokeAnim() 
{
  // std::cerr << EMP_TRACK_STATUS << std::endl;
  // emp::CappedAlert(10, EMP_TRACK_STATUS);

  evoke.physics.Update();

  // Test which organisms should replicate.
  double repro_prob = 0.003;

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

int main() {
  UI::Initialize();

  // Add bodies to the physics...
  for (int i = 0; i < 300; i++) {
    auto new_circle = emp::Circle<>(evoke.random.GetDouble(cw), evoke.random.GetDouble(ch), 7);
    dBODY * new_body = new dBODY( new_circle );
    new_body->SetColorID(evoke.random.GetInt(360));  // Set color to random hue.
    evoke.physics.AddBody( new_body );
  }

  anim.SetCallback(EvokeAnim);

  auto & canvas = doc.AddCanvas(cw, ch, "can");
  doc << "<br>";
  doc.AddButton([](){
      anim.ToggleActive();
      auto & but = doc.Button("toggle");
      if (anim.GetActive()) but.Label("Pause");
      else but.Label("Start");
      but.Update();
    }, "Start", "toggle").Size(50,30);

  doc.Update();

  // Draw the surface on a new canvas!
  emp::UI::Draw(canvas, evoke.physics.GetSurface(), emp::GetHueMap(360));

}
