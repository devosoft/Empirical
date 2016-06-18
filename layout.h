#ifndef __LAYOUT_H__
#define __LAYOUT_H__

#include "d3_init.h"
#include "dataset.h"
#include "selection.h"

namespace D3{

  class Layout : public D3_Base {
  protected:
    Layout(int id) : D3_Base(id){;};
    Layout() {;};
  };

  class TreeLayout : public Layout {
  protected:

  public:
    JSONDataset data;
    JSFunction link_fun;
    JSFunction update_fun;
    JSObject ToolTip; //TODO: this shouldn't be handled by the layout, once EMP_CLASS works


    TreeLayout(){
        //Create layout object
        EM_ASM_ARGS({js.objects[$0] = d3.layout.tree();}, this->id);

        EM_ASM_ARGS({
          js.objects[$0] = d3.svg.diagonal()
    	                         .projection(function(d) {
                                              return [d.y, d.x];
                                            });
        }, link_fun.GetID());

        EM_ASM_ARGS({
            js.objects[$0] = d3.tip().attr('class', 'd3-tip')
                                      .offset([-10, 0])
                                      .html(function(d, i) { return "Name: " + d.name + "<br>" + "Persist: " + d.persist + "<br>" + "Genome:"+d.genome; });
        }, ToolTip.GetID());

        EM_ASM_ARGS({
          js.objects[$3] = (function(svg, tooltip) {

              // Compute the new tree layout.
              var nodes = js.objects[$0].nodes(js.objects[$1][0]).reverse(),
            	  links = js.objects[$0].links(nodes);

              // Normalize for fixed-depth.
              nodes.forEach(function(d) { d.y = d.depth * 20; });

              // Declare the nodesâ€¦
              var node = svg.selectAll("g.node")
            	  .data(nodes, function(d) { return d.name; });

              // Enter the nodes.
              var nodeEnter = node.enter().append("g")
            	  .attr("class", "node")
            	  .attr("transform", function(d) {
            		  return "translate(" + d.y + "," + d.x + ")"; });

              nodeEnter.append("circle")
            	  .attr("r", 2)
                  .style("fill", function(d){
                    if (d.alive){
                      return "red";
                    } else if (d.persist) {
                      return "blue";
                    } else {
                      return "black";
                    }});

              nodeEnter.call(tooltip);
              nodeEnter.on("mouseover", tooltip.show).on("mouseout", tooltip.hide);

              /*nodeEnter.append("text")
            	  .attr("x", function(d) {
            		  return d.children || d._children ? -13 : 13; })
            	  .attr("dy", ".35em")
            	  .attr("text-anchor", function(d) {
            		  return d.children || d._children ? "end" : "start"; })
            	  .text(function(d) { return d.name; })
            	  .style("fill-opacity", 1);*/

                node.selectAll("circle").style("fill", function(d){
                      if (d.alive){
                        return "red";
                      } else if (d.persist) {
                        return "blue";
                      } else {
                        return "black";
                    }});

                node.attr("transform", function(d) {
                		  return "translate(" + d.y + "," + d.x + ")"; });


              /*node.selectAll("text").attr("x", function(d) {
              		  return d.children || d._children ? -13 : 13; })
              	  .attr("dy", ".35em")
              	  .attr("text-anchor", function(d) {
              		  return d.children || d._children ? "end" : "start"; })
              	  .text(function(d) { return d.name; })
              	  .style("fill-opacity", 1).style("font-size", "xx-small");*/


              // Declare the linksâ€¦
              var link = svg.selectAll("path.link")
            	  .data(links, function(d) { return d.target.name; });

              // Enter the links.
              link.enter().insert("path", "g")
            	  .attr("class", "link")
            	  .attr("d", js.objects[$2])
                .attr("fill", "none")
                .attr("stroke", "black")
                .attr("stroke-width", 1);

                link.attr("class", "link")
              	  .attr("d", js.objects[$2])
                  .attr("fill", "none")
                  .attr("stroke", "black")
                  .attr("stroke-width", 1);

          });
        }, this->id, data.GetID(), link_fun.GetID(), update_fun.GetID());
    };

    void SetSize(int w, int h) {
      EM_ASM_ARGS({js.objects[$0].size([$1,$2]);}, this->id, w, h);
    }

    void Update(Selection & svg) {

      EM_ASM_ARGS({
        js.objects[$1](js.objects[$0], js.objects[$2]);

      }, svg.GetID(), update_fun.GetID(), ToolTip.GetID());
    }

  };
}


#endif
