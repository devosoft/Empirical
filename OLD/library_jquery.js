mergeInto(LibraryManager.library, {

    EMP_TestUI: function(tag_name, value) {
      $( document ).ready(function() {
          $( Pointer_stringify(tag_name) ).click(function( event ) {
              alert( "Thanks for visiting!" + value );
              event.preventDefault();
            });
      });
    }

});
