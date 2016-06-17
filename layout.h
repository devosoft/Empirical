#ifndef __LAYOUT_H__
#define __LAYOUT_H__

#include "d3_init.h"
#include "load_data.h"
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
    //Since no one wants to deal with JSON in C++
    JSONDataset data;
    D3Function link_fun;
    D3Function update_fun;
    D3Function find_parent;
    D3Function ToolTip; //TODO: this shouldn't be handled by the layout, once EMP_CLASS works


    TreeLayout(){
        //Create layout object
        EM_ASM_ARGS({js.objects[$0] = d3.layout.tree();}, this->id);
        //Initialize data
        EM_ASM_ARGS({
            treeData = ([{"name": 0, "parent": "null", "alive":false, "children" : []}]);
            js.objects[$0] = treeData;
        }, data.GetID());

        EM_ASM_ARGS({
          js.objects[$0] = d3.svg.diagonal()
    	                         .projection(function(d) {
                                              return [d.y, d.x];
                                            });
        }, link_fun.GetID());

        EM_ASM_ARGS({
            js.objects[$0] = d3.tip().attr('class', 'd3-tip')
                                      .offset([-10, 0])
                                      .html(function(d, i) { return d.name; });
        }, ToolTip.GetID());

        EM_ASM_ARGS({
          //Inspired by Niels' answer to
          //http://stackoverflow.com/questions/12899609/how-to-add-an-object-to-a-nested-javascript-object-using-a-parent-id/37888800#37888800
          js.objects[$0] = function(root, id) {
            if (root.name == id){
              return root;
            }
            if (root.children) {
              for (var k in root.children) {
                if (root.children[k].name == id) {
                  return root.children[k];
                }
                else if (root.children[k].children) {
                  result = js.objects[$0](root.children[k], id);
                  if (result) {
                    return result;
                  }
                }
              }
            }
          };
        }, find_parent.GetID());


        EM_ASM_ARGS({
          js.objects[$3] = (function(source, svg, tooltip) {

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


    void AddNode(int parent, int child, int pos, Selection & svg, D3Function & alive) {

      EM_ASM_ARGS({
        var child_node = ({"name":$2, "parent":$1, "alive":true, "children":[]});
        //var parent_node = js.objects[$3](js.objects[$0][0], $1);
        var parent_node = null;
        for (var item in js.objects[$7]) {
          if (js.objects[$7][item].name == $1) {
            parent_node = js.objects[$7][item];
          }
        }

        js.objects[$7][$8].alive = false;
        js.objects[$7][$8] = child_node;
        if (!parent_node.hasOwnProperty("children")){
          parent_node.children = [];
        }
        parent_node.children.push(child_node);
        js.objects[$5](parent_node, js.objects[$4], js.objects[$6]);

    }, data.GetID(), parent, child, find_parent.GetID(), svg.GetID(), update_fun.GetID(), ToolTip.GetID(), alive.GetID(), pos);
    }

  };
}


#endif
