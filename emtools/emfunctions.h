#ifndef EMP_EM_FUNCTIONS_H
#define EMP_EM_FUNCTIONS_H

#include "../tools/callbacks.h"
#include "../tools/string_utils.h"

namespace emp {

  void Alert(const std::string & msg) { EM_ASM_ARGS({ msg = Pointer_stringify($0); alert(msg); }, msg.c_str()); }
  template <typename... TYPE_SET>
  void Alert(TYPE_SET... inputs) { Alert(emp::to_string(inputs...)); }

  // void Alert(int val) { Alert(std::to_string(val)); }
  // void Alert(double val) { Alert(std::to_string(val)); }

#define AlertVar(VAR) emp::Alert(std::string(#VAR) + std::string("=") + std::to_string(VAR))

  template<class T> void ScheduleMethod(T * target, void (T::*method_ptr)(), int delay) {
    MethodCallback<T> * new_callback = new MethodCallback<T>(target, method_ptr);
    new_callback->SetDisposible(); // Get rid of this object after one use.
    EM_ASM_ARGS({ window.setTimeout(function() { empJSDoCallback($0); }, $1); }, (int) new_callback, delay);
  }

  int GetWindowInnerWidth() { return EM_ASM_INT_V({ return window.innerWidth; }); }
  int GetWindowInnerHeight() { return EM_ASM_INT_V({ return window.innerHeight; }); }

  // These may already be in HTML5 for Emscripten
  void SetCursor(const char * type) {
    EM_ASM_ARGS({
        var type = Pointer_stringify($0);
        document.body.style.cursor = type;
    }, type);
  }

  void OpenWindow(const std::string & url) {
    EM_ASM_ARGS({
        var url = Pointer_stringify($0);
        window.open = url;
    }, url.c_str());
  }
};

#endif
