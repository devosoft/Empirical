var D3Library = {
  $emp_d3: {objects: {}, counts: {}, next_id:0},
  get_emp_d3: function() {
    return this.emp_d3;
  },
  // If the 'func_name' is a function in the global namespace, the d3 namespace, or the emp namespace
  // return the proper function, otherwise just return 'func_name'
  find_function: function(sel) {
    return (window["d3"][sel] === "function") ? window["d3"][sel] :
           (window["emp"][sel] === "function") ? window["emp"][sel] :
           (window[sel] === "function") ? window[sel] :
           sel;
  },
  // Is the 'func_name' a function in the global namespace, the d3 namespace, or the emp namespace?
  is_function: function(func_name) {
    return ((window["d3"][func_name] === "function") ||
            (window["emp"][func_name] === "function") ||
            (window[func_name] === "function"));
  }
};

autoAddDeps(D3Library, '$emp_d3');
mergeInto(LibraryManager.library, D3Library);