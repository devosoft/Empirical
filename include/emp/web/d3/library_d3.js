var D3Library = {
    $js: {objects:{}, counts:{}, next_id:0},
    n_objects: function(){
      return js.objects.length;
    },
    // D3_Initialize: function() {
    //   $.getScript('d3.min.js', function(data, textStatus) { console.log("yay", textStatus); return true ;});
    // },
};

autoAddDeps(D3Library, '$js');
mergeInto(LibraryManager.library, D3Library);
