mergeInto(LibraryManager.library, {
    // Data accessible to library users.
    $emp: { Callback: function() {
                // Copy over the additional arguments
                emp_i.cb_args = [];
                for (var i = 1; i < arguments.length; i++) {
                    emp_i.cb_args[i-1] = arguments[i];
                }

                // Callback to the original function.
                empCppCallback(arguments[0]);
            }
          },

    // Data internal to EMP
    $emp_i: { cb_args:[]
            },

    EMP_GetCBArgCount__deps: ['$emp', '$emp_i'],
    EMP_GetCBArgCount: function() { return emp_i.cb_args.length; },
});
