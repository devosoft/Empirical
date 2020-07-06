#ifndef EMP_LOADING_MODAL_H
#define EMP_LOADING_MODAL_H

#include "../web/Element.h"
#include "../web/Div.h"
#include "../web/Widget.h"
#include "../tools/string_utils.h"

namespace emp {
    namespace prefab{
        void CloseLoadingModal(){
        emscripten_run_script("CloseLoadingModal();"); 
        }
    }
}

#endif

// Turn on
// $('.modal').modal('show');
// setTimeout(function () {
// $('.modal').modal('hide');
// }, 10000);

// Turn off
// emscripten_run_script("$('.modal').modal('hide');");