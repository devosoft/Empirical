var D3Library = {
  $emp_d3: {objects: {}, counts: {}, next_id:0},
  get_emp_d3: function() {
    return this.emp_d3;
  }
};

autoAddDeps(D3Library, '$emp_d3');
mergeInto(LibraryManager.library, D3Library);