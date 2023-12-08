selector_to_html = {"a[href=\"#_CPPv411hash_namifyK6size_t\"]": "<dt class=\"sig sig-object cpp\" id=\"_CPPv411hash_namifyK6size_t\">\n<span id=\"_CPPv311hash_namifyK6size_t\"></span><span id=\"_CPPv211hash_namifyK6size_t\"></span><span id=\"hash_namify__sC\"></span><span class=\"target\" id=\"hash__namify_8hpp_1a4ec2223097a8cd32aebc8190637d5c69\"></span><a class=\"reference internal\" href=\"../../bits/api/Bits.html#_CPPv4St\" title=\"std\"><span class=\"n\"><span class=\"pre\">std</span></span></a><span class=\"p\"><span class=\"pre\">::</span></span><span class=\"n\"><span class=\"pre\">string</span></span><span class=\"w\"> </span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">hash_namify</span></span></span><span class=\"sig-paren\">(</span><span class=\"k\"><span class=\"pre\">const</span></span><span class=\"w\"> </span><span class=\"n\"><span class=\"pre\">size_t</span></span><span class=\"w\"> </span><span class=\"n sig-param\"><span class=\"pre\">hash</span></span><span class=\"sig-paren\">)</span><br/></dt><dd></dd>", "a[href=\"#hash-namify-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">hash_namify.hpp<a class=\"headerlink\" href=\"#hash-namify-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>A method for mapping hash values to intuitive names. </p>", "a[href=\"../../bits/api/Bits.html#_CPPv4St\"]": "<dt class=\"sig sig-object cpp\" id=\"_CPPv4St\">\n<span id=\"_CPPv3St\"></span><span id=\"_CPPv2St\"></span><span id=\"std\"></span><span class=\"target\" id=\"namespacestd\"></span><span class=\"k\"><span class=\"pre\">namespace</span></span><span class=\"w\"> </span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">std</span></span></span><br/></dt><dd><p>STL namespace. </p></dd>"}
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
