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

namespace emp {
namespace web {

  struct NodeDomShim {

    NodeDomShim() { EM_ASM(

      // setup jsdom
      var jsdom = require("jsdom");
      var JSDOM = jsdom.JSDOM;

      global.dom = (new JSDOM(`<div id="emp_base"></div>`));
      global.document = dom.window.document;

      // setup jquery
      var jquery = require('jquery');
      global.$ = jquery( dom.window );

      // shim for alert
      global.alert = console.log;

    ); }

  };

}
}

#endif
