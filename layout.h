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
    //Since no one wants to deal with JSON in C++
    JSONDataset data;
    D3Function link_fun;
    D3Function update_fun;
    D3Function find_parent;
  public:
    TreeLayout(){
        //Create layout object
        EM_ASM_ARGS({js.objects[$0] = d3.layout.tree();}, this->id);
        //Initialize data
        EM_ASM_ARGS({
            treeData = ([{"name": 0, "parent": "null","children" : []}]);
            js.objects[$0]=treeData;
            console.log(js.objects, $0);
        }, data.GetID());

        EM_ASM_ARGS({
          js.objects[$0] = d3.svg.diagonal()
    	                         .projection(function(d) {
                                              return [d.y, d.x];
                                            });
        }, link_fun.GetID());

        EM_ASM_ARGS({
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
          js.objects[$3] = (function(source, svg) {

              // Compute the new tree layout.
              var nodes = js.objects[$0].nodes(js.objects[$1][0]).reverse(),
            	  links = js.objects[$0].links(nodes);

              // Normalize for fixed-depth.
              nodes.forEach(function(d) { d.y = d.depth * 60; });

              // Declare the nodesâ€¦
              var node = svg.selectAll("g.node")
            	  .data(nodes, function(d) { return d.id || (d.id = ++i); });

              // Enter the nodes.
              var nodeEnter = node.enter().append("g")
            	  .attr("class", "node")
            	  .attr("transform", function(d) {
            		  return "translate(" + d.y + "," + d.x + ")"; });

              nodeEnter.append("circle")
            	  .attr("r", 10)
            	  .style("fill", "#000");

              nodeEnter.append("text")
            	  .attr("x", function(d) {
            		  return d.children || d._children ? -13 : 13; })
            	  .attr("dy", ".35em")
            	  .attr("text-anchor", function(d) {
            		  return d.children || d._children ? "end" : "start"; })
            	  .text(function(d) { return d.name; })
            	  .style("fill-opacity", 1);

                node.attr("r", 10)
              	  .style("fill", "#000")
                    .attr("transform", function(d) {
                		  return "translate(" + d.y + "," + d.x + ")"; });


              node.selectAll("text").attr("x", function(d) {
              		  return d.children || d._children ? -13 : 13; })
              	  .attr("dy", ".35em")
              	  .attr("text-anchor", function(d) {
              		  return d.children || d._children ? "end" : "start"; })
              	  .text(function(d) { return d.name; })
              	  .style("fill-opacity", 1);


              // Declare the linksâ€¦
              var link = svg.selectAll("path.link")
            	  .data(links, function(d) { return d.target.id; });

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


    void AddNode(int parent, int child, Selection & svg) {
      EM_ASM_ARGS({
        var parent_node = js.objects[$3](js.objects[$0][0], $1);
        if (!parent_node.hasOwnProperty("children")){
         parent_node.children = [];
        }
        parent_node.children.push({"name":$2, "parent":$1, "children":[]});
        js.objects[$5](parent_node, js.objects[$4]);
    }, data.GetID(), parent, child, find_parent.GetID(), svg.GetID(), update_fun.GetID());
    }

  };
}


#endif
