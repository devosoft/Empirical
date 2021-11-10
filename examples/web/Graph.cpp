/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file Graph.cpp
 */

#include "emp/base/vector.hpp"
#include "emp/datastructs/vector_utils.hpp"
#include "emp/math/Random.hpp"
#include "emp/web/Animate.hpp"
#include "emp/web/canvas_utils.hpp"
#include "emp/web/emfunctions.hpp"
#include "emp/web/Selector.hpp"
#include "emp/web/web.hpp"

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
  int id;
  int state = 0;

  Node(double _x, double _y, int _id) : x(_x), y(_y), id(_id) { ; }
};

struct Edge {
  size_t from;
  size_t to;

  Edge(size_t f, size_t t) : from(f), to(t) { ; }

  bool operator==(const Edge & in) const {
    return (from == in.from) && (to == in.to);
  }
  bool operator<(const Edge & in) const {
    if (from < in.from) return true;
    return (from == in.from && to < in.to);
  }
  bool operator!=(const Edge & in) const { return !(*this == in); }
  bool operator>(const Edge & in) const { return in < *this; }
  bool operator<=(const Edge & in) const { return !(in < *this); }
  bool operator>=(const Edge & in) const { return !(*this < in); }
};

class GraphDriver : public UI::Animate {
private:
  double can_w = 500;
  double can_h = 500;
  double node_r = 15;

  double node_r_sqr = node_r * node_r;

  UI::Document doc;
  UI::Table main_table;
  UI::Canvas graph_canvas;
  UI::Div info_panel;
  UI::Selector mode_select;
  UI::Table table_list;
  UI::Table table_matrix;

  emp::Random random;
  emp::vector<Node> nodes;
  emp::vector<Edge> edges;

  emp::vector< emp::vector<size_t> > adj_list;
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
    emp_assert(from != to);
    emp_assert(from < nodes.size());
    emp_assert(to < nodes.size());
    if (from > to) std::swap(from,to);

    edges.emplace_back(from, to);
    adj_matrix(from, to) = 1;
    adj_matrix(to, from) = 1;
    adj_list[from].push_back(to);
    adj_list[to].push_back(from);
    edge_node = -1;
    update_graph = true;
  }

  void RemoveEdge(size_t from, size_t to) {
    emp_assert(from != to);
    emp_assert(from < nodes.size());
    emp_assert(to < nodes.size());
    if (from > to) std::swap(from,to);

    edges.erase(edges.begin() + emp::FindValue(edges, Edge(from, to)));
    adj_matrix(from, to) = 0;
    adj_matrix(to, from) = 0;
    // adj_list[from].push_back(to);
    // adj_list[to].push_back(from);
    edge_node = -1;
    update_graph = true;
  }

  void ToggleEdge(size_t from, size_t to) {
    if (from > to) std::swap(from,to);
    if (emp::Has(edges, Edge(from, to))) RemoveEdge(from,to);
    else AddEdge(from,to);
  }

  template <typename T>
  char ID2Symbol(T id) {
    if (id < 0) return '?';
    if (id < 26) return 'A' + (char) id;
    if (id < 52) return 'a' + (char) (id-26);
    if (id < 62) return '0' + (char) (id-52);
    return '+';
  }

  void MouseDown(int x, int y) {
    // Test if the mouse is on an existing node.
    for (auto & node : nodes) {
      double x_dist = node.x - x;
      double y_dist = node.y - y;
      double sqr_dist = x_dist * x_dist + y_dist * y_dist;
      if (sqr_dist < node_r_sqr) {
        active_node = (int) node.id;
      }
    }

    // If we did not find an existing node, make a new one and stop.
    if (active_node == -1) {
      active_node = (int) AddNode(x,y);
      if (edge_node >= 0) AddEdge((size_t) edge_node, (size_t) active_node);
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
    AddEdge((size_t) edge_node, (size_t) active_node);
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

  void ActivateAdjMatrix() {
    info_panel.Clear();
    info_panel << table_matrix;
  }
  void ActivateAdjList() {
    info_panel.Clear();
    info_panel << table_list;
  }
  void ActivateNodeViewer() {
    info_panel.Clear();
  }

public:
  GraphDriver()
  : doc("emp_base")
  , main_table(1,2,"main_table")
  , graph_canvas(can_w, can_h, "graph_canvas")
  , info_panel("info_panel")
  , mode_select("mode")
  , table_list(1,1,"adj_list")
  , table_matrix(1,1,"adj_matrix")
  {
    doc << "<h2>Graph Explorer</h2>";

    doc << main_table;
    main_table.GetCell(0,0) << graph_canvas;
    main_table.GetCell(0,1) << mode_select;
    main_table.GetCell(0,1) << info_panel;

    info_panel << table_matrix;

    main_table.GetCell(0,0).SetCSS("vertical-align", "top");
    main_table.GetCell(0,1).SetCSS("vertical-align", "top");

    graph_canvas.OnMouseDown([this](int x, int y){ MouseDown(x,y); });
    graph_canvas.OnMouseUp([this](){ MouseUp(); });
    graph_canvas.OnMouseMove([this](int x, int y){ MouseMove(x,y); });

    mode_select.SetOption("Adjacency Matrix", [this](){ ActivateAdjMatrix(); });
    mode_select.SetOption("Adjacency List", [this](){ ActivateAdjList(); });
    mode_select.SetOption("Vertex Info", [this](){ ActivateNodeViewer(); });

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

    // Draw all edges on the canvas.
    for (auto & edge : edges) {
      Node & node1 = nodes[edge.from];
      Node & node2 = nodes[edge.to];
      graph_canvas.Line(node1.x, node1.y, node2.x, node2.y, "yellow");
    }

    // If we are in the middle of drawing an edge, place it.
    if (edge_node >= 0 && mouse_x > 0) {
      auto & node = nodes[(size_t)edge_node];
      graph_canvas.Line(node.x, node.y, mouse_x, mouse_y, "red");
    }

    // Draw all vertices on the canvas.
    graph_canvas.Font("20px Arial");
    for (auto & node : nodes) {
      std::string color = "white";
      if (node.id == active_node) color = "yellow";
      else if (node.id == edge_node) color = "purple";
      graph_canvas.Circle(node.x, node.y, node_r, color, "blue");
      std::string symbol = emp::to_string(ID2Symbol(node.id));
      graph_canvas.CenterText(node.x, node.y, symbol, "black", "red");
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
        for (size_t s : adj_list[r]) {
          table_list.GetCell(r+1,1) << ID2Symbol((int)s) << " ";
        }
      }
      table_list.SetCSS("border-collapse", "collapse");
      table_list.SetCSS("border", "3px solid black");
      table_list.CellsCSS("border", "1px solid black");
      table_list.Redraw();

      // Update the adjacency matrix.
      table_matrix.Freeze();  // About to make major changes; don't update live!
      table_matrix.Clear();
      table_matrix.Resize(nodes.size()+1, nodes.size()+1);
      for (size_t r = 0; r < nodes.size(); r++) {
        char symbol = ID2Symbol(r);
        table_matrix.GetCell(r+1,0).SetHeader() << symbol;
        table_matrix.GetCell(0,r+1).SetHeader() << symbol;
        for (size_t c = 0; c < nodes.size(); c++) {
          auto cell = table_matrix.GetCell(r+1,c+1);
          cell << ((int) adj_matrix(r,c));
          if (r!=c) cell.OnClick([this,r,c](){ToggleEdge(r,c);});
        }
      }
      table_matrix.SetCSS("border-collapse", "collapse");
      table_matrix.SetCSS("border", "3px solid black");
      table_matrix.CellsCSS("border", "1px solid black");
      table_matrix.CellsCSS("width", "20px");
      table_matrix.CellsCSS("height", "20px");
      table_matrix.Activate();
    }
  }
};

GraphDriver driver;

int main()
{
}
