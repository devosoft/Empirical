selector_to_html = {"a[href=\"#init-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">init.hpp<a class=\"headerlink\" href=\"#init-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>Define Initialize() and other functions to set up Empirical to build Emscripten projects. </p><p>Init.hpp should always be included if you are compiling Empirical\u2019s web tools with Emscripten. It handles making sure that behind the scenes stuff is all set up properly. It also defines some useful stubs and dummy functions so that your code will still be possible to comple with a normal C++ compiler (although the web part won\u2019t do anything, of course). These stubs are also helpful for avoiding confusion in linters and IDEs. </p>"}
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
