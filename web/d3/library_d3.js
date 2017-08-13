var D3Library = {
    $js: {objects:{}, counts:{}, next_id:0},
    n_objects: function(){
	return js.objects.length;
    },
};

autoAddDeps(D3Library, '$js');
mergeInto(LibraryManager.library, D3Library);
