#include <emscripten.h>

#include "../../tools/alert.h"
#include "../../emtools/JSWrap.h"
#include "../../emtools/init.h"

void TestFun1(int w, int x, int y, double z) {
  emp::Alert(w + x*y*z);
}

void TestFun2(double a, double b) {
  emp::Alert( a - b );
}


int main() {

  emp::Initialize();

  uint32_t fun_id1 = emp::JSWrap(TestFun1, "TestName1", false);
  uint32_t fun_id2 = emp::JSWrap(TestFun2, "TestName2", false);


  EM_ASM_ARGS({
      emp.Callback($0, 5, 17, 4, 1.5);
    }, fun_id1);

  EM_ASM_ARGS({
      emp.Callback($0, 4.0, 2.5);
    }, fun_id2);

  EM_ASM({
      emp.TestName1(20, 10, 1, 0.5);
    });

  EM_ASM({
      emp.TestName2(1.5, 1.5);
    });
}
