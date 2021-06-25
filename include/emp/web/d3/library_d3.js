var D3Library = {
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

    // Inspired by Niels' answer to
    // http://stackoverflow.com/questions/12899609/how-to-add-an-object-to-a-nested-javascript-object-using-a-parent-id/37888800#37888800
    find_in_hierarchy: function(root, id) {
      if (root.name == id) {
        return root;
      }
      if (root.children) {
        for (var k in root.children) {
          if (root.children[k].name == id) {
            return root.children[k];
          }
          else if (root.children[k].children) {
            result = this.find_in_hierarchy(root.children[k], id);
            if (result) {
              return result;
            }
          }
        }
      }
    },

  },

  // TODO: Rename to InitializeEmpD3?
  get_emp_d3: function() {
    return this.emp_d3;
  }
};

autoAddDeps(D3Library, '$emp_d3');
mergeInto(LibraryManager.library, D3Library);