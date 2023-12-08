selector_to_html = {"a[href=\"#c.emp_assert\"]": "<dt class=\"sig sig-object cpp\" id=\"c.emp_assert\">\n<span class=\"target\" id=\"assert_8hpp_1ac3ebd16db76018ef2b9bea2555396b77\"></span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">emp_assert</span></span></span><span class=\"sig-paren\">(</span><span class=\"p\"><span class=\"pre\">...</span></span><span class=\"sig-paren\">)</span><br/></dt><dd><p>NDEBUG hould trigger its EMP equivalent. </p><p>Require a specified condition to be true. If it is false, immediately halt execution. Print also extra information on any variables or expressions provided as variadic args. Note: If NDEBUG is defined, emp_assert() will not do anything. Due to macro parsing limitations, extra information will not be printed when compiling with MSVC. </p></dd>", "a[href=\"#_CPPv413is_debug_mode\"]": "<dt class=\"sig sig-object cpp\" id=\"_CPPv413is_debug_mode\">\n<span id=\"_CPPv313is_debug_mode\"></span><span id=\"_CPPv213is_debug_mode\"></span><span id=\"is_debug_mode__b\"></span><span class=\"target\" id=\"assert_8hpp_1a37875fba245f33146df58bb0a895479c\"></span><span class=\"k\"><span class=\"pre\">static</span></span><span class=\"w\"> </span><span class=\"k\"><span class=\"pre\">constexpr</span></span><span class=\"w\"> </span><span class=\"kt\"><span class=\"pre\">bool</span></span><span class=\"w\"> </span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">is_debug_mode</span></span></span><span class=\"w\"> </span><span class=\"p\"><span class=\"pre\">=</span></span><span class=\"w\"> </span><span class=\"k\"><span class=\"pre\">true</span></span><br/></dt><dd></dd>", "a[href=\"#assert-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">assert.hpp<a class=\"headerlink\" href=\"#assert-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>A more dynamic replacement for standard library asserts. Status: RELEASE. </p><p>A replacement for the system-level assert.h, called \u201cemp_assert\u201d Added functionality:<ul class=\"simple\">\n<li><p>If compiled with Emscripten, will provide pop-up alerts in a web browser.</p></li>\n<li><p>emp_assert can take additional arguments. If the assert is triggered, those extra arguments will be evaluated and printed.</p></li>\n<li><p>if NDEBUG -or- EMP_NDEBUG is defined, the expression in emp_assert() is not evaluated.</p></li>\n<li><p>if EMP_TDEBUG is defined, emp_assert() goes into test mode and records failures, but does not abort. (useful for unit tests of asserts)</p></li>\n</ul>\n</p>"}
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
