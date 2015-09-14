var D3Library = {
    $js: {objects:[], charts:[]},
    n_objects: function(){
	return js.objects.length;
    },
};

autoAddDeps(D3Library, '$js');
mergeInto(LibraryManager.library, D3Library);
