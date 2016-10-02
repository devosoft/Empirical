
#include "../../web/Document.h"
#include "../../web/d3/visualizations.h"
#include "../../tools/tuple_struct.h"

#include <iostream>
#include <functional>

emp::web::Document doc("line_graph");
emp::web::Document tree_viz("tree_viz");
emp::web::LineGraph<std::array<double, 2> > line_graph("x", "y", 500, 250);

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

emp::web::TreeVisualization<LineageTreeNode> tree(500, 250);

void MakeLineGraph(std::string callback) {
  doc << line_graph;
  line_graph.SetDrawCallback(callback);
  line_graph.LoadDataFromFile("../test-data/test-line-graph.csv");
}

void TestAnimateStep_LineGraph(std::string callback) {
  line_graph.SetDrawCallback(callback);
  line_graph.AddDataPoint({6,12});
}

void MakeTreeViz(std::string callback) {
  tree_viz << tree;
  tree.SetDrawCallback(callback);
  tree.LoadDataFromFile("../test-data/lineage-example.json");
}

void TestAnimateStep_Tree(std::string callback) {
  tree.SetDrawCallback(callback);
  tree.AddDataPoint(0,10);
}

int main() {
  // emp::JSWrap(MakeLineGraph, "MakeLineGraph");
  // emp::JSWrap(TestAnimateStep_LineGraph, "TestAnimateStep_LineGraph");
  emp::JSWrap(MakeTreeViz, "MakeTreeViz");
  // emp::JSWrap(TestAnimateStep_Tree, "TestAnimateStep_Tree");
  //
  // EM_ASM({
  //
  //   describe('Line Graph', function() {
  //
  //     before( function(done) {
  //       emp.done = done;
  //       emp.MakeLineGraph("done");
  //     });
  //
  //     it('should have data-points for each piece of test data', function() {
  //       var data_points = d3.select("#line_graph").selectAll(".data-point");
  //       chai.assert.equal(data_points[0].length, 5);
  //       chai.assert.deepEqual(data_points.data(), [[1, 5], [2, 3], [3, 6], [4, 1], [5, 10]]);
  //     });
  //
  //     it('they should be connected by a line', function() {
  //       var path = d3.select("#line_graph").selectAll(".line-seg").attr("d");
  //       chai.assert.equal(path, "M60,110L162.5,150L265,90L367.5,190L470,10");
  //     });
  //
  //     it('should have an x and y axis', function() {
  //       var data_points = d3.select("#line_graph").selectAll(".data-point");
  //       chai.assert.equal(d3.select("#x_axis").select("path").attr("d"), "M60,6V0H470V6");
  //       chai.assert.equal(d3.select("#x_axis").select("#axis_label").text(), "x");
  //       chai.assert.equal(d3.select("#y_axis").select("path").attr("d"), "M-6,10H0V190H-6");
  //       chai.assert.equal(d3.select("#y_axis").select("#axis_label").text(), "y");
  //     });
  //
  //     describe('Adding data', function(){
  //
  //       before( function(done) {
  //         emp.done = done;
  //         emp.TestAnimateStep_LineGraph("done");
  //       });
  //
  //       it('should let you add data and rescale accordingly', function() {
  //         var data_points = d3.select("#line_graph").selectAll(".data-point");
  //         chai.assert.equal(data_points[0].length, 6);
  //         chai.assert.deepEqual(data_points.data(), [[1, 5], [2, 3], [3, 6], [4, 1], [5, 10], [6,12]]);
  //         var paths = d3.select("#line_graph").selectAll(".line-seg");
  //         chai.assert.equal(paths[0][0].getAttribute("d"), "M60,136.2686567164179L126.12903225806451,163.13432835820896L192.25806451612902,122.83582089552237L258.3870967741936,190L324.5161290322581,69.1044776119403");
  //         chai.assert.equal(paths[0][1].getAttribute("d"), "M324.5161290322581,69.1044776119403L390.6451612903226,42.23880597014924");
  //       });
  //
  //     });
  //
  //   });
  // });
  //
  // EM_ASM({
  //
  //   describe('Tree visualization', function() {
  //
  //     before( function(done) {
  //       emp.done = done;
  //       emp.MakeTreeViz("done");
  //     });
  //
  //     it('should have a node and link for each piece of data', function() {
  //       var data_points = d3.select("#tree_viz").selectAll("circle");
  //       var links = d3.select("#tree_viz").selectAll(".link");
  //       chai.assert.equal(data_points[0].length, 163);
  //       chai.assert.equal(links[0].length, 162);
  //     });
  //
  //     describe('Adding a node', function(){
  //
  //       before( function(done) {
  //         emp.done = done;
  //         emp.TestAnimateStep_Tree("done");
  //       });
  //
  //       it('should let you add data', function() {
  //         var data = d3.select("#tree_viz").selectAll("circle").data();
  //         var new_data = data.filter(function(d){return d.name==10});
  //         chai.assert.equal(new_data[0].parent.name, 0);
  //       });
  //
  //     });
  //
  //   });
  // });

emp::JSWrap([](std::string callback){tree_viz << tree; tree.SetDrawCallback(callback);}, "test");
emp::JSWrap([](){tree.AddDataPoint(0,1);}, "call_callback");
  EM_ASM({
    describe("Test", function(){
        before(function(done){
            emp.done = done;
            emp.test("done");
            emp.call_callback();
        });
        it ('should assert correctly', function(){
            chai.assert.equal(1,1);
        });
    });
});

}
