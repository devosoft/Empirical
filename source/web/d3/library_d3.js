var D3Library = {
  $emp_d3: {objects: {}, counts: {}, next_id:0},
  get_emp_d3: function() {
    return this.emp_d3;
  },
  find_function: function(sel) {
    return (window["d3"][sel] === "function") ? window["d3"][sel] :
           (window["emp"][sel] === "function") ? window["emp"][sel] :
           (window[sel] === "function") ? window[sel] :
           sel;
  }
};

autoAddDeps(D3Library, '$emp_d3');
mergeInto(LibraryManager.library, D3Library);