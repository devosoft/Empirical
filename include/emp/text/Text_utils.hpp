/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2024
 *
 *  @file Text_utils.hpp
 *  @brief Helper functions for building Text objects with different encodings.
 *  @note Status: ALPHA
 *
 */

#ifndef EMP_TEXT_TEXT_UTILS_HPP_INCLUDE
#define EMP_TEXT_TEXT_UTILS_HPP_INCLUDE

#include <map>
#include <type_traits>
#include <unordered_map>

#include "../base/assert.hpp"
#include "../base/notify.hpp"
#include "../bits/BitVector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../tools/String.hpp"

#include "_TextCharRef.hpp"

namespace emp {

  /// A mapping of html tags to the emphatic style that they represent.
  /// Since the mapping between HTML and Emphatic text encodings are useful in multiple contexts,
  /// include easy access to them here.
  static const auto & GetTextStyleMap_FromHTML() {
    static std::map<String, String> html_map;
    if (html_map.size() == 0) {
      html_map["b"] = "bold";
      html_map["big"] = "big";
      html_map["code"] = "code";
      html_map["del"] = "strikethrough";
      html_map["i"] = "italic";
      html_map["small"] = "small";
      html_map["sub"] = "subscript";
      html_map["sup"] = "superscript";
      html_map["u"] = "underline";
      html_map["h1"] = "header1";
      html_map["h2"] = "header2";
      html_map["h3"] = "header3";
      html_map["h4"] = "header4";
      html_map["h5"] = "header5";
      html_map["h6"] = "header6";
    }
    return html_map;
  }

  /// Request a map of html tags to the emphatic style that they represent.
  /// Since the mapping between HTML and Emphatic text encodings are useful in multiple contexts,
  /// include easy access to them here.
  static const auto & GetTextStyleMap_ToHTML() {
    using map_t = std::map<String, String>;
    static map_t out_map = emp::flip_map<String, String, map_t>(GetTextStyleMap_FromHTML());
    return out_map;
  }
}

#endif // #ifndef EMP_TEXT_TEXT_UTILS_HPP_INCLUDE
