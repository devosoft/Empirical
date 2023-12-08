selector_to_html = {"a[href=\"api/span.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">span.hpp<a class=\"headerlink\" href=\"#span-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>Polyfill for C++20 std::span. Since deprecated. </p>", "a[href=\"#polyfill\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Polyfill<a class=\"headerlink\" href=\"#polyfill\" title=\"Link to this heading\">\uf0c1</a></h1><h2>API<a class=\"headerlink\" href=\"#api\" title=\"Link to this heading\">\uf0c1</a></h2>", "a[href=\"#api\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">API<a class=\"headerlink\" href=\"#api\" title=\"Link to this heading\">\uf0c1</a></h2>"}
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
