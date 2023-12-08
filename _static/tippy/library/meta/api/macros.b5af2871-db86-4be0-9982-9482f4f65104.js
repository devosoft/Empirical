selector_to_html = {"a[href=\"#macros-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">macros.hpp<a class=\"headerlink\" href=\"#macros-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>Generally useful macros that can perform cools tricks. Status: RELEASE. </p><p>Generally useful macros that can perform cools tricks. As with all macros, use only after careful exclusion of alternative approaches.</p>"}
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
