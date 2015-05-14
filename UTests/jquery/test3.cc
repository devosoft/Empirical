#include <emscripten.h>
#include <iostream>

extern "C" {
  extern void EMP_TestJQ(const char * tag_name, int value);
}

int main() {
  EMP_TestJQ("a", 100);
}
