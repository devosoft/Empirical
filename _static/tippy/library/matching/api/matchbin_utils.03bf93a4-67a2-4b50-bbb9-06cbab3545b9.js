selector_to_html = {"a[href=\"#matchbin-utils-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">matchbin_utils.hpp<a class=\"headerlink\" href=\"#matchbin-utils-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>Metric, Selector, and Regulator structs that can be plugged into MatchBin. </p>"}
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
