selector_to_html = {"a[href=\"#assert-warning-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">assert_warning.hpp<a class=\"headerlink\" href=\"#assert-warning-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>A non-terminating replacement for standard library asserts. </p><p>A supplement for the system-level assert.h, called \u201cemp_assert_warning\u201d Added functionality:<ul class=\"simple\">\n<li><p>If compiled with Emscripten, will provide pop-up alerts in a web browser.</p></li>\n<li><p>emp_assert_warning can take additional arguments. If the assert is triggered, those extra arguments will be evaluated and printed.</p></li>\n<li><p>if NDEBUG -or- EMP_NDEBUG is defined, the expression in emp_assert_warning() is not evaluated.</p></li>\n<li><p>emp_assert_warning() records failures, but does not abort.</p></li>\n</ul>\n</p>", "a[href=\"#c.emp_assert_warning\"]": "<dt class=\"sig sig-object cpp\" id=\"c.emp_assert_warning\">\n<span class=\"target\" id=\"assert__warning_8hpp_1a7ef595f16412986e9a4f040e8de6c600\"></span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">emp_assert_warning</span></span></span><span class=\"sig-paren\">(</span><span class=\"p\"><span class=\"pre\">...</span></span><span class=\"sig-paren\">)</span><br/></dt><dd><p>NDEBUG should trigger its EMP equivalent. </p><p>Require a specified condition to be true. If it is false, print extra information on any variables or expressions provided as variadic args. Note: If NDEBUG is defined, emp_assert_warning() will not do anything. Due to macro parsing limitations, extra information will not be printed when compiling with MSVC. </p></dd>"}
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
