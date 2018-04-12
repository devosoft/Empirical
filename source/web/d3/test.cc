
#include "../Empirical/web/init.h"
#include <iostream>
#include <vector>
#include <string>
#include <typeinfo>
#include "selection.h"
#include "../Empirical/web/JSWrap.h"
#include "../Empirical/tools/tuple_struct.h"
#include "scales.h"
#include "svg_shapes.h"
#include "axis.h"
#include "load_data.h"

struct JSDataObject;

int return_val(JSDataObject d, int i=0, int k=0){
  //std::cout << d.word() << std::endl;
  return d.val();
}

int return_d(int d, int i=0, int k=0){
  return d;
}

void mouseover(int id){
  D3::Selection(id).SetAttr("cx", 500);
}

void make_graph() {
  D3::Selection svg = D3::Selection("body").Append("svg");
  //emp::array<emp::array<int, 2>, 5> data;
  //EM_ASM({emp.__outgoing_array = emp.__incoming_data;});
  //emp::pass_array_to_cpp(data);
  D3::Dataset data = D3::Dataset();
  D3::Axis<D3::LinearScale> x_axis = D3::Axis<D3::LinearScale>();
  x_axis.SetScale(D3::LinearScale());
  emp::array<int, 2> domain = {0, 60};
  emp::array<int, 2> range = {0, 250};
  x_axis.GetScale().SetDomain(domain);
  x_axis.GetScale().SetRange(range);
  x_axis.SetTicks(3);

  D3::CartesianLineGenerator<D3::LinearScale, D3::LinearScale> make_line = D3::CartesianLineGenerator<D3::LinearScale, D3::LinearScale>();
  make_line.SetXScale(x_axis.GetScale());
  //make_line.SetYScale(x_axis.GetScale());
  D3::Selection group = make_line.DrawShape(data);
  group.SetAttr("fill", "none");
  group.SetAttr("stroke", "blue");
  group.SetAttr("stroke-width", 2);

  x_axis.Draw(svg);
}

int main()
{
  emp::Initialize();

  emp::JSWrap(make_graph, "make_graph");

  EM_ASM({emp.__outgoing_array = ["hi", "eeee", "l", "l", "o"]; });
  emp::array<std::string, 5> new_array;
  emp::pass_array_to_cpp(new_array);
  std::cout << new_array[0] << " " << new_array[1] << " " << new_array[2] << std::endl;

  //D3::CSVDataset csv = D3::CSVDataset("test_no_headers.csv", "make_graph", false);

  //JSObject();

  D3::Selection svg = D3::Selection("body").Append("svg");
  D3::Selection text = D3::Selection("body").Append("text");
  text.SetText("Testing");
  std::cout << text.GetText() << std::endl;

  //D3::Selection s = svg.SelectAll("circle");

  std::cout << n_objects() << std::endl;

  D3::LinearScale s = D3::LinearScale();
  s.SetRange(emp::array<int,2>({2,3}));

  emp::array<int32_t, 3> test_data = {10,30,60};
  JSDataObject test_obj_1;
  test_obj_1.val() = 10;
  test_obj_1.word() = "hi";
  test_obj_1.val2() = 4.4;

  JSDataObject test_obj_2;
  test_obj_2.val() = 40;
  test_obj_2.word() = "hi2";
  test_obj_2.val2() = 11.2;

  emp::array<JSDataObject, 2> test_data_2 = {test_obj_1, test_obj_2};

  emp::array<emp::array<int,2>, 5> test_path = {{{0,0}, {0,10}, {10,10}, {20,20}, {30, 30}}};

  emp::array<emp::array<emp::array<int,2>, 2>, 3> test_paths = {{ {{{0,0}, {100,0}}}, {{{0,10}, {100,100}}}, {{{20,20}, {300, 300}}} }};

  //D3::ShapesFromData(test_data, "circle");
  //EM_ASM({d3.select("svg").selectAll("circle").data([{val:5, word:"hi", val2:6.3}]).enter().append("circle")});

  svg.SelectAll("circle").Data(test_data_2).EnterAppend("circle");

  //D3::Selection path = svg.Append("path");
  /*D3::LineGenerator make_line = D3::LineGenerator();
  D3::Selection path = make_line.DrawShape(test_path);
  path.SetAttr("fill", "none");
  path.SetAttr("stroke", "blue");
  path.SetAttr("stroke-width", 2);
  D3::Selection group = make_line.DrawShapes(test_paths);
  //group = D3::SelectAll("path");
  group.SetAttr("fill", "none");
  group.SetAttr("stroke", "blue");
  group.SetAttr("stroke-width", 2);*/

  //
  //path.SetAttr("d", make_line.MakeLine(test_path).c_str());

  //D3::Selection update = s.Data(test_data, 3);
  //D3::Selection update = data_bind[0];
  //D3::Selection enter = data_bind[1];
  //update.EnterAppend("circle");
  D3::Selection circles = D3::Selection("circle", true);
  circles.Sort();
  circles.SetAttr("cx", 25);
  circles.SetAttr("cy", 25);
  circles.SetAttr("r", 25);
  circles.SetStyle("fill", "purple");
  D3::Selection t = circles.Transition();
  //t.Interrupt();
  //t.SetAttr("r", 5);

  //Now let's try making a callback function with JSWrap
  //uint32_t fun_id = emp::JSWrap(return_d, "return_d");
  uint32_t fun_id = emp::JSWrap(return_val, "return_val");
  emp::JSWrap(mouseover, "mouseover");
  //int jsresult = EM_ASM_INT({return emp.return_d($0);},5);

  std::cout << "about to do callbacks" << std::endl;

  t.SetAttr("r", "return_val");
  std::cout << "first callback done" << std::endl;
  t.SetAttr("cy", "return_val");
  t.SetAttr("cx", "return_val");
  t.SetStyle("fill", "green");
  D3::Selection("circle", true).On("mouseover", "mouseover");
  //D3::Selection("circle", true).On("mouseover");

}


//The only way to bind data appropriately?
//d3.selectAll("svg").selectAll("line").data([1,2,3]).enter().append("line")
