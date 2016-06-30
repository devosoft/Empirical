#ifndef EVO_ANIMATION_H
#define EVO_ANIMATION_H

#include "../web/web_init.h"
#include "../web/Animate.h"
#include "../emtools/JSWrap.h"

#include "../../d3-emscripten/selection.h"
#include "../../d3-emscripten/scales.h"
#include "../../d3-emscripten/axis.h"
#include "../../d3-emscripten/svg_shapes.h"

#include <functional>
#include <algorithm>

#include "NK-const.h"
#include "../tools/BitSet.h"
#include "../tools/Random.h"
#include "../tools/stats.h"
#include "../tools/string_utils.h"
#include "visualization_utils.h"
#include "World.h"
#include "NKConfig.h"

template <int N, int K, typename ORG, typename... MANAGERS>
class NKAnimation{
public:
  NKConfig config;
  emp::Random * r;
  emp::evo::NKLandscapeConst<N, K> * landscape;
  emp::evo::World<ORG, MANAGERS...> * world;
  emp::web::Animate anim;
  bool initialized = false;

  NKAnimation(){
    r = new emp::Random();
    std::cout << "Random seed " << r->GetSeed() << std::endl;
    landscape = new emp::evo::NKLandscapeConst<N, K>(*r);
    world = new emp::evo::World<ORG, MANAGERS...>(*r);
    std::function<double(ORG*)> fit_fun = [this](ORG * org){ return landscape->GetFitness(*org); };
    world->SetDefaultFitnessFun(fit_fun);
    anim.SetCallback([this](){Evolve();});
  }

  void Evolve() {
    if (!initialized) {
      Initialize();
      initialized = true;
    }
    world->TournamentSelect(config.TOURNAMENT_SIZE(), config.POP_SIZE());
    world->Update();
    world->MutatePop();
    if (anim.GetFrameCount() >= config.MAX_GENS()){
      anim.Stop();
    }
  }

  void Initialize() {
    // Build a random initial population
    world->SetDefaultMutateFun( [this](ORG* org, emp::Random& random) {
      for (int i = 0; i < N; ++i){
        if (random.P(config.MUT_RATE())) {
          (*org)[i] = !(*org)[i];
        }
      }
      return true;
    } );

    for (int i = 0; i < config.POP_SIZE(); i++) {
      ORG next_org;
      for (int j = 0; j < N; j++) next_org[j] = r->P(0.5);
      world->Insert(next_org);
    }
  }

};

#endif
