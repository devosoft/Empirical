selector_to_html = {"a[href=\"#always-assert-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">always_assert.hpp<a class=\"headerlink\" href=\"#always-assert-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>A more dynamic replacement for standard library asserts. Status: RELEASE. </p><p>A replacement for the system-level assert.h, called \u201cemp_always_assert\u201d Added functionality:<ul class=\"simple\">\n<li><p>If compiled with Emscripten, will provide pop-up alerts in a web browser.</p></li>\n<li><p>emp_assert can take additional arguments. If the assert is triggered, those extra arguments will be evaluated and printed.</p></li>\n<li><p>If a literal string is provided as an argument, it will be printed as an error message.</p></li>\n<li><p>if EMP_TDEBUG is defined, emp_assert() goes into test mode and records failures, but does not abort. (useful for unit tests of asserts)</p></li>\n</ul>\n</p>", "a[href=\"#c.emp_always_assert\"]": "<dt class=\"sig sig-object cpp\" id=\"c.emp_always_assert\">\n<span class=\"target\" id=\"always__assert_8hpp_1a6926c00a699fdb241cc0a4d2ee9b792e\"></span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">emp_always_assert</span></span></span><span class=\"sig-paren\">(</span><span class=\"p\"><span class=\"pre\">...</span></span><span class=\"sig-paren\">)</span><br/></dt><dd><p>Require a specified condition to be true. If it is false, immediately halt execution. Print also extra information on any variables or expressions provided as variadic args. Will be evaluated when compiled in both debug and release mode. </p></dd>", "a[href=\"#c.emp_always_assert_impl\"]": "<dt class=\"sig sig-object cpp\" id=\"c.emp_always_assert_impl\">\n<span class=\"target\" id=\"always__assert_8hpp_1ab93a3043e24c296a5b01231cc3662f73\"></span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">emp_always_assert_impl</span></span></span><span class=\"sig-paren\">(</span><span class=\"p\"><span class=\"pre\">...</span></span><span class=\"sig-paren\">)</span><br/></dt><dd></dd>"}
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
