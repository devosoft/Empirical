/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018
 *
 *  @file visualizations.hpp
 *  @brief Tools to build D3 visualizations.
 */

#ifndef EMP_WEB_D3_VISUALIZATIONS_HPP_INCLUDE
#define EMP_WEB_D3_VISUALIZATIONS_HPP_INCLUDE

#include <algorithm>
#include <deque>
#include <functional>

#include "../../bits/BitSet.hpp"
#include "../../config/config.hpp"
#include "../../functional/FunctionSet.hpp"
#include "../../math/constants.hpp"
#include "../../math/Random.hpp"
#include "../../math/stats.hpp"
#include "../../tools/string_utils.hpp"

#include "../Animate.hpp"
#include "../init.hpp"
#include "../JSWrap.hpp"

#include "axis.hpp"
#include "histogram.hpp"
#include "layout.hpp"
#include "scales.hpp"
#include "selection.hpp"
#include "svg_shapes.hpp"
#include "visual_elements.hpp"

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
      D3::Selection svg;

      D3VisualizationInfo(D3Visualization * parent, const std::string & in_id="") : internal::WidgetInfo(in_id) {
        this->parent = parent;
      }

      D3VisualizationInfo(const D3VisualizationInfo &) = delete;
      D3VisualizationInfo & operator=(const D3VisualizationInfo &) = delete;
      virtual ~D3VisualizationInfo(){;}

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
          svg = D3::Selection("#"+id);
          parent->Setup();
        }
      }

    public:
      virtual std::string GetType() override {return "D3VisualizationInfo";}

  };

  // Get a properly cast version of info.
  D3VisualizationInfo * Info() { return (D3VisualizationInfo *) info; }
  const D3VisualizationInfo * Info() const { return (D3VisualizationInfo *) info; }

  D3Visualization(D3VisualizationInfo * in_info) : WidgetFacet(in_info) { ; }

  double axis_width = 60;
  double y_margin = 10;
  double x_margin = 30;

public:

  D3Visualization(int w, int h, const std::string & in_id="")
        : WidgetFacet(in_id) {
      #ifdef __EMSCRIPTEN__
      n_objects();
      #endif

      info = new D3VisualizationInfo(this, in_id);
      Info()->width = w;
      Info()->height = h;
      MAIN_THREAD_EM_ASM({window["emp"]["__default_draw_data_callback"] =
              function(){;};});
  }

  D3Visualization(const D3Visualization & in) : WidgetFacet(in) { ; }
  D3Visualization(const Widget & in) : WidgetFacet(in) { emp_assert(in.IsD3Visualization()); }
  virtual ~D3Visualization() { ; }

  using INFO_TYPE = D3VisualizationInfo;

  int GetWidth() const { return Info()->width; }
  int GetHeight() const { return Info()->height; }
  D3::Selection * GetSVG() {return &(Info()->svg);}
  std::string GetID() {return Info()->id;}

  emp::vector<std::string> variables;
  FunctionSet<void()> pending_funcs;
  bool init = false;

  /// Callback function for drawing data after rescale animation
  std::string draw_data_callback = "__default_draw_data_callback";

  virtual void Setup(){}

  /// @cond TEMPLATES
  template <typename T>
  emp::sfinae_decoy<void, decltype(&T::operator())>
  SetDrawCallback(T func) {
    emp::JSWrap(func, GetID()+"draw_data_callback");
    draw_data_callback = GetID()+"draw_data_callback";
  }
  /// @endcond

  /// This callback function will be called every time data is done being drawn.
  /// Can be a string representing the name of a function in Javascript (can be in the current
  /// window, the emp namespace, or the d3 namespace)
  void SetDrawCallback(std::string func) {
    draw_data_callback = func;
  }

  void CallDrawCallback() {
    MAIN_THREAD_EM_ASM({window["emp"][UTF8ToString($0)]()}, draw_data_callback.c_str());
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
  D3::Selection circles;
  D3::ToolTip * tip;

  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  std::function<double(double, int, int)> scaled_d = [&](double d, int i, int k){
      return y_scale->ApplyScale(d);
  };

  std::function<double(double, int, int)> scaled_i = [&](double d, int i, int k){
      return x_scale->ApplyScale(i);
  };
  #endif // DOXYGEN_SHOULD_SKIP_THIS

  DotPlot(int w=500, int h=500) : D3Visualization(w, h){;}

  void Setup(){

    D3::Selection * svg = GetSVG();

    double lowest = 10;//*(std::min_element(values.begin(), values.end()));
    double highest = 20;//*(std::max_element(values.begin(), values.end()));
    double x_max = 100;

    //Set up scales
    y_scale = new D3::LinearScale();
    x_scale = new D3::LinearScale();
    y_scale->SetDomain(emp::array<double, 2>{{highest*value_growth_margin, lowest*value_loss_margin}});
    y_scale->SetRange(emp::array<double, 2>{{margin, GetHeight() - margin}});
    x_scale->SetDomain(emp::array<double, 2>{{0, x_max}});
    x_scale->SetRange(emp::array<double, 2>{{axis_width, GetHeight()-margin}});

    //Set up axis
    ax = new D3::Axis<D3::LinearScale>("right");
    ax->SetScale(*y_scale);
    ax->Draw(*svg);

    //Make callback functions
    JSWrap(scaled_d, GetID()+"scaled_d");
    JSWrap(scaled_i, GetID()+"scaled_i");
    this->pending_funcs.Run();
  }

  virtual void AddDataPoint(int update, emp::vector<double> & values){
      //Draw circles that represent values
      circles = D3::Selection(GetSVG()->SelectAll("circle").Data(values));
      circles.ExitRemove();
      circles = circles.EnterAppend("circle").Merge(circles);
      circles.SetAttr("r", 5);
      circles.SetAttr("cx", GetID()+"scaled_i");
      circles.SetAttr("cy", GetID()+"scaled_d");

     // circles->AddToolTip(tip);

      circles = D3::Selection(circles.Data(values));
      circles.MakeTransition().SetAttr("cy", GetID()+"scaled_d");
  }

};

class HeatMap : public D3Visualization {
public:

    int grid_width = 10;
    int grid_height = 10;
    int cell_size = 15;
    D3::TileGrid<> grid;

    virtual void Setup() {
        grid = D3::TileGrid<>(grid_width, grid_height, cell_size, *GetSVG());
    }
};

template <typename DATA_TYPE = int, typename X_SCALE_TYPE = D3::LinearScale, typename Y_SCALE_TYPE = D3::LinearScale>
class HistogramChart : public D3Visualization {
protected:

    D3::Axis<X_SCALE_TYPE> x_ax;
    D3::Axis<Y_SCALE_TYPE> y_ax;

    D3::Histogram h;

    DATA_TYPE x_min = 100;
    DATA_TYPE x_max = 0;
    DATA_TYPE y_max = 10;


public:
    HistogramChart(std::string x_var, int w=800, int h=400) :
            D3Visualization(w, h),
            x_ax("bottom", x_var),
            y_ax("left", "Frequency"){;}

    virtual void Setup() {
        x_ax.GetScale().SetRangeRound(axis_width, GetWidth()-x_margin);
        x_ax.GetScale().SetDomain(x_min, x_max);
        h.SetDomain(x_min, x_max);
        y_ax.GetScale().SetRange(GetHeight() - axis_width, y_margin);
        y_ax.GetScale().SetDomain(0, y_max);
        D3::DrawAxes(x_ax, y_ax, *GetSVG());
    }


    void DrawData(emp::vector<DATA_TYPE> & data) {
        X_SCALE_TYPE& x = x_ax.GetScale();
        Y_SCALE_TYPE& y = y_ax.GetScale();
        double new_x_min = *std::min_element(data.begin(), data.end());
        double new_x_max = *std::max_element(data.begin(), data.end());
        bool rescale = false;

        if (new_x_min < x_min || (new_x_min - x_min) > .5*(new_x_max - new_x_min)) {
            x_min = new_x_min - (new_x_max-new_x_min)*.05;
            rescale = true;
        }
        if (new_x_max > x_max || (x_max - new_x_max) > .5*(new_x_max - new_x_min)) {
            x_max = new_x_max + (new_x_max-new_x_min)*.05;
            rescale = true;
        }

        D3::Transition t = GetSVG()->MakeTransition();
        t.SetDuration(500);
        if (rescale) {
            x.SetDomain(x_min, x_max);
            h.Domain(x_min, x_max);
            x_ax.Rescale(x_min, x_max, t);
        }

        D3::Dataset bin_data = h.Call(data);

        double new_y_max = bin_data.Max([](D3::HistogramBin d){return d.length();});

        if (new_y_max > y_max) {
            y_max = new_y_max * 1.2;
            y_ax.Rescale(0, y_max, t);
        }

        D3::Selection bins = GetSVG()->SelectAll(".bar").Data(bin_data);
        D3::Selection enter = bins.Enter().Append("rect")
                                          .SetAttr("class", "bar")
                                          .SetAttr("x", (GetWidth() - axis_width)/2)
                                          .SetAttr("y", GetHeight() - axis_width);
        bins.ExitRemove();
        bins.Merge(enter).MakeTransition(t).SetAttr("x", [this, &x](D3::HistogramBin d){
                                      return x.ApplyScale(d.x0());
                          })
                         .SetAttr("y", [this, &y](D3::HistogramBin d){
                                                        return y.ApplyScale(d.length());
                          })
                          .SetAttr("width", [this, &x](D3::HistogramBin d){
                               return x.ApplyScale(d.x1()) - x.ApplyScale(d.x0()) - 1;
                          })
                          .SetAttr("height", [this, &y](D3::HistogramBin d){
                               return GetHeight() - y.ApplyScale(d.length()) - axis_width;
                          });

    }

    // void Clear() {
    //     GetSVG()->SelectAll(".bar").Remove();
    // }
};

template <typename DATA_TYPE = emp::array<double, 2>,
          typename  X_SCALE_TYPE = D3::LinearScale,
          typename  Y_SCALE_TYPE = D3::LinearScale >
class LineGraph : public D3Visualization {
protected:
  // Formatting constants
  double y_min = 1000;
  double y_max = 0;
  double x_min = 1000;
  double x_max = 0;

  // Components of the graph
  X_SCALE_TYPE * x_scale;
  Y_SCALE_TYPE * y_scale;
  D3::Axis<X_SCALE_TYPE> * x_axis;
  D3::Axis<Y_SCALE_TYPE> * y_axis;
  D3::LineGenerator * line_gen;
  D3::ToolTip * tip;
  // In case we need to store a dataset
  D3::CSVDataset * dataset;

  // Containers to keep track of data state
  std::deque<DATA_TYPE> data;
  DATA_TYPE prev_data  = {{-1,-1}};

  // Callback functions for accessing and scaling data

  #ifndef DOXYGEN_SHOULD_SKIP_THIS
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

  #endif // DOXYGEN_SHOULD_SKIP_THIS

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

    // Wrap necessary callback functions
    JSWrap([this](){exit.Remove(); this->DrawData(true);}, GetID()+"draw_data");

    // Create tool tip
    tip = new D3::ToolTip([this](DATA_TYPE d) {return D3::FormatFunction(".2f")(return_y(d));});
    GetSVG()->SetupToolTip(*tip);

    // Set up scales
    y_scale = new Y_SCALE_TYPE();
    x_scale = new X_SCALE_TYPE();
    y_scale->SetDomain(emp::array<double, 2>{{y_max, y_min}});
    y_scale->SetRange(emp::array<double, 2>{{y_margin, (double)GetHeight() - axis_width}});
    x_scale->SetDomain(emp::array<double, 2>{{x_min,x_max}});
    x_scale->SetRange(emp::array<double, 2>{{axis_width, GetWidth()-x_margin}});

    //Set up axes
    x_axis = new D3::Axis<X_SCALE_TYPE>("bottom", variables[0]);
    x_axis->SetScale(*x_scale);
    y_axis = new D3::Axis<Y_SCALE_TYPE>("left", variables[1]);
    y_axis->SetScale(*y_scale);
    D3::DrawAxes(*x_axis, *y_axis, *svg);

    line_gen = new D3::LineGenerator();

    // Set up accessors (for retriving coords from data points)
    SetXAccessor(return_x);
    SetYAccessor(return_y);

    dataset = new D3::CSVDataset();

    // In case functions were called before initialization
    init = true;
    this->pending_funcs.Run();
  }

  D3::Selection exit;
  X_SCALE_TYPE * GetXScale() {return x_scale;}
  Y_SCALE_TYPE * GetYScale() {return y_scale;}
  D3::Axis<X_SCALE_TYPE> * GetXAxis() {return x_axis;}
  D3::Axis<Y_SCALE_TYPE> * GetYAxis() {return y_axis;}
  D3::LineGenerator * GetLineGenerator() {return line_gen;}
  D3::CSVDataset * GetDataset() {return dataset;}
  D3::ToolTip * GetToolTip() {return tip;}
  std::function<double(DATA_TYPE)> GetXAccessor() {return return_x;}
  std::function<double(DATA_TYPE)> GetYAccessor() {return return_y;}
  std::function<double(DATA_TYPE)> GetScaledX() {return x;}
  std::function<double(DATA_TYPE)> GetScaledY() {return y;}


  void SetXScale(X_SCALE_TYPE * scale) {x_scale = scale;}
  void SetYScale(Y_SCALE_TYPE * scale) {y_scale = scale;}
  void SetXAxis(D3::Axis<X_SCALE_TYPE> * ax) {x_axis = ax;}
  void SetYAxis(D3::Axis<Y_SCALE_TYPE> * ax) {y_axis = ax;}
  void SetLineGenerator(D3::LineGenerator * line) {line_gen = line;}
  void SetDataset(D3::CSVDataset * d) {dataset = d;}

  void SetTooltipFunction(std::string func) {
    tip->SetHtml(func);
  }

  /// @cond TEMPLATES
  template <typename T>
  emp::sfinae_decoy<void, decltype(&T::operator())>
  SetTooltipFunction(T func) {
    tip->SetHtml(func);
  }
  /// @endcond


  /// @cond TEMPLATES
  template <typename T>
  emp::sfinae_decoy<void, decltype(&T::operator())>
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
  /// Can be a string representing the name of a function in Javascript (can be in the current
  /// window, the emp namespace, or the d3 namespace)

  void SetXAccessor(std::string func) {
    return_x = [func](DATA_TYPE d){
      emp::StoreReturn(d);
      return MAIN_THREAD_EM_ASM_DOUBLE({
        var func_string = UTF8ToString($0);
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
  emp::sfinae_decoy<void, decltype(&T::operator())>
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
  /// Can be a string representing the name of a function in Javascript (can be in the current
  /// window, the emp namespace, or the d3 namespace)
  void SetYAccessor(std::string func) {
    return_y = [func](DATA_TYPE d){
      emp::StoreReturn(d);
      return MAIN_THREAD_EM_ASM_DOUBLE({
        var func_string = UTF8ToString($0);
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
  void DrawPointsFromDataset() {

    // Adjust axes
    x_min = std::min(MAIN_THREAD_EM_ASM_DOUBLE({
        return d3.min(js.objects[$0], window["emp"][UTF8ToString($1)+"return_x"]);
    }, dataset->GetID(), this->GetID().c_str()), x_min);

    x_max = std::max(MAIN_THREAD_EM_ASM_DOUBLE({
        return d3.max(js.objects[$0], window["emp"][UTF8ToString($1)+"return_x"]);
    }, dataset->GetID(), this->GetID().c_str()), x_max);

    y_min = std::min(MAIN_THREAD_EM_ASM_DOUBLE({
        return d3.min(js.objects[$0], window["emp"][UTF8ToString($1)+"return_y"]);
    }, dataset->GetID(), this->GetID().c_str()), y_min);

    y_max = std::max(MAIN_THREAD_EM_ASM_DOUBLE({
        return d3.max(js.objects[$0], window["emp"][UTF8ToString($1)+"return_y"]);
    }, dataset->GetID(), this->GetID().c_str()), y_max);

    y_scale->SetDomain(y_max, y_min);
    x_scale->SetDomain(x_min, x_max);

    y_axis->Rescale(y_max, y_min, *GetSVG());
    x_axis->Rescale(x_min, x_max, *GetSVG());

    // Bind data and update graphics
    D3::Selection update = GetSVG()->SelectAll(".data-point")
                                    .Data(*dataset, GetID()+"return_x");
    update = update.EnterAppend("circle").Merge(update);
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
  }

  /// Load data from the file at [filename]. Expected to be a CSV dataset
  void LoadDataFromFile(std::string filename) {
    emp::JSWrap([this](){DrawPointsFromDataset();}, "draw");

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

      D3::Transition t = GetSVG()->MakeTransition();
      MAIN_THREAD_EM_ASM({js.objects[$0].ease(d3.easeLinear).delay(10).duration(300);}, t.GetID());
      y_axis->Rescale(y_max, y_min, t);
      x_axis->Rescale(x_min, x_max, t);
      t.On("end", GetID()+"draw_data");
      Redraw(t);

    } else {
      DrawData(false);
    }
  }

  /// Redraws all data on the given selection or transition, which should contain an SVG canvas.
  /// Useful if you've adjusted scales.
  // template <typename T>
  // void Redraw(D3::SelectionOrTransition<T> & s) {
  //   s.SelectAll(".data-point").Log();
  //   s.SelectAll(".data-point").SetAttr("cy", GetID()+"y");
  //   s.SelectAll(".data-point").SetAttr("cx", GetID()+"x");
  //
  //   D3::Dataset circle_data = GetSVG()->SelectAll(".data-point").Data();
  //   circle_data.Log();
  //   D3::Selection new_segs = GetSVG()->SelectAll(".line-seg").Data(circle_data);
  //   new_segs.ExitRemove();
  //   new_segs.EnterAppend("path").SetAttr("class", "line-seg");
  //   new_segs.SetAttr("d", GetID()+"genpath");
  //
  //   // MAIN_THREAD_EM_ASM({
  //   //
  //   //   js.objects[$0].selectAll(".line-seg").attr("d", function(d){console.log("in d", d, $1, js.objects[$1]); return js.objects[$1](d);});
  //   // }, GetSVG()->GetID(), line_gen->GetID(), s.GetID());
  //   std::cout << "Done redrawing" << std::endl;
  // }
  template <typename T>
  void Redraw(D3::SelectionOrTransition<T> & s) {
    // s.SelectAll(".data-point").SetAttr("cy", GetID()+"y");
    // s.SelectAll(".data-point").SetAttr("cx", GetID()+"x");

    MAIN_THREAD_EM_ASM({
        function pathTween(d1, precision) {
          return function() {
            var path0 = this;
                path1 = path0.cloneNode();
                n0 = path0.getTotalLength();
                n1 = (path1.setAttribute("d", d1), path1).getTotalLength();

            // Uniform sampling of distance based on specified precision.
            var distances = [0];
            i = 0;
            dt = precision / Math.max(n0, n1);
            while ((i += dt) < 1) distances.push(i);
            distances.push(1);

            // Compute point-interpolators at each distance.
            var points = distances.map(function(t) {
              var p0 = path0.getPointAtLength(t * n0);
                  p1 = path1.getPointAtLength(t * n1);
              return d3.interpolate([p0.x, p0.y], [p1.x, p1.y]);
            });

            return function(t) {
              return t < 1 ? "M" + points.map(function(p) { return p(t); }).join("L") : d1;
            };
          };
      };

      circle_data = js.objects[$0].selectAll(".data-point").data();
      ls = js.objects[$2].selectAll(".line-seg");
      var s = js.objects[$0].selectAll(".line-seg").data([circle_data]);
      js.objects[$3] = s.exit();
      js.objects[$2].ease(d3.easeLinear).duration(300).selectAll(".data-point")
                    .attr("cy", emp[UTF8ToString($4)+"y"])
                    .attr("cx", emp[UTF8ToString($4)+"x"]);
      t = s.transition(js.objects[$2]).duration(300).attrTween("d", pathTween).ease(d3.easeLinear);
      t.attr("d", js.objects[$1]);
      js.objects[$3]
        .transition(js.objects[$2])
        .duration(300)
        .attrTween("d", pathTween)
        .ease(d3.easeLinear)
        .attr("d",js.objects[$1](circle_data.slice(circle_data.length-2, circle_data.length-1)));
    }, GetSVG()->GetID(), line_gen->GetID(), s.GetID(), exit.GetID(), GetID().c_str());
  }

  void DrawData(bool backlog = false) {

    //If there's a backlog, then we're only allowed to clear it if this
    //was called recursively or from javascript (since javascript handles)
    //using this as a callback to asynchronous stuff)
    if ((!backlog && data.size() > 1) || data.size() == 0){
      return;
    }

    // //We can't draw a line on the first update
    if (prev_data[0] >= 0 ){
      emp::array<DATA_TYPE, 2> line_data;
      line_data[0] = prev_data;
      line_data[1] = data[0];

      line_gen->DrawShape(line_data, *GetSVG())
               .SetAttr("fill", "none")
               .SetAttr("stroke-width", 1)
               .SetAttr("stroke", "black")
               .SetAttr("class", "line-seg");
    }

    // If it isn't nested, D3 will think it's 2 separate points
    emp::array<DATA_TYPE, 1> new_point = {{data[0]}};
    // GetSVG()->SelectAll(".data-point").Log();
    GetSVG()->SelectAll(".data-point")
            .Data(new_point, GetID()+"return_x")
            .EnterAppend("circle")
            .SetAttr("cy", GetID()+"y")
            .SetAttr("cx", GetID()+"x")
            .SetAttr("r", 2)
            .SetAttr("class", "data-point")
            .BindToolTipMouseover(*tip);
    prev_data = data[0];
    data.pop_front();
    // GetSVG()->SelectAll("circle").Log();
    if (data.size() > 0) {
      DrawData(true);
    } else if (data.size() == 0) {
    //   std::cout << "About to draw callback" << std::endl;
      CallDrawCallback();
    }
  }

  void Clear() {
    data.clear();
    GetSVG()->SelectAll(".data-point").Remove();
    GetSVG()->SelectAll(".line-seg").Remove();
    y_axis->Rescale(0, 1000, *(GetSVG()));
    x_axis->Rescale(0, 0, *(GetSVG()));
    y_min = 1000;
    y_max = 0;
    x_min = 0;
    x_max = 0;
    prev_data  = {{-1,-1}};
  }
};

struct TreeDataNode {
    EMP_BUILD_INTROSPECTIVE_TUPLE( int, name)
};

struct TreeNode {
  EMP_BUILD_INTROSPECTIVE_TUPLE( double, x,
                                 double, y,
                                 TreeDataNode, data,
                                 int, parent,
                                 int, depth
                              )
};

template <typename NODE = TreeNode>
class TreeVisualization : public D3Visualization {
protected:
  double y_margin = 10;
  double x_margin = 30;

  // struct TreeEdge {
  //   EMP_BUILD_INTROSPECTIVE_TUPLE( NODE, source,
  //                                  NODE, target)
  // };

  D3::ToolTip * tip;

  void InitializeVariables() {
    JSWrap(color_fun_node, GetID()+"color_fun_node");
    JSWrap(color_fun_link, GetID()+"color_fun_link");
    data = new D3::JSONDataset();
    tip = new D3::ToolTip([](NODE d, int i){return "Name: " + to_string(d.data().name());});
    GetSVG()->Move(0,0);
    data->Append(std::string("{\"name\": 0, \"parent\": \"null\", \"children\" : []}"));
    tree.SetDataset(data);
    tree.SetSize(GetHeight(), GetWidth());
  }

  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  std::function<std::string(NODE, int)> color_fun_node = [](NODE d, int i){
    return "black";
  };

  std::function<std::string(NODE, int)> color_fun_link = [](NODE d, int i){
    return "black";
  };
  #endif // DOXYGEN_SHOULD_SKIP_THIS

public:
  D3::TreeLayout<NODE> tree;
  D3::JSONDataset * data;

  TreeVisualization(int width=800, int height=400) : D3Visualization(width, height){variables.push_back("Persist");}

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
  emp::sfinae_decoy<void, decltype(&T::operator())>
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
    // std::cout << "Vis data id: " << data->GetID() << std::endl;
    emp::array<D3::Selection, 4> nodes_links = tree.GenerateNodesAndLinks(*GetSVG());
    // std::cout << "Links generated" << std::endl;
    nodes_links[0].Append("circle").SetAttr("r", 2).AddToolTip(*tip);
    nodes_links[1].Remove();
    nodes_links[3].Remove();
    // std::cout << "Circles appended" << std::endl;
    GetSVG()->SelectAll("g.node").SelectAll("circle").SetStyle("fill", GetID()+"color_fun_node");
    // std::cout << "Circles styled" << std::endl;
    GetSVG()->SelectAll(".link").SetStyle("stroke", GetID()+"color_fun_link");
    // std::cout << "links styled" << std::endl;
    CallDrawCallback();
  }

  void Clear() {
    GetSVG()->SelectAll(".node").Remove();
    GetSVG()->SelectAll(".link").Remove();
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

    MAIN_THREAD_EM_ASM({
      js.objects[$0] = [js.objects[$1][0]];
    }, possible_parents.GetID(), data->GetID());

    this->init = true;
    this->pending_funcs.Run();
  }

  void AddDataPoint(int parent, int child){
    std::string child_json = std::string("{\"name\":" + to_string(child) + ", \"parent\":" + to_string(parent) + ", \"children\":[]}");
    int pos = data->AppendNestedFromList(child_json, possible_parents);
    (void) pos;

    MAIN_THREAD_EM_ASM({
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
  using TREE_TYPE<NODE>::data;

public:

  int grid_width = 10;
  int grid_height = 10;
  int legend_cell_size = 15;

  D3::Selection legend;

  struct TreeEdge {
    EMP_BUILD_INTROSPECTIVE_TUPLE( NODE, source,
                                   NODE, target)
  };

  struct LegendNode {
    EMP_BUILD_INTROSPECTIVE_TUPLE( int, loc)
  };

  std::function<void(NODE, int)> node_mouseover = [this](NODE d, int i){
    MAIN_THREAD_EM_ASM({

      var trace_lineage = function(root, id) {
        if (root.name == id){
          return [root.loc];
        }
        if (root.children) {
          for (var k in root.children) {
            if (root.children[k].name == id) {
              return [root.children[k].loc];
            }
            else if (root.children[k].children) {
              result = trace_lineage(root.children[k], id);
              if (result) {
                result.push(root.children[k].loc);
                return result;
              }
            }
          }
        }
      };
    //   console.log("about to trace");
      var result = trace_lineage(js.objects[$0][0], $1);
      var paths = ([[result[0]%$2, Math.floor(result[0]/$2)]]);
      for (i=1; i <result.length; i++) {
        var old_point = paths[paths.length-1];
        var new_point = ([result[i]%$2, Math.floor(result[i]/$2)]);
        paths.push(new_point);
      }
      var scale = d3.scale.linear().domain([0,$2]).range([0,500]);
      var l = d3.svg.line().x(function(d){return scale(d[0]);}).y(function(d){return scale(d[1]);});
      var svg = d3.select("body").append("svg");
      svg.attr("width", 500).attr("height",500);
    //   console.log(paths);
      svg.selectAll("path").data([paths]).enter().append("path").attr("d", function(d){console.log(d, l(d)); return l(d);}).attr("stroke", "white").attr("stroke-width", 1).attr("fill","none");
    //   console.log(path.length);
    //   console.log(l(path));


}, data->GetID(), d.name(), grid_width);
  };

  std::function<std::string(NODE, int)> color_fun_node = [this](NODE d, int i){
    if (d.loc() < 0) {
      return std::string("black");
    }

    double x = (d.loc() % grid_width) - grid_width/2;
    double y = (d.loc() / grid_width) - grid_height/2;

    double r = sqrt(std::pow((int)x,2)+std::pow((int)y,2)) / sqrt(std::pow(grid_width,2)+std::pow(grid_height,2));
    (void) r;

    //atan2 takes sign into account
    double theta = atan2(y, x)*(180/emp::PI);
    (void) theta;

    char * color = (char *) MAIN_THREAD_EM_ASM_INT({
        var text = d3.hcl($1, 150, $0*175).toString();
      var buffer = Module._malloc(text.length+1);
      Module.stringToUTF8(text, buffer, lengthBytesUTF8(text)+1);
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

    double r = sqrt(std::pow((int)x,2)+std::pow((int)y,2)) / sqrt(std::pow(grid_width,2)+std::pow(grid_height,2));
    (void) r;

    //atan2 takes sign into account
    double theta = atan2(y, x)*(180/emp::PI);
    (void) theta;

    char * color = (char *) MAIN_THREAD_EM_ASM_INT({
        var text = d3.hcl($1, 150, $0*175).darker().toString();
      var buffer = Module._malloc(text.length+1);
      Module.stringToUTF8(text, buffer, lengthBytesUTF8(text)+1);
      return buffer;
    }, r, theta);

    std::string result = std::string(color);
    free(color);
    return result;
  };

  std::function<std::string(TreeEdge, int)> color_fun_link = [this](TreeEdge d, int i){
    return this->color_fun_node(d.source(), i);
  };


  std::function<void(LegendNode, int)> legend_mouseover = [this](LegendNode d, int il) {
    legend.SelectAll("rect").Filter([d](LegendNode in_data){return d.loc() != in_data.loc();}).SetClassed("faded", true);
    GetSVG()->SelectAll(".node").Filter([d](LegendNode in_data){return d.loc() != in_data.loc();}).SetClassed("faded", true);
    MAIN_THREAD_EM_ASM({emp.filter_fun = function(d){return d.source.loc != $0;}}, d.loc());
    GetSVG()->SelectAll(".link").Filter("filter_fun").SetClassed("faded", true);
  };

  std::function<void(LegendNode, int)> legend_mouseout = [this](LegendNode d, int i) {
    legend.SelectAll("rect")
          .Filter([d](LegendNode in_data){return d.loc() != in_data.loc();})
          .SetClassed("faded", false);
    GetSVG()->SelectAll(".node")
             .Filter([d](LegendNode in_data){return d.loc() != in_data.loc();})
             .SetClassed("faded", false);

    MAIN_THREAD_EM_ASM({emp.filter_fun = function(d){return d.source.loc != $0;}}, d.loc());
    GetSVG()->SelectAll(".link").Filter("filter_fun").SetClassed("faded", false);
  };

  emp::vector<int> GetLocHistory(int id) {
    MAIN_THREAD_EM_ASM({
      var org = js.objects[$1](js.objects[$0][0], $2);
      var loc_history = [];
      loc_history.push(org.loc);
      console.log(org);
      while (+org.name > 0) {
        org = js.objects[$1](js.objects[$0][0], org.parent);
        loc_history.push(org.loc);
      }
      emp_i.__outgoing_array = loc_history;

    }, data->GetID(), data->FindInHierarchy.GetID(), id);

    emp::vector<int> loc_history;
    emp::pass_vector_to_cpp(loc_history);
    return loc_history;
  }

  SpatialGridTreeVisualization(int width, int height) : TreeVisualization<NODE>(width, height){;}

  virtual void Setup() {
    TREE_TYPE<NODE>::Setup();
    JSWrap(color_fun_node, GetID()+"color_fun_node");
    JSWrap(dark_color_fun, GetID()+"dark_color_fun");
    JSWrap(color_fun_link, GetID()+"color_fun_link");
    JSWrap(legend_mouseover, GetID()+"legend_mouseover");
    JSWrap(legend_mouseout, GetID()+"legend_mouseout");
    JSWrap(node_mouseover, GetID()+"node_mouseover");

    tip->SetHtml([this](NODE d){
                    return "ID: " + to_string(d.name()) + ", Pos: ("
                           + to_string(d.loc()% grid_width) + ", "
                           + to_string(d.loc()/grid_width) + ")";
                });

    legend = D3::Select("body").Append("svg");

    legend.SetAttr("x", 1000).SetAttr("y", 0);//.SetAttr("width", legend_cell_size*grid_width).SetAttr("height", legend_cell_size*grid_height);
    legend.SetStyle("position", "fixed").SetStyle("right", "10px").SetStyle("top", "10px");//.SetStyle("width", "10%");//.SetStyle("height", "10%");

    D3::TileGrid<LegendNode> legend_grid(grid_width, grid_height, legend_cell_size, legend);

    legend.SelectAll("rect").SetStyle("fill", GetID()+"color_fun_node")
                            .SetStyle("stroke", GetID()+"color_fun_node")
                            .On("mouseover", GetID()+"legend_mouseover")
                            .On("mouseout", GetID()+"legend_mouseout");

    GetSVG()->SelectAll(".node").On("click", [this](NODE d){std::cout << emp::to_string(GetLocHistory(d.name())) << std::endl;});

  }



};

}
}

#endif // #ifndef EMP_WEB_D3_VISUALIZATIONS_HPP_INCLUDE
