selector_to_html = {"a[href=\"#_CPPv414GetEmptyConfigv\"]": "<dt class=\"sig sig-object cpp\" id=\"_CPPv414GetEmptyConfigv\">\n<span id=\"_CPPv314GetEmptyConfigv\"></span><span id=\"_CPPv214GetEmptyConfigv\"></span><span id=\"GetEmptyConfig\"></span><span class=\"target\" id=\"config__utils_8hpp_1a05344e985188f810bf59099e9533023d\"></span><span class=\"k\"><span class=\"pre\">static</span></span><span class=\"w\"> </span><a class=\"reference internal\" href=\"config.html#_CPPv46Config\" title=\"Config\"><span class=\"n\"><span class=\"pre\">Config</span></span></a><span class=\"w\"> </span><span class=\"p\"><span class=\"pre\">&amp;</span></span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">GetEmptyConfig</span></span></span><span class=\"sig-paren\">(</span><span class=\"sig-paren\">)</span><br/></dt><dd></dd>", "a[href=\"#config-utils-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">config_utils.hpp<a class=\"headerlink\" href=\"#config-utils-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>Helper functions for working with Config objects. </p>", "a[href=\"config.html#_CPPv46Config\"]": "<dt class=\"sig sig-object cpp\" id=\"_CPPv46Config\">\n<span id=\"_CPPv36Config\"></span><span id=\"_CPPv26Config\"></span><span id=\"Config\"></span><span class=\"target\" id=\"classConfig\"></span><span class=\"k\"><span class=\"pre\">class</span></span><span class=\"w\"> </span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">Config</span></span></span><br/></dt><dd><p>Master configuration class that manages all of the settings. </p></dd>"}
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
