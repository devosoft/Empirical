selector_to_html = {"a[href=\"1-HelloWorld.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Hello World<a class=\"headerlink\" href=\"#hello-world\" title=\"Link to this heading\">\uf0c1</a></h1><p>Empirical allows you to compile your C++ to target</p>", "a[href=\"#overview\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Overview<a class=\"headerlink\" href=\"#overview\" title=\"Link to this heading\">\uf0c1</a></h1><p>The goal of Empirical is to simplify the development of efficient,\nreliable, and accessible scientific software.</p><p>We have several quick-start guides for different portions of the code.</p>"}
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
