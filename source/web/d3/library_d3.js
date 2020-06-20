var D3Library = {
  $emp_d3: {
    objects: {},
    counts: {},
    next_id: 0,
    //, usable_namespaces: ["d3", "emp"]
    //  add_namespace: function (sel) {
    //   this.emp_d3.usable_namespaces.push(sel);
    // },

    // remove_namespace: function (sel) {
    //   const index = this.emp_d3.usable_namespaces.indexOf(sel);
    //   if (index !== -1) this.emp_d3.usable_namespaces.splice(index, 1);
    // },
    // This is a useful utility function for testing. You shouldn't call this in application code unless
    // you really know what you're doing. This WILL break any existing emp d3 objects.
    clear_emp_d3: function() {
      // this.emp_d3 = {objects: {}, counts: {}, next_id:0};
      this.objects = {};
      this.counts = {};
      this.next_id = 0;
    },
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
  },

  get_emp_d3: function() {
    return this.emp_d3;
  },

  // // This is a useful utility function for testing. You shouldn't call this in application code unless
  // // you really know what you're doing. This WILL break any existing emp d3 objects.
  // clear_emp_d3: function() {
  //   // this.emp_d3 = {objects: {}, counts: {}, next_id:0};
  //   this.emp_d3["objects"] =  {};
  //   this.emp_d3["counts"] = {};
  //   this.emp_d3["next_id"] = 0;
  // },

};

autoAddDeps(D3Library, '$emp_d3');
mergeInto(LibraryManager.library, D3Library);