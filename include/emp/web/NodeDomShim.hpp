/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  NodeDomShim.h
 *  @brief Shim for Empirical compatibility with node.js
 */

#ifndef EMP_NODE_DOM_SHIM_H
#define EMP_NODE_DOM_SHIM_H

#include <string>

#include "../base/vector.hpp"
#include "../tools/string_utils.hpp"

#include "Document.hpp"

namespace emp {
namespace web {

  struct NodeDomShim {

    NodeDomShim(const emp::vector<std::string>& init_divs={}) { MAIN_THREAD_EM_ASM(

      // setup jsdom
      var jsdom = require("jsdom");
      var JSDOM = jsdom.JSDOM;

      global.dom = (new JSDOM(`<div id="emp_base"></div>`));

      // setup jquery
      var jquery = require('jquery');
      global.$ = jquery( dom.window );

      global.window = dom.window;
      global.document = dom.window.document;

      // shim for alert
      global.alert = console.log;

    );

    for (const auto& id : init_divs) {
      const std::string command = emp::format_string(
        R"(
          var to_add = document.createElement('div');
          to_add.setAttribute('id', '%s');
          document.getElementById('emp_base').appendChild(to_add);
        )",
        id.c_str()
      );
      emscripten_run_script( command.c_str() );
    }

  }

  };

}
}

#endif
