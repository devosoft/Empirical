/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  color_logic_utils.hpp
 *  @brief Represents detection of color type.
 *
 *  Adapted largely from https://github.com/mapbox/css-color-parser-cpp/.
 */


#ifndef EMP_WEB_COLOR_LOGIC_HPP
#define EMP_WEB_COLOR_LOGIC_HPP

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <string>

#if __has_include(<SFML/Color.hpp>)
#include <SFML/Color.hpp>
#endif

#include "../base/vector.hpp"
#include "../geometry/Circle2D.hpp"
#include "../tools/string_utils.hpp"
#include "Color.hpp"

namespace emp {
namespace web {
    bool DetectSyntaxABC(const std::string& str) {
      if (str.length() && str.front() == '#')
          return true;
      return false;
  }
}
}

#endif
