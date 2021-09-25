/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file UrlParams.hpp
 *  @brief Get an unordered_map containing url query key/value parameters.
 *
 */

#ifndef EMP_WEB_URLPARAMS_HPP_INCLUDE
#define EMP_WEB_URLPARAMS_HPP_INCLUDE

#include <map>
#include <string>

#include "js_utils.hpp"
#include "JSWrap.hpp"

namespace emp {
namespace web {
  /**
   * Extracts the query portion of a url and parses for key value pairs.
   *
   * @note In a query space encoded by "%20" is interpreted as a space character
   * while the space encoded by "+" is used to separate a list of values for a key.
   * For example querying "?seed=100&strings=hi%20there+blah" will be parsed
   * to the parameter array [["seed", "100"], ["strings", "hi there", "blah"]]
   * then converted to a multimap.
   *
   * Some esoterica: "?string=this+that" and "?string=this&string=that"
   * have different meanings and may result in different behavior.
   * Similarly "?test" becomes [["test"]] while "?test=" becomes
   * [["test", ""]].
   */
  std::multimap<std::string, emp::vector<std::string>> GetUrlParams() {

    emp::vector<emp::vector<std::string>> incoming;

    MAIN_THREAD_EM_ASM({
      emp_i.__outgoing_array = location.search.includes('?')
      ? location.search.substring(1).split('&'
        ).map(
          expr => expr.split("=")
        ).map(
          (list) => [list[0].split("+").join(" ")].concat(
            list[1] && list[1].split('+')
          ).filter(
            item => item !== undefined
          )
        ).map(
          list => list.map(decodeURIComponent)
        ).map(
          p => p[0].split(" ").join("").length == 0
            ?  ["_illegal", "_empty=" + p[1]] : p
        ).map(
          p => p[0].includes(" ") ? ["_illegal", p[0] + "=" + p[1]] : p
        )
      : [];
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

#endif // #ifndef EMP_WEB_URLPARAMS_HPP_INCLUDE
