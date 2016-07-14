#ifndef EVO_ANIMATION_H
#define EVO_ANIMATION_H

#include "../web/web_init.h"
#include "../web/Animate.h"
#include "../emtools/JSWrap.h"
#include "../tools/BitSet.h"
#include "../tools/Random.h"
#include "../tools/stats.h"
#include "../tools/string_utils.h"

#include "../web/d3/selection.h"
#include "../web/d3/scales.h"
#include "../web/d3/axis.h"
#include "../web/d3/svg_shapes.h"

#include <functional>
#include <algorithm>

#include "NK.h"
#include "visualization_utils.h"
#include "World.h"
#include "NKConfig.h"

template <typename ORG, typename... MANAGERS>
class NKAnimation{
public:
  NKConfig config;
  emp::Random * r;
  emp::evo::NKLandscape * landscape;
  emp::evo::NKLandscape * alternate;
  emp::evo::World<ORG, MANAGERS...> * world;
  emp::web::Animate anim;
  bool initialized = false;
  bool use_alternate = false;

  NKAnimation(){
    r = new emp::Random();
    std::cout << "Random seed " << r->GetSeed() << std::endl;
    landscape = new emp::evo::NKLandscape(config.N(), config.K(), *r);
    alternate = new emp::evo::NKLandscape(config.N(), config.K(), *r);
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
    delete alternate;
    initialized = false;
    landscape = new emp::evo::NKLandscape(config.N(), config.K(),*r);
    alternate = new emp::evo::NKLandscape(config.N(), config.K(), *r);
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
    if (config.FIT_SHARE() && use_alternate && config.CHANGE_ENV()){
      world->FitnessSharingTournamentSelect([this](ORG * org){ return alternate->GetFitness(*org); }, [](ORG* org1, ORG* org2){ return (double)(org1->XOR(*org2)).CountOnes();}, 10, 1, config.TOURNAMENT_SIZE(), config.POP_SIZE());
    } else if (config.FIT_SHARE()) {
      world->FitnessSharingTournamentSelect([this](ORG * org){ return landscape->GetFitness(*org); }, [](ORG* org1, ORG* org2){ return (double)(org1->XOR(*org2)).CountOnes();}, 10, 1, config.TOURNAMENT_SIZE(), config.POP_SIZE());
  } else if (use_alternate && config.CHANGE_ENV()) {
      world->TournamentSelect([this](ORG * org){ return alternate->GetFitness(*org); }, config.TOURNAMENT_SIZE(), config.POP_SIZE());
    } else {
      world->TournamentSelect([this](ORG * org){ return landscape->GetFitness(*org); }, config.TOURNAMENT_SIZE(), config.POP_SIZE());
    }
    world->Update();

    if (config.CHANGE_ENV() && world->update%100 == 0 ) {
      use_alternate = !use_alternate;
    }

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
