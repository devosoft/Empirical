selector_to_html = {"a[href=\"#c.emp_error\"]": "<dt class=\"sig sig-object cpp\" id=\"c.emp_error\">\n<span class=\"target\" id=\"error_8hpp_1ab8648d1f94ae85c9748ee74ff07c8a15\"></span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">emp_error</span></span></span><span class=\"sig-paren\">(</span><span class=\"p\"><span class=\"pre\">...</span></span><span class=\"sig-paren\">)</span><br/></dt><dd></dd>", "a[href=\"#error-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">error.hpp<a class=\"headerlink\" href=\"#error-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>Nearly-universal error, to use in place of emp_assert(false, \u2026). Aborts program in both debug and release mode, but does NOT terminate in TDEBUG for testing. </p>"}
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
