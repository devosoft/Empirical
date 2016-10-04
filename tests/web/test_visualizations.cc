// This file is a monstrosity, because including multiple emscripten-generated
// Javascript files in karma will cause horrible problems.


#include "../../web/Document.h"
#include "../../web/d3/visualizations.h"
#include "../../tools/tuple_struct.h"

#include <iostream>
#include <functional>

struct LineageTreeNode {
  EMP_BUILD_INTROSPECTIVE_TUPLE( double, x,
                                 double, y,
                                 int, name,
                                 int, parent,
                                 int, depth,
                                 int, alive,
                                 int, persist,
                                 std::string, genome
                              )
};

emp::web::Document doc("line_graph");
emp::web::Document tree_viz("tree_viz");
emp::web::LineGraph<std::array<double, 2> > line_graph("x", "y", 500, 250);

emp::web::TreeVisualization<LineageTreeNode> tree(500, 250);
D3::Selection example_selection;
D3::Selection circles;

void MakeLineGraph(std::string callback) {
  doc << line_graph;
  line_graph.SetDrawCallback(callback);
  line_graph.LoadDataFromFile("/base/tests/test-data/test-line-graph.csv");
};

void TestAnimateStep_LineGraph(std::string callback) {
  line_graph.SetDrawCallback(callback);
  line_graph.AddDataPoint({6,12});
};

void ClearLineGraph() {
  line_graph.Clear();
}

void MakeTreeViz(std::string callback) {
  tree_viz << tree;
  tree.SetDrawCallback(callback);
  tree.LoadDataFromFile("/base/tests/test-data/lineage-example.json");
};

void TestAnimateStep_Tree(std::string callback) {
  tree.SetDrawCallback(callback);
  tree.AddDataPoint(0,10);
};

void ClearTreeViz() {
  tree.Clear();
}

int MakeSVG(){
  example_selection = D3::Select("body").Append("svg");
  return example_selection.GetID();
}

int BindData() {
  circles = example_selection.SelectAll("circle").Data(std::array<int, 4>({8,3,5,2})).Enter().Append("circle");
  return circles.GetID();
}

void TestSetAttrString(){
  circles.SetAttr("transform", "skewX(10)");
}

void TestSetAttrInt(){
  circles.SetAttr("r", 3);
}

void TestSetAttrFunc(){
  circles.SetAttr("cx", [](int d){return d;});
  circles.SetAttr("cy", [](int d){return d;});
}

void TestSetStyleString(){
  circles.SetStyle("fill", "purple");
}

void TestSetStyleInt(){
  circles.SetStyle("stroke-width", 5);
}

void TestSetStyleFunc(){
  circles.SetStyle("stroke", [](int d){
      if (d>4) {
        return "green";
      } else {
        return "blue";
      }
  });
}

int TestFilterByFunc(){
  D3::Selection filtered = circles.Filter([](int d){return d > 4;});
  return filtered.GetID();
}

int TestFilterBySel(){
  example_selection.Append("div").SetAttr("id", "example_id");
  D3::Selection filtered = D3::SelectAll("div").Filter("#example_id");
  return filtered.GetID();
}


int main() {

  emp::JSWrap(MakeLineGraph, "MakeLineGraph");
  emp::JSWrap(TestAnimateStep_LineGraph, "TestAnimateStep_LineGraph");
  emp::JSWrap(MakeTreeViz, "MakeTreeViz");
  emp::JSWrap(TestAnimateStep_Tree, "TestAnimateStep_Tree");
  emp::JSWrap(MakeSVG, "MakeSVG");
  emp::JSWrap(BindData, "BindData");
  emp::JSWrap(ClearLineGraph, "ClearLineGraph");
  emp::JSWrap(ClearTreeViz, "ClearTreeViz");

  EM_ASM({

    describe('Line Graph', function() {

      before( function(done) {
        emp.done = done;
        emp.MakeLineGraph("done");
      });

      it('should have data-points for each piece of test data', function() {
        var data_points = d3.select("#line_graph").selectAll(".data-point");
        chai.assert.equal(data_points[0].length, 5);
        chai.assert.deepEqual(data_points.data(), [[1, 5], [2, 3], [3, 6], [4, 1], [5, 10]]);
      });

      it('they should be connected by a line', function() {
        var path = d3.select("#line_graph").selectAll(".line-seg").attr("d");
        chai.assert.equal(path, "M60,110L162.5,150L265,90L367.5,190L470,10");
      });

      it('should have an x and y axis', function() {
        var data_points = d3.select("#line_graph").selectAll(".data-point");
        chai.assert.equal(d3.select("#x_axis").select("path").attr("d"), "M60,6V0H470V6");
        chai.assert.equal(d3.select("#x_axis").select("#axis_label").text(), "x");
        chai.assert.equal(d3.select("#y_axis").select("path").attr("d"), "M-6,10H0V190H-6");
        chai.assert.equal(d3.select("#y_axis").select("#axis_label").text(), "y");
      });

      describe('Adding data', function(){

        before( function(done) {
          emp.done = done;
          emp.TestAnimateStep_LineGraph("done");
        });

        it('should let you add data and rescale accordingly', function() {
          emp.emp__0draw_data();
          var data_points = d3.select("#line_graph").selectAll(".data-point");
          chai.assert.equal(data_points[0].length, 6);
          chai.assert.deepEqual(data_points.data(), [[1, 5], [2, 3], [3, 6], [4, 1], [5, 10], [6,12]]);
          var paths = d3.select("#line_graph").selectAll(".line-seg");
          chai.assert.equal(paths[0][0].getAttribute("d"), "M60,136.2686567164179L126.12903225806451,163.13432835820896L192.25806451612902,122.83582089552237L258.3870967741936,190L324.5161290322581,69.1044776119403");
          chai.assert.equal(paths[0][1].getAttribute("d"), "M324.5161290322581,69.1044776119403L390.6451612903226,42.23880597014924");
        });

      });

      after(function(){
        emp.ClearLineGraph();
      });

    });
  });

  EM_ASM({

    describe('Tree visualization', function() {

      before( function(done) {
        emp.done = done;
        emp.MakeTreeViz("done");
      });

      it('should have a node and link for each piece of data', function() {
        var data_points = d3.select("#tree_viz").selectAll("circle");
        var links = d3.select("#tree_viz").selectAll(".link");
        chai.assert.equal(data_points[0].length, 163);
        chai.assert.equal(links[0].length, 162);
      });

      describe('Adding a node', function(){

        before( function(done) {
          emp.done = done;
          emp.TestAnimateStep_Tree("done");
        });

        it('should let you add data', function() {
          var data = d3.select("#tree_viz").selectAll("circle").data();
          var new_data = data.filter(function(d){return d.name==10});
          chai.assert.equal(new_data[0].parent.name, 0);
        });

      });

      after(function(){
        emp.ClearTreeViz();
      });

    });
  });

  emp::JSWrap(TestSetAttrString, "TestSetAttrString");
  emp::JSWrap(TestSetAttrInt, "TestSetAttrInt");
  emp::JSWrap(TestSetAttrFunc, "TestSetAttrFunc");
  emp::JSWrap(TestSetStyleString, "TestSetStyleString");
  emp::JSWrap(TestSetStyleInt, "TestSetStyleInt");
  emp::JSWrap(TestSetStyleFunc, "TestSetStyleFunc");
  emp::JSWrap(TestFilterByFunc, "TestFilterByFunc");
  emp::JSWrap(TestFilterBySel, "TestFilterBySel");

  EM_ASM({
    emp.svg_id = -1;
    emp.id = -1;

    describe('Selections', function(){
        before(function(){
          emp.svg_id = emp.MakeSVG();
          emp.id = emp.BindData();
        });

        it('should be possible to make and append to', function() {
            chai.assert.isNotNull(js.objects[emp.svg_id], "Selection created!");
        });

        it('should let you bind data to it', function(){
            chai.assert.equal(js.objects[emp.id][0].length, 4);
        });

        it('should correctly set attributes to strings', function(){
            emp.TestSetAttrString();
            chai.assert.equal(js.objects[emp.id].attr("transform"), "skewX(10)");
        });

        it('should correctly set attributes to ints', function(){
            emp.TestSetAttrInt();
            chai.assert.equal(js.objects[emp.id].attr("r"), 3);
        });

        it('should correctly set attributes with callback functions', function(){
            emp.TestSetAttrFunc();
            chai.assert.equal(js.objects[emp.id].attr("cx"), 8);
            chai.assert.equal(js.objects[emp.id].attr("cy"), 8);
            chai.assert.equal(js.objects[emp.id].filter(function(d,i){return i==2}).attr("cx"), 5);
            chai.assert.equal(js.objects[emp.id].filter(function(d,i){return i==2}).attr("cy"), 5);
        });

        it('should correctly set styles to strings', function(){
            emp.TestSetStyleString();
            chai.assert.equal(js.objects[emp.id].style("fill"), "rgb(128, 0, 128)");
        });

        it('should correctly set styles to ints', function(){
            emp.TestSetStyleInt();
            chai.assert.equal(js.objects[emp.id].style("stroke-width"), 5);
        });

        it('should correctly set styles with callback functions', function(){
            emp.TestSetStyleFunc();
            chai.assert.equal(js.objects[emp.id].style("stroke"), "rgb(0, 128, 0)");
            chai.assert.equal(js.objects[emp.id].filter(function(d,i){return i==3}).style("stroke"), "rgb(0, 0, 255)");
        });

        it('should support filtering by function', function(){
            var id = emp.TestFilterByFunc();
            chai.assert.deepEqual(js.objects[id].data(), [8, 5]);
        });

        it('should support filtering by selector', function(){
            var id = emp.TestFilterBySel();
            chai.assert.equal(js.objects[id].attr("id"), "example_id");
        });


    });


});


}
