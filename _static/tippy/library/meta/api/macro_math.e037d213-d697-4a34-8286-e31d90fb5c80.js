selector_to_html = {"a[href=\"#macro-math-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">macro_math.hpp<a class=\"headerlink\" href=\"#macro-math-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>Macros to build a pre-processor calculator system. Status: RELEASE. </p><p>Working macros include: EMP_INC(A) : converts to result of A+1 EMP_DEC(A) : converts to result of A-1 EMP_SHIFTL(A) : converts to result of A*2 EMP_SHIFTR(A) : converts to result of A/2</p>"}
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
