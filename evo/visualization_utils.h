#include "../web/web_init.h"
#include "../web/Animate.h"
#include "../emtools/JSWrap.h"

#include "../../d3-emscripten/selection.h"
#include "../../d3-emscripten/scales.h"
#include "../../d3-emscripten/axis.h"

#include <functional>
#include <algorithm>

#include "NK-const.h"
#include "OEE.h"
#include "World.h"
#include "../tools/BitSet.h"
#include "../tools/Random.h"
#include "../evo/StatsManager.h"

EMP_BUILD_CONFIG( NKConfig,
  GROUP(DEFAULT, "Default settings for NK model"),
  CONST(K, int, 10, "Level of epistasis in the NK model"),
  CONST(N, int, 50, "Number of bits in each organisms (must be > K)"), ALIAS(GENOME_SIZE),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  CONST(POP_SIZE, int, 100, "Number of organisms in the popoulation."),
  CONST(MAX_GENS, int, 2000, "How many generations should we process?"),
  CONST(TOURNAMENT_SIZE, int, 5, "How many orgs are chosen per tournament?"),
  VALUE(MUT_COUNT, int, 3, "How many bit positions should be randomized?"), ALIAS(NUM_MUTS),
)

const static NKConfig config;

template <typename ORG, typename... MANAGERS>
class NKInfoHolder{
public:
  emp::Random * r;
  emp::evo::NKLandscapeConst<config.N(), config.K()> * landscape;
  emp::evo::World<ORG, MANAGERS...> * world;
  emp::web::Animate anim;

  NKInfoHolder(){
    r = new emp::Random();
    landscape = new emp::evo::NKLandscapeConst<config.N(), config.K()>(*r);
    world = new emp::evo::World<ORG, MANAGERS...>(*r);
    std::function<double(ORG*)> fit_fun = [this](ORG * org){ return landscape->GetFitness(*org); };
    world->SetDefaultFitnessFun(fit_fun);
  }

};

class D3Visualization {
public:
  emp::vector<D3::D3_Base*> d3_objects;
  D3::Selection * svg;

  D3Visualization(){
    emp::web::Initialize();
    n_objects();
    d3_objects.push_back(new D3::Selection("body"));
    D3::Selection temp_svg = ((D3::Selection*)(d3_objects[0]))->Append("svg");
    svg = new D3::Selection(temp_svg);
  }
};

template <typename ORG, typename... MANAGERS>
class FitnessVisualization : D3Visualization {
private:
  int height = 500;
  int width = 500;
  double margin = 10;
  double axis_width = 40;
  double fitness_growth_margin = 1.5;
  double fitness_loss_margin = .8;
public:
  D3::LinearScale * x_scale;
  D3::LinearScale * fitness_scale;
  D3::Axis<D3::LinearScale> * ax;
  D3::Selection * circles;
  NKInfoHolder<ORG, MANAGERS...> info;

  std::function<double(double, int, int)> scaled_d = [&](double d, int i, int k){
      return fitness_scale->ApplyScale(d);
  };

  std::function<double(double, int, int)> scaled_i = [&](double d, int i, int k){
      return x_scale->ApplyScale(i);
  };

  std::function<void()> animate_function = [this](){Evolve(info.world, circles);};

  FitnessVisualization(){

    //Set up svg
    svg->SetAttr("height", height);
    svg->SetAttr("width", width);
  }

  void Setup(){
    EM_ASM({tip = d3.tip().html(function(d, i) { return d; });});

    emp::vector<double> fitnesses = RunFunctionOnContainer(info.world->GetFitFun(), info.world->popM);
    double lowest = *(std::min_element(fitnesses.begin(), fitnesses.end()));
    double highest = *(std::max_element(fitnesses.begin(), fitnesses.end()));

    //Set up scales
    fitness_scale = new D3::LinearScale();
    x_scale = new D3::LinearScale();
    fitness_scale->SetDomain(std::array<double, 2>({highest*fitness_growth_margin, lowest*fitness_loss_margin}));
    fitness_scale->SetRange(std::array<double, 2>({margin, height - margin}));
    x_scale->SetDomain(std::array<double, 2>({0, config.POP_SIZE()-1}));
    x_scale->SetRange(std::array<double, 2>({axis_width, height-margin}));

    //Set up axis
    ax = new D3::Axis<D3::LinearScale>();
    ax->SetScale(*fitness_scale);
    ax->SetOrientation("right");
    ax->Draw(*svg);

    //Make callback functions
    emp::JSWrap(scaled_d, "scaled_d");
    emp::JSWrap(scaled_i, "scaled_i");

    //Draw circles that represent fitnesses
    circles = new D3::Selection(svg->SelectAll("circle").Data(fitnesses));
    circles->EnterAppend("circle");
    circles->SetAttr("r", 5);
    circles->SetAttr("cx", "scaled_i");
    circles->SetAttr("cy", "scaled_d");
    circles->SetStyle("fill", "green");

    EM_ASM_ARGS({js.objects[$0].call(tip);}, svg->GetID());
    EM_ASM_ARGS({js.objects[$0].on("mouseover", tip.show).on("mouseout", tip.hide);}, circles->GetID());

    info.anim.SetCallback(animate_function);
  }

  template <typename WORLD>
  void Evolve(WORLD * pop, D3::Selection * circles) {
    pop->TournamentSelect(config.TOURNAMENT_SIZE(), config.POP_SIZE());
    pop->Update();
    pop->MutatePop();
    emp::vector<double> fitnesses = RunFunctionOnContainer(pop->GetFitFun(), pop->popM);
    circles = new D3::Selection(circles->Data(fitnesses));
    circles->Transition().SetAttr("cy", "scaled_d");
  }
};
