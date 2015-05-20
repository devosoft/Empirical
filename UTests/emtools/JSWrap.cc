#include <emscripten.h>

#include "../../tools/alert.h"
#include "../../emtools/JSWrap.h"


void TestFun(int w, int x, int y, double z) {
  emp::Alert(w + x*y*z);
}


int main() {
  unsigned int fun_id = emp::JSWrap(TestFun);

  EM_ASM({
      emp.cb_args = [100, 2.5, -4, 8.5];
    });

  empCppCallback(fun_id);
}
