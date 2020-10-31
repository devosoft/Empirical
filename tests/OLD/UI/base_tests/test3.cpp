#include <emscripten.h>
#include <iostream>

extern "C" {
  extern void EMP_TestJQ(const char * tag_name, int value);
}

int main() {
  EMP_TestJQ("a", 100);
  EM_ASM({
      $( document ).ready(function() {
          $( "a" ).after("<p>More text here.</p>");
          $( "p" ).before("<p>And still more...</p>");
        });
    });
  
}
