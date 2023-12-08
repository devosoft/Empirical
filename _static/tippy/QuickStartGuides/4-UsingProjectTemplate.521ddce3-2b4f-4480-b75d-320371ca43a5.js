selector_to_html = {"a[href=\"#using-the-project-template\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Using the Project Template<a class=\"headerlink\" href=\"#using-the-project-template\" title=\"Link to this heading\">\uf0c1</a></h1><p>Goals of the project template - Simple web interface - Easy toggle -\nEasy debug</p><p>Organizing core code files</p>"}
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
