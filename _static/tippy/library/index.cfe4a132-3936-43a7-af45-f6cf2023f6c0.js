selector_to_html = {"a[href=\"data/data.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Data Collection and Recording Tools<a class=\"headerlink\" href=\"#data-collection-and-recording-tools\" title=\"Link to this heading\">\uf0c1</a></h1><p>Empirical includes a variety of tools for gathering and recording data.\nThe core of these tools is the DataNode class. DataNodes are containers\nthat you can pass as much data as you like into. When DataNodes are\nbuilt, they can be given various modifiers (specified as template\narguments) which control how much information they will collect about\nthe data they are passed. For instance, the <code class=\"docutils literal notranslate\"><span class=\"pre\">data::Current</span></code> modifier\ngives the DataNode the power to remember the last value it was passed,\nwhereas the <code class=\"docutils literal notranslate\"><span class=\"pre\">data::Stats</span></code> modifier keeps track of a variety of\nstatistics about the distribution of data that the node has been\npassed. Except where otherwise noted, modifiers can be combined freely.\nSome also have dependencies on simpler modifiers. On the whole,\nDataNodes are designed to be as light-weight as possible while still\nkeeping track of the desired information.</p><p>DataNodes that accept the same type of data and have the same modifiers\ncan be grouped together using a DataManager.</p>", "a[href=\"#using-empirical\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Using Empirical<a class=\"headerlink\" href=\"#using-empirical\" title=\"Link to this heading\">\uf0c1</a></h1><p>Contents:</p>", "a[href=\"tools/tools.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Other Tools<a class=\"headerlink\" href=\"#other-tools\" title=\"Link to this heading\">\uf0c1</a></h1><h2>API<a class=\"headerlink\" href=\"#api\" title=\"Link to this heading\">\uf0c1</a></h2>", "a[href=\"web/web.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Web Tools (for use with Emscripten)<a class=\"headerlink\" href=\"#web-tools-for-use-with-emscripten\" title=\"Link to this heading\">\uf0c1</a></h1><h2>D3 Data Visualization Tools<a class=\"headerlink\" href=\"#d3-data-visualization-tools\" title=\"Link to this heading\">\uf0c1</a></h2>", "a[href=\"testing/testing.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Testing<a class=\"headerlink\" href=\"#testing\" title=\"Link to this heading\">\uf0c1</a></h1><h2>API<a class=\"headerlink\" href=\"#api\" title=\"Link to this heading\">\uf0c1</a></h2>", "a[href=\"prefab/prefab.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Prefab<a class=\"headerlink\" href=\"#prefab\" title=\"Link to this heading\">\uf0c1</a></h1><h2>API<a class=\"headerlink\" href=\"#api\" title=\"Link to this heading\">\uf0c1</a></h2>", "a[href=\"debug/debug.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Debug<a class=\"headerlink\" href=\"#debug\" title=\"Link to this heading\">\uf0c1</a></h1><h2>API<a class=\"headerlink\" href=\"#api\" title=\"Link to this heading\">\uf0c1</a></h2>", "a[href=\"datastructs/datastructs.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Data structures<a class=\"headerlink\" href=\"#data-structures\" title=\"Link to this heading\">\uf0c1</a></h1><h2>API<a class=\"headerlink\" href=\"#api\" title=\"Link to this heading\">\uf0c1</a></h2>", "a[href=\"io/io.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">IO<a class=\"headerlink\" href=\"#io\" title=\"Link to this heading\">\uf0c1</a></h1><h2>API<a class=\"headerlink\" href=\"#api\" title=\"Link to this heading\">\uf0c1</a></h2>", "a[href=\"compiler/compiler.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Compiler<a class=\"headerlink\" href=\"#compiler\" title=\"Link to this heading\">\uf0c1</a></h1><h2>API<a class=\"headerlink\" href=\"#api\" title=\"Link to this heading\">\uf0c1</a></h2>", "a[href=\"base/base.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">General Purpose Debugging Tools<a class=\"headerlink\" href=\"#general-purpose-debugging-tools\" title=\"Link to this heading\">\uf0c1</a></h1><h2>Empirical vectors and arrays<a class=\"headerlink\" href=\"#empirical-vectors-and-arrays\" title=\"Link to this heading\">\uf0c1</a></h2><p>Empirical contains of intelligent versions of STL vectors and arrays\nthat will warn you if you make common errors, but otherwise behave\nidentically to the STL version. Most importantly, they will detect\nattempts to index to out-of-bounds locations and throw an error. These\nfeatures are critical for writing code that will be compiled to\nJavascript with Emscripten, because Valgrind doesn\u2019t run on Javascript.\nThey also save a lot of debugging time when writing native C++ code.</p><p>\u201cBut wait,\u201d you might say, \u201csurely all of these additional checks\nslow down your code!\u201d This is true when you compile in debug mode (the\ndefault). However, when you compile with the -DNDEBUG flag, these\nobjects are directly replaced with their STL equivalent, removing any\nslowdown. That way, you can get all the debugging benefits while you\u2019re\nwriting your program, but all the speed benefits when you\u2019re actually\nusing it.</p>", "a[href=\"functional/functional.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Functional<a class=\"headerlink\" href=\"#functional\" title=\"Link to this heading\">\uf0c1</a></h1><h2>API<a class=\"headerlink\" href=\"#api\" title=\"Link to this heading\">\uf0c1</a></h2>", "a[href=\"bits/bits.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Bits<a class=\"headerlink\" href=\"#bits\" title=\"Link to this heading\">\uf0c1</a></h1><p>Stuff about bits</p>", "a[href=\"math/math.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Math<a class=\"headerlink\" href=\"#math\" title=\"Link to this heading\">\uf0c1</a></h1><h2>API<a class=\"headerlink\" href=\"#api\" title=\"Link to this heading\">\uf0c1</a></h2>"}
skip_classes = ["headerlink", "sd-stretched-link"]

window.onload = function () {
    for (const [select, tip_html] of Object.entries(selector_to_html)) {
        const links = document.querySelectorAll(` ${select}`);
        for (const link of links) {
            if (skip_classes.some(c => link.classList.contains(c))) {
                continue;
            }

            tippy(link, {
                content: tip_html,
                allowHTML: true,
                arrow: true,
                placement: 'auto-start', maxWidth: 500, interactive: false,

            });
        };
    };
    console.log("tippy tips loaded!");
};
