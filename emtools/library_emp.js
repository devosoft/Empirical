mergeInto(LibraryManager.library, {
    $emp: { cb_args:[],
            CppCallback: function() {
                // Copy over the additional arguments
                emp.cb_args = [];
                for (var i = 1; i < arguments.length; i++) {
                    emp.cb_args[i-1] = arguments[i];
                }

                // Callback to the original function.
                empCppCallback(arguments[0]);
            }
          },

    EMP_GetCBArgCount__deps: ['$emp'],
    EMP_GetCBArgCount: function() { return emp.cb_args.length; },
});
