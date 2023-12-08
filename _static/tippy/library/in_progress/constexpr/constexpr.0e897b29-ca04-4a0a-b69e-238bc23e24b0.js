selector_to_html = {"a[href=\"api/ce_array.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">ce_array.hpp<a class=\"headerlink\" href=\"#ce-array-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"#constexpr\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Constexpr<a class=\"headerlink\" href=\"#constexpr\" title=\"Link to this heading\">\uf0c1</a></h1><h2>API<a class=\"headerlink\" href=\"#api\" title=\"Link to this heading\">\uf0c1</a></h2>", "a[href=\"api/ce_random.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">ce_random.hpp<a class=\"headerlink\" href=\"#ce-random-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"#api\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">API<a class=\"headerlink\" href=\"#api\" title=\"Link to this heading\">\uf0c1</a></h2>", "a[href=\"api/ce_string.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">ce_string.hpp<a class=\"headerlink\" href=\"#ce-string-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>"}
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
