selector_to_html = {"a[href=\"#drawing-random-values-from-from-non-uniform-distributions-a-binomial-case-study\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Drawing random values from from non-uniform distributions: A binomial case study<a class=\"headerlink\" href=\"#drawing-random-values-from-from-non-uniform-distributions-a-binomial-case-study\" title=\"Link to this heading\">\uf0c1</a></h1><p>One common challenge in scientific computing is drawing from specific random distributions.\nThese can be time-consuming and hard to be acurate, especially when rare events are\nimportant to include.</p><p>There are many different mathematical distributions to consider.  For any common distribution,\nyou should be able to find plenty of information about it on the internet.  There are a\nhandful of specific questions to ask:</p>"}
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
