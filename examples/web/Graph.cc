//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "base/vector.h"
#include "tools/Random.h"
#include "web/Animate.h"
#include "web/canvas_utils.h"
#include "web/emfunctions.h"
#include "web/Selector.h"
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

  char operator()(size_t x, size_t y) const { return vals[x][y]; }
  char & operator()(size_t x, size_t y) { return vals[x][y]; }
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
  UI::Selector mode_select;
  UI::Table table_list;
  UI::Table table_matrix;

  emp::Random random;
  emp::vector<Node> nodes;
  emp::vector<Edge> edges;

  emp::vector< emp::vector<int> > adj_list;
  AdjMatrix adj_matrix;

  int active_node = -1;
  int edge_node = -1;
  int mouse_x = -1;
  int mouse_y = -1;
  bool update_graph = true;

  size_t AddNode(double x, double y) {
    size_t id = nodes.size();
    nodes.emplace_back(x,y,id);
    adj_list.emplace_back();
    adj_matrix.Inc();
    update_graph = true;
    return id;
  }

  void AddEdge(size_t from, size_t to) {
    edges.emplace_back(from, to);
    adj_matrix(from,to) = 1;
    adj_matrix(to,from) = 1;
    adj_list[from].push_back(to);
    adj_list[to].push_back(from);
    edge_node = -1;
    update_graph = true;
  }

  char ID2Symbol(size_t id) {
    char symbol = '+';
    if (id < 26) symbol = 'A' + (char) id;
    else if (id < 52) symbol = 'a' + (char) (id-26);
    else if (id < 62) symbol = '0' + (char) (id-52);
    return symbol;
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
      if (edge_node >= 0) AddEdge(edge_node, active_node);
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
    AddEdge(edge_node, active_node);
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
  GraphDriver()
    : doc("emp_base")
    , graph_canvas(doc.AddCanvas(can_w, can_h, "graph_canvas"))
    , mode_select(doc.AddSelector("mode"))
    , table_list(doc.AddTable(1,1,"adj_list"))
    , table_matrix(doc.AddTable(1,1,"adj_matrix"))
  {
    doc << "<h2>Graph Explorer</h2>";

    graph_canvas.OnMouseDown([this](int x, int y){ MouseDown(x,y); });
    graph_canvas.OnMouseUp([this](){ MouseUp(); });
    graph_canvas.OnMouseMove([this](int x, int y){ MouseMove(x,y); });

    mode_select.SetOption("Adjacency List");
    mode_select.SetOption("Adjacency Matrix");
    mode_select.SetOption("Vertex Info");

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

    // Only update the graph if we know we need to.
    if (update_graph) {
      update_graph = false;  // Don't update again unless there is another change.

      // Update the adjacency list.
      table_list.Clear();
      table_list.Resize(nodes.size()+1, 2);
      table_list.GetCell(0,0).SetHeader() << "ID";
      table_list.GetCell(0,1).SetHeader() << "Connections";
      for (size_t r = 0; r < nodes.size(); r++) {
        char symbol = ID2Symbol(r);
        table_list.GetCell(r+1,0).SetHeader() << symbol;
        for (size_t s : adj_list[r]) table_list.GetCell(r+1,1) << ID2Symbol(s) << " ";
      }
      table_list.SetCSS("border-collapse", "collapse");
      table_list.SetCSS("border", "3px solid black");
      table_list.CellsCSS("border", "1px solid black");
      table_list.Redraw();

      // Update the adjacency matrix.
      table_matrix.Clear();
      table_matrix.Resize(nodes.size()+1, nodes.size()+1);
      for (size_t r = 0; r < nodes.size(); r++) {
        char symbol = ID2Symbol(r);
        table_matrix.GetCell(r+1,0).SetHeader() << symbol;
        table_matrix.GetCell(0,r+1).SetHeader() << symbol;
        for (size_t c = 0; c < nodes.size(); c++) {
          table_matrix.GetCell(r+1,c+1) << ((int) adj_matrix(r,c));
        }
      }
      table_matrix.SetCSS("border-collapse", "collapse");
      table_matrix.SetCSS("border", "3px solid black");
      table_matrix.CellsCSS("border", "1px solid black");
      table_matrix.Redraw();
    }
  }
};

GraphDriver driver;

int main()
{
}
