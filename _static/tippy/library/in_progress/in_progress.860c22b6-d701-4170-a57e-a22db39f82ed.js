selector_to_html = {"a[href=\"api/Parser.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Parser.hpp<a class=\"headerlink\" href=\"#parser-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"api/class.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">class.hpp<a class=\"headerlink\" href=\"#class-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"api/fixed.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">fixed.hpp<a class=\"headerlink\" href=\"#fixed-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"api/SimpleLexer.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">SimpleLexer.hpp<a class=\"headerlink\" href=\"#simplelexer-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"api/Ptr-overload-fix.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Ptr-overload-fix.hpp<a class=\"headerlink\" href=\"#ptr-overload-fix-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"api/SimpleParser.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">SimpleParser.hpp<a class=\"headerlink\" href=\"#simpleparser-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"api/ConfigLexer.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">ConfigLexer.hpp<a class=\"headerlink\" href=\"#configlexer-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"api/struct.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">struct.hpp<a class=\"headerlink\" href=\"#struct-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"api/AST.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">AST.hpp<a class=\"headerlink\" href=\"#ast-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"api/ConfigParser.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">ConfigParser.hpp<a class=\"headerlink\" href=\"#configparser-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"api/TrackedPtr.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">TrackedPtr.hpp<a class=\"headerlink\" href=\"#trackedptr-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"api/Trait.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Trait.hpp<a class=\"headerlink\" href=\"#trait-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"#in-progress\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">In_progress<a class=\"headerlink\" href=\"#in-progress\" title=\"Link to this heading\">\uf0c1</a></h1><h2>API<a class=\"headerlink\" href=\"#api\" title=\"Link to this heading\">\uf0c1</a></h2>", "a[href=\"#api\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">API<a class=\"headerlink\" href=\"#api\" title=\"Link to this heading\">\uf0c1</a></h2>", "a[href=\"api/BatchConfig.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">BatchConfig.hpp<a class=\"headerlink\" href=\"#batchconfig-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>"}
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
