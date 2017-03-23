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

struct Edge {
  size_t from;
  size_t to;

  Edge(size_t f, size_t t) : from(f), to(t) { ; }
};

class GraphDriver : public UI::Animate {
private:
  size_t can_w = 400;
  size_t can_h = 400;
  size_t node_r = 15;

  double node_r_sqr = node_r * node_r;

  UI::Document doc;
  UI::Canvas graph_canvas;
  emp::Random random;
  emp::vector<Node> nodes;
  emp::vector<Edge> edges;

  int active_node = -1;
  int edge_node = -1;
  int mouse_x = -1;
  int mouse_y = -1;

  size_t AddNode(double x, double y) {
    size_t id = nodes.size();
    nodes.emplace_back(x,y,id);
    return id;
  }

  void MouseDown(int x, int y) {
    // Test if the mouse is on an existing node.
    for (auto & node : nodes) {
      double x_dist = node.x - x;
      double y_dist = node.y - y;
      double sqr_dist = x_dist * x_dist + y_dist * y_dist;
      if (sqr_dist < node_r_sqr) {
        active_node = node.id;
      }
    }

    // If we did not find an existing node, make a new one and stop.
    if (active_node == -1) {
      active_node = (int) AddNode(x,y);
      return;
    }

    // If we are clicking on the same node again, stop trying to make an edge.
    if (active_node == edge_node) {
      edge_node = -1;
      mouse_x = mouse_y = -1;
      return;
    }

    // If we are not trying to make an edge, then start a new edge!
    if (edge_node == -1) {
      edge_node = active_node;
      return;
    }

    // If we made it this far, we are tyring to finish an edge!
    edges.emplace_back(edge_node, active_node);
    edge_node = -1;
  }

  void MouseUp() {
    // edge_node = active_node;
    active_node = -1;
  }

  void MouseMove(int x, int y) {
    mouse_x = mouse_y = -1;
    if (active_node >= 0) {
      auto & node = nodes[(size_t) active_node];
      node.x = x;
      node.y = y;
      edge_node = -1;
    }
    else if (edge_node >= 0) {
      mouse_x = x;
      mouse_y = y;
    }
  }
public:
  GraphDriver() : doc("emp_base"), graph_canvas(doc.AddCanvas(can_w, can_h, "graph_canvas")) {
    doc << "<h2>Graph Explorer</h2>";

    graph_canvas.OnMouseDown([this](int x, int y){ MouseDown(x,y); });
    graph_canvas.OnMouseUp([this](){ MouseUp(); });
    graph_canvas.OnMouseMove([this](int x, int y){ MouseMove(x,y); });

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
      std::string color = "white";
      if (node.id == active_node) color = "yellow";
      graph_canvas.Circle(node.x, node.y, node_r, color, "blue");
    }

    for (auto & edge : edges) {
      Node & node1 = nodes[edge.from];
      Node & node2 = nodes[edge.to];
      graph_canvas.Line(node1.x, node1.y, node2.x, node2.y, "white");
    }

    if (edge_node >= 0 && mouse_x > 0) {
      auto & node = nodes[edge_node];
      graph_canvas.Line(node.x, node.y, mouse_x, mouse_y);
    }

    doc.Text("fps").Redraw();
  }
};

GraphDriver driver;

int main()
{
}
