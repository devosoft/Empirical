selector_to_html = {"a[href=\"#_CPPv47TimeFunNSt8functionIFvvEEE\"]": "<dt class=\"sig sig-object cpp\" id=\"_CPPv47TimeFunNSt8functionIFvvEEE\">\n<span id=\"_CPPv37TimeFunNSt8functionIFvvEEE\"></span><span id=\"_CPPv27TimeFunNSt8functionIFvvEEE\"></span><span class=\"target\" id=\"timing_8hpp_1abc60d47ba21c8f103afa81393890a27a\"></span><span class=\"k\"><span class=\"pre\">static</span></span><span class=\"w\"> </span><span class=\"k\"><span class=\"pre\">inline</span></span><span class=\"w\"> </span><span class=\"kt\"><span class=\"pre\">double</span></span><span class=\"w\"> </span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">TimeFun</span></span></span><span class=\"sig-paren\">(</span><a class=\"reference internal\" href=\"../../bits/api/Bits.html#_CPPv4St\" title=\"std\"><span class=\"n\"><span class=\"pre\">std</span></span></a><span class=\"p\"><span class=\"pre\">::</span></span><span class=\"n\"><span class=\"pre\">function</span></span><span class=\"p\"><span class=\"pre\">&lt;</span></span><span class=\"kt\"><span class=\"pre\">void</span></span><span class=\"p\"><span class=\"pre\">(</span></span><span class=\"p\"><span class=\"pre\">)</span></span><span class=\"p\"><span class=\"pre\">&gt;</span></span><span class=\"w\"> </span><span class=\"n sig-param\"><span class=\"pre\">test_fun</span></span><span class=\"sig-paren\">)</span><br/></dt><dd><p>A function timer that takes a functor an identifies how long it takes to complete when run. </p></dd>", "a[href=\"../../bits/api/Bits.html#_CPPv4St\"]": "<dt class=\"sig sig-object cpp\" id=\"_CPPv4St\">\n<span id=\"_CPPv3St\"></span><span id=\"_CPPv2St\"></span><span id=\"std\"></span><span class=\"target\" id=\"namespacestd\"></span><span class=\"k\"><span class=\"pre\">namespace</span></span><span class=\"w\"> </span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">std</span></span></span><br/></dt><dd><p>STL namespace. </p></dd>", "a[href=\"#timing-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">timing.hpp<a class=\"headerlink\" href=\"#timing-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>A collection of tools to help measure timing of code. </p>"}
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
