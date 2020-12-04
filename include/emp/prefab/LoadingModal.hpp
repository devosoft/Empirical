#ifndef EMP_LOADING_MODAL_HPP
#define EMP_LOADING_MODAL_HPP

/**
 * To add a loading modal to your web page, you must
 * link the LoadingModal.js script directly after the
 * body tag in the HTML doc. To close the script, call
 * CloseLoadingModal() at the end of your .cc file, or
 * at the point which you wish to loading modal to disappear.
 *
 * See https://devosoft.github.io/empirical-prefab-demo/empirical-prefab-demo
 * for more details.
 */
#include "../tools/string_utils.hpp"
#include "../web/Element.hpp"
#include "../web/Div.hpp"
#include "../web/Widget.hpp"

namespace emp {
namespace prefab {
  /**
   * This method does not belong to a class, but it is used
   * to close a loading modal that is added with the
   * LoadingModal.js script. See the prefab demo site
   * for more details on how to implement the Loading Modal
   * in your web app. https://devosoft.github.io/empirical-prefab-demo
   */
  void CloseLoadingModal() {
    emscripten_run_script("CloseLoadingModal();");
  }
}
}

#endif
