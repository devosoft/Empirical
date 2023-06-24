/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2021
*/
/**
 *  @file
 */

#include <iostream>

#include <emscripten.h>

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
