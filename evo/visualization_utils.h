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
#include "OEE.h"
#include "World.h"
#include "../tools/BitSet.h"
#include "../tools/Random.h"
#include "../tools/stats.h"
#include "../tools/string_utils.h"
#include "../evo/StatsManager.h"

EMP_BUILD_CONFIG( NKConfig,
  GROUP(DEFAULT, "Default settings for NK model"),
  CONST(K, int, 0, "Level of epistasis in the NK model"),
  CONST(N, int, 100, "Number of bits in each organisms (must be > K)"), ALIAS(GENOME_SIZE),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  CONST(POP_SIZE, int, 1000, "Number of organisms in the popoulation."),
  CONST(MAX_GENS, int, 2000, "How many generations should we process?"),
  CONST(TOURNAMENT_SIZE, int, 20, "How many orgs are chosen per tournament?"),
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

template <typename ORG, typename... MANAGERS>
class GraphVisualization : D3Visualization {
private:
  int height = 500;
  int width = 1000;
  double margin = 30;
  double axis_width = 40;
  double fitness_growth_margin = 1.5;
  double fitness_loss_margin = .8;
public:
  D3::LinearScale * x_scale;
  D3::LinearScale * y_scale;
  D3::Axis<D3::LinearScale> * x_axis;
  D3::Axis<D3::LinearScale> * y_axis;
  D3::Selection * circles;
  NKInfoHolder<ORG, MANAGERS...> info;

  std::function<double(std::array<double, 2>, int, int)> y = [&](std::array<double, 2> d, int i=0, int k=0){
      return y_scale->ApplyScale(d[1]);
  };

  std::function<double(std::array<double,2>, int, int)> x = [&](std::array<double, 2> d, int i=0, int k=0){
      return x_scale->ApplyScale(d[0]);
  };

  std::function<double(std::array<double,2>, int)> return_x = [&](std::array<double, 2> d, int i=0){
      return d[0];
  };

  std::function<void()> animate_function = [this](){Evolve(info.world, circles);};

  double update = 0;

  GraphVisualization(){

    //Set up svg
    svg->SetAttr("height", height);
    svg->SetAttr("width", width);
  }

  std::array<std::array<double, 2>, 1> data;
      D3::LineGenerator * make_line;

  void Setup(){


    //Set up scales
    y_scale = new D3::LinearScale();
    x_scale = new D3::LinearScale();
    y_scale->SetDomain(std::array<double, 2>({10, 0}));
    y_scale->SetRange(std::array<double, 2>({margin, (double)height - axis_width}));
    x_scale->SetDomain(std::array<double, 2>({0, config.MAX_GENS()}));
    x_scale->SetRange(std::array<double, 2>({axis_width, width-margin}));

    //Set up axes
    D3::Selection x_axis_group = svg->Append("g");
    x_axis_group.SetAttr("transform", std::string(std::string("translate(0,")+emp::to_string(height-axis_width)+std::string(")")).c_str());
    x_axis = new D3::Axis<D3::LinearScale>();
    x_axis->SetScale(*x_scale);
    x_axis->SetOrientation("bottom");
    x_axis->SetTicks(10);
    x_axis->SetTickFormat("g");
    x_axis->Draw(x_axis_group);
    D3::Selection x_axis_path = x_axis_group.SelectAll("line, .domain");
    x_axis_path.SetAttr("stroke-width", 1);
    x_axis_path.SetAttr("fill", "none");
    x_axis_path.SetAttr("stroke", "black");


    D3::Selection y_axis_group = svg->Append("g");
    y_axis = new D3::Axis<D3::LinearScale>();
    y_axis->SetScale(*y_scale);
    y_axis->SetOrientation("left");
    y_axis_group.SetAttr("transform", std::string(std::string("translate(")+emp::to_string(axis_width)+std::string(",0)")).c_str());
    y_axis->SetTickFormat("g");
    y_axis->Draw(y_axis_group);
    D3::Selection y_axis_path = y_axis_group.SelectAll("line, .domain");
    y_axis_path.SetAttr("stroke-width", 1);
    y_axis_path.SetAttr("fill", "none");
    y_axis_path.SetAttr("stroke", "black");

    make_line = new D3::LineGenerator();

    //Make callback functions
    emp::JSWrap(x, "x");
    emp::JSWrap(y, "y");
    emp::JSWrap(return_x, "return_x");

    data[0] = std::array<double, 2>({update, ShannonEntropy(info.world->popM)});

    //Draw circles that represent fitnesses
    circles = new D3::Selection(svg->SelectAll("circle").Data(data));
    circles->EnterAppend("circle");
    circles->SetAttr("r", 2);
    circles->SetAttr("cx", "x");
    circles->SetAttr("cy", "y");
    circles->SetStyle("fill", "green");

    EM_ASM_ARGS({js.objects[$0].call(tip);}, circles->GetID());
    EM_ASM_ARGS({js.objects[$0].on("mouseover", tip.show).on("mouseout", tip.hide);}, circles->GetID());

    info.anim.SetCallback(animate_function);
  }

  template <typename WORLD>
  void Evolve(WORLD * pop, D3::Selection * circles) {
    pop->TournamentSelect(config.TOURNAMENT_SIZE(), config.POP_SIZE());
    pop->Update();
    pop->MutatePop();
    update++;
    if ((int)update % 10 == 0){
      std::array<std::array<double, 2>, 1> prev_data = data;
      data[0] = std::array<double, 2>({update, ShannonEntropy(info.world->popM)});

      std::array<std::array<double, 2>, 2> line_data;
      prev_data[0][0] = x(prev_data[0], 0, 0);
      prev_data[0][1] = y(prev_data[0],0 ,0);
      line_data[0] = prev_data[0];

      std::array<std::array<double, 2>, 1> new_data;
      new_data = data;
      new_data[0][0] = x(new_data[0],0,0);
      new_data[0][1] = y(new_data[0],0,0);
      line_data[1] = new_data[0];

      D3::Selection line = make_line->DrawShape(line_data);
      line.SetAttr("fill", "none");
      line.SetAttr("stroke", "green");
      line.SetAttr("stroke-width", 1);

      circles = new D3::Selection(circles->Data(data, "return_x"));
      D3::Selection enter = circles->EnterAppend("circle");
      enter.SetAttr("cy", "y");
      enter.SetAttr("cx", "x");
      enter.SetAttr("r", 2);
      enter.SetStyle("fill", "green");
      EM_ASM_ARGS({js.objects[$0].call(tip);}, circles->GetID());
      EM_ASM_ARGS({js.objects[$0].on("mouseover", tip.show).on("mouseout", tip.hide);}, circles->GetID());

    }
  }
};
