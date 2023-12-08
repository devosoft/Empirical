selector_to_html = {"a[href=\"#built-with-empirical-gallery\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Built With Empirical Gallery<a class=\"headerlink\" href=\"#built-with-empirical-gallery\" title=\"Link to this heading\">\uf0c1</a></h1><p>This listing showcases web tools built using Empirical. Put in a PR or\nraise an issue to have <em>your</em> project featured!</p>"}
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
