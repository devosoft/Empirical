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
#include <deque>

#include "../config/config.h"
#include "NK-const.h"
#include "../tools/BitSet.h"
#include "../tools/Random.h"
#include "../tools/stats.h"
#include "../tools/string_utils.h"
#include "NKConfig.h"

//Pretty sure D3VisualizationInfo can't be shared among multiple D3Visualizations

namespace emp{
namespace web{
class D3Visualization : public internal::WidgetFacet<D3Visualization> {
    friend class D3VisualizationInfo;
protected:
  class D3VisualizationInfo : public internal::WidgetInfo {
    friend D3Visualization;

    protected:
      int width;
      int height;
      D3Visualization * parent;
      D3::Selection * svg;

      D3VisualizationInfo(D3Visualization * parent, const std::string & in_id="") : internal::WidgetInfo(in_id) {
        this->parent = parent;
      }

      D3VisualizationInfo(const D3VisualizationInfo &) = delete;
      D3VisualizationInfo & operator=(const D3VisualizationInfo &) = delete;
      virtual ~D3VisualizationInfo(){;}

      virtual bool IsD3VisualiationInfo() const override {return true;}

      virtual void GetHTML(std::stringstream & HTML) override {
        HTML.str("");
        HTML << "<svg id=\"" << id
             << "\" width=\"" << width
             << "\" height=\"" << height << "\">";

        HTML << "</svg>";
      }

      // Trigger any JS code needed on re-draw.
      void TriggerJS() override {
        if (state == Widget::ACTIVE) {            // Only draw on active canvases
          svg = new D3::Selection(D3::Select("#"+id));
          parent->Setup();
        }
      }

    public:
      virtual std::string GetType() override {return "web::D3VisualizationInfo";}

  };

  // Get a properly cast version of indo.
  D3VisualizationInfo * Info() { return (D3VisualizationInfo *) info; }
  const D3VisualizationInfo * Info() const { return (D3VisualizationInfo *) info; }

  D3Visualization(D3VisualizationInfo * in_info) : WidgetFacet(in_info) { ; }

public:

  D3Visualization(int w, int h, const std::string & in_id="")
        : WidgetFacet(in_id) {
      #ifdef EMSCRIPTEN
      Initialize();
      n_objects();
      #endif

      info = new D3VisualizationInfo(this, in_id);
      Info()->width = w;
      Info()->height = h;
  }

  D3Visualization(const D3Visualization & in) : WidgetFacet(in) { ; }
  D3Visualization(const Widget & in) : WidgetFacet(in) { emp_assert(info->IsD3VisualiationInfo()); }
  virtual ~D3Visualization() { ; }

  using INFO_TYPE = D3VisualizationInfo;

  int GetWidth() const { return Info()->width; }
  int GetHeight() const { return Info()->height; }
  D3::Selection * GetSVG() {return Info()->svg;}
  std::string GetID() {return Info()->id;}

  int POP_SIZE = 100;
  int MAX_GENS = 1000;
  virtual void Setup(){;}

  virtual void AnimateStep(...){;}
  virtual void AnimateStep(emp::vector<double>){;}
};

class FitnessVisualization : public D3Visualization {
private:
  double margin = 10;
  double axis_width = 40;
  double fitness_growth_margin = 1.5;
  double fitness_loss_margin = .8;
public:
  D3::LinearScale * x_scale;
  D3::LinearScale * fitness_scale;
  D3::Axis<D3::LinearScale> * ax;
  D3::Selection * circles;
  D3::ToolTip * tip;

  std::function<double(double, int, int)> scaled_d = [&](double d, int i, int k){
      return fitness_scale->ApplyScale(d);
  };

  std::function<double(double, int, int)> scaled_i = [&](double d, int i, int k){
      return x_scale->ApplyScale(i);
  };

  FitnessVisualization(int w=500, int h=500) : D3Visualization(w, h){;}

  void Setup(){

    D3::Selection * svg = GetSVG();

    double lowest = 10;//*(std::min_element(fitnesses.begin(), fitnesses.end()));
    double highest = 20;//*(std::max_element(fitnesses.begin(), fitnesses.end()));

    //Set up scales
    fitness_scale = new D3::LinearScale();
    x_scale = new D3::LinearScale();
    fitness_scale->SetDomain(std::array<double, 2>({highest*fitness_growth_margin, lowest*fitness_loss_margin}));
    fitness_scale->SetRange(std::array<double, 2>({margin, GetHeight() - margin}));
    x_scale->SetDomain(std::array<double, 2>({0, (double)POP_SIZE-1}));
    x_scale->SetRange(std::array<double, 2>({axis_width, GetHeight()-margin}));

    //Set up axis
    ax = new D3::Axis<D3::LinearScale>();
    ax->SetScale(*fitness_scale);
    ax->SetOrientation("right");
    ax->Draw(*svg);

    //Make callback functions
    JSWrap(scaled_d, GetID()+"scaled_d");
    JSWrap(scaled_i, GetID()+"scaled_i");
  }

  void AnimateStep(emp::vector<double> fitnesses){
      //Draw circles that represent fitnesses
      circles = new D3::Selection(GetSVG()->SelectAll("circle").Data(fitnesses));
      circles->EnterAppend("circle");
      circles->ExitRemove();
      circles->SetAttr("r", 5);
      circles->SetAttr("cx", GetID()+"scaled_i");
      circles->SetAttr("cy", GetID()+"scaled_d");
      circles->SetStyle("fill", "green");

     // circles->AddToolTip(tip);

      circles = new D3::Selection(circles->Data(fitnesses));
      circles->Transition().SetAttr("cy", GetID()+"scaled_d");
  }

};

class GraphVisualization : public D3Visualization {
private:
  double y_margin = 10;
  double x_margin = 30;
  double axis_width = 60;
  double y_min = 1000;
  double y_max = 0;
  double x_min = 0;
  double x_max = 0;

public:
  emp::vector<std::string> variables;
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

  //Callback function for drawing data after rescale animation
  std::function<void(int, int)> draw_data = [this](int i=0, int j=0, int k=0){
    DrawData(true);
  };

  //Format function for tooltip
  D3::FormatFunction rounded = D3::FormatFunction(".2f");

  //Function telling tooltip how to display a data point
  std::function<std::string(std::array<double, 2>, int, int)> tooltip_display =
                                       [this](std::array<double, 2> d, int i, int k) {
                                         return to_string(rounded.Call(d[1]));
                                     };

  GraphVisualization(std::string y_var, std::string x_var, int w=800, int h=400) : D3Visualization(w, h){
    this->variables.push_back(x_var);
    this->variables.push_back(y_var);
  }

  GraphVisualization(std::string y_var, int w=800, int h=400) : D3Visualization(w, h){
    this->variables.push_back("Update");
    this->variables.push_back(y_var);
  }

  std::deque<std::array<std::array<double, 2>, 1> > data;
  std::array<std::array<double, 2>, 1> prev_data  = {{-1,-1}};
  D3::LineGenerator * make_line;

  D3::ToolTip* tip;
  D3::Selection t;

  void SetupConfigs(const NKConfig & config){
    //Setup config parameters
    POP_SIZE = config.POP_SIZE();
    MAX_GENS = config.MAX_GENS();
    x_max = MAX_GENS;
  }

  virtual void Setup(){
    EM_ASM({emp["waiting"] = 0});
    D3::Selection * svg = GetSVG();

    //Wrap ncessary callback functions
    JSWrap(tooltip_display, GetID()+"tooltip_display");
    JSWrap(x, GetID()+"x");
    JSWrap(y, GetID()+"y");
    JSWrap(return_x, GetID()+"return_x");
    JSWrap(draw_data, GetID()+"draw_data");


    //Create tool top
    tip = new D3::ToolTip(GetID()+"tooltip_display");

    //Set up scales
    y_scale = new D3::LinearScale();
    x_scale = new D3::LinearScale();
    y_scale->SetDomain(std::array<double, 2>({y_max, y_min}));
    y_scale->SetRange(std::array<double, 2>({y_margin, (double)GetHeight() - axis_width}));
    x_scale->SetDomain(std::array<double, 2>({0, (double)MAX_GENS}));
    x_scale->SetRange(std::array<double, 2>({axis_width, GetWidth()-x_margin}));

    //Set up axes
    x_axis = new D3::Axis<D3::LinearScale>(variables[0]);
    x_axis->SetScale(*x_scale);
    y_axis = new D3::Axis<D3::LinearScale>(variables[1]);
    y_axis->SetScale(*y_scale);
    D3::DrawAxes(*x_axis, *y_axis, *svg);
    make_line = new D3::LineGenerator();

    circles = new D3::Selection(svg->SelectAll("circle"));
  }

  virtual void AnimateStep(emp::vector<double> data_point) {
    data.push_back(std::array<std::array<double, 2>, 1>({data_point[0], data_point[1]}));
    D3::Selection * svg = GetSVG();

    if (data_point[1] > y_max || data_point[1] < y_min
        || data_point[0] > x_max || data_point[0] < x_min) {

      y_max = std::max(data_point[1]*1.2, y_max);
      y_min = std::min(data_point[1]*.8, y_min);
      x_max = std::max(data_point[0]*1.2, x_max);
      x_min = std::min(data_point[0]*.8, x_min);

      t = svg->Transition();
      y_axis->Rescale(y_max, y_min, t);
      x_axis->Rescale(x_min, x_max, t);

      Redraw(t);

      t.Each("end", GetID()+"draw_data");
    } else {
      DrawData(false);
    }
  }

  void Redraw(D3::Selection & s) {
    s.SelectAll(".data-point").SetAttr("cy", GetID()+"y");
    s.SelectAll(".data-point").SetAttr("cx", GetID()+"x");

    EM_ASM_ARGS({
      circle_data = js.objects[$0].selectAll(".data-point").data();
      path_data = [];
      for (iter=0; iter<circle_data.length-1; iter++){
        path_data.push(js.objects[$1]([[emp[Pointer_stringify($3)+"x"](circle_data[iter],0,0), emp[Pointer_stringify($3)+"y"](circle_data[iter],0,0)],
                     [emp[Pointer_stringify($3)+"x"](circle_data[iter+1],0,0), emp[Pointer_stringify($3)+"y"](circle_data[iter+1],0,0)]]));
      }
      js.objects[$0].selectAll(".line-seg").data(path_data);
      js.objects[$2].selectAll(".line-seg").attr("d", function(d){return d;});
    }, GetSVG()->GetID(), make_line->GetID(), s.GetID(), this->GetID().c_str());
  }

  void DrawData(bool backlog = false) {

    //If there's a backlog, then we're only allowed to clear it if this
    //was called recursively or from jacascript (since javascript handles)
    //using this as a callback to asynchronous stuff)
    if (!backlog && data.size() > 1){
      return;
    }

    //We can't draw a line on the first update
    if (prev_data[0][0] >=0 ){
      std::array<std::array<double, 2>, 2> line_data;
      prev_data[0][0] = x(prev_data[0], 0, 0);
      prev_data[0][1] = y(prev_data[0],0 ,0);
      line_data[0] = prev_data[0];

      std::array<std::array<double, 2>, 1> new_data;
      new_data = data[0];
      new_data[0][0] = x(new_data[0],0,0);
      new_data[0][1] = y(new_data[0],0,0);
      line_data[1] = new_data[0];

      D3::Selection line = make_line->DrawShape(line_data, *GetSVG());
      line.SetAttr("fill", "none");
      line.SetAttr("stroke", "green");
      line.SetAttr("stroke-width", 1);
      line.SetAttr("class", "line-seg");
    }

    circles = new D3::Selection(GetSVG()->SelectAll(".data-point").Data(data[0], GetID()+"return_x"));
    D3::Selection enter = circles->EnterAppend("circle");
    enter.SetAttr("cy", GetID()+"y");
    enter.SetAttr("cx", GetID()+"x");
    enter.SetAttr("r", 2);
    enter.SetAttr("class", "data-point");
    enter.SetStyle("fill", "green");
    enter.AddToolTip(*tip);
    prev_data = data[0];
    data.pop_front();

    if (data.size() > 0) {
      DrawData(true);
    }
  }
};
}
}
#endif
