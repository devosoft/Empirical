//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "base/vector.h"
#include "tools/Random.h"
#include "web/Animate.h"
#include "web/canvas_utils.h"
#include "web/emfunctions.h"
#include "web/web.h"

namespace UI = emp::web;

struct AdjMatrix {
  emp::vector< emp::vector< char > > vals;

  AdjMatrix() { ; }

  size_t size() { return vals.size(); }
  void Inc() {
    size_t start_size = vals.size();
    for (size_t i = 0; i < start_size; i++) {
      vals[i].push_back(false);
    }
    vals.emplace_back(start_size+1, false);
  }

  bool operator()(size_t x, size_t y) { return vals[x][y]; }
};

struct Node {
  double x;
  double y;
  size_t id;

  Node(double _x, double _y, size_t _id) : x(_x), y(_y), id(_id) { ; }
};

class GraphDriver : public UI::Animate {
private:
  size_t can_w = 400;
  size_t can_h = 400;
  size_t node_r = 15;

  UI::Document doc;
  UI::Canvas graph_canvas;
  emp::Random random;
  emp::vector<Node> nodes;

  void AddNode(double x, double y) {
    size_t id = nodes.size();
    nodes.emplace_back(x,y,id);
  }
public:
  GraphDriver() : doc("emp_base"), graph_canvas(doc.AddCanvas(can_w, can_h, "graph_canvas")) {
    doc << "<h2>Graph Explorer</h2>";

    graph_canvas.OnClick([this](int x, int y){ AddNode(x,y); });

    AddNode(50,50);
    AddNode(100,100);
    AddNode(100,200);

    doc << UI::Text("fps") << "FPS = " << UI::Live( [this](){return 1000.0 / GetStepTime();} ) ;
    Start();
  }
  GraphDriver(const GraphDriver &) = delete;
  GraphDriver(GraphDriver &&) = delete;

  void DoFrame() {
    graph_canvas.Clear("black");

    for (auto & node : nodes) {
      graph_canvas.Circle(node.x, node.y, node_r, "white");
    }

    doc.Text("fps").Redraw();
  }
};

GraphDriver driver;

int main()
{
}
