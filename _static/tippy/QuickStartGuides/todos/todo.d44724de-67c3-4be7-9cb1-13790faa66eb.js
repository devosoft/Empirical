selector_to_html = {"a[href=\"../../dev/contribution-guidelines-and-review.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Coding guidelines and review checklist<a class=\"headerlink\" href=\"#coding-guidelines-and-review-checklist\" title=\"Link to this heading\">\uf0c1</a></h1><p>This document is for those who want to contribute code or documentation\nfixes to the Empirical project and describes our coding standards as\nwell as our code review process.</p><p>This document has been adapted from the <a class=\"reference external\" href=\"https://khmer.readthedocs.org/en/v1.4.1/dev/coding-guidelines-and-review.html\">khmer\nproject</a></p>", "a[href=\"X-EvoTools.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Evolutionary Tools<a class=\"headerlink\" href=\"#evolutionary-tools\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"#work-in-progress\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Work In Progress<a class=\"headerlink\" href=\"#work-in-progress\" title=\"Link to this heading\">\uf0c1</a></h1><p>The following pages are a work in progress \ud83d\udea7.\n<a class=\"reference internal\" href=\"../../dev/contribution-guidelines-and-review.html\"><span class=\"doc std std-doc\">Contributions</span></a> are welcome!</p>", "a[href=\"X-TrackData.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Data Tracking<a class=\"headerlink\" href=\"#data-tracking\" title=\"Link to this heading\">\uf0c1</a></h1><p>Overview of data tracking:</p>", "a[href=\"X-CreateWorld.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Create a world<a class=\"headerlink\" href=\"#create-a-world\" title=\"Link to this heading\">\uf0c1</a></h1><p>To create a new world:</p>"}
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
