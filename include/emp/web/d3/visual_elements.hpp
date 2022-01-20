/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file visual_elements.hpp
 *  @brief TODO.
 *
 */

#ifndef EMP_WEB_D3_VISUAL_ELEMENTS_HPP_INCLUDE
#define EMP_WEB_D3_VISUAL_ELEMENTS_HPP_INCLUDE

#include "../../base/vector.hpp"

#include "../JSWrap.hpp"

#include "selection.hpp"

namespace D3 {

struct LocNode {
  EMP_BUILD_INTROSPECTIVE_TUPLE( int, loc)
};

template <typename NODE = LocNode>
class TileGrid {
public:

  int grid_width;
  int grid_height;
  int cell_size;
  D3::Selection svg;

  emp::vector<NODE> data = emp::vector<NODE>(grid_width*grid_height);

  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  std::function<int(NODE)> GetX = [this](NODE n) {
    return cell_size*(n.loc() % grid_width);
  };

  std::function<int(NODE)> GetY = [this](NODE n) {
    return cell_size*(n.loc() / grid_width);
  };
  #endif // DOXYGEN_SHOULD_SKIP_THIS

  void SetCellSize(int size) {
    cell_size = size;
    svg.SetAttr("width", cell_size*grid_width).SetAttr("height", cell_size*grid_height);
    svg.SelectAll("rect").SetAttr("width", cell_size).SetAttr("height", cell_size);
  }

  TileGrid(int width, int height, int cell_size, D3::Selection& svg) :
            grid_width(width), grid_height(height), cell_size(cell_size), svg(svg) {

    emp::JSWrap(GetX, emp::to_string(svg.GetID())+"get_x");
    emp::JSWrap(GetY, emp::to_string(svg.GetID())+"get_y");

    svg.SetAttr("width", cell_size*grid_width).SetAttr("height", cell_size*grid_height);
    for (int i = 0; i < grid_width*grid_height; ++i) {
      data[i].loc(i);
    }

    svg.SelectAll("rect").Data(data)
                         .EnterAppend("rect")
                         .SetStyle("fill", "black")
                         .SetStyle("stroke", "white")
                         .SetStyle("stroke-width", 1)
                         .SetAttr("width", cell_size)
                         .SetAttr("height", cell_size)
                         .SetAttr("x", emp::to_string(svg.GetID())+"get_x")
                         .SetAttr("y", emp::to_string(svg.GetID())+"get_y");
  }

  TileGrid(int width, int height, int cell_size = 15) :
            TileGrid(width, height, cell_size, D3::Select("body").Append("svg")){}

};

}

#endif // #ifndef EMP_WEB_D3_VISUAL_ELEMENTS_HPP_INCLUDE
