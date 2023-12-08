selector_to_html = {"a[href=\"#regex-utils-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">regex_utils.hpp<a class=\"headerlink\" href=\"#regex-utils-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>Helper functions for building regular expressions. </p>", "a[href=\"#_CPPv4NSt15string_literalsE\"]": "<dt class=\"sig sig-object cpp\" id=\"_CPPv4NSt15string_literalsE\">\n<span id=\"_CPPv3NSt15string_literalsE\"></span><span id=\"_CPPv2NSt15string_literalsE\"></span><span id=\"std::string_literals\"></span><span class=\"target\" id=\"namespacestd_1_1string__literals\"></span><span class=\"k\"><span class=\"pre\">namespace</span></span><span class=\"w\"> </span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">string_literals</span></span></span><br/></dt><dd></dd>", "a[href=\"../../bits/api/Bits.html#_CPPv4St\"]": "<dt class=\"sig sig-object cpp\" id=\"_CPPv4St\">\n<span id=\"_CPPv3St\"></span><span id=\"_CPPv2St\"></span><span id=\"std\"></span><span class=\"target\" id=\"namespacestd\"></span><span class=\"k\"><span class=\"pre\">namespace</span></span><span class=\"w\"> </span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">std</span></span></span><br/></dt><dd><p>STL namespace. </p></dd>", "a[href=\"#_CPPv412regex_nestedcc6size_tb\"]": "<dt class=\"sig sig-object cpp\" id=\"_CPPv412regex_nestedcc6size_tb\">\n<span id=\"_CPPv312regex_nestedcc6size_tb\"></span><span id=\"_CPPv212regex_nestedcc6size_tb\"></span><span id=\"regex_nested__c.c.s.b\"></span><span class=\"target\" id=\"regex__utils_8hpp_1afc4ba1b2f2356bc0b5686b97375b44c7\"></span><a class=\"reference internal\" href=\"../../bits/api/Bits.html#_CPPv4St\" title=\"std\"><span class=\"n\"><span class=\"pre\">std</span></span></a><span class=\"p\"><span class=\"pre\">::</span></span><span class=\"n\"><span class=\"pre\">string</span></span><span class=\"w\"> </span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">regex_nested</span></span></span><span class=\"sig-paren\">(</span><span class=\"kt\"><span class=\"pre\">char</span></span><span class=\"w\"> </span><span class=\"n sig-param\"><span class=\"pre\">open</span></span><span class=\"w\"> </span><span class=\"p\"><span class=\"pre\">=</span></span><span class=\"w\"> </span><span class=\"sc\"><span class=\"pre\">'('</span></span>, <span class=\"kt\"><span class=\"pre\">char</span></span><span class=\"w\"> </span><span class=\"n sig-param\"><span class=\"pre\">close</span></span><span class=\"w\"> </span><span class=\"p\"><span class=\"pre\">=</span></span><span class=\"w\"> </span><span class=\"sc\"><span class=\"pre\">')'</span></span>, <span class=\"n\"><span class=\"pre\">size_t</span></span><span class=\"w\"> </span><span class=\"n sig-param\"><span class=\"pre\">depth</span></span><span class=\"w\"> </span><span class=\"p\"><span class=\"pre\">=</span></span><span class=\"w\"> </span><span class=\"m\"><span class=\"pre\">0</span></span>, <span class=\"kt\"><span class=\"pre\">bool</span></span><span class=\"w\"> </span><span class=\"n sig-param\"><span class=\"pre\">stop_at_newline</span></span><span class=\"w\"> </span><span class=\"p\"><span class=\"pre\">=</span></span><span class=\"w\"> </span><span class=\"k\"><span class=\"pre\">true</span></span><span class=\"sig-paren\">)</span><br/></dt><dd></dd>"}
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
