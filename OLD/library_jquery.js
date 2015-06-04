mergeInto(LibraryManager.library, {

    EMP_TestJQ: function(tag_name, value) {
      $( document ).ready(function() {
          $( Pointer_stringify(tag_name) ).click(function( event ) {
              alert( "Thanks for visiting!" + value );
              event.preventDefault();
            });
      });
    }

});
