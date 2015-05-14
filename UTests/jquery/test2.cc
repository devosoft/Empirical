#include <emscripten.h>
#include <iostream>

int main() {
  int a = EM_ASM_INT({
      $( document ).ready(function() {
          $( "a" ).click(function( event ) {
              alert( "Thanks for visiting! " + $0 );
              event.preventDefault();
            });
          return 42;
        });
    }, 12);
}
