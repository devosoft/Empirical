var D3Library = {
    $js: {selections:[]},
    n_selections: function(){
	return js.selections.length;
    },
};

autoAddDeps(D3Library, '$js');
mergeInto(LibraryManager.library, D3Library);
