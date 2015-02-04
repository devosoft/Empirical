#include <emscripten.h>
#include <iostream>

extern "C" {
  extern void my_js();
}

int main()
{
  my_js();
}
