#include <emscripten.h>

#include "../../tools/alert.h"
#include "../../emtools/JSWrap.h"


void TestFun(int x, int y, int z) {
  emp::Alert(x*y*z);
}


int main() {
  std::function<void(int,int,int)> test_ptr(TestFun);
  // emp::JSWrap<int,int,int>(&TestFun);
  emp::JSWrap<int,int,int>(test_ptr);

  emp::Alert("Testing!");
}
