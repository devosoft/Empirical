mergeInto(LibraryManager.library, {
    $emp: { cb_args:[] },

    EMP_GetCBArgCount__deps: ['$emp'],
    EMP_GetCBArgCount: function() { return emp.cb_args.length; },
});
