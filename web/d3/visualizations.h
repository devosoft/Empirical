#ifndef VISUALIZATION_UTILS_H
#define VISUALIZATION_UTILS_H

#include "../../web/web_init.h"
#include "../../web/Animate.h"
#include "../../web/JSWrap.h"
#include "../../config/config.h"
#include "../../tools/BitSet.h"
#include "../../tools/Random.h"
#include "../../tools/stats.h"
#include "../../tools/string_utils.h"
#include "../../tools/FunctionSet.h"
#include "../../tools/const.h"

#include "../../web/d3/selection.h"
#include "../../web/d3/scales.h"
#include "../../web/d3/axis.h"
#include "../../web/d3/svg_shapes.h"
#include "../../web/d3/layout.h"

#include <functional>
#include <algorithm>
#include <deque>

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

  emp::vector<std::string> variables;
  FunctionSet<void> pending_funcs;
  bool init = false;

  /// Callback function for drawing data after rescale animation
  std::string draw_data_callback = "";

  virtual void Setup(){;}

  /// @cond TEMPLATES
  template <typename T>
  typename emp::sfinae_decoy<void, decltype(&T::operator())>::type
  SetDrawCallback(T func) {
    emp::JSWrap(func, GetID()+"draw_data_callback");
    draw_data_callback = GetID()+"draw_data_callback";
  }
  /// @endcond

  /// This callback function will be called every time data is done being drawn.
  /// Can be a string represnting the name of a function in Javascript (can be in the current
  /// window, the emp namespace, or the d3 namespace)
  void SetDrawCallback(std::string func) {
    draw_data_callback = func;
  }

  void CallDrawCallback() {
    EM_ASM_ARGS({window["emp"][Pointer_stringify($0)]()}, draw_data_callback.c_str());
  }

  /*
  virtual void AnimateStep(...){;}
  virtual void AnimateStep(emp::vector<double>){;}
  virtual void AnimateStep(int update, emp::vector<double>){;}
  virtual void AnimateStep(int parent, int offspring){;}
  */
};

class DotPlot : public D3Visualization {
private:
  double margin = 10;
  double axis_width = 40;
  double value_growth_margin = 1.5;
  double value_loss_margin = .8;
public:
  D3::LinearScale * x_scale;
  D3::LinearScale * y_scale;
  D3::Axis<D3::LinearScale> * ax;
  D3::Selection * circles;
  D3::ToolTip * tip;

  std::function<double(double, int, int)> scaled_d = [&](double d, int i, int k){
      return y_scale->ApplyScale(d);
  };

  std::function<double(double, int, int)> scaled_i = [&](double d, int i, int k){
      return x_scale->ApplyScale(i);
  };

  DotPlot(int w=500, int h=500) : D3Visualization(w, h){;}

  void Setup(){

    D3::Selection * svg = GetSVG();

    double lowest = 10;//*(std::min_element(values.begin(), values.end()));
    double highest = 20;//*(std::max_element(values.begin(), values.end()));
    double x_max = 100;

    //Set up scales
    y_scale = new D3::LinearScale();
    x_scale = new D3::LinearScale();
    y_scale->SetDomain(std::array<double, 2>({highest*value_growth_margin, lowest*value_loss_margin}));
    y_scale->SetRange(std::array<double, 2>({margin, GetHeight() - margin}));
    x_scale->SetDomain(std::array<double, 2>({0, x_max}));
    x_scale->SetRange(std::array<double, 2>({axis_width, GetHeight()-margin}));

    //Set up axis
    ax = new D3::Axis<D3::LinearScale>();
    ax->SetScale(*y_scale);
    ax->SetOrientation("right");
    ax->Draw(*svg);

    //Make callback functions
    JSWrap(scaled_d, GetID()+"scaled_d");
    JSWrap(scaled_i, GetID()+"scaled_i");
    this->pending_funcs.Run();
  }

  virtual void AddDataPoint(int update, emp::vector<double> values){
      //Draw circles that represent values
      circles = new D3::Selection(GetSVG()->SelectAll("circle").Data(values));
      circles->EnterAppend("circle");
      circles->ExitRemove();
      circles->SetAttr("r", 5);
      circles->SetAttr("cx", GetID()+"scaled_i");
      circles->SetAttr("cy", GetID()+"scaled_d");

     // circles->AddToolTip(tip);

      circles = new D3::Selection(circles->Data(values));
      circles->MakeTransition().SetAttr("cy", GetID()+"scaled_d");
  }

};

template <typename DATA_TYPE = std::array<double, 2> >
class LineGraph : public D3Visualization {
protected:
  // Formatting constants
  double y_margin = 10;
  double x_margin = 30;
  double axis_width = 60;
  double y_min = 1000;
  double y_max = 0;
  double x_min = 1000;
  double x_max = 0;

  // Components of the graph
  D3::LinearScale * x_scale;
  D3::LinearScale * y_scale;
  D3::Axis<D3::LinearScale> * x_axis;
  D3::Axis<D3::LinearScale> * y_axis;
  D3::LineGenerator * line_gen;
  D3::ToolTip * tip;
  // In case we need to store a dataset
  D3::CSVDataset * dataset;

  // Containers to keep track of data state
  std::deque<DATA_TYPE> data;
  DATA_TYPE prev_data  = {-1,-1};

  // Callback functions for accessing and scaling data

  //Callback function for getting unscaled x value of data point (used as key function for data binding)
  std::function<double(DATA_TYPE)> return_x = [](DATA_TYPE d){ return d[0]; };

  //Callback function for getting unscaled x value of data point (used as key function for data binding)
  std::function<double(DATA_TYPE)> return_y = [](DATA_TYPE d){ return d[1]; };

  //Callback function for taking a datapoint and getting appropriately scaled y val
  std::function<double(DATA_TYPE)> y = [this](DATA_TYPE d){
      return y_scale->ApplyScale(this->return_y(d));
  };

  //Callback function for taking a datapoint and getting appropriately scaled x val
  std::function<double(DATA_TYPE)> x = [this](DATA_TYPE d){
      return x_scale->ApplyScale(this->return_x(d));
  };

  std::function<void()> draw_data = [this]() {
    DrawData(true);
  };

public:

  LineGraph(std::string x_var="", std::string y_var="", int w=800, int h=400) : D3Visualization(w, h){
    this->variables.push_back(x_var);
    this->variables.push_back(y_var);
  }

  ~LineGraph() {
    delete x_scale;
    delete y_scale;
    delete x_axis;
    delete y_axis;
    delete line_gen;
    delete tip;
    delete dataset;
  }

  /// Initializes the graph. This function is called automatically when the
  /// emp::Document this has been added to is ready.
  virtual void Setup(){
    D3::Selection * svg = GetSVG();

    //Wrap ncessary callback functions
    JSWrap(draw_data, GetID()+"draw_data");

    //Create tool tip
    tip = new D3::ToolTip([this](DATA_TYPE d) {return D3::FormatFunction(".2f")(return_y(d));});
    GetSVG()->SetupToolTip(*tip);

    //Set up scales
    y_scale = new D3::LinearScale();
    x_scale = new D3::LinearScale();
    y_scale->SetDomain(std::array<double, 2>({y_max, y_min}));
    y_scale->SetRange(std::array<double, 2>({y_margin, (double)GetHeight() - axis_width}));
    x_scale->SetDomain(std::array<double, 2>({x_min,x_max}));
    x_scale->SetRange(std::array<double, 2>({axis_width, GetWidth()-x_margin}));

    //Set up axes
    x_axis = new D3::Axis<D3::LinearScale>(variables[0]);
    x_axis->SetScale(*x_scale);
    y_axis = new D3::Axis<D3::LinearScale>(variables[1]);
    y_axis->SetScale(*y_scale);
    D3::DrawAxes(*x_axis, *y_axis, *svg);

    line_gen = new D3::LineGenerator();

    // Set up accessors (for retriving coords from data points)
    SetXAccessor(return_x);
    SetYAccessor(return_y);

    dataset = new D3::CSVDataset();

    // In case functions were called before initilization
    init = true;
    this->pending_funcs.Run();
  }

  D3::LinearScale * GetXScale() {return x_scale;}
  D3::LinearScale * GetYScale() {return y_scale;}
  D3::Axis<D3::LinearScale> * GetXAxis() {return x_axis;}
  D3::Axis<D3::LinearScale> * GetYAxis() {return y_axis;}
  D3::LineGenerator * GetLineGenerator() {return line_gen;}
  D3::CSVDataset * GetDataset() {return dataset;}
  D3::ToolTip * GetToolTip() {return tip;}
  std::function<double(DATA_TYPE)> GetXAccessor() {return return_x;}
  std::function<double(DATA_TYPE)> GetYAccessor() {return return_y;}
  std::function<double(DATA_TYPE)> GetScaledX() {return x;}
  std::function<double(DATA_TYPE)> GetScaledY() {return y;}

  void SetXScale(D3::LinearScale * scale) {x_scale = scale;}
  void SetYScale(D3::LinearScale * scale) {y_scale = scale;}
  void SetXAxis(D3::Axis<D3::LinearScale> * ax) {x_axis = ax;}
  void SetYAxis(D3::Axis<D3::LinearScale> * ax) {y_axis = ax;}
  void SetLineGenerator(D3::LineGenerator * line) {line_gen = line;}
  void SetDataset(D3::CSVDataset * d) {dataset = d;}

  void SetTooltipFunction(std::string func) {
    tip->SetHtml(func);
  }

  /// @cond TEMPLATES
  template <typename T>
  typename emp::sfinae_decoy<void, decltype(&T::operator())>::type
  SetTooltipFunction(T func) {
    tip->SetHtml(func);
  }
  /// @endcond


  /// @cond TEMPLATES
  template <typename T>
  typename emp::sfinae_decoy<void, decltype(&T::operator())>::type
  SetXAccessor(T func) {
    return_x = func;
    JSWrap(return_x, GetID()+"return_x");
    JSWrap(x, GetID()+"x");
    line_gen->SetX(GetID()+"x");
  }
  /// @endcond

  /// Set the function that is used to get the x variable from within a DATA_TYPE object.
  /// For instance, the default accessor assumes that DATA_TYPE is an array with two elements
  /// (x, y): `[](DATA_TYPE d) {return d[0];}`
  /// This accessor will be fed into the x_scale to get x values scaled to the plotting area.
  /// Can be a string represnting the name of a function in Javascript (can be in the current
  /// window, the emp namespace, or the d3 namespace)

  void SetXAccessor(std::string func) {
    return_x = [func](DATA_TYPE d){
      emp::StoreReturn(d);
      return EM_ASM_DOUBLE({
        var func_string = Pointer_stringify($0);
        if (typeof window[func_string] === function) {
          func_string = window[func_string];
        } else if (typeof window["emp"][func_string] === function) {
          func_string = window["emp"][func_string];
        } else if (typeof window["d3"][func_string] === function) {
          func_string = window["d3"][func_string];
        }
        return func_string(emp_i.cb_return);
      }, func.c_str());
    };

    JSWrap(return_x, GetID()+"return_x");
    JSWrap(x, GetID()+"x");
    line_gen->SetX(GetID()+"x");
  }

  /// @cond TEMPLATES
  template <typename T>
  typename emp::sfinae_decoy<void, decltype(&T::operator())>::type
  SetYAccessor(T func) {
    return_y = func;
    JSWrap(return_y, GetID()+"return_y");
    JSWrap(y, GetID()+"y");
    line_gen->SetY(GetID()+"y");
  }
  /// @endcond

  /// Set the function that is used to get the y variable from within a DATA_TYPE object.
  /// For instance, the default accessor assumes that DATA_TYPE is an array with two elements
  /// (x, y): `[](DATA_TYPE d) {return d[1];}`
  /// This accessor will be fed into the y_scale to get y values scaled to the plotting area.
  /// Can be a string represnting the name of a function in Javascript (can be in the current
  /// window, the emp namespace, or the d3 namespace)
  void SetYAccessor(std::string func) {
    return_y = [func](DATA_TYPE d){
      emp::StoreReturn(d);
      return EM_ASM_DOUBLE({
        var func_string = Pointer_stringify($0);
        if (typeof window[func_string] === function) {
          func_string = window[func_string];
        } else if (typeof window["emp"][func_string] === function) {
          func_string = window["emp"][func_string];
        } else if (typeof window["d3"][func_string] === function) {
          func_string = window["d3"][func_string];
        }

        return func_string(emp_i.cb_return);
      }, func.c_str());
    };
    JSWrap(return_y, GetID()+"return_y");
    JSWrap(y, GetID()+"y");
    line_gen->SetY(GetID()+"y");
  }

  /// Draw points and lines for data in this object's dataset object
  // Needs to be a std::function object or else JSWrap complains
  std::function<void()> DrawPointsFromDataset = [this]() {

    // Adjust axes
    x_min = std::min(EM_ASM_DOUBLE({
        return d3.min(js.objects[$0], window["emp"][Pointer_stringify($1)+"return_x"]);
    }, dataset->GetID(), this->GetID().c_str()), x_min);

    x_max = std::max(EM_ASM_DOUBLE({
        return d3.max(js.objects[$0], window["emp"][Pointer_stringify($1)+"return_x"]);
    }, dataset->GetID(), this->GetID().c_str()), x_max);

    y_min = std::min(EM_ASM_DOUBLE({
        return d3.min(js.objects[$0], window["emp"][Pointer_stringify($1)+"return_y"]);
    }, dataset->GetID(), this->GetID().c_str()), y_min);

    y_max = std::max(EM_ASM_DOUBLE({
        return d3.max(js.objects[$0], window["emp"][Pointer_stringify($1)+"return_y"]);
    }, dataset->GetID(), this->GetID().c_str()), y_max);

    y_scale->SetDomain(std::array<double, 2>({y_max, y_min}));
    x_scale->SetDomain(std::array<double, 2>({x_min, x_max}));

    y_axis->Rescale(y_max, y_min, *GetSVG());
    x_axis->Rescale(x_min, x_max, *GetSVG());

    // Bind data and update graphics
    D3::Selection update = GetSVG()->SelectAll(".data-point")
                                    .Data(*dataset, GetID()+"return_x");
    update.EnterAppend("circle");
    update.SetAttr("cy", GetID()+"y")
          .SetAttr("cx", GetID()+"x")
          .SetAttr("r", 2)
          .SetAttr("class", "data-point")
          .BindToolTipMouseover(*tip);

    D3::Selection line = line_gen->DrawShape(*dataset, *GetSVG());
    line.SetAttr("fill", "none")
        .SetAttr("stroke-width", 1)
        .SetAttr("stroke", "black")
        .SetAttr("class", "line-seg");

    // Set prev_data appropriately
    dataset->GetLastRow(prev_data);
    CallDrawCallback();
  };

  /// Load data from the file at [filename]. Expected to be a CSV dataset
  void LoadDataFromFile(std::string filename) {
    emp::JSWrap(DrawPointsFromDataset, "draw");

    if (this->init) {
      dataset->LoadDataFromFile(filename, "draw", false);

    } else {
      this->pending_funcs.Add([this, filename](){
          dataset->LoadDataFromFile(filename, "draw", false);
      });
    }
  }

  /// Smoothly (i.e. with animation) add data_point to the graph
  void AddDataPoint(DATA_TYPE data_point) {
    data.push_back(data_point);
    D3::Selection * svg = GetSVG();

    if (data_point[1] > y_max || data_point[1] < y_min
        || data_point[0] > x_max || data_point[0] < x_min) {
      y_max = std::max(data_point[1]*1.2, y_max);
      y_min = std::min(data_point[1]*.8, y_min);
      x_max = std::max(data_point[0]*1.2, x_max);
      x_min = std::min(data_point[0]*.8, x_min);

      if (y_min == y_max) {
        y_min -= .2;
        y_max += .2;
      }

      if (x_min == x_max) {
        x_min -= .2;
        x_max += .2;
      }

      D3::Transition t = svg->MakeTransition();
      y_axis->Rescale(y_max, y_min, t);
      x_axis->Rescale(x_min, x_max, t);
      t.Each("end", GetID()+"draw_data");
      Redraw(t);
      CallDrawCallback();

    } else {
      DrawData(false);
    }
  }

  /// Redraws all data on the given selection or transition, which should contain an SVG canvas.
  /// Useful if you've adjusted scales.
  template <typename T>
  void Redraw(D3::SelectionOrTransition<T> & s) {
    s.SelectAll(".data-point").SetAttr("cy", GetID()+"y");
    s.SelectAll(".data-point").SetAttr("cx", GetID()+"x");

    EM_ASM_ARGS({
      circle_data = js.objects[$0].selectAll(".data-point").data();
      js.objects[$0].selectAll(".line-seg").data([circle_data]);
      js.objects[$2].selectAll(".line-seg").attr("d", function(d){return js.objects[$1](d);});
    }, GetSVG()->GetID(), line_gen->GetID(), s.GetID());
  }

  void DrawData(bool backlog = false) {

    //If there's a backlog, then we're only allowed to clear it if this
    //was called recursively or from javascript (since javascript handles)
    //using this as a callback to asynchronous stuff)
    // if ((!backlog && data.size() > 1) || data.size() == 0){
    //   //EM_ASM_ARGS({window["emp"][Pointer_stringify($0)]()}, draw_data_callback.c_str());
    //   return;
    // }
    //
    // //We can't draw a line on the first update
    // if (prev_data[0] >= 0 ){
    //   std::array<DATA_TYPE, 2> line_data;
    //   line_data[0] = prev_data;
    //   line_data[1] = data[0];
    //
    //   D3::Selection line = line_gen->DrawShape(line_data, *GetSVG());
    //   line.SetAttr("fill", "none");
    //   line.SetAttr("stroke-width", 1);
    //   line.SetAttr("stroke", "black");
    //   line.SetAttr("class", "line-seg");
    // }

    // If it isn't nested, D3 will think it's 2 separate points
    std::array<DATA_TYPE, 1> new_point = {data[0]};

    D3::Selection enter = GetSVG()->SelectAll(".data-point").Data(new_point, GetID()+"return_x").EnterAppend("circle");
    enter.SetAttr("cy", GetID()+"y");
    enter.SetAttr("cx", GetID()+"x");
    enter.SetAttr("r", 2);
    enter.SetAttr("class", "data-point");
    enter.BindToolTipMouseover(*tip);
    prev_data = data[0];
    data.pop_front();

    // if (data.size() > 0) {
    //   DrawData(true);
    // }
    //
    // // Call callback
    // if (data.size() == 0) {
    //   CallDrawCallback();
    // }
  }

  void Clear() {
    data.clear();
    GetSVG()->SelectAll(".data-point").Data({}).ExitRemove();
    GetSVG()->SelectAll(".line-seg").Data({}).ExitRemove();
    y_axis->Rescale(0, 1000, *(GetSVG()));
    x_axis->Rescale(0, 0, *(GetSVG()));
    y_min = 1000;
    y_max = 0;
    x_min = 0;
    x_max = 0;
    prev_data  = {-1,-1};
  }
};

struct TreeNode {
  EMP_BUILD_INTROSPECTIVE_TUPLE( double, x,
                                 double, y,
                                 int, name,
                                 int, parent,
                                 int, depth
                              )
};

template <typename NODE = TreeNode>
class TreeVisualization : public D3Visualization {
protected:
  double y_margin = 10;
  double x_margin = 30;

  struct TreeEdge {
    EMP_BUILD_INTROSPECTIVE_TUPLE( NODE, source,
                                   NODE, target)
  };

  D3::ToolTip * tip;

  void InitializeVariables() {
    JSWrap(color_fun_node, GetID()+"color_fun_node");
    JSWrap(color_fun_link, GetID()+"color_fun_link");
    data = new D3::JSONDataset();
    tip = new D3::ToolTip([](NODE d, int i){return "Name: " + to_string(d.name());});
    GetSVG()->Move(0,0);
    data->Append(std::string("{\"name\": 0, \"parent\": \"null\", \"children\" : []}"));
    tree.SetDataset(data);
    tree.SetSize(GetHeight(), GetWidth());
  }

  std::function<std::string(NODE, int)> color_fun_node = [](NODE d, int i){
    return "black";
  };

  std::function<std::string(TreeEdge, int)> color_fun_link = [this](TreeEdge d, int i){
    return "black";
  };

public:
  D3::TreeLayout<NODE> tree;
  D3::JSONDataset * data;

  TreeVisualization(int width, int height) : D3Visualization(width, height){variables.push_back("Persist");}

  virtual void Setup() {
    InitializeVariables();
    this->init = true;
    this->pending_funcs.Run();
  }

  void SetTooltipFunction(std::string func) {
    tip->SetHtml(func);
  }

  /// @cond TEMPLATES
  template <typename T>
  typename emp::sfinae_decoy<void, decltype(&T::operator())>::type
  SetTooltipFunction(T func) {
    tip->SetHtml(func);
  }
  /// @endcond

  D3::TreeLayout<NODE> * GetTreeLayout() {return &tree;}
  D3::JSONDataset * GetDataset() {return data;}
  D3::ToolTip * GetToolTip() {return tip;}

  void SetDataset(D3::JSONDataset * d) {data = d;}

  void LoadDataFromFile(std::string filename) {
    if (this->init) {
      data->LoadDataFromFile(filename, [this](){DrawTree();});
    } else {
      this->pending_funcs.Add([this, filename](){
          data->LoadDataFromFile(filename, [this](){
              DrawTree();
          });
      });
    }
  }

  void AddDataPoint(int parent, int child){
    std::string child_json = std::string("{\"name\":" + to_string(child) + ", \"parent\":" + to_string(parent) + ", \"children\":[]}");
    data->AppendNested(child_json);
    DrawTree();
  }

  void DrawTree() {
    D3::Selection nodeEnter = tree.GenerateNodesAndLinks(*GetSVG())[0];
    nodeEnter.Append("circle").SetAttr("r", 2).AddToolTip(*tip);
    GetSVG()->SelectAll("g.node").SelectAll("circle").SetStyle("fill", GetID()+"color_fun_node");
    GetSVG()->SelectAll(".link").SetStyle("stroke", GetID()+"color_fun_link");
    CallDrawCallback();
  }

};

template <typename NODE = TreeNode>
class TreeVisualizationReplacement : public TreeVisualization<NODE> {
protected:
  using TreeVisualization<NODE>::GetID;
  using TreeVisualization<NODE>::GetSVG;
  using TreeVisualization<NODE>::InitializeVariables;
  using TreeVisualization<NODE>::DrawTree;
  using TreeVisualization<NODE>::tip;
  using TreeVisualization<NODE>::variables;
  using TreeVisualization<NODE>::data;

public:

  int next_pos;
  int next_parent = 0;
  int next_child;
  D3::JSObject possible_parents;

  TreeVisualizationReplacement(int width, int height) : TreeVisualization<NODE>(width, height){variables.push_back("Persist");}

  virtual void Setup() {
    InitializeVariables();

    EM_ASM_ARGS({
      js.objects[$0] = [js.objects[$1][0]];
    }, possible_parents.GetID(), data->GetID());

    this->init = true;
    this->pending_funcs.Run();
  }

  void AddDataPoint(int parent, int child){
    std::string child_json = std::string("{\"name\":" + to_string(child) + ", \"parent\":" + to_string(parent) + ", \"children\":[]}");
    int pos = data->AppendNestedFromList(child_json, possible_parents);
    (void) pos;

    EM_ASM_ARGS({
        while (js.objects[$0].length < $1 + 1) {
          js.objects[$0].push(-1);
        }
        js.objects[$0][$1] = js.objects[$0][$2].children[js.objects[$0][$2].children.length-1];
    }, possible_parents.GetID(), next_pos, pos);
    DrawTree();
  }

  void RecordPlacement(int pos) {
    next_pos = pos + 1;
    AddDataPoint(next_parent, next_child);
  }

  void RecordParent(int parent, int child) {
    next_parent = parent;
    next_child = child;
  }
};

struct SpatialGridTreeNode {
  EMP_BUILD_INTROSPECTIVE_TUPLE( double, x,
                                 double, y,
                                 int, name,
                                 int, parent,
                                 int, depth,
                                 int, loc
                              )
};

template <typename NODE = SpatialGridTreeNode, template <typename> class TREE_TYPE = TreeVisualization>
class SpatialGridTreeVisualization : public TREE_TYPE<NODE> {
protected:
  using TREE_TYPE<NODE>::GetID;
  using TREE_TYPE<NODE>::GetSVG;
  using TREE_TYPE<NODE>::InitializeVariables;
  using TREE_TYPE<NODE>::DrawTree;
  using TREE_TYPE<NODE>::tip;

public:

  const int grid_width = 10;
  const int grid_height = 10;
  const int legend_cell_size = 15;

  D3::Selection legend;

  struct TreeEdge {
    EMP_BUILD_INTROSPECTIVE_TUPLE( NODE, source,
                                   NODE, target)
  };

  struct LegendNode {
    EMP_BUILD_INTROSPECTIVE_TUPLE( int, loc)
  };

  std::function<std::string(NODE, int)> color_fun_node = [this](NODE d, int i){
    if (d.loc() < 0) {
      return std::string("black");
    }

    double x = (d.loc() % grid_width) - grid_width/2;
    double y = (d.loc() / grid_width) - grid_height/2;

    double r = sqrt(emp::pow(x,2)+emp::pow(y,2)) / sqrt(emp::pow(grid_width,2)+emp::pow(grid_height,2));
    (void) r;

    //atan2 takes sign into account
    double theta = atan2(y, x)*(180/emp::PI);
    (void) theta;

    char * color = (char *) EM_ASM_INT({
        var text = d3.hcl($1, 150, $0*175).toString();
	    var buffer = Module._malloc(text.length+1);
	    Module.writeStringToMemory(text, buffer);
	    return buffer;
    }, r, theta);

    std::string result = std::string(color);
    free(color);
    return result;
  };

  std::function<std::string(NODE, int)> dark_color_fun = [this](NODE d, int i){
    if (d.loc() < 0) {
      return std::string("black");
    }

    double x = (d.loc() % grid_width) - grid_width/2;
    double y = (d.loc() / grid_width) - grid_height/2;

    double r = sqrt(emp::pow(x,2)+emp::pow(y,2)) / sqrt(emp::pow(grid_width,2)+emp::pow(grid_height,2));
    (void) r;

    //atan2 takes sign into account
    double theta = atan2(y, x)*(180/emp::PI);
    (void) theta;

    char * color = (char *) EM_ASM_INT({
        var text = d3.hcl($1, 150, $0*175).darker().toString();
      var buffer = Module._malloc(text.length+1);
      Module.writeStringToMemory(text, buffer);
      return buffer;
    }, r, theta);

    std::string result = std::string(color);
    free(color);
    return result;
  };

  std::function<std::string(TreeEdge, int)> color_fun_link = [this](TreeEdge d, int i){
    return this->color_fun_node(d.source(), i);
  };

  std::function<int(LegendNode)> get_x = [this](LegendNode d) {
    return legend_cell_size*(d.loc() % grid_width);
  };

  std::function<int(LegendNode)> get_y = [this](LegendNode d) {
    return legend_cell_size*(d.loc() / grid_width);
  };

  std::function<void(LegendNode, int, int)> legend_mouseover = [this](LegendNode d, int i, D3::Selection s) {
    legend.SelectAll("rect").Filter([d](LegendNode in_data){return d.loc() != in_data.loc();}).SetClassed("faded", true);
    GetSVG()->SelectAll(".node").Filter([d](LegendNode in_data){return d.loc() != in_data.loc();}).SetClassed("faded", true);
    EM_ASM_ARGS({emp.filter_fun = function(d){return d.source.loc != $0;}}, d.loc());
    GetSVG()->SelectAll(".link").Filter("filter_fun").SetClassed("faded", true);
  };

  std::function<void(LegendNode, int, int)> legend_mouseout = [this](LegendNode d, int i, D3::Selection s) {
    legend.SelectAll("rect")
          .Filter([d](LegendNode in_data){return d.loc() != in_data.loc();})
          .SetClassed("faded", false);
    GetSVG()->SelectAll(".node")
             .Filter([d](LegendNode in_data){return d.loc() != in_data.loc();})
             .SetClassed("faded", false);

    EM_ASM_ARGS({emp.filter_fun = function(d){return d.source.loc != $0}}, d.loc());
    GetSVG()->SelectAll(".link").Filter("filter_fun").SetClassed("faded", false);
  };

  SpatialGridTreeVisualization(int width, int height) : TreeVisualization<NODE>(width, height){;}

  virtual void Setup() {
    TREE_TYPE<NODE>::Setup();
    JSWrap(color_fun_node, GetID()+"color_fun_node");
    JSWrap(dark_color_fun, GetID()+"dark_color_fun");
    JSWrap(color_fun_link, GetID()+"color_fun_link");
    JSWrap(legend_mouseover, GetID()+"legend_mouseover");
    JSWrap(legend_mouseout, GetID()+"legend_mouseout");
    JSWrap(get_x, GetID()+"get_x");
    JSWrap(get_y, GetID()+"get_y");
    tip.SetHtml(D3::ToolTip([this](NODE d){
                    return "ID: " + to_string(d.name()) + ", Pos: ("
                           + to_string(d.loc()% grid_width) + ", "
                           + to_string(d.loc()/grid_width) + ")";
                }));

    legend = D3::Select("body").Append("svg");

    legend.SetAttr("x", 1000).SetAttr("y", 0).SetAttr("width", legend_cell_size*grid_width).SetAttr("height", legend_cell_size*grid_height);
    legend.SetStyle("position", "fixed").SetStyle("right", "10px").SetStyle("top", "10px");//.SetStyle("width", "10%");//.SetStyle("height", "10%");
    emp::vector<LegendNode> legend_data(grid_width*grid_height);
    for (int i = 0; i < grid_width*grid_height; ++i) {
      legend_data[i].loc(i);
    }

    legend.SelectAll("rect").Data(legend_data)
                            .EnterAppend("rect")
                            .SetStyle("fill", GetID()+"color_fun_node")
                            .SetStyle("stroke", GetID()+"color_fun_node")
                            .SetStyle("stroke-width", 1)
                            .SetAttr("width", legend_cell_size)
                            .SetAttr("height", legend_cell_size)
                            .SetAttr("x", GetID()+"get_x")
                            .SetAttr("y", GetID()+"get_y")
                            .On("mouseover", GetID()+"legend_mouseover")
                            .On("mouseout", GetID()+"legend_mouseout");
  }

};

}
}
#endif
