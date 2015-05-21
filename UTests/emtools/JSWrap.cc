#include <emscripten.h>

#include "../../tools/alert.h"
#include "../../emtools/JSWrap.h"
#include "../../emtools/init.h"

void TestFun(int w, int x, int y, double z) {
  emp::Alert(w + x*y*z);
}


int main() {

  emp::Initialize();

  unsigned int fun_id = emp::JSWrap(TestFun);


  EM_ASM_ARGS({
      emp.CppCallback($0, 5, 17, 4, 1.5);
    }, fun_id);
}
