var D3Library = {
<<<<<<< HEAD
  $emp_d3: {objects: {}, counts: {}, next_id:0, usable_namespaces: ["d3", "emp"]},
  get_emp_d3: function() {
    return this.emp_d3;
  },
  
  add_namespace: function (sel) {
    this.emp_d3.usable_namespaces.push(sel);
  },

  remove_namespace: function (sel) {
    const index = this.emp_d3.usable_namespaces.indexOf(sel);
    if (index !== -1) this.emp_d3.usable_namespaces.splice(index, 1);
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
=======
  $emp_d3: {
    objects: {},
    counts: {},
    next_id: 0,
    searchable_namespaces: ["d3", "emp"], // This defines search priority order.

    // Add namespace (name) to set of namespaces that the d3 wrapper searches when looking for a javascript
    // function (via is_/find_function).
    // The namespace must exist in 'window'.
    // Returns whether namespace is successfully added.
    add_searchable_namespace: function(name) {
      if (!(name in this.searchable_namespaces) && name in window) {
        this.searchable_namespaces.push(name);
        return true;
      }
      return false;
    },

    // Remove a namespace from the set of namespaces.
    // Namespace must exist in searchable_namespaces.
    // Returns whether or not namespace is successfully removed.
    remove_searchable_namespace: function(name) {
      const index = this.searchable_namespaces.indexOf(name);
      if (index != -1) {
        this.searchable_namespaces.splice(index, 1);
        return true;
      }
      return false;
    },

    // This is a useful utility function for testing. You shouldn't call this in application code unless
    // you really know what you're doing. This WILL break any existing emp d3 objects.
    clear_emp_d3: function() {
      // this.emp_d3 = {objects: {}, counts: {}, next_id:0};
      this.objects = {};
      this.counts = {};
      this.next_id = 0;
      this.searchable_namespaces = ["d3", "emp"];
    },

    // If the 'func_name' is a function in the global namespace, the d3 namespace, or the emp namespace
    // return the proper function, otherwise just return 'func_name'
    find_function: function(func_name) {
      for (namespace of this.searchable_namespaces) {
        if (!(func_name in window[namespace])) { continue; }
        if (typeof window[namespace][func_name] === "function") {
          return window[namespace][func_name];
        }
      }
      if (!(func_name in window)) {
        return func_name;
      }
      return (typeof window[func_name] === "function") ? window[func_name] : func_name;
    },

    // Is the 'func_name' a function in the global namespace, the d3 namespace, or the emp namespace?
    is_function: function(func_name) {
      for (namespace of this.searchable_namespaces) {
        if (!(func_name in window[namespace])) { continue; }
        if (typeof window[namespace][func_name] === "function") {
          return true;
        }
      }
      if (!(func_name in window)) {
        return false;
      }
      return (typeof window[func_name] === "function");
    },

  },

  // TODO: Rename to InitializeEmpD3?
  get_emp_d3: function() {
    return this.emp_d3;
>>>>>>> d3-wrapper
  }
};

autoAddDeps(D3Library, '$emp_d3');
mergeInto(LibraryManager.library, D3Library);