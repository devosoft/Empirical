#ifndef __LAYOUT_H__
#define __LAYOUT_H__

#include "d3_init.h"
#include "dataset.h"
#include "selection.h"
#include "svg_shapes.h"

#include "../../tools/tuple_struct.h"
#include "../../emtools/JSWrap.h"

#include <functional>
#include <array>

namespace D3{

  struct JSONTreeNode {
    EMP_BUILD_INTROSPECTIVE_TUPLE( double, x,
				   int, name,
				   int, parent,
                   double, y,
                   int, depth
				   )
  };

  class Layout : public D3_Base {
  protected:
    Layout(int id) : D3_Base(id){;};
    Layout() {;};
  };

  template <typename NODE_TYPE = JSONTreeNode>
  class TreeLayout : public Layout {
  protected:

  public:
    JSONDataset * data;
    DiagonalGenerator * make_line;

    TreeLayout(){
        //Create layout object
        EM_ASM_ARGS({js.objects[$0] = d3.layout.tree();}, this->id);

        make_line = new D3::DiagonalGenerator();

        std::function<std::array<double, 2>(NODE_TYPE, int, int)> projection = [](NODE_TYPE n, int i=0, int k=0){
          return std::array<double, 2>({n.y(), n.x()});
        };

        emp::JSWrap(projection, "projection");
        make_line->SetProjection("projection");
    };

    void SetDataset(JSONDataset * dataset) {
      this->data = dataset;
    }

    //Returns the enter selection for the nodes in case the user wants
    //to do more with it. It would be nice to return the enter selection for
    //links too, but C++ makes that super cumbersome, and it's definitely the less
    //common use case
    Selection GenerateNodesAndLinks(Selection svg) {
      int sel_id = EM_ASM_INT_V({return js.objects.length});

      EM_ASM_ARGS({
        var nodes = js.objects[$0].nodes(js.objects[$1][0]).reverse();
        links = js.objects[$0].links(nodes);

        nodes.forEach(function(d) { d.y = d.depth * 20; });

        // Declare the nodesâ€¦
        var node = js.objects[$3].selectAll("g.node")
            .data(nodes, function(d) { return d.name; });

        var nodeEnter = node.enter().append("g")
                .attr("class", "node")
                .attr("transform", function(d) {
                    return "translate(" + d.y + "," + d.x + ")"; });

        node.attr("transform", function(d) {
        		  return "translate(" + d.y + "," + d.x + ")"; });

        var link = js.objects[$3].selectAll("path.link")
      	  .data(links, function(d) { return d.target.name; });

        // Enter the links.
        link.enter().insert("path", "g")
      	    .attr("class", "link")
      	    .attr("d", js.objects[$2])
            .attr("fill", "none")
            .attr("stroke", "black")
            .attr("stroke-width", 1);

        link.attr("class", "link")
            .attr("d", js.objects[$2]);

        js.objects.push(nodeEnter);
    }, this->id, data->GetID(), make_line->GetID(), svg.GetID());
      return Selection(sel_id);
    }

    void SetSize(int w, int h) {
      EM_ASM_ARGS({js.objects[$0].size([$1,$2]);}, this->id, w, h);
    }

  };
}


#endif
