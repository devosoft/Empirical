mergeInto(LibraryManager.library, {
    $emp_data: { callback_args:[] },

    EMP_GetCBArgCount__deps: ['$emp_data'],
    EMP_GetCBArgCount: function() { return emp_data.callback_args.length; },

    EMP_GetCBArg_int: function(arg_id) { return emp_data.callback_args[arg_id]; },

});
