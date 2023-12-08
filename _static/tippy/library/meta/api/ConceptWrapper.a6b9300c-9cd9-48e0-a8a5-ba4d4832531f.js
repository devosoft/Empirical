selector_to_html = {"a[href=\"#conceptwrapper-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">ConceptWrapper.hpp<a class=\"headerlink\" href=\"#conceptwrapper-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>A template wrapper that will either enforce functionality or provide default functions. </p><p>Starting in future versions of C++, a concept is a set of requirements for a class to be used in a template. This wrapper around a class is slightly more powerful than that. It can either REQUIRE or marks as OPTIONAL specific member functions, variables or types for a wrapped class. REQUIRE-d members must be available in the internal class. OPTIONAL members are giving a default that will be used for classes where that member is missing.</p>"}
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
