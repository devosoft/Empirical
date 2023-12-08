selector_to_html = {"a[href=\"#mem-track-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">mem_track.hpp<a class=\"headerlink\" href=\"#mem-track-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>A set of macros to track how many instances of specific classes are made. </p><p>\nOne way of tracking memory leaks is to simply count instances of classes. The macros here simplify this process.</p>"}
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
