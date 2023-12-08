selector_to_html = {"a[href=\"#loadingmodal-hpp\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">LoadingModal.hpp<a class=\"headerlink\" href=\"#loadingmodal-hpp\" title=\"Link to this heading\">\uf0c1</a></h1><p>Wrapper for loading moadal. </p><p>To add a loading modal to your web page, you must link the LoadingModal.js script directly after the body tag in the HTML doc. To close the script, call CloseLoadingModal() at the end of your .cc file, or at the point which you wish to loading modal to disappear.</p>", "a[href=\"#_CPPv417CloseLoadingModalv\"]": "<dt class=\"sig sig-object cpp\" id=\"_CPPv417CloseLoadingModalv\">\n<span id=\"_CPPv317CloseLoadingModalv\"></span><span id=\"_CPPv217CloseLoadingModalv\"></span><span id=\"CloseLoadingModal\"></span><span class=\"target\" id=\"LoadingModal_8hpp_1aade0ed7b8fcf273dd91da7a857a678e4\"></span><span class=\"kt\"><span class=\"pre\">void</span></span><span class=\"w\"> </span><span class=\"sig-name descname\"><span class=\"n\"><span class=\"pre\">CloseLoadingModal</span></span></span><span class=\"sig-paren\">(</span><span class=\"sig-paren\">)</span><br/></dt><dd><p>This method does not belong to a class, but it is used to close a loading modal that is added with the LoadingModal.js script. See the prefab demo site for more details on how to implement the Loading Modal in your web app. <a class=\"reference external\" href=\"https://devosoft.github.io/empirical-prefab-demo\">https://devosoft.github.io/empirical-prefab-demo</a></p></dd>"}
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
