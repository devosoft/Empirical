/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  UrlParams.h
 *  @brief Get an unordered_map containing url query key/value parameters.
 *
 */

#ifndef EMP_WEB_UrlParams_H
#define EMP_WEB_UrlParams_H

#include <map>
#include <string>

#include "JSWrap.h"
#include "js_utils.h"

namespace emp {
namespace web {

  std::multimap<std::string, emp::vector<std::string>> GetUrlParams() {

    emp::vector<emp::vector<std::string>> incoming;

    EM_ASM({
      const params = new URLSearchParams(location.search);
      emp_i.__outgoing_array = Array.from(
        params.entries()
      ).map(
        p => p[0].replace(" ", "").length == 0
          ?  ["_illegal", "_empty" + " " + p[1]] : p
      ).map(
        p => p[0].includes(" ") ? ["_illegal", p[0] + " " + p[1]] : p
      ).map(
        p => [p[0]].concat(p[1].split(" "))
      ).map(
        p => p.filter(w => w.length > 0)
      );
    });

    emp::pass_vector_to_cpp(incoming);

    std::multimap<std::string, emp::vector<std::string>> res;

    for (const auto & pack : incoming) {
      res.insert({
        pack.front(),
        emp::vector<std::string>(
          std::next(std::begin(pack)),
          std::end(pack)
        )
      });
    }

    return res;

  }

}
}

#endif
