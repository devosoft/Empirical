selector_to_html = {"a[href=\"#text-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Text.hpp<a class=\"headerlink\" href=\"#text-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>Specs for the Text widget. </p><p>A representation of text on a web page. Text Widgets can be included inside of Divs or Tables to cordon off a section of text (and will be automatically created when text is streamed into these other widgets). The primary benefit of explicitly creating your own text widget is to control the text style. </p>"}
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
