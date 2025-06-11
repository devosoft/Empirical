/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2020 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/NodeDomShim.hpp
 * @brief Shim for Empirical compatibility with node.js. Mimics the browser
 * document object model (DOM).
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_NODE_DOM_SHIM_HPP_GUARD
#define INCLUDE_EMP_WEB_NODE_DOM_SHIM_HPP_GUARD

#include <string>

#include "../base/vector.hpp"
#include "../tools/string_utils.hpp"

#include "Document.hpp"

namespace emp { namespace web {

  struct NodeDomShim {
    NodeDomShim(const emp::vector<std::string> & init_divs = {}) {
      MAIN_THREAD_EM_ASM(

        // setup jsdom
        var jsdom = require("jsdom"); var JSDOM = jsdom.JSDOM;

        global.dom = (new JSDOM(`<div id = "emp_base"></ div>`));

        // shim for alert
        global.alert = console.log;);

      for (const auto & id : init_divs) {
        const std::string command = emp::format_string(
          R"(
          var to_add = document.createElement('div');
          to_add.setAttribute('id', '%s');
          document.getElementById('emp_base').appendChild(to_add);
        )",
          id.c_str());
        emscripten_run_script(command.c_str());
      }
    }
  };

}}  // namespace emp::web

#endif  // #ifndef INCLUDE_EMP_WEB_NODE_DOM_SHIM_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: jsdom
