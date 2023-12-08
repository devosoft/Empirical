selector_to_html = {"a[href=\"#_CPPv412OrgInterface\"]": "<dt class=\"sig sig-object cpp\" id=\"_CPPv412OrgInterface\">\n<span id=\"_CPPv312OrgInterface\"></span><span id=\"_CPPv212OrgInterface\"></span><span id=\"OrgInterface\"></span><span class=\"target\" id=\"classOrgInterface\"></span><span class=\"k\"><span class=\"pre\">class</span></span><span class=\"w\"> </span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">OrgInterface</span></span></span><br/></dt><dd></dd>", "a[href=\"#orginterface-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">OrgInterface.hpp<a class=\"headerlink\" href=\"#orginterface-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>An interface between an organism and the outside world. </p>"}
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
