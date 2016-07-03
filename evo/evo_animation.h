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

#include "NK.h"
#include "../tools/BitSet.h"
#include "../tools/Random.h"
#include "../tools/stats.h"
#include "../tools/string_utils.h"
#include "visualization_utils.h"
#include "World.h"
#include "NKConfig.h"

template <typename ORG, typename... MANAGERS>
class NKAnimation{
public:
  NKConfig config;
  emp::Random * r;
  emp::evo::NKLandscape * landscape;
  emp::evo::World<ORG, MANAGERS...> * world;
  emp::web::Animate anim;
  bool initialized = false;

  NKAnimation(){
    r = new emp::Random();
    std::cout << "Random seed " << r->GetSeed() << std::endl;
    landscape = new emp::evo::NKLandscape(config.N(), config.K(), *r);
    world = new emp::evo::World<ORG, MANAGERS...>(*r);
    std::function<double(ORG*)> fit_fun = [this](ORG * org){ return landscape->GetFitness(*org); };
    world->SetDefaultFitnessFun(fit_fun);
    anim.SetCallback([this](){Evolve();});
    world->SetDefaultMutateFun( [this](ORG* org, emp::Random& random) {
      for (int i = 0; i < config.N(); ++i){
        if (random.P(config.MUT_RATE())) {
          (*org)[i] = !(*org)[i];
        }
      }
      return true;
    } );
  }

  void NewWorld() {
    delete world;
    delete landscape;
    initialized = false;
    landscape = new emp::evo::NKLandscape(config.N(), config.K(),*r);
    world = new emp::evo::World<ORG, MANAGERS...>(*r);
    std::function<double(ORG*)> fit_fun = [this](ORG * org){ return landscape->GetFitness(*org); };
    world->SetDefaultFitnessFun(fit_fun);
    std::cout << config.MUT_RATE() << std::endl;
    world->SetDefaultMutateFun( [this](ORG* org, emp::Random& random) {
      for (int i = 0; i < config.N(); ++i){
        if (random.P(config.MUT_RATE())) {
          (*org)[i] = !(*org)[i];
        }
      }
      return true;
    } );
  }

  void Evolve() {
    if (!initialized) {
      Initialize();
      initialized = true;
    }
    world->TournamentSelect(config.TOURNAMENT_SIZE(), config.POP_SIZE());
    world->Update();
    world->MutatePop();
  }

  void Initialize() {

    for (int i = 0; i < config.POP_SIZE(); i++) {
      ORG next_org;
      next_org.Resize(config.N());
      for (int j = 0; j < config.N(); j++) next_org[j] = r->P(0.5);
      world->Insert(next_org);
    }
  }

};

#endif
