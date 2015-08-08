#ifndef EMP_WEB_H
#define EMP_WEB_H

#include "../emtools/init.h"

#include "Button.h"
#include "Canvas.h"
#include "Image.h"
#include "Selector.h"
#include "Slate.h"
#include "Table.h"
#include "Text.h"


namespace emp {
namespace web {

  bool Initialize() {

    // Make sure we only run Initialize once!
    static bool init = false;
    if (init) return false;
    init = true;

    // Run the base-level initialize in case it hasn't be run yet.
    emp::Initialize();

    // Setup the animation callback in Javascript
    EM_ASM({
        window.requestAnimFrame = (function(callback) {
            return window.requestAnimationFrame
              || window.webkitRequestAnimationFrame
              || window.mozRequestAnimationFrame
              || window.oRequestAnimationFrame
              || window.msRequestAnimationFrame
              || function(callback) { window.setTimeout(callback, 1000 / 60); };
          })();
      });

    return true;
  }


  //  A Document is a root-level slate that automatically initializes Empirical and starts
  //  out in an active state.

  class Document : public Slate {
  public:
    Document(const std::string & doc_id) : Slate(doc_id) {
      emp::web::Initialize();
      Activate();
    }
    ~Document() { ; }
  };

};
};

#endif
