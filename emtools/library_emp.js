mergeInto(LibraryManager.library, {
    // Data accessible to library users.
    $emp: {
        Callback: function() {
            // Copy over the additional arguments
            emp_i.cb_args = [];
            emp_i.cb_return = 0;
            for (var i = 1; i < arguments.length; i++) {
                emp_i.cb_args[i-1] = arguments[i];
            }

            // Callback to the original function.
            empCppCallback(arguments[0]);

            return emp_i.cb_return;
        }
    },
    
    // Data internal to EMP
    $emp_i: { cb_args:[], cb_return:0
            },

    EMP_GetCBArgCount__deps: ['$emp', '$emp_i'],
    EMP_GetCBArgCount: function() { return emp_i.cb_args.length; },
});
