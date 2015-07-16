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
        },

        InspectObj: function(o,i) {
            // From: http://stackoverflow.com/questions/5357442/how-to-inspect-javascript-objects
            if (typeof i == 'undefined') i='';
            if (i.length > 50) return '[MAX ITERATIONS]';
            var r = [];
            for(var p in o){
                var t = typeof o[p];
                r.push(i + '"' + p + '" (' + t + ') => '
                       + (t == 'object' ? 'object:' + InspectObj(o[p], i+'  ') : o[p] + ''));
            }
            return r.join(i+'\n');
        }

    },
    
    // Data internal to EMP
    $emp_i: { cb_args:[], cb_return:0
            },

    EMP_Initialize__deps: ['$emp', '$emp_i'],
    EMP_Initialize: function () {
        empCppCallback = Module.cwrap('empCppCallback', null, ['number']);
    },

    EMP_GetCBArgCount__deps: ['$emp', '$emp_i'],
    EMP_GetCBArgCount: function() { return emp_i.cb_args.length; },
});
