/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file visualizations.cpp
 */

// This file is a monstrosity, because including multiple emscripten-generated
// Javascript files in karma will cause horrible problems.

#include <functional>
#include <iostream>

#include "emp/datastructs/tuple_struct.hpp"
#include "emp/web/d3/visualizations.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/_MochaTestRunner.hpp"

struct LineageTreeDataNode{
        EMP_BUILD_INTROSPECTIVE_TUPLE(
                                 int, name,
                                 int, alive,
                                 int, persist,
                                 std::string, genome
        )
};

struct LineageTreeNode {
  EMP_BUILD_INTROSPECTIVE_TUPLE( double, x,
                                 double, y,
                                 int, parent,
                                 int, depth,
                                 LineageTreeDataNode, data
                              )
};


struct Test_Visualizations : public emp::web::BaseTest {

  int MakeSVG(){
    example_selection = D3::Select("body").Append("svg");
    return example_selection.GetID();
  }

  int BindData() {
    D3::Selection temp_circles = example_selection.SelectAll("circle").Data(emp::array<int, 4>{{8,3,5,2}});
    circles = temp_circles.Enter().Append("circle").Merge(temp_circles);
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

  void TestSetStyleInt() {
    circles.SetStyle("stroke-width", 5);
  }

  void TestSetStyleFunc() {
    circles.SetStyle("stroke", [](int d){
        if (d>4) {
          return "green";
        } else {
          return "blue";
        }
    });
  }

  int TestFilterByFunc() {
    filtered = circles.Filter([](int d){return d > 4;});
    return filtered.GetID();
  }

  int TestFilterBySel() {
    example_selection.Append("div").SetAttr("id", "example_id");
    filtered = D3::SelectAll("div").Filter("#example_id");
    return filtered.GetID();
  }


  int TestSelectionCall() {
    // Example function from D3 documentation
    example_selection.Call([](int selection){D3::Selection(selection).SetAttr("first-name", "John").SetAttr("last-name", "Snow");});
    return example_selection.GetID();
  }

  int TestSubSelection() {
    // Make extraneous div
    D3::Select("body").Append("div");
    sub_div = example_selection.SelectAll("div");
    return sub_div.GetID();
  }

  int TestSetText() {
    example_selection.SetText("Look! Text!");
    return example_selection.GetID();
  }

  int TestMove() {
    example_selection.Move(6,7);
    return example_selection.GetID();
  }

  int TestRotate() {
    example_selection.Rotate(-10);
    return example_selection.GetID();
  }

  std::string TestGetAttrString() {
    circles.SetAttr("test", "some text");
    return circles.GetAttrString("test");
  }

  int TestGetAttrInt() {
    circles.SetAttr("test", 4);
    return circles.GetAttrInt("test");
  }

  double TestGetAttrDouble() {
    circles.SetAttr("test", 5.4);
    return circles.GetAttrDouble("test");
  }

  D3::Selection example_selection;
  D3::Selection circles;
  D3::LinearScale scale;
  D3::Selection filtered;
  D3::Selection sub_div;
  D3::Axis<> ax;
  D3::Selection svg;

  Test_Visualizations()
  : BaseTest(), // we can tell BaseTest that we want to create a set of emp::web::Document objects for each given html element ids.
     ax("left", "example axis")
  {

    svg = D3::Select("body").Append("svg");

    emp::JSWrap([this](){return MakeSVG();}, "MakeSVG");
    emp::JSWrap([this](){return BindData();}, "BindData");

    emp::JSWrap([this](){TestSetAttrString();}, "TestSetAttrString");
    emp::JSWrap([this](){TestSetAttrInt();}, "TestSetAttrInt");
    emp::JSWrap([this](){TestSetAttrFunc();}, "TestSetAttrFunc");
    emp::JSWrap([this](){TestSetStyleString();}, "TestSetStyleString");
    emp::JSWrap([this](){TestSetStyleInt();}, "TestSetStyleInt");
    emp::JSWrap([this](){TestSetStyleFunc();}, "TestSetStyleFunc");
    emp::JSWrap([this](){return TestFilterByFunc();}, "TestFilterByFunc");
    emp::JSWrap([this](){return TestFilterBySel();}, "TestFilterBySel");
    emp::JSWrap([this](){return TestSelectionCall();}, "TestSelectionCall");
    emp::JSWrap([this](){return TestSubSelection();}, "TestSubSelection");
    emp::JSWrap([this](){return TestSetText();}, "TestSetText");
    emp::JSWrap([this](){return TestGetAttrString();}, "TestGetAttrString");
    emp::JSWrap([this](){return TestGetAttrInt();}, "TestGetAttrInt");
    emp::JSWrap([this](){return TestGetAttrDouble();}, "TestGetAttrDouble");
    emp::JSWrap([this](){return TestMove();}, "TestMove");
    emp::JSWrap([this](){return TestRotate();}, "TestRotate");

    emp::JSWrap([this](){ax.SetScale(scale);}, "TestSetScale");
    emp::JSWrap([this](){return ax.GetScale().GetID();}, "TestGetScale");
    emp::JSWrap([this](){return ax.GetID();}, "TestGetAx");
    emp::JSWrap([this](){return ax.group.GetID();}, "TestGetAxGroup");
    emp::JSWrap([this](){ax.SetTickValues(emp::array<int, 3>{{4,5,7}});}, "TestSetTickValues");
    emp::JSWrap([this](){ax.SetTickSize(.2);}, "TestSetTickSize");
    emp::JSWrap([this](){ax.SetTickSizeInner(.7);}, "TestSetInnerTickSize");
    emp::JSWrap([this](){ax.SetTickSizeOuter(1.1);}, "TestSetOuterTickSize");
    emp::JSWrap([this](){ax.SetTicks(7);}, "TestSetTicks");
    emp::JSWrap([this](){ax.SetTickPadding(3);}, "TestSetTickPadding");
    emp::JSWrap([this](){ax.SetTickFormat(".3f");}, "TestSetTickFormat");
    emp::JSWrap([this](){ax.Draw(svg);}, "TestDraw");
    emp::JSWrap([this](){ax.Rescale(20, 30, svg);}, "TestRescale");
    emp::JSWrap([this](){ax.AdjustLabelOffset("-4em");ax.Move(100,0);}, "TestAdjustLabelOffset");

    scale.SetDomain(0, 1);
    scale.SetRange(30, 300);
    svg.SetAttr("height", 500);

  }


  void Describe() override {
    MAIN_THREAD_EM_ASM({

    emp.svg_id = -1;
    emp.id = -1;

    describe('Selections', function(){


        it('should be possible to make and append to', function() {
            emp.svg_id = emp.MakeSVG();
            chai.assert.isNotNull(js.objects[emp.svg_id], "Selection created!");
        });

        it('should let you bind data to it', function(){
            emp.id = emp.BindData();
            console.log("emp.id", emp.id, js.objects[emp.id]);
            chai.assert.equal(js.objects[emp.id].size(), 4);
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
            chai.assert.equal(js.objects[emp.id].style("fill"), "purple");
        });

        it('should correctly set styles to ints', function(){
            emp.TestSetStyleInt();
            chai.assert.oneOf(js.objects[emp.id].style("stroke-width"), ["5","5px"]);
        });

        it('should correctly set styles with callback functions', function(){
            emp.TestSetStyleFunc();
            chai.assert.equal(js.objects[emp.id].style("stroke"), "green");
            chai.assert.equal(js.objects[emp.id].filter(function(d,i){return i==3}).style("stroke"), "blue");
        });

        it('should support filtering by function', function(){
            var id = emp.TestFilterByFunc();
            chai.assert.deepEqual(js.objects[id].data(), [8, 5]);
        });

        it('should support filtering by selector', function(){
            var id = emp.TestFilterBySel();
            chai.assert.equal(js.objects[id].attr("id"), "example_id");
        });

        it('should support the call method', function(){
            var id = emp.TestSelectionCall();
            chai.assert.equal(js.objects[id].attr("first-name"), "John");
            chai.assert.equal(js.objects[id].attr("last-name"), "Snow");
        });

        it('should support sub-selections', function(){
            var id = emp.TestSubSelection();
            chai.assert.equal(js.objects[id].size(), 1);
        });

        it('should support setting text', function(){
            var id = emp.TestSetText();
            chai.assert.equal(js.objects[id].text(), "Look! Text!");
        });

        it('should support getting string attrs', function(){
            chai.assert.equal(emp.TestGetAttrString(), "some text");
        });

        it('should support getting int attrs', function(){
            chai.assert.equal(emp.TestGetAttrInt(), 4);
        });

        it('should support getting double attrs', function(){
            chai.assert.equal(emp.TestGetAttrDouble(), 5.4);
        });

        it('should support the move method', function(){
            var id = emp.TestMove();
            chai.assert.equal(js.objects[id].attr("transform"), "translate(6,7)");
        });

        it('should support the rotate method', function(){
            var id = emp.TestRotate();
            chai.assert.equal(js.objects[id].attr("transform"), "rotate(-10)");
        });
    });

    describe("axes", function(){

      it("should support setting scale", function(){
        emp.TestSetScale();
        var ax_id = emp.TestGetAx();
        chai.assert.deepEqual(js.objects[ax_id].scale().range(), [30,300]);
      });

      it("should support getting scale", function(){
        var id = emp.TestGetScale();
        var ax_id = emp.TestGetAx();
        chai.assert.equal(js.objects[id], js.objects[ax_id].scale());
      });

      it("should support setting tick values", function(){
        emp.TestSetTickValues();
        var ax_id = emp.TestGetAx();
        chai.assert.deepEqual(js.objects[ax_id].tickValues(), [4,5,7]);
        js.objects[ax_id].tickValues(null);
      });


      it("should support setting tick size", function(){
        emp.TestSetTickSize();
        var ax_id = emp.TestGetAx();
        chai.assert.approximately(js.objects[ax_id].tickSize(), .2, .00001);
        chai.assert.approximately(js.objects[ax_id].tickSizeInner(), .2, .00001);
        chai.assert.approximately(js.objects[ax_id].tickSizeOuter(), .2, .00001);
      });

      it("should support setting tick count", function(){
        emp.TestSetTicks();
        var ax_id = emp.TestGetAx();
        chai.assert.equal(js.objects[ax_id].tickArguments()['0'], 7);
      });

      it("should support setting inner tick size", function(){
        emp.TestSetInnerTickSize();
        var ax_id = emp.TestGetAx();
        chai.assert.approximately(js.objects[ax_id].tickSizeInner(), .7, .00001);
      });

      it("should support setting outer tick size", function(){
        emp.TestSetOuterTickSize();
        var ax_id = emp.TestGetAx();
        chai.assert.approximately(js.objects[ax_id].tickSizeOuter(), 1.1, .00001);
      });

      it("should support setting tick padding", function(){
        emp.TestSetTickPadding();
        var ax_id = emp.TestGetAx();
        chai.assert.equal(js.objects[ax_id].tickPadding(), 3);
      });

      it("should support setting tick format", function(){
        emp.TestSetTickFormat();
        var ax_id = emp.TestGetAx();
        chai.assert.equal(js.objects[ax_id].tickFormat()(3.45365), "3.454");
      });

      it("should support drawing", function(){
        emp.TestDraw();
        chai.assert.equal(d3.select("#exampleaxis_axis_label").text(), "example axis");
        chai.assert(d3.select("#exampleaxis_axis_label").attr("transform"), "rotate(-90)");
      });

      it("should support rescaling", function(){
        emp.TestRescale();
        chai.assert.equal(d3.select("#exampleaxis_axis").select(".tick").text(), "20.000");
      });

      it("should support adjusting label offset and moving axis", function(){
        emp.TestAdjustLabelOffset();
        var ax_id = emp.TestGetAxGroup();
        chai.assert.equal(d3.select("#exampleaxis_axis_label").attr("dy"), "-4em");
        chai.assert.equal(js.objects[ax_id].attr("transform"), "translate(100,0)");
      });

    });

  });
  }

};


struct Test_LineGraph : public emp::web::BaseTest {

  void MakeLineGraph(std::string callback) {
    line_graph.SetDrawCallback(callback);
    line_graph.LoadDataFromFile("/assets/test-line-graph.csv");
  };

  void TestAnimateStep_LineGraph(std::string callback) {
    line_graph.SetDrawCallback(callback);
    line_graph.AddDataPoint({{6,12}});
  };


  emp::web::Document& doc;
  emp::web::LineGraph<emp::array<double, 2> > line_graph;

  Test_LineGraph()
  : BaseTest({"line_graph"}), // we can tell BaseTest that we want to create a set of emp::web::Document objects for each given html element ids.
     line_graph("x", "y", 500, 250),
     doc(Doc("line_graph"))
  {

    EM_ASM({
      emp_i.done = function(){};
    });

    // MakeLineGraph("done");
    doc << line_graph;
    // line_graph.SetDrawCallback("my_draw_callback");
    // line_graph.LoadDataFromFile("/assets/test-line-graph.csv");

    // EM_ASM({
    //   while (true) {
    //     if (emp_i.ready) {
    //       break;
    //     }
    //   }
    // });
    emp::JSWrap([this](std::string cb){MakeLineGraph(cb);}, "MakeLineGraph");
    emp::JSWrap([this](std::string cb){TestAnimateStep_LineGraph(cb);}, "TestAnimateStep_LineGraph");

    line_graph.SetDrawCallback("done");
    line_graph.LoadDataFromFile("/assets/test-line-graph.csv");

    // EM_ASM({
    //     emp.done = done;
    //     emp.MakeLineGraph("done");
    // });

  }

  ~Test_LineGraph() {
    line_graph.Clear();
  }

  void Describe() override {
    MAIN_THREAD_EM_ASM({

      describe('Line Graph', function() {

        it('should have data-points for each piece of test data', function() {
          var data_points = d3.select("#line_graph").selectAll(".data-point");
          console.log(data_points);
          chai.assert.equal(data_points.size(), 5);
          chai.assert.deepEqual(data_points.data(), [[1, 5], [2, 3], [3, 6], [4, 1], [5, 10]]);
        });

        it('they should be connected by a line', function() {
          var path = d3.select("#line_graph").selectAll(".line-seg").attr("d");
          chai.assert.equal(path, "M60,110L162.5,150L265,90L367.5,190L470,10");
        });

        it('should have an x and y axis', function() {
          var data_points = d3.select("#line_graph").selectAll(".data-point");
          chai.assert.equal(d3.select("#x_axis").select("path").attr("d"), "M60.5,6V0.5H470.5V6");
          chai.assert.equal(d3.select("#x_axis_label").text(), "x");
          chai.assert.equal(d3.select("#y_axis").select("path").attr("d"), "M-6,10.5H0.5V190.5H-6");
          chai.assert.equal(d3.select("#y_axis_label").text(), "y");
        });

        // describe('Adding data', function(){

        //   before( function(done) {
        //     emp.done = done;
        //     emp.TestAnimateStep_LineGraph("done");
        //   });

        //   it('should let you add data and rescale accordingly', function() {
        //     emp.emp__0draw_data();
        //     var data_points = d3.select("#line_graph").selectAll(".data-point");
        //     chai.assert.equal(data_points.size(), 6);
        //     chai.assert.deepEqual(data_points.data(), [[1, 5], [2, 3], [3, 6], [4, 1], [5, 10], [6,12]]);
        //     var paths = d3.select("#line_graph").selectAll(".line-seg");
        //     chai.assert.equal(paths.nodes()[0].getAttribute("d"), "M60,136.26865671641792L126.12903225806453,163.13432835820896L192.25806451612905,122.83582089552239L258.38709677419354,190L324.5161290322581,69.10447761194028");
        //     chai.assert.equal(paths.nodes()[1].getAttribute("d"), "M324.5161290322581,69.10447761194028L390.6451612903226,42.23880597014926");
        //   });

        // });

      });

    });
  }

};

struct Test_TreeGraph : public emp::web::BaseTest {

  void MakeTreeViz(std::string callback) {
    tree.SetDrawCallback(callback);
    tree.LoadDataFromFile("/assets/lineage-example.json");
  };

  void TestAnimateStep_Tree(std::string callback) {
    tree.SetDrawCallback(callback);
    tree.AddDataPoint(0,10);
  };

  void ClearTreeViz() {
    tree.Clear();
  }

  emp::web::Document& tree_viz;
  emp::web::TreeVisualization<LineageTreeNode> tree;

  Test_TreeGraph()
  : BaseTest({"tree_viz"}), // we can tell BaseTest that we want to create a set of emp::web::Document objects for each given html element ids.
     tree_viz(Doc("tree_viz")),
     tree(500, 250)

  {
    tree_viz << tree;
    emp::JSWrap([this](std::string cb){MakeTreeViz(cb);}, "MakeTreeViz");
    emp::JSWrap([this](std::string cb){TestAnimateStep_Tree(cb);}, "TestAnimateStep_Tree");

  }

  ~Test_TreeGraph() {
    ClearTreeViz();
  }

  void Describe() override {
    MAIN_THREAD_EM_ASM({

      describe('Tree visualization', function() {

        before( function(done) {
          emp.done = done;
          emp.MakeTreeViz("done");
        });

        it('should have a node and link for each piece of data', function() {
          var data_points = d3.select("#tree_viz").selectAll("circle");
          var links = d3.select("#tree_viz").selectAll(".link");
          chai.assert.equal(data_points.size(), 163);
          chai.assert.equal(links.size(), 162);
        });

        describe('Adding a node', function(){

          before( function(done) {
            emp.done = done;
            emp.TestAnimateStep_Tree("done");
          });

          it('should let you add data', function() {
            var data = d3.select("#tree_viz").selectAll("circle").data();
            var new_data = data.filter(function(d){return d.data.name==10});
            chai.assert.equal(new_data[0].parent.data.name, 0);
          });

        });

      });

  });
  }

};

emp::web::MochaTestRunner test_runner;
int main() {

  test_runner.Initialize({"tree_viz", "line_graph"});

  test_runner.AddTest<Test_Visualizations>("Test Visualizations");
  // test_runner.AddTest<Test_TreeGraph>("Test TreeGraph");
  // test_runner.AddTest<Test_LineGraph>("Test LineGraph");
  test_runner.Run();
}


  // emp::JSWrap([](){D3::SymbolGenerator g;
  //                  g.SetType("diamond");
  //                  g.SetSize(2);
  //                  return g.GetID();}, "TestSymbolGenerator");

  // emp::JSWrap([](){D3::LineGenerator g;
  //                 g.SetX([](int d){return d;});
  //                 //g.SetY([](int d){return d;});
  //                 g.AddXScale(scale);
  //                 g.AddYScale(scale);
  //                 return g.GetID();}, "TestLineGenerator");


  // EM_ASM({
  //   describe("paths", function(){
  //
  //     it("can be a symbol generator", function(){
  //       var id = emp.TestSymbolGenerator();
  //       console.log(id);
  //       chai.assert.equal(js.objects[id].type()([1,2]), "diamond");
  //       chai.assert.equal(js.objects[id].size()([1,2]), 2);
  //     });
  //
  //   //   it("can be a line generator", function(){
  //   //     var id = emp.TestLineGenerator();
  //   //     console.log(id);
  //   //     chai.assert.equal(js.objects[id]([[25]]), "M165,165");
  //   //     chai.assert.equal(js.objects[id].size(), 2);
  //   //   });
  //
  //   });
  // });
