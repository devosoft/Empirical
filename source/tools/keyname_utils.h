/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file keyname_utils.h
 *  @brief Utility functions that support the keyname file naming convention.
 *
 *  Plays nice with the Python package keyname.
 *  https://github.com/mmore500/keyname
 */


#ifndef EMP_KEY_NAME_UTILS_H
#define EMP_KEY_NAME_UTILS_H

#include <algorithm>
#include <unordered_map>
#include <map>
#include <string>
#ifndef EMSCRIPTEN
#include <filesystem>
#else
#include <experimental/filesystem>
#endif
#include <array>

#include "../base/assert.h"
#include "../base/vector.h"
#include "../tools/string_utils.h"

namespace emp::keyname {

  using unpack_t = std::unordered_map<std::string, std::string>;

  std::string pack(const unpack_t & dict) {

    std::map<std::string, std::string> regular, underscore, ext;

    for (const auto &[k, v] : dict) {
      if (k == "ext") ext[k] = v;
      else if (k.size() && k[0] == '_') {
        // ignore "_", this is a reserved keyword
        if (k.size() > 1) underscore[k] = v;
      } else {
        regular[k] = v;
      }
    }

    emp::vector<std::string> res;

    for (const auto & map : {regular, underscore, ext}) {
      std::transform(
        std::cbegin(map),
        std::cend(map),
        std::back_inserter(res),
        [](const std::pair<std::string, std::string> & pair){
          std::array<std::string, 2> arr{pair.first, pair.second};
          emp_assert([&arr](){ // check for illegal characters
            for (const char c : {'=', '+'}) {
              for (const auto &s : arr) {
                if(s.find(c) != std::string::npos) return false;
              }
            }
            return true;
          }());
          return emp::join_on(arr, "=");
        }
      );
    }

    return emp::join_on(res, "+");

  }

  unpack_t unpack(const std::string & filename) {

    unpack_t res;

    const auto kv_strs = emp::slice(
#ifndef EMSCRIPTEN
      std::filesystem::path(filename).filename(), // get basename
#else
      std::experimental::filesystem::path(filename).filename(), // get basename
#endif
      '+'
    );

    std::transform(
      std::cbegin(kv_strs),
      std::cend(kv_strs),
      std::inserter(res, std::end(res)),
      [](const std::string & kv_str){
        const auto kv_vec = emp::slice(kv_str, '=', 1);
        emp_assert(kv_vec.size() > 0);
        emp_assert(kv_vec.size() <= 2);
        return (
          kv_vec.size() == 1
          ? decltype(res)::value_type(kv_vec[0], "")
          : decltype(res)::value_type(kv_vec[0], kv_vec[1])
        );
      }
    );

    res["_"] = filename;

    return res;

  }



}

#endif
