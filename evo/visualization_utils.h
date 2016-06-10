#ifndef VISUALIZATION_UTILS_H
#define VISUALIZATION_UTILS_H

#include "../web/web_init.h"
#include "../web/Animate.h"
#include "../emtools/JSWrap.h"

#include "../../d3-emscripten/selection.h"
#include "../../d3-emscripten/scales.h"
#include "../../d3-emscripten/axis.h"
#include "../../d3-emscripten/svg_shapes.h"

#include <functional>
#include <algorithm>

#include "../config/config.h"
#include "NK-const.h"
#include "../tools/BitSet.h"
#include "../tools/Random.h"
#include "../tools/stats.h"
#include "../tools/string_utils.h"
#include "NKConfig.h"

class D3Visualization {
public:
  emp::vector<D3::D3_Base*> d3_objects;
  D3::Selection * svg;
  int POP_SIZE = 100;
  int MAX_GENS = 1000;

  D3Visualization(){
    #ifdef EMSCRIPTEN
    emp::web::Initialize();
    n_objects();
    #endif
    d3_objects.push_back(new D3::Selection("body"));
    D3::Selection temp_svg = ((D3::Selection*)(d3_objects[0]))->Append("svg");
    svg = new D3::Selection(temp_svg);
  }

};

class FitnessVisualization : public D3Visualization {
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

  std::function<double(double, int, int)> scaled_d = [&](double d, int i, int k){
      return fitness_scale->ApplyScale(d);
  };

  std::function<double(double, int, int)> scaled_i = [&](double d, int i, int k){
      return x_scale->ApplyScale(i);
  };

  FitnessVisualization(){

    //Set up svg
    svg->SetAttr("height", height);
    svg->SetAttr("width", width);
  }

  void Setup(emp::vector<double> fitnesses){
    D3::ToolTip tip;

    double lowest = *(std::min_element(fitnesses.begin(), fitnesses.end()));
    double highest = *(std::max_element(fitnesses.begin(), fitnesses.end()));

    //Set up scales
    fitness_scale = new D3::LinearScale();
    x_scale = new D3::LinearScale();
    fitness_scale->SetDomain(std::array<double, 2>({highest*fitness_growth_margin, lowest*fitness_loss_margin}));
    fitness_scale->SetRange(std::array<double, 2>({margin, height - margin}));
    x_scale->SetDomain(std::array<double, 2>({0, (double)POP_SIZE-1}));
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

    circles->AddToolTip(tip);
  }

  void AnimateStep(emp::vector<double> fitnesses){
      circles = new D3::Selection(circles->Data(fitnesses));
      circles->Transition().SetAttr("cy", "scaled_d");
  }

};

class GraphVisualization : public D3Visualization {
private:
  int height = 500;
  int width = 1000;
  double margin = 30;
  double axis_width = 60;
  double y_min = 5;
  double y_max = 10;

public:
  D3::LinearScale * x_scale;
  D3::LinearScale * y_scale;
  D3::Axis<D3::LinearScale> * x_axis;
  D3::Axis<D3::LinearScale> * y_axis;
  D3::Selection * circles;

  //Callback function for taking a datapoint and getting appropriately scaled y val
  std::function<double(std::array<double, 2>, int, int)> y = [this](std::array<double, 2> d, int i=0, int k=0){
      return y_scale->ApplyScale(d[1]);
  };

  //Callback function for taking a datapoint and getting appropriately scaled x val
  std::function<double(std::array<double,2>, int, int)> x = [this](std::array<double, 2> d, int i=0, int k=0){
      return x_scale->ApplyScale(d[0]);
  };

  //Callback function for getting unscaled x value of data point (used as key function for data binding)
  std::function<double(std::array<double,2>, int)> return_x = [&](std::array<double, 2> d, int i=0){
      return d[0];
  };

  //Format function for tooltip
  D3::FormatFunction rounded = D3::FormatFunction(".2f");

  //Function telling tooltip how to display a data point
  std::function<std::string(std::array<double, 2>, int, int)> tooltip_display =
                                       [this](std::array<double, 2> d, int i, int k) {
                                         return rounded.Call(d[1]);
                                     };

  GraphVisualization(){
    //Set up svg
    svg->SetAttr("height", height);
    svg->SetAttr("width", width);
  }

  std::array<std::array<double, 2>, 1> data = {-1,-1};
  D3::LineGenerator * make_line;

  D3::ToolTip* tip;
  D3::Selection t;

  void Setup(const NKConfig & config){

    //Setup config parameters
    POP_SIZE = config.POP_SIZE();
    MAX_GENS = config.MAX_GENS();

    //Wrap ncessary callback functions
    emp::JSWrap(tooltip_display, "tooltip_display");
    emp::JSWrap(x, "x");
    emp::JSWrap(y, "y");
    emp::JSWrap(return_x, "return_x");

    //Create tool top
    tip = new D3::ToolTip("tooltip_display");

    //Set up scales
    y_scale = new D3::LinearScale();
    x_scale = new D3::LinearScale();
    y_scale->SetDomain(std::array<double, 2>({y_max, y_min}));
    y_scale->SetRange(std::array<double, 2>({margin, (double)height - axis_width}));
    x_scale->SetDomain(std::array<double, 2>({0, (double)MAX_GENS}));
    x_scale->SetRange(std::array<double, 2>({axis_width, width-margin}));

    //Set up axes
    x_axis = new D3::Axis<D3::LinearScale>("Update");
    x_axis->SetScale(*x_scale);
    y_axis = new D3::Axis<D3::LinearScale>("Shannon Entropy");
    y_axis->SetScale(*y_scale);
    D3::DrawAxes(*x_axis, *y_axis, *svg);
    make_line = new D3::LineGenerator();

    circles = new D3::Selection(svg->SelectAll("circle"));
  }

  void AnimateStep(std::array<double, 2> data_point) {

    if (data_point[1] > y_max || data_point[1] < y_min) {
      std::function<void()> draw_data = [this, data_point](){
        DrawData(data_point);
      };
      emp::JSWrap(draw_data, "draw_data");

      y_max = std::max(data_point[1]*1.2, y_max);
      y_min = std::min(data_point[1]*.8, y_min);
      y_scale->SetDomain(std::array<double,2>({y_max, y_min}));
      t = svg->Transition();
      y_axis->ApplyAxis(t.Select("#ShannonEntropy_axis"));
      int y_id = emp::JSWrap(y, "y");
      t.SelectAll("circle").SetAttr("cy", "y");
      EM_ASM_ARGS({
        circle_data = js.objects[$0].selectAll(".data-point").data();
        path_data = [];
        for (iter=0; iter<circle_data.length-1; iter++){
          path_data.push(js.objects[$1]([[emp.x(circle_data[iter]), emp.y(circle_data[iter])],
                         [emp.x(circle_data[iter+1]), emp.y(circle_data[iter+1])]]));
        }
        js.objects[$0].selectAll(".line-seg").data(path_data);
        js.objects[$3].selectAll(".line-seg").attr("d", function(d){return d;});
      }, svg->GetID(), make_line->GetID(), y_id, t.GetID());

      t.Each("end", "draw_data");
    } else {
      DrawData(data_point);
    }
  }

  void DrawData(std::array<double, 2> data_point) {
    std::array<std::array<double, 2>, 1> prev_data = data;
    data[0] = data_point;

    //We can't draw a line on the first update
    if (prev_data[0][0] >=0 ){
      std::array<std::array<double, 2>, 2> line_data;
      prev_data[0][0] = x(prev_data[0], 0, 0);
      prev_data[0][1] = y(prev_data[0],0 ,0);
      line_data[0] = prev_data[0];

      std::array<std::array<double, 2>, 1> new_data;
      new_data = data;
      new_data[0][0] = x(new_data[0],0,0);
      new_data[0][1] = y(new_data[0],0,0);
      line_data[1] = new_data[0];

      D3::Selection line = make_line->DrawShape(line_data, *svg);
      line.SetAttr("fill", "none");
      line.SetAttr("stroke", "green");
      line.SetAttr("stroke-width", 1);
      line.SetAttr("class", "line-seg");
    }

    circles = new D3::Selection(circles->Data(data, "return_x"));
    D3::Selection enter = circles->EnterAppend("circle");
    enter.SetAttr("cy", "y");
    enter.SetAttr("cx", "x");
    enter.SetAttr("r", 2);
    enter.SetAttr("class", "data-point");
    enter.SetStyle("fill", "green");
    enter.AddToolTip(*tip);
  }
};

#endif
