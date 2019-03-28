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

#include <unordered_map>
#include <string>

#include "js_utils.h"

namespace emp {
namespace web {

  std::unordered_map<std::string, std::string> GetUrlParams() {

    emp::vector<emp::vector<std::string>> incoming;

    EM_ASM({
      const params = new URLSearchParams(location.search);
      emp_i.__outgoing_array = [... params.keys()].map(
        function(x) { return [x, params.get(x)]; }
      );
    });

    emp::pass_vector_to_cpp(incoming);

    std::unordered_map<std::string, std::string> res;

    for (const auto & pair : incoming) res[pair[0]] = pair[1];

    return res;

  }

}
}

#endif
