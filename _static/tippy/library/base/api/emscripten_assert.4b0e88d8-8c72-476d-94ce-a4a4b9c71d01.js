selector_to_html = {"a[href=\"#emscripten-assert-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">emscripten_assert.hpp<a class=\"headerlink\" href=\"#emscripten-assert-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>Assert evaluated only in debug mode with Emscripten. </p>", "a[href=\"#c.emp_emscripten_assert\"]": "<dt class=\"sig sig-object cpp\" id=\"c.emp_emscripten_assert\">\n<span class=\"target\" id=\"emscripten__assert_8hpp_1a2929052f6da94f9c83a5762cf7074f56\"></span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">emp_emscripten_assert</span></span></span><span class=\"sig-paren\">(</span><span class=\"p\"><span class=\"pre\">...</span></span><span class=\"sig-paren\">)</span><br/></dt><dd><p>Require a specified condition to be true if this program was compiled to Javascript with Emscripten. Note: If NDEBUG is defined, emp_emscripten_assert() will not do anything. </p></dd>"}
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
