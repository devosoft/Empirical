selector_to_html = {"a[href=\"#tuple-struct-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">tuple_struct.hpp<a class=\"headerlink\" href=\"#tuple-struct-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>These macros will build a tuple and accessors to that tuple\u2019s members inside of a class definintion. </p><p>\n\u201cBut WHY???\u201d you ask. Let me explain: Keeping a tuple allows us to easily track the members in the stuct or class, and makes possible powerful types of reflection including identifying all members and performing an action on each (such as serialization). Since tuples instantiate members directly, these benefits should come at no cost to performance.</p>"}
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
